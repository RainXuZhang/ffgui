#include "RenderDialog.h"
#include "../core/ProjectModel.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QPushButton>
#include <QLineEdit>
#include <QComboBox>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QCheckBox>
#include <QFileDialog>
#include <QMessageBox>
#include <QProcess>
#include <QProgressBar>
#include <QTextEdit>
#include <QTimer>

RenderDialog::RenderDialog(QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle("Render Project");
    setMinimumSize(600, 400);

    auto* mainLayout = new QVBoxLayout(this);

    // Output settings
    auto* outputLayout = new QFormLayout();
    m_outputPathEdit = new QLineEdit(this);
    m_outputPathEdit->setPlaceholderText("Output file path");
    auto* browseButton = new QPushButton("Browse", this);
    connect(browseButton, &QPushButton::clicked, this, &RenderDialog::onBrowseOutputClicked);

    auto* formatLayout = new QHBoxLayout();
    m_formatCombo = new QComboBox(this);
    m_formatCombo->addItems({"MP4", "MKV", "MOV", "AVI"});
    m_formatCombo->setCurrentText("MP4");

    formatLayout->addWidget(m_formatCombo);
    formatLayout->addStretch();

    outputLayout->addRow("Output Path:", m_outputPathEdit);
    outputLayout->addRow("Format:", formatLayout);
    outputLayout->addRow(browseButton);

    // Video settings
    auto* videoLayout = new QFormLayout();
    m_videoCodecCombo = new QComboBox(this);
    m_videoCodecCombo->addItems({"libx264", "libx265", "h264_nvenc", "h264_amf"});
    m_videoCodecCombo->setCurrentText("libx264");

    m_bitrateSpin = new QSpinBox(this);
    m_bitrateSpin->setRange(500, 100000);
    m_bitrateSpin->setValue(5000);
    m_bitrateSpin->setSuffix(" kbps");

    m_fpsSpin = new QDoubleSpinBox(this);
    m_fpsSpin->setRange(10, 120);
    m_fpsSpin->setValue(30);
    m_fpsSpin->setSuffix(" fps");

    videoLayout->addRow("Video Codec:", m_videoCodecCombo);
    videoLayout->addRow("Bitrate:", m_bitrateSpin);
    videoLayout->addRow("FPS:", m_fpsSpin);

    // Audio settings
    auto* audioLayout = new QFormLayout();
    m_audioCodecCombo = new QComboBox(this);
    m_audioCodecCombo->addItems({"aac", "libmp3lame", "libopus", "libvorbis"});
    m_audioCodecCombo->setCurrentText("aac");

    m_audioBitrateSpin = new QSpinBox(this);
    m_audioBitrateSpin->setRange(32, 320);
    m_audioBitrateSpin->setValue(128);
    m_audioBitrateSpin->setSuffix(" kbps");

    audioLayout->addRow("Audio Codec:", m_audioCodecCombo);
    audioLayout->addRow("Audio Bitrate:", m_audioBitrateSpin);

    // Advanced settings
    m_advancedCheck = new QCheckBox("Show Advanced Settings", this);
    connect(m_advancedCheck, &QCheckBox::toggled, this, &RenderDialog::onAdvancedToggled);

    m_advancedWidget = new QWidget(this);
    m_advancedWidget->setVisible(false);
    auto* advancedLayout = new QFormLayout(m_advancedWidget);

    m_customArgsEdit = new QLineEdit(this);
    m_customArgsEdit->setPlaceholderText("Additional FFmpeg arguments");

    advancedLayout->addRow("Custom Arguments:", m_customArgsEdit);

    // Progress and output
    m_progressBar = new QProgressBar(this);
    m_progressBar->setRange(0, 100);
    m_progressBar->setValue(0);

    m_outputText = new QTextEdit(this);
    m_outputText->setReadOnly(true);

    // Buttons
    auto* buttonLayout = new QHBoxLayout();
    m_renderButton = new QPushButton("Render", this);
    m_renderButton->setStyleSheet("QPushButton { padding: 6px 12px; font-weight: bold; }");
    connect(m_renderButton, &QPushButton::clicked, this, &RenderDialog::onRenderClicked);

    m_cancelButton = new QPushButton("Cancel", this);
    connect(m_cancelButton, &QPushButton::clicked, this, &RenderDialog::reject);

    buttonLayout->addStretch();
    buttonLayout->addWidget(m_cancelButton);
    buttonLayout->addWidget(m_renderButton);

    // Assemble layout
    mainLayout->addLayout(outputLayout);
    mainLayout->addLayout(videoLayout);
    mainLayout->addLayout(audioLayout);
    mainLayout->addWidget(m_advancedCheck);
    mainLayout->addWidget(m_advancedWidget);
    mainLayout->addWidget(m_progressBar);
    mainLayout->addWidget(m_outputText);
    mainLayout->addLayout(buttonLayout);

    // FFmpeg process
    m_ffmpegProcess = new QProcess(this);
    connect(m_ffmpegProcess, &QProcess::readyReadStandardOutput, this, &RenderDialog::onProcessOutput);
    connect(m_ffmpegProcess, &QProcess::readyReadStandardError, this, &RenderDialog::onProcessError);
    connect(m_ffmpegProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &RenderDialog::onProcessFinished);
}

