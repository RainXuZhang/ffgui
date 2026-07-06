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
#include <QComboBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QVBoxLayout>
#include "core/ProjectModel.h"
#include "widgets/ProjectBinWidget.h"
#include "widgets/MonitorWidget.h"
#include "widgets/TimelineWidget.h"
#include "widgets/EffectStackWidget.h"
#include "widgets/RenderDialog.h"
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;

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
    void applyThemeString(int themeIndex);

private:
    void createMenusAndToolbars();
    void setupSplitterLayout();
    void setupProjectModel();
    void setupMenuBar();
    void setupTheme();
    void setupFormatComboBox();
    void updateCommandPreview();
    Project* m_project = nullptr;
    QString m_currentInputFilePath;

    // New UI elements
    QLabel* timecodeLabel = nullptr;
    QLineEdit* commandPreviewEdit = nullptr;
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

    // Format selection combo box
    QComboBox* formatComboBox = nullptr;

    // FFmpeg process management
    QProcess* ffmpegProcess = nullptr;
    void handleFFmpegOutput();
    void handleFFmpegFinished(int exitCode);

    // Progress bar and cut duration
    QProgressBar* progressBar = nullptr;
    double totalCutDurationSeconds = 0.0;
    QListWidget* historyListWidget = nullptr;
    RenderDialog* m_renderDialog = nullptr;
};
private:
    void createMenusAndToolbars();
    void setupSplitterLayout();
    void setupProjectModel();
    void setupMenuBar();
    void setupTheme();
    void setupFormatComboBox();
    void updateCommandPreview();
    Project* m_project = nullptr;
    QString m_currentInputFilePath;

    // New UI elements
    QLabel* timecodeLabel = nullptr;
    QLineEdit* commandPreviewEdit = nullptr;
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

    // Format selection combo box
    QComboBox* formatComboBox = nullptr;

    // FFmpeg process management
    QProcess* ffmpegProcess = nullptr;
    void handleFFmpegOutput();
    void handleFFmpegFinished(int exitCode);

    // Progress bar and cut duration
    QProgressBar* progressBar = nullptr;
    double totalCutDurationSeconds = 0.0;
    QListWidget* historyListWidget = nullptr;
    RenderDialog* m_renderDialog = nullptr;
};

#endif // MAINWINDOW_H