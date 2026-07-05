#include "MainWindow.h"
#include "widgets/RenderDialog.h"
#include "core/FFmpegProbe.h"
#include <QMenuBar>
#include <QToolBar>
#include <QStatusBar>
#include <QMessageBox>
#include <QFileDialog>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QFileInfo>
#include <QDebug>
#include <QCloseEvent>
#include <QSplitter>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QUrl>
#include <QKeyEvent>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
{
    setWindowTitle("FFGui - New Project");
    setMinimumSize(1024, 768);
    // Initialize layout and setups
    setupProjectModel();
    setupSplitterLayout();
    setupMenuBar();
    setupTheme();
    statusBar()->showMessage("Ready");
}

MainWindow::~MainWindow() {
    delete m_project;
}

void MainWindow::setupProjectModel() {
    delete m_project;
    m_project = new Project();

    // Add standard tracks matching Kdenlive tracks (V2, V1, A1, A2)
    m_project->tracks.append({1, "Video 2", false, false, false, {}});
    m_project->tracks.append({2, "Video 1", false, false, false, {}});
    m_project->tracks.append({3, "Audio 1", true, false, false, {}});
    m_project->tracks.append({4, "Audio 2", true, false, false, {}});
}

void MainWindow::createMenusAndToolbars() {
    // This function will likely be replaced by setupMenuBar,
    // but keeping it for now in case there are other toolbar related
    // items.
    // Main Toolbar
    auto* toolbar = addToolBar("Main Toolbar");
    toolbar->setMovable(false);
    toolbar->setStyleSheet("QToolBar { background-color: #252525; border-bottom: 1px solid #1a1a1a; spacing: 4px; }");

    // Actions are now defined in setupMenuBar, so they would need to be passed or re-created if toolbar buttons are desired here.
    // For now, toolbar will be empty or require manual linking if this function is kept.
}

void MainWindow::setupMenuBar() {
    // Menu Bar
    QMenuBar* menuBar = QMainWindow::menuBar();
    menuBar->setStyleSheet("QMenuBar { background-color: #252525; color: #d2d2d2; } QMenuBar::item:selected { background-color: #2a82da; }");

    // File Menu
    QMenu* fileMenu = menuBar->addMenu(tr("&File"));
    fileMenu->addAction(tr("&New Project"), QKeySequence::New, this, &MainWindow::onNewProject);
    fileMenu->addAction(tr("&Open Project..."), QKeySequence::Open, this, &MainWindow::onOpenProject);
    fileMenu->addAction(tr("Open Video File..."), this, &MainWindow::openVideoFile);
    fileMenu->addAction(tr("&Save Project"), QKeySequence::Save, this, &MainWindow::onSaveProject);
    fileMenu->addAction(tr("Save Project &As..."), QKeySequence::SaveAs, [this]() {
        // TODO: Implement "Save As" functionality
        qDebug() << "Save Project As triggered";
    });
    fileMenu->addSeparator();
    fileMenu->addAction(tr("&Render Project..."), QKeySequence(Qt::CTRL | Qt::Key_R), this, &MainWindow::onRenderProject);
    fileMenu->addSeparator();
    fileMenu->addAction(tr("&Quit"), QKeySequence::Quit, this, &QWidget::close);

    // Edit Menu
    QMenu* editMenu = menuBar->addMenu(tr("&Edit"));
    editMenu->addAction(tr("Undo"), QKeySequence::Undo, this, []() {
        qDebug() << "Undo triggered";
    })->setEnabled(false);
    editMenu->addAction(tr("Redo"), QKeySequence::Redo, this, []() {
        qDebug() << "Redo triggered";
    })->setEnabled(false);

    // View Menu
    QMenu* viewMenu = menuBar->addMenu(tr("&View"));
    viewMenu->addAction(tr("Fullscreen"), QKeySequence::FullScreen, this, [this]() {
        if (isFullScreen()) {
            showNormal();
        } else {
            showFullScreen();
        }
    });

    // Tool Menu
    QMenu* toolMenu = menuBar->addMenu(tr("&Tool"));
    toolMenu->addAction(tr("Render Project..."), QKeySequence(Qt::CTRL | Qt::Key_R), this, &MainWindow::onRenderProject);
    toolMenu->addSeparator();
    toolMenu->addAction(tr("Selection Tool"), this, []() {
        qDebug() << "Selection Tool triggered";
    });
    toolMenu->addAction(tr("Razor Tool"), this, []() {
        qDebug() << "Razor Tool triggered";
    });

    // Clip Menu
    auto* clipMenu = menuBar->addMenu("&Clip");
    clipMenu->addAction("Add Clip...", m_projectBinWidget, SLOT(onAddClipClicked()));
    clipMenu->addAction("Delete Selected Clip", m_projectBinWidget, SLOT(onRemoveClipClicked()));

    // Monitor Menu
    auto* monitorMenu = menuBar->addMenu("&Monitor");
    monitorMenu->addAction("Play / Pause (Space)", m_projectMonitor, SLOT(onPlayPauseClicked()));
    monitorMenu->addAction("Stop", m_projectMonitor, SLOT(onStopClicked()));

    // Help Menu
    QMenu* helpMenu = menuBar->addMenu(tr("&Help"));
    helpMenu->addAction(tr("&About"), this, [this]() {
        QMessageBox::about(this, "About FFGui",
            "<h3>FFGui v1.0.0</h3>"
            "<p>A powerful, professional C++ Qt6-based GUI frontend for FFmpeg, "
            "designed to match the comprehensive multi-track editing layout of Kdenlive.</p>"
            "<p>Features real-time previews, custom timeline dragging/splitting/trimming, "
            "and dynamic filter-complex generation for high-performance exports.</p>");
    });
}

