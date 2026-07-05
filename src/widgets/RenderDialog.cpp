#include "RenderDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QFileDialog>
#include <QMessageBox>
#include <QFileInfo>
#include <QDir>
#include <QDebug>
#include <QRegularExpression>
#include <QCheckBox>

RenderDialog::RenderDialog(Project* project, QWidget* parent)
    : QDialog(parent)
    , m_project(project)
{
    setWindowTitle("Render / Export Project");
    setMinimumSize(500, 400);
    setStyleSheet("background-color: #252525; color: #d2d2d2;");

    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(10, 10, 10, 10);
    mainLayout->setSpacing(10);

    auto* formLayout = new QFormLayout();
    formLayout->setSpacing(8);

    // Output Path
    auto* pathLayout = new QHBoxLayout();
    m_outputPathEdit = new QLineEdit(this);
    m_outputPathEdit->setText(QDir::homePath() + "/rendered_output.mp4");
    m_outputPathEdit->setStyleSheet("QLineEdit { background-color: #1e1e1e; border: 1px solid #444444; padding: 4px; color: white; }");
    
    auto* browseBtn = new QPushButton("Browse...", this);
    browseBtn->setStyleSheet("QPushButton { background-color: #3e3e3e; padding: 4px 10px; }");
    connect(browseBtn, &QPushButton::clicked, this, &RenderDialog::onBrowseClicked);
    
    pathLayout->addWidget(m_outputPathEdit);
    pathLayout->addWidget(browseBtn);
    formLayout->addRow("Output File:", pathLayout);

    // Profile / Format Selection (Converting feature)
    m_formatCombo = new QComboBox(this);
    m_formatCombo->addItem("MP4 (H.264 / AAC) - Highly Compatible", "mp4");
    m_formatCombo->addItem("MKV (Matroska HEVC / AAC) - High Quality", "mkv");
    m_formatCombo->addItem("WebM (VP9 / Opus) - Web Optimized", "webm");
    m_formatCombo->addItem("GIF (Animated Image)", "gif");
    m_formatCombo->addItem("MP3 (Audio Only)", "mp3");
    m_formatCombo->setStyleSheet("QComboBox { background-color: #333333; color: white; padding: 4px; }");
    formLayout->addRow("Render Format:", m_formatCombo);

    // Resolution selection
    m_resolutionCombo = new QComboBox(this);
    m_resolutionCombo->addItem("Source Resolution (Original)", "source");
    m_resolutionCombo->addItem("Full HD 1080p (1920x1080)", "1920x1080");
    m_resolutionCombo->addItem("HD 720p (1280x720)", "1280x720");
    m_resolutionCombo->addItem("SD 480p (854x480)", "854x480");
    m_resolutionCombo->setStyleSheet("QComboBox { background-color: #333333; color: white; padding: 4px; }");
    formLayout->addRow("Resolution:", m_resolutionCombo);

    // Quality/Bitrate profile
    m_bitrateCombo = new QComboBox(this);
    m_bitrateCombo->addItem("High Quality (default)", "high");
    m_bitrateCombo->addItem("Medium Quality (smaller size)", "medium");
    m_bitrateCombo->addItem("Low Quality (very small size)", "low");
    m_bitrateCombo->setStyleSheet("QComboBox { background-color: #333333; color: white; padding: 4px; }");
    formLayout->addRow("Video Bitrate:", m_bitrateCombo);

    // Add CFR Checkbox
    m_cfrCheckBox = new QCheckBox("Force Constant Frame Rate (CFR)", this);
    m_cfrCheckBox->setStyleSheet("QCheckBox { background-color: #333333; color: white; padding: 4px; }");
    m_cfrCheckBox->setChecked(true); // Default to checked for safer re-encodes
    formLayout->addRow("Options:", m_cfrCheckBox);

    mainLayout->addLayout(formLayout);

    // Progress Bar
    m_progressBar = new QProgressBar(this);
    m_progressBar->setRange(0, 100);
    m_progressBar->setValue(0);
    m_progressBar->setStyleSheet(
        "QProgressBar {"
        "    background-color: #1e1e1e;"
        "    border: 1px solid #444444;"
        "    text-align: center;"
        "    color: white;"
        "    border-radius: 4px;"
        "    height: 20px;"
        "}"
        "QProgressBar::chunk {"
        "    background-color: #2a82da;"
        "    border-radius: 3px;"
        "}"
    );
    mainLayout->addWidget(m_progressBar);

    // Logger
    m_logOutput = new QTextEdit(this);
    m_logOutput->setReadOnly(true);
    m_logOutput->setStyleSheet("QTextEdit { background-color: #151515; border: 1px solid #333333; font-family: monospace; font-size: 10px; color: #a0a0a0; }");
    mainLayout->addWidget(m_logOutput, 1);

    // Dialog Buttons
    auto* btnLayout = new QHBoxLayout();
    m_renderButton = new QPushButton("Render", this);
    m_renderButton->setStyleSheet("QPushButton { background-color: #2a82da; color: white; font-weight: bold; padding: 6px 15px; }");
    connect(m_renderButton, &QPushButton::clicked, this, &RenderDialog::onRenderClicked);

    m_cancelButton = new QPushButton("Close", this);
    m_cancelButton->setStyleSheet("QPushButton { background-color: #3e3e3e; padding: 6px 15px; }");
    connect(m_cancelButton, &QPushButton::clicked, this, &RenderDialog::onCancelClicked);

    btnLayout->addStretch();
    btnLayout->addWidget(m_renderButton);
    btnLayout->addWidget(m_cancelButton);
    mainLayout->addLayout(btnLayout);
}

