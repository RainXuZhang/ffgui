#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStringList>
#include <QProcess>
#include <QDateTime>
#include <QUuid>
#include "widgets/ProjectBinWidget.h"
#include "widgets/MonitorWidget.h"
#include "widgets/TimelineWidget.h"
#include "widgets/EffectStackWidget.h"
#include "widgets/RenderDialog.h"
#include "core/ProjectModel.h"

class QSplitter;
class QTreeWidget;
class QTreeWidgetItem;
class QListWidget;
class QListWidgetItem;
class QProgressBar;
class QPushButton;
class QComboBox;
class QMediaPlayer;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

    void openProject(const QString& path);
    void addMediaFiles(const QStringList& paths);

protected:
    void dragEnterEvent(QDragEnterEvent* event) override;
    void dropEvent(QDropEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;

private slots:
    void onNewProject();
    void onOpenProject();
    void onSaveProject();
    void onRenderProject();
    void openVideoFile();
    void onPreferences();

    void onBinClipAdded(const MediaClip& clip);
    void onBinClipSelected(const MediaClip& clip);
    void onBinClipDoubleClicked(const MediaClip& clip);
    void onTimelineClipSelected(const QString& clipId);
    void onTimelinePlayheadChanged(double seconds);
    void onEffectChanged();
    void updatePlayPauseButton();

private:
    void createMenusAndToolbars();
    void setupSplitterLayout();
    void setupProjectModel();
    void setupMenuBar();
    void setupTheme();
    void setupFormatComboBox();
    void updateCommandPreview();
    void applyThemeString(int themeIndex);
    void handleFFmpegOutput();
    void handleFFmpegFinished(int exitCode);

    // UI Widgets & Elements
    QSplitter* m_mainVerticalSplitter = nullptr;
    QSplitter* m_topHorizontalSplitter = nullptr;
    ProjectBinWidget* m_projectBinWidget = nullptr;
    MonitorWidget* m_clipMonitor = nullptr;
    MonitorWidget* m_projectMonitor = nullptr;
    TimelineWidget* m_timelineWidget = nullptr;
    EffectStackWidget* m_effectStack = nullptr;
    RenderDialog* m_renderDialog = nullptr;

    QTreeWidget* m_projectBinTree = nullptr;
    QTreeWidget* m_timelineTracksTree = nullptr;
    QListWidget* historyListWidget = nullptr;
    QProgressBar* progressBar = nullptr;
    QPushButton* m_playPauseButton = nullptr;
    QComboBox* m_formatComboBox = nullptr;

    // Backend Core elements
    Project* m_project = nullptr;
    QProcess* ffmpegProcess = nullptr;
    QMediaPlayer* m_mediaPlayer = nullptr;

    double totalCutDurationSeconds = 0.0;
    QString m_currentInputFilePath;
};

#endif // MAINWINDOW_H