void MainWindow::setupTheme() {
    // Dark anthracite Kdenlive-style QSS stylesheet
    QString stylesheet = R"(
        QMainWindow {
            background-color: #2b2b2b;
            color: #d2d2d2;
        }

        QMenuBar {
            background-color: #353535;
            color: #d2d2d2;
            border-bottom: 1px solid #1a1a1a;
        }

        QMenuBar::item {
            background-color: transparent;
            padding: 4px 8px;
        }

        QMenuBar::item:selected {
            background-color: #2a82da;
            color: white;
        }

        QMenu {
            background-color: #353535;
            color: #d2d2d2;
            border: 1px solid #1a1a1a;
        }

        QMenu::item {
            padding: 6px 24px;
        }

        QMenu::item:selected {
            background-color: #2a82da;
            color: white;
        }

        QSplitter::handle {
            background-color: #1a1a1a;
        }

        QSplitter::handle:horizontal {
            width: 2px;
        }

        QSplitter::handle:vertical {
            height: 2px;
        }

        QTreeWidget {
            background-color: #252525;
            color: #d2d2d2;
            border: 1px solid #1a1a1a;
            gridline-color: #1a1a1a;
        }

        QTreeWidget::item {
            padding: 4px;
        }

        QTreeWidget::item:selected {
            background-color: #2a82da;
            color: white;
        }

        QTreeWidget::item:hover {
            background-color: #3a3a3a;
        }

        QTreeWidget::item:alternate {
            background-color: #2a2a2a;
        }

        QHeaderView::section {
            background-color: #353535;
            color: #d2d2d2;
            padding: 4px;
            border: none;
            border-right: 1px solid #1a1a1a;
            border-bottom: 1px solid #1a1a1a;
        }

        QVideoWidget {
            background-color: #000000;
            border: 1px solid #1a1a1a;
        }

        QStatusBar {
            background-color: #353535;
            color: #d2d2d2;
            border-top: 1px solid #1a1a1a;
        }

        QToolBar {
            background-color: #353535;
            color: #d2d2d2;
            border-bottom: 1px solid #1a1a1a;
        }

        QDockWidget {
            background-color: #252525;
            color: #d2d2d2;
            titlebar-close-icon: url(close.png);
        }

        QDockWidget::title {
            background-color: #353535;
            color: #d2d2d2;
            padding: 4px;
            border-bottom: 1px solid #1a1a1a;
        }

        QPushButton {
            background-color: #3a3a3a;
            color: #d2d2d2;
            border: 1px solid #1a1a1a;
            padding: 4px 8px;
            border-radius: 2px;
        }

        QPushButton:hover {
            background-color: #4a4a4a;
        }

        QPushButton:pressed {
            background-color: #2a82da;
        }

        QSlider::groove:horizontal {
            background-color: #1a1a1a;
            height: 4px;
            border-radius: 2px;
        }

        QSlider::handle:horizontal {
            background-color: #2a82da;
            width: 12px;
            height: 12px;
            margin: -4px 0;
            border-radius: 6px;
        }

        QScrollBar:vertical {
            background-color: #252525;
            width: 12px;
            border: none;
        }

        QScrollBar::handle:vertical {
            background-color: #4a4a4a;
            min-height: 20px;
            border-radius: 6px;
        }

        QScrollBar::handle:vertical:hover {
            background-color: #5a5a5a;
        }

        QScrollBar:horizontal {
            background-color: #252525;
            height: 12px;
            border: none;
        }

        QScrollBar::handle:horizontal {
            background-color: #4a4a4a;
            min-width: 20px;
            border-radius: 6px;
        }

        QScrollBar::handle:horizontal:hover {
            background-color: #5a5a5a;
        }
    )";

    setStyleSheet(stylesheet);
}