RenderDialog::~RenderDialog() {
    if (m_process) {
        m_process->kill();
        delete m_process;
    }
}

void RenderDialog::onBrowseClicked() {
    QString ext = m_formatCombo->currentData().toString();
    QString filter = QString("%1 Files (*.%1)").arg(ext.toUpper());
    if (ext == "mp3") filter = "Audio Files (*.mp3)";
    
    QString path = QFileDialog::getSaveFileName(this, "Select Render Destination", m_outputPathEdit->text(), filter);
    if (!path.isEmpty()) {
        m_outputPathEdit->setText(path);
    }
}

void RenderDialog::setOutputFilePath(const QString& path) {
    m_outputPathEdit->setText(path);
}

QString RenderDialog::getOutputFilePath() const {
    return m_outputPathEdit->text();
}

void RenderDialog::onCancelClicked() {
    if (m_process && m_process->state() == QProcess::Running) {
        if (QMessageBox::question(this, "Cancel Render", "Are you sure you want to stop the render process?") == QMessageBox::Yes) {
            m_process->kill();
            m_logOutput->append("\n*** Render Cancelled by User ***\n");
            m_renderButton->setEnabled(true);
            m_cancelButton->setText("Close");
        }
    } else {
        reject();
    }
}

void RenderDialog::onRenderClicked() {
    m_renderButton->setEnabled(false);
    m_cancelButton->setText("Cancel");
    m_logOutput->clear();
    m_progressBar->setValue(0);

    QString program;
    QStringList arguments;
    buildFFmpegCommand(program, arguments);

    m_logOutput->append("Executing Command:\nffmpeg " + arguments.join(" ") + "\n\n");

    m_process = new QProcess(this);
    m_process->setProcessChannelMode(QProcess::MergedChannels);
    connect(m_process, &QProcess::readyRead, this, &RenderDialog::onProcessReadyRead);
    connect(m_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, &RenderDialog::onProcessFinished);

    m_process->start(program, arguments);
}