void RenderDialog::onBrowseOutputClicked() {
    QString filePath = QFileDialog::getSaveFileName(
        this,
        "Save Rendered File",
        "",
        "MP4 Files (*.mp4);;MKV Files (*.mkv);;MOV Files (*.mov);;AVI Files (*.avi)"
    );

    if (!filePath.isEmpty()) {
        m_outputPathEdit->setText(filePath);
    }
}

void RenderDialog::onAdvancedToggled(bool checked) {
    m_advancedWidget->setVisible(checked);
}

void RenderDialog::onRenderClicked() {
    if (m_outputPathEdit->text().isEmpty()) {
        QMessageBox::warning(this, "Error", "Please specify an output path");
        return;
    }

    // Build FFmpeg command
    QStringList args;
    args << "-y"; // Overwrite output file

    // Add input files with trimming
    for (const auto& item : m_project->projectSequence) {
        args << "-ss" << QString::number(item.inPoint);
        args << "-to" << QString::number(item.outPoint);
        args << "-i" << item.assetPath;
    }

    // Build filter complex for concatenation and text overlays
    QString filterComplex;
    QStringList videoStreams;
    QStringList audioStreams;

    for (int i = 0; i < m_project->projectSequence.size(); ++i) {
        const auto& item = m_project->projectSequence[i];

        // Video stream
        QString videoStream = QString("[%1:v]").arg(i);
        if (!item.overlayText.isEmpty()) {
            videoStream = QString("[%1:v]drawtext=text='%2':fontcolor=white:fontsize=36:x=50:y=50[v%3]")
                .arg(i)
                .arg(item.overlayText)
                .arg(i);
        }
        videoStreams << videoStream;

        // Audio stream
        audioStreams << QString("[%1:a]").arg(i);
    }

    // Concatenate video streams
    filterComplex = videoStreams.join("") + QString("concat=n=%1:v=1[vout]").arg(m_project->projectSequence.size());

    // Concatenate audio streams
    filterComplex += ";" + audioStreams.join("") + QString("concat=n=%1:a=1[aout]").arg(m_project->projectSequence.size());

    args << "-filter_complex" << filterComplex;
    args << "-map" << "[vout]";
    args << "-map" << "[aout]";

    // Video settings
    args << "-c:v" << m_videoCodecCombo->currentText();
    args << "-b:v" << QString("%1k").arg(m_bitrateSpin->value());
    args << "-r" << QString::number(m_fpsSpin->value());

    // Audio settings
    args << "-c:a" << m_audioCodecCombo->currentText();
    args << "-b:a" << QString("%1k").arg(m_audioBitrateSpin->value());

    // Custom arguments
    if (m_advancedCheck->isChecked() && !m_customArgsEdit->text().isEmpty()) {
        args << m_customArgsEdit->text().split(" ");
    }

    // Output file
    args << m_outputPathEdit->text();

    // Start process
    m_outputText->clear();
    m_progressBar->setValue(0);
    m_renderButton->setEnabled(false);
    m_cancelButton->setEnabled(true);

    m_ffmpegProcess->start("ffmpeg", args);
}

void RenderDialog::onProcessOutput() {
    QString output = m_ffmpegProcess->readAllStandardOutput();
    m_outputText->append(output);

    // Parse progress (simplified example)
    if (output.contains("frame=")) {
        int progress = output.section("frame=", 1, 1).section(" ", 0, 0).toInt();
        m_progressBar->setValue(progress);
    }
}