void MainWindow::keyPressEvent(QKeyEvent* event) {
    switch (event->key()) {
        case Qt::Key_Space:
            if (m_mediaPlayer->playbackState() == QMediaPlayer::PlayingState) {
                m_mediaPlayer->pause();
            } else {
                m_mediaPlayer->play();
            }
            event->accept();
            break;
        case Qt::Key_Left:
            {
                qint64 currentPos = m_mediaPlayer->position();
                qint64 newPos = qMax(currentPos - 5000, 0LL);
                m_mediaPlayer->setPosition(newPos);
                event->accept();
            }
            break;
        case Qt::Key_Right:
            {
                qint64 currentPos = m_mediaPlayer->position();
                qint64 totalDuration = m_mediaPlayer->duration();
                qint64 newPos = qMin(currentPos + 5000, totalDuration);
                m_mediaPlayer->setPosition(newPos);
                event->accept();
            }
            break;
        default:
            QMainWindow::keyPressEvent(event);
    }
}

void MainWindow::setupSplitterLayout() {
    // Create main vertical splitter (top section and bottom timeline)
    m_mainVerticalSplitter = new QSplitter(Qt::Vertical, this);
    setCentralWidget(m_mainVerticalSplitter);

    // Create top horizontal splitter (left bin and right player)
    m_topHorizontalSplitter = new QSplitter(Qt::Horizontal, this);

    // === Top-Left: Project Bin (QTreeWidget) ===
    m_projectBinTree = new QTreeWidget(this);
    m_projectBinTree->setHeaderLabels({"Name", "Duration", "Type"});
    m_projectBinTree->setRootIsDecorated(false);
    m_projectBinTree->setAlternatingRowColors(true);
    m_projectBinTree->setSelectionMode(QAbstractItemView::SingleSelection);
    m_topHorizontalSplitter->addWidget(m_projectBinTree);

    // === Top-Right: Video Player (QVideoWidget) ===
    // Will be connected to project monitor's video widget later
    m_videoPlayerWidget = new QVideoWidget(this);
    m_videoPlayerWidget->setMinimumSize(400, 300);
    m_topHorizontalSplitter->addWidget(m_videoPlayerWidget);

    // === Initialize Multimedia Objects ===
    m_mediaPlayer = new QMediaPlayer(this);
    m_audioOutput = new QAudioOutput(this);
    m_mediaPlayer->setAudioOutput(m_audioOutput);
    m_mediaPlayer->setVideoOutput(m_videoPlayerWidget);

    // Set initial sizes for top splitter (30% left, 70% right)
    m_topHorizontalSplitter->setStretchFactor(0, 1);
    m_topHorizontalSplitter->setStretchFactor(1, 2);

    // Add top splitter to main vertical splitter
    m_mainVerticalSplitter->addWidget(m_topHorizontalSplitter);

    // === Bottom: Timeline Tracks (QTreeWidget) ===
    m_timelineTracksTree = new QTreeWidget(this);
    m_timelineTracksTree->setHeaderLabels({"Track", "Clips", "Duration", "Locked"});
    m_timelineTracksTree->setRootIsDecorated(false);
    m_timelineTracksTree->setAlternatingRowColors(true);
    m_mainVerticalSplitter->addWidget(m_timelineTracksTree);

    // Set initial sizes for main splitter (60% top, 40% bottom)
    m_mainVerticalSplitter->setStretchFactor(0, 3);
    m_mainVerticalSplitter->setStretchFactor(1, 2);

    // Initialize existing widgets for functionality (hidden but connected)
    m_projectBinWidget = new ProjectBinWidget(this);
    m_projectBinWidget->hide();
    m_clipMonitor = new MonitorWidget("Clip Preview", this);
    m_clipMonitor->hide();
    m_projectMonitor = new MonitorWidget("Project Timeline Preview", this);
    m_projectMonitor->hide();
    m_timelineWidget = new TimelineWidget(this);
    m_timelineWidget->setProject(m_project);
    m_timelineWidget->hide();
    connect(m_timelineWidget, &TimelineWidget::seekRequested, this, [this](double seconds) {
        m_mediaPlayer->setPosition(static_cast<qint64>(seconds * 1000));
    });

    connect(m_mediaPlayer, &QMediaPlayer::positionChanged, this, [this](qint64 position) {
        double seconds = static_cast<double>(position) / 1000.0;
        m_timelineWidget->setPosition(seconds);
    });
    m_effectStack = new EffectStackWidget(this);
    m_effectStack->setProject(m_project);
    m_effectStack->hide();

    // Connect video player to project monitor for playback
    // Use the project monitor's video widget as the main video player
    m_videoPlayerWidget = m_projectMonitor->getVideoWidget();

    // Event connections
    connect(m_projectBinWidget, &ProjectBinWidget::clipAdded, this, &MainWindow::onBinClipAdded);
    connect(m_projectBinWidget, &ProjectBinWidget::clipSelected, this, &MainWindow::onBinClipSelected);
    connect(m_projectBinWidget, &ProjectBinWidget::clipDoubleClicked, this, &MainWindow::onBinClipDoubleClicked);

    connect(m_timelineWidget, &TimelineWidget::clipSelected, this, &MainWindow::onTimelineClipSelected);
    connect(m_timelineWidget, &TimelineWidget::playheadChanged, this, &MainWindow::onTimelinePlayheadChanged);
    connect(m_timelineWidget, &TimelineWidget::seekRequested, this, [this](double seconds) {
        m_mediaPlayer->setPosition(static_cast<qint64>(seconds * 1000));
    });
    connect(m_effectStack, &EffectStackWidget::effectChanged, this, &MainWindow::onEffectChanged);

    // Connect QTreeWidget selection to existing functionality
    connect(m_projectBinTree, &QTreeWidget::itemDoubleClicked, this, [this](QTreeWidgetItem* item) {
        if (item) {
            QString clipId = item->data(0, Qt::UserRole).toString();
            MediaClip* clip = m_project->findMediaClip(clipId);
            if (clip) {
                onBinClipDoubleClicked(*clip);
            }
        }
    });
}

