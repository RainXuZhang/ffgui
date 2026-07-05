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
    explicit RenderDialog(QWidget* parent = nullptr);
    void setProject(Project* project) { m_project = project; }

private slots:
    void onBrowseOutputClicked();
    void onAdvancedToggled(bool checked);
    void onRenderClicked();
    void onProcessOutput();
    void onProcessError();
    void onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);

private:
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
};

#endif // RENDERDIALOG_H