void RenderDialog::onProcessError() {
    QString error = m_ffmpegProcess->readAllStandardError();
    m_outputText->append("<span style='color:red;'>" + error + "</span>");
}

void RenderDialog::onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus) {
    m_renderButton->setEnabled(true);
    m_cancelButton->setEnabled(false);

    if (exitCode == 0 && exitStatus == QProcess::NormalExit) {
        m_outputText->append("<span style='color:green;'>Render completed successfully!</span>");
        m_progressBar->setValue(100);
    } else {
        m_outputText->append("<span style='color:red;'>Render failed!</span>");
    }
}
        
        arguments << "-filter_complex" << filterComplex
                  << "-map" << "[outv]"
                  << "-map" << "[outa]";
    }

    if (ext == "mp4") {
        arguments << "-c:v" << "libx264" << "-pix_fmt" << "yuv420p";
        if (bitrate == "high") arguments << "-crf" << "18";
        else if (bitrate == "medium") arguments << "-crf" << "23";
        else arguments << "-crf" << "28";
        arguments << "-c:a" << "aac" << "-b:a" << "192k";
    } else if (ext == "mkv") {
        arguments << "-c:v" << "libx265" << "-pix_fmt" << "yuv420p10le";
        if (bitrate == "high") arguments << "-crf" << "19";
        else if (bitrate == "medium") arguments << "-crf" << "24";
        else arguments << "-crf" << "30";
        arguments << "-c:a" << "aac" << "-b:a" << "256k";
    } else if (ext == "webm") {
        arguments << "-c:v" << "libvpx-vp9" << "-pix_fmt" << "yuv420p";
        if (bitrate == "high") arguments << "-crf" << "25" << "-b:v" << "0";
        else if (bitrate == "medium") arguments << "-crf" << "31" << "-b:v" << "0";
        else arguments << "-crf" << "40" << "-b:v" << "0";
        arguments << "-c:a" << "libopus" << "-b:a" << "128k";
    } else if (ext == "gif") {
        // GIFs don't have audio
        arguments.removeAll("-map");
        arguments.removeAll("[outa]");
        arguments << "-c:v" << "gif" << "-loop" << "0";
    } else if (ext == "mp3") {
        // Audio only
        arguments.removeAll("-map");
        arguments.removeAll("[outv]");
        arguments << "-c:a" << "libmp3lame";
        if (bitrate == "high") arguments << "-q:a" << "2";
        else if (bitrate == "medium") arguments << "-q:a" << "5";
        else arguments << "-q:a" << "7";
    }

    // Add CFR flag if re-encoding and CFR checkbox is checked
    if (m_cfrCheckBox->isChecked() && ext != "gif" && ext != "mp3") {
        arguments << "-fps_mode" << "cfr";
    }

    arguments << "-y" << m_outputPathEdit->text();
}

void RenderDialog::onProcessReadyRead() {
    QString output = QString::fromUtf8(m_process->readAllStandardOutput());
    m_logOutput->append(output);
    m_logOutput->ensureCursorVisible();

    // Parse progress time=HH:MM:SS.ms
    QStringList lines = output.split('\n');
    for (const QString& line : lines) {
        parseProgress(line);
    }
}

void RenderDialog::parseProgress(const QString& line) {
    static QRegularExpression re("time=(\\d+):(\\d+):(\\d+)\\.(\\d+)");
    QRegularExpressionMatch match = re.match(line);
    if (match.hasMatch()) {
        int h = match.captured(1).toInt();
        int m = match.captured(2).toInt();
        int s = match.captured(3).toInt();
        int ms = match.captured(4).toInt();
        
        double currentSec = h * 3600.0 + m * 60.0 + s + (ms / 100.0);
        if (m_totalDuration > 0.0) {
            int percentage = static_cast<int>((currentSec / m_totalDuration) * 100.0);
            m_progressBar->setValue(qMin(100, percentage));
        }
    }
}

void RenderDialog::onProcessFinished(int exitCode, QProcess::ExitStatus status) {
    m_renderButton->setEnabled(true);
    m_cancelButton->setText("Close");

    if (status == QProcess::NormalExit && exitCode == 0) {
        m_progressBar->setValue(100);
        QMessageBox::information(this, "Success", "Project Render Completed Successfully!\nOutput saved to:\n" + m_outputPathEdit->text());
    } else {
        QMessageBox::critical(this, "Error", "Render Failed!\nSee the logs for detail.");
    }
}