void MainWindow::onNewProject() {
    if (QMessageBox::question(this, "New Project", "Create a new project? Any unsaved edits will be lost.") == QMessageBox::Yes) {
        setupProjectModel();
        m_projectBinWidget->clear();
        m_projectBinTree->clear();
        m_timelineTracksTree->clear();
        m_timelineWidget->setProject(m_project);
        m_effectStack->setProject(m_project);
        m_clipMonitor->stop();
        m_projectMonitor->stop();
        statusBar()->showMessage("New project created.");
        setWindowTitle("FFGui - New Project");
    }
}

void MainWindow::onOpenProject() {
    QString path = QFileDialog::getOpenFileName(this, "Open FFGui Project", "", "FFGui Projects (*.ffgui)");
    if (!path.isEmpty()) {
        openProject(path);
    }
}

void MainWindow::openProject(const QString& path) {
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::critical(this, "Error", "Failed to open project file.");
        return;
    }

    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    if (doc.isNull()) {
        QMessageBox::critical(this, "Error", "Invalid project file format.");
        return;
    }

    setupProjectModel();
    m_projectBinWidget->clear();
    m_projectBinTree->clear();

    QJsonObject rootObj = doc.object();
    m_project->filePath = path;
    m_project->duration = rootObj["duration"].toDouble(300.0);
    m_project->currentPlayhead = rootObj["currentPlayhead"].toDouble(0.0);

    // Read Media Clips
    QJsonArray mediaClipsArr = rootObj["mediaClips"].toArray();
    for (const auto& val : mediaClipsArr) {
        QJsonObject obj = val.toObject();
        MediaClip clip;
        clip.id = obj["id"].toString();
        clip.filePath = obj["filePath"].toString();
        clip.fileName = obj["fileName"].toString();
        clip.duration = obj["duration"].toDouble();
        clip.width = obj["width"].toInt();
        clip.height = obj["height"].toInt();
        clip.fps = obj["fps"].toDouble();
        clip.thumbnailPath = obj["thumbnailPath"].toString();
        clip.hasAudio = obj["hasAudio"].toBool();
        clip.hasVideo = obj["hasVideo"].toBool();

        m_project->mediaClips.append(clip);
        m_projectBinWidget->addClip(clip);

        // Also add to QTreeWidget
        auto* item = new QTreeWidgetItem(m_projectBinTree);
        item->setText(0, clip.fileName);
        item->setText(1, QString::number(clip.duration, 'f', 2) + "s");
        item->setText(2, clip.hasVideo ? (clip.hasAudio ? "Video/Audio" : "Video") : "Audio");
        item->setData(0, Qt::UserRole, clip.id);
    }

    // Read Tracks & Timeline Clips
    QJsonArray tracksArr = rootObj["tracks"].toArray();
    for (int i = 0; i < qMin(tracksArr.size(), m_project->tracks.size()); ++i) {
        QJsonObject tObj = tracksArr[i].toObject();
        auto& track = m_project->tracks[i];
        track.name = tObj["name"].toString();
        track.isLocked = tObj["isLocked"].toBool();
        track.isMutedOrHidden = tObj["isMutedOrHidden"].toBool();

        track.clips.clear();
        QJsonArray clipsArr = tObj["clips"].toArray();
        for (const auto& cVal : clipsArr) {
            QJsonObject cObj = cVal.toObject();
            TimelineClip tClip;
            tClip.id = cObj["id"].toString();
            tClip.mediaClipId = cObj["mediaClipId"].toString();
            tClip.timelineIn = cObj["timelineIn"].toDouble();
            tClip.timelineOut = cObj["timelineOut"].toDouble();
            tClip.sourceIn = cObj["sourceIn"].toDouble();
            tClip.duration = cObj["duration"].toDouble();

            // Read Effects
            QJsonArray effectsArr = cObj["effects"].toArray();
            for (const auto& eVal : effectsArr) {
                QJsonObject eObj = eVal.toObject();
                ClipEffect fx;
                fx.type = eObj["type"].toString();
                fx.enabled = eObj["enabled"].toBool();
                fx.params = eObj["params"].toVariant().toMap();
                tClip.effects.append(fx);
            }

            track.clips.append(tClip);
        }
    }

    m_timelineWidget->setProject(m_project);
    m_effectStack->setProject(m_project);
    m_timelineWidget->updateTimeline();

    // Update timeline tracks tree
    m_timelineTracksTree->clear();
    for (const auto& track : m_project->tracks) {
        auto* item = new QTreeWidgetItem(m_timelineTracksTree);
        item->setText(0, track.name);
        item->setText(1, QString::number(track.clips.size()));
        double totalDuration = 0.0;
        for (const auto& clip : track.clips) {
            totalDuration = qMax(totalDuration, clip.timelineOut);
        }
        item->setText(2, QString::number(totalDuration, 'f', 2) + "s");
        item->setText(3, track.isLocked ? "Yes" : "No");
    }
    statusBar()->showMessage("Project loaded: " + QFileInfo(path).fileName());
    setWindowTitle("FFGui - " + QFileInfo(path).fileName());
}

