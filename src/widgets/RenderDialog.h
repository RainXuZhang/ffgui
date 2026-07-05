#ifndef RENDERDIALOG_H
#define RENDERDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QComboBox>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QCheckBox>
#include <QProgressBar>
#include <QTextEdit>
#include <QPushButton>
#include <QProcess>

class Project;

class RenderDialog : public QDialog {
    Q_OBJECT

public:
    explicit RenderDialog(Project* project, QWidget* parent = nullptr);

    void setOutputFilePath(const QString& path);
    QString getOutputFilePath() const;
    void setTotalDuration(double duration);

private slots:
    void onBrowseOutputClicked();
    void onAdvancedToggled(bool checked);
    void onRenderClicked();
    void onProcessReadyRead();
    void onProcessError();
    void onProcessFinished(int exitCode, QProcess::ExitStatus status);

private:
    void parseProgress(const QString& line);

    Project* m_project = nullptr;

    // UI elements
    QLineEdit* m_outputPathEdit;
    QComboBox* m_formatCombo;
    QComboBox* m_videoCodecCombo;
    QSpinBox* m_bitrateSpin;
    QDoubleSpinBox* m_fpsSpin;
    QComboBox* m_audioCodecCombo;
    QSpinBox* m_audioBitrateSpin;
    QCheckBox* m_advancedCheck;
    QWidget* m_advancedWidget;
    QLineEdit* m_customArgsEdit;
    QProgressBar* m_progressBar;
    QTextEdit* m_outputText;
    QPushButton* m_renderButton;
    QPushButton* m_cancelButton;

    // Process
    QProcess* m_ffmpegProcess;

    // Data members
    QString m_outputFilePath;
    double m_totalDuration = 0.0;

    // Member variables
    QProcess* m_process;
    QTextEdit* m_logOutput;
};

#endif // RENDERDIALOG_H