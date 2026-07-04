#ifndef RENDERDIALOG_H
#define RENDERDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QComboBox>
#include <QProgressBar>
#include <QPushButton>
#include <QProcess>
#include <QTextEdit>
#include "../core/ProjectModel.h"

class RenderDialog : public QDialog {
    Q_OBJECT

public:
    explicit RenderDialog(Project* project, QWidget* parent = nullptr);
    ~RenderDialog() override;

private slots:
    void onBrowseClicked();
    void onRenderClicked();
    void onCancelClicked();
    void onProcessReadyRead();
    void onProcessFinished(int exitCode, QProcess::ExitStatus status);

private:
    void buildFFmpegCommand(QString& program, QStringList& arguments);
    void parseProgress(const QString& line);

    Project* m_project;
    QProcess* m_process = nullptr;
    double m_totalDuration = 0.0;

    QLineEdit* m_outputPathEdit;
    QComboBox* m_formatCombo;
    QComboBox* m_resolutionCombo;
    QComboBox* m_bitrateCombo;
    QProgressBar* m_progressBar;
    QPushButton* m_renderButton;
    QPushButton* m_cancelButton;
    QTextEdit* m_logOutput;
};

#endif // RENDERDIALOG_H