void MainWindow::onSaveProject() {
    QString path = m_project->filePath;
    if (path.isEmpty()) {
        path = QFileDialog::getSaveFileName(this, "Save FFGui Project", "", "FFGui Projects (*.ffgui)");
        if (path.isEmpty()) return;
        m_project->filePath = path;
    }

    // Update timeline tracks tree before saving
    m_timelineTracksTree->clear();
    for (const auto& track : m_project->tracks) {
        auto* item = new QTreeWidgetItem(m_timelineTracksTree);
        item->setText(0, track.name);
        item->setText(1, QString::number(track.clips.size()));
        double totalDuration = 0.0;
        for (const auto& clip : track.clips) {
            totalDuration = qMax(totalDuration, clip.timelineOut);
        }
        item->setText(2, QString::number(totalDuration, 'f', 2) + "s");
        item->setText(3, track.isLocked ? "Yes" : "No");
    }

    QJsonObject rootObj;
    rootObj["duration"] = m_project->duration;
    rootObj["currentPlayhead"] = m_project->currentPlayhead;

    // Serialize Media Clips
    QJsonArray mediaClipsArr;
    for (const auto& clip : m_project->mediaClips) {
        QJsonObject obj;
        obj["id"] = clip.id;
        obj["filePath"] = clip.filePath;
        obj["fileName"] = clip.fileName;
        obj["duration"] = clip.duration;
        obj["width"] = clip.width;
        obj["height"] = clip.height;
        obj["fps"] = clip.fps;
        obj["thumbnailPath"] = clip.thumbnailPath;
        obj["hasAudio"] = clip.hasAudio;
        obj["hasVideo"] = clip.hasVideo;
        mediaClipsArr.append(obj);
    }
    rootObj["mediaClips"] = mediaClipsArr;

    // Serialize Tracks
    QJsonArray tracksArr;
    for (const auto& track : m_project->tracks) {
        QJsonObject tObj;
        tObj["id"] = track.id;
        tObj["name"] = track.name;
        tObj["isLocked"] = track.isLocked;
        tObj["isMutedOrHidden"] = track.isMutedOrHidden;

        QJsonArray clipsArr;
        for (const auto& clip : track.clips) {
            QJsonObject cObj;
            cObj["id"] = clip.id;
            cObj["mediaClipId"] = clip.mediaClipId;
            cObj["timelineIn"] = clip.timelineIn;
            cObj["timelineOut"] = clip.timelineOut;
            cObj["sourceIn"] = clip.sourceIn;
            cObj["duration"] = clip.duration;

            // Serialize Effects
            QJsonArray effectsArr;
            for (const auto& fx : clip.effects) {
                QJsonObject eObj;
                eObj["type"] = fx.type;
                eObj["enabled"] = fx.enabled;
                eObj["params"] = QJsonObject::fromVariantMap(fx.params);
                effectsArr.append(eObj);
            }
            cObj["effects"] = effectsArr;

            clipsArr.append(cObj);
        }
        tObj["clips"] = clipsArr;
        tracksArr.append(tObj);
    }
    rootObj["tracks"] = tracksArr;

    QFile file(path);
    if (file.open(QIODevice::WriteOnly)) {
        QJsonDocument doc(rootObj);
        file.write(doc.toJson());
        statusBar()->showMessage("Project saved: " + QFileInfo(path).fileName());
        setWindowTitle("FFGui - " + QFileInfo(path).fileName());
    } else {
        QMessageBox::critical(this, "Error", "Failed to save project file.");
    }
}

