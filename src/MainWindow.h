#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMenu>
#include <QMenuBar>
#include <QToolBar>
#include <QSplitter>
#include <QTreeWidget>
#include <QVideoWidget>
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QPushButton>
#include "core/ProjectModel.h"
#include "widgets/ProjectBinWidget.h"
#include "widgets/MonitorWidget.h"
#include "widgets/TimelineWidget.h"
#include "widgets/EffectStackWidget.h"
#include "widgets/RenderDialog.h"

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;

    void openProject(const QString& path);
    void addMediaFiles(const QStringList& paths);

private slots:
    void onNewProject();
    void onOpenProject();
    void onSaveProject();
    void onRenderProject();
    void openVideoFile();

    // Bin & Timeline Connections
    void onBinClipAdded(const MediaClip& clip);
    void onBinClipSelected(const MediaClip& clip);
    void onBinClipDoubleClicked(const MediaClip& clip);
    void onTimelineClipSelected(const QString& clipId);
    void onTimelinePlayheadChanged(double seconds);
    void onEffectChanged();

    // Play/Pause button functionality
    void updatePlayPauseButton();

private:
    void createMenusAndToolbars();
    void setupSplitterLayout();
    void setupProjectModel();
    void setupMenuBar();
    void setupTheme();
    Project* m_project = nullptr;

    // Layout - Nested QSplitter
    QSplitter* m_mainVerticalSplitter = nullptr;
    QSplitter* m_topHorizontalSplitter = nullptr;

    // New Layout Widgets
    QTreeWidget* m_projectBinTree = nullptr;      // Top-Left: Project Bin
    QVideoWidget* m_videoPlayerWidget = nullptr;   // Top-Right: Video Player
    QTreeWidget* m_timelineTracksTree = nullptr;   // Bottom: Timeline Tracks

    // Existing Widgets (kept for functionality)
    ProjectBinWidget* m_projectBinWidget = nullptr;
    MonitorWidget* m_clipMonitor = nullptr;
    MonitorWidget* m_projectMonitor = nullptr;
    TimelineWidget* m_timelineWidget = nullptr;
    EffectStackWidget* m_effectStack = nullptr;

    // Multimedia objects
    QMediaPlayer* m_mediaPlayer = nullptr;
    QAudioOutput* m_audioOutput = nullptr;

    // Play/Pause button
    QPushButton* m_playPauseButton = nullptr;
};

#endif // MAINWINDOW_H