void RenderDialog::buildFFmpegCommand(QString& program, QStringList& arguments) {
    program = "ffmpeg";

    // Gather all active clips in chronological order on the timeline
    QList<TimelineClip> sortedClips;
    for (const auto& track : m_project->tracks) {
        for (const auto& clip : track.clips) {
            sortedClips.append(clip);
        }
    }

    // Sort by timelineIn
    std::sort(sortedClips.begin(), sortedClips.end(), [](const TimelineClip& a, const TimelineClip& b) {
        return a.timelineIn < b.timelineIn;
    });

    if (sortedClips.isEmpty()) {
        QMessageBox::warning(this, "Error", "The timeline is empty. Place clips on tracks before rendering.");
        m_renderButton->setEnabled(true);
        m_cancelButton->setText("Close");
        return;
    }

    // Calculate total render duration
    m_totalDuration = 0.0;
    for (const auto& clip : sortedClips) {
        m_totalDuration = qMax(m_totalDuration, clip.timelineOut);
    }

    // Define Inputs
    // We add inputs sequentially
    for (const auto& clip : sortedClips) {
        MediaClip* mc = m_project->findMediaClip(clip.mediaClipId);
        if (mc) {
            arguments << "-ss" << QString::number(clip.sourceIn)
                      << "-t" << QString::number(clip.duration)
                      << "-i" << mc->filePath;
        }
    }

    // Filter complex setup
    // We'll generate standard chains to apply trimming, crop, and drawtext, then concatenate
    QString filterComplex;
    QString concatInputsVideo;
    QString concatInputsAudio;

    for (int i = 0; i < sortedClips.size(); ++i) {
        const auto& clip = sortedClips[i];
        QString lastVideoLabel = QString("%1:v").arg(i);
        QString lastAudioLabel = QString("%1:a").arg(i);

        // Apply crop if available
        for (const auto& fx : clip.effects) {
            if (!fx.enabled) continue;
            
            if (fx.type == "crop") {
                int left = fx.params["left"].toInt();
                int top = fx.params["top"].toInt();
                int right = fx.params["right"].toInt();
                int bottom = fx.params["bottom"].toInt();

                QString cropLabel = QString("[v_crop_%1]").arg(i);
                filterComplex += QString("[%2]crop=iw*(1-(%3+%4)/100.0):ih*(1-(%5+%6)/100.0):iw*%3/100.0:ih*%5/100.0%1;")
                                 .arg(cropLabel)
                                 .arg(lastVideoLabel)
                                 .arg(left).arg(right).arg(top).arg(bottom);
                lastVideoLabel = cropLabel;
            } else if (fx.type == "text") {
                QString text = fx.params["text"].toString();
                int size = fx.params["size"].toInt();
                QString color = fx.params["color"].toString();
                int x = fx.params["x"].toInt();
                int y = fx.params["y"].toInt();

                // Find a standard font path on Linux
                QString fontPath = "/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf";
                if (!QFileInfo::exists(fontPath)) {
                    fontPath = "/usr/share/fonts/liberation/LiberationSans-Bold.ttf";
                }

                QString textLabel = QString("[v_text_%1]").arg(i);
                filterComplex += QString("[%2]drawtext=fontfile='%3':text='%4':fontsize=%5:fontcolor='%6':x=%7:y=%8%1;")
                                 .arg(textLabel)
                                 .arg(lastVideoLabel)
                                 .arg(fontPath)
                                 .arg(text)
                                 .arg(size)
                                 .arg(color)
                                 .arg(x)
                                 .arg(y);
                lastVideoLabel = textLabel;
            }
        }

        concatInputsVideo += lastVideoLabel;
        concatInputsAudio += lastAudioLabel;
    }

    // Append concat filters
    int nClips = sortedClips.size();
    filterComplex += QString("%1%2concat=n=%3:v=1:a=1[outv][outa]")
                         .arg(concatInputsVideo)
                         .arg(concatInputsAudio)
                         .arg(nClips);

    arguments << "-filter_complex" << filterComplex
              << "-map" << "[outv]"
              << "-map" << "[outa]";

    // Output options depending on the format chosen
    QString ext = m_formatCombo->currentData().toString();
    QString res = m_resolutionCombo->currentData().toString();
    QString bitrate = m_bitrateCombo->currentData().toString();

    // Scale resolution if selected
    if (res != "source") {
        // Find outv and scale it
        // We can append scale filter to maps or filter complex
        // Let's modify filter complex: replace [outv] with scale filter
        arguments.removeAll("-map");
        arguments.removeAll("[outv]");
        arguments.removeAll("[outa]");
        
        filterComplex.replace("[outv]", "[outv_pre]");
        filterComplex += QString(";[outv_pre]scale=%1[outv]").arg(res.replace('x', ':'));
        
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