void MainWindow::onRenderProject() {
    RenderDialog dialog(m_project, this);
    dialog.exec();
}

void MainWindow::onBinClipAdded(const MediaClip& clip) {
    m_project->mediaClips.append(clip);
}

void MainWindow::onBinClipSelected(const MediaClip& clip) {
    m_clipMonitor->setMedia(clip.filePath);
    m_clipMonitor->raise(); // Auto focus clip monitor tab
}

void MainWindow::onBinClipDoubleClicked(const MediaClip& clip) {
    // Insert onto Video Track 1 (Index 1) at current playhead
    TimelineClip tClip;
    tClip.id = QUuid::createUuid().toString(QUuid::WithoutBraces);
    tClip.mediaClipId = clip.id;
    tClip.timelineIn = m_project->currentPlayhead;
    tClip.timelineOut = m_project->currentPlayhead + clip.duration;
    tClip.sourceIn = 0.0;
    tClip.duration = clip.duration;

    m_project->tracks[1].clips.append(tClip);
    m_timelineWidget->updateTimeline();

    // Update timeline tracks tree
    m_timelineTracksTree->clear();
    for (const auto& track : m_project->tracks) {
        auto* item = new QTreeWidgetItem(m_timelineTracksTree);
        item->setText(0, track.name);
        item->setText(1, QString::number(track.clips.size()));
        double totalDuration = 0.0;
        for (const auto& clip : track.clips) {
            totalDuration = qMax(totalDuration, clip.timelineOut);
        }
        item->setText(2, QString::number(totalDuration, 'f', 2) + "s");
        item->setText(3, track.isLocked ? "Yes" : "No");
    }

    statusBar()->showMessage("Inserted clip onto Timeline.");
}

void MainWindow::onTimelineClipSelected(const QString& clipId) {
    m_effectStack->setSelectedClip(clipId);

    if (!clipId.isEmpty()) {
        TimelineClip* tc = m_project->findTimelineClip(clipId);
        if (tc) {
            MediaClip* mc = m_project->findMediaClip(tc->mediaClipId);
            if (mc) {
                m_projectMonitor->setMedia(mc->filePath);
                m_projectMonitor->seek(tc->sourceIn);
            }
        }
    }
}

void MainWindow::onTimelinePlayheadChanged(double seconds) {
    // Find if there is any active clip under the playhead
    for (const auto& track : m_project->tracks) {
        for (const auto& clip : track.clips) {
            if (seconds >= clip.timelineIn && seconds <= clip.timelineOut) {
                MediaClip* mc = m_project->findMediaClip(clip.mediaClipId);
                if (mc) {
                    double offset = seconds - clip.timelineIn;
                    m_projectMonitor->setMedia(mc->filePath);
                    m_projectMonitor->seek(clip.sourceIn + offset);
                    return;
                }
            }
        }
    }
}

void MainWindow::onEffectChanged() {
    m_timelineWidget->update();
}

void MainWindow::addMediaFiles(const QStringList& paths) {
    for (const QString& file : paths) {
        MediaClip clip;
        if (FFmpegProbe::probeFile(file, clip)) {
            m_projectBinWidget->addClip(clip);

            // Also add to QTreeWidget
            auto* item = new QTreeWidgetItem(m_projectBinTree);
            item->setText(0, clip.fileName);
            item->setText(1, QString::number(clip.duration, 'f', 2) + "s");
            item->setText(2, clip.hasVideo ? (clip.hasAudio ? "Video/Audio" : "Video") : "Audio");
            item->setData(0, Qt::UserRole, clip.id);
        }
    }
}

void MainWindow::openVideoFile() {
    QString filePath = QFileDialog::getOpenFileName(
        this,
        tr("Open Video File"),
        "",
        tr("Video Files (*.mp4 *.mkv *.avi)")
    );

    if (!filePath.isEmpty()) {
        m_mediaPlayer->setSource(QUrl::fromLocalFile(filePath));
        m_mediaPlayer->play();
        statusBar()->showMessage("Playing: " + QFileInfo(filePath).fileName());
    }
}