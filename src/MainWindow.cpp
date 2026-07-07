#include "MainWindow.h"
#include "core/ThemeManager.h"
#include "core/FFmpegProbe.h"
#include <QDialog>
#include <QFormLayout>
#include <QDialogButtonBox>
#include <QVBoxLayout>
#include <QComboBox>
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
#include <QProgressBar>
#include <QMimeData>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QDesktopServices>
#include "widgets/TimelineWidget.h"
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
    setupFormatComboBox();
    statusBar()->showMessage("Ready");
}

MainWindow::~MainWindow() {
    delete m_project;
}

void MainWindow::setupProjectModel() {
    delete m_project;
    m_project = new Project();

    // Add standard tracks matching Kdenlive tracks (V2, V1, A1, A2)
    m_project->tracks.append({1, "Video 2", false, false, false, 1.0, {}});
    m_project->tracks.append({2, "Video 1", false, false, false, 1.0, {}});
    m_project->tracks.append({3, "Audio 1", true, false, false, 1.0, {}});
    m_project->tracks.append({4, "Audio 2", true, false, false, 1.0, {}});
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

    QAction* prefAction = editMenu->addAction(tr("&Preferences"));
    connect(prefAction, &QAction::triggered, this, &MainWindow::onPreferences);

    // View Menu
    QMenu* viewMenu = menuBar->addMenu(tr("&View"));
    viewMenu->addAction(tr("Fullscreen"), QKeySequence::FullScreen, this, [this]() {
        if (isFullScreen()) {
            showNormal();
        } else {
            showFullScreen();
        }
    });
    
    // Add Themes submenu to View menu
    QMenu* themesMenu = viewMenu->addMenu(tr("&Themes"));
    QAction* darkThemeAction = themesMenu->addAction(tr("&Dark"));
    QAction* lightThemeAction = themesMenu->addAction(tr("&Light"));
    QAction* kdenliveThemeAction = themesMenu->addAction(tr("&Kdenlive"));
    
    // Connect theme actions to slots
    connect(darkThemeAction, &QAction::triggered, this, [this]() {
        ThemeManager::applyTheme(ThemeManager::Dark, qApp);
        ThemeManager::saveTheme(ThemeManager::Dark);
        this->centralWidget()->update();
    });
    connect(lightThemeAction, &QAction::triggered, this, [this]() {
        ThemeManager::applyTheme(ThemeManager::Light, qApp);
        ThemeManager::saveTheme(ThemeManager::Light);
        this->centralWidget()->update();
    });
    connect(kdenliveThemeAction, &QAction::triggered, this, [this]() {
        ThemeManager::applyTheme(ThemeManager::Kdenlive, qApp);
        ThemeManager::saveTheme(ThemeManager::Kdenlive);
        this->centralWidget()->update();
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
    QFile configFile(".config");
    if (configFile.exists() && configFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&configFile);
        while (!in.atEnd()) {
            QString line = in.readLine();
            if (line.startsWith("theme=")) {
                int themeIndex = line.mid(6).toInt();
                applyThemeString(themeIndex);
                this->setProperty("currentThemeIndex", themeIndex);
                return;
            }
        }
    }

    // Fallback to default theme if config file doesn't exist or is unreadable
    applyThemeString(0);
    this->setProperty("currentThemeIndex", 0);
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
    if (event->modifiers() & Qt::ControlModifier) {
        qint64 currentPos = m_mediaPlayer->position();
        qint64 newPos = qMax(currentPos - 5000, 0LL);
        m_mediaPlayer->setPosition(newPos);
        event->accept();
    } else if (event->modifiers() & Qt::ShiftModifier) {
        m_mediaPlayer->setPosition(static_cast<qint64>(m_timelineWidget->getInPointSeconds() * 1000));
        event->accept();
    } else {
        qint64 currentPos = m_mediaPlayer->position();
        qint64 newPos = qMax(currentPos - 33, 0LL);
        m_mediaPlayer->setPosition(newPos);
        event->accept();
    }
}
break;
case Qt::Key_Right:
{
    if (event->modifiers() & Qt::ControlModifier) {
        qint64 currentPos = m_mediaPlayer->position();
        qint64 totalDuration = m_mediaPlayer->duration();
        qint64 newPos = qMin(currentPos + 5000, totalDuration);
        m_mediaPlayer->setPosition(newPos);
        event->accept();
    } else if (event->modifiers() & Qt::ShiftModifier) {
        m_mediaPlayer->setPosition(static_cast<qint64>(m_timelineWidget->getOutPointSeconds() * 1000));
        event->accept();
    } else {
        qint64 currentPos = m_mediaPlayer->position();
        qint64 totalDuration = m_mediaPlayer->duration();
        qint64 newPos = qMin(currentPos + 33, totalDuration);
        m_mediaPlayer->setPosition(newPos);
        event->accept();
    }
}
break;
        default:
            QMainWindow::keyPressEvent(event);
    }
}

void MainWindow::setupSplitterLayout() {
    // 1. Create the central container widget
    QWidget* centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    auto* mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setContentsMargins(2, 2, 2, 2);

    // 2. Initialize the nested splitters
    m_mainVerticalSplitter = new QSplitter(Qt::Vertical, this);
    m_topHorizontalSplitter = new QSplitter(Qt::Horizontal, this);

// 3. SAFE INSTANTIATION OF ACTIVE WIDGETS (Prevents Segfaults)
m_projectBinWidget = new ProjectBinWidget(this);

// Create a tab widget or layout deck to hold your video monitor preview frames
QTabWidget* monitorTabs = new QTabWidget(this);
m_clipMonitor = new MonitorWidget("Clip Preview", this);
m_projectMonitor = new MonitorWidget("Project Timeline Preview", this);
monitorTabs->addTab(m_clipMonitor, "Clip Monitor");
monitorTabs->addTab(m_projectMonitor, "Project Monitor");

m_timelineWidget = new TimelineWidget(this);

    // 4. ASSEMBLE SYSTEM HIERARCHY
    // Top Row: Project Bin on the Left, Video Monitor on the Right
    m_topHorizontalSplitter->addWidget(m_projectBinWidget);
    m_topHorizontalSplitter->addWidget(monitorTabs);

    // Main Stack: Top Row goes on top, Timeline Track goes below
    m_mainVerticalSplitter->addWidget(m_topHorizontalSplitter);
    m_mainVerticalSplitter->addWidget(m_timelineWidget);

    // Put the entire splitter tree into the window's layout view frame
    mainLayout->addWidget(m_mainVerticalSplitter);

    // 5. FLEXIBLE RESIZING & ALLOCATION (Kdenlive ratios)
    // Horizontal: Give Bin 25% width (1 share), Monitors 75% width (3 shares)
    m_topHorizontalSplitter->setStretchFactor(0, 1);
    m_topHorizontalSplitter->setStretchFactor(1, 3);

    // Vertical: Upper Deck gets 65% height (13 shares), Timeline gets 35% height (7 shares)
    m_mainVerticalSplitter->setStretchFactor(0, 13);
    m_mainVerticalSplitter->setStretchFactor(1, 7);

    // Set handles to collapsible = false so elements can never snap shut to 0px accidentally
    m_topHorizontalSplitter->setChildrenCollapsible(false);
    m_mainVerticalSplitter->setChildrenCollapsible(false);

    // 6. FORCE REFRESH VISIBILITY
    m_projectBinWidget->show();
    monitorTabs->show();
    m_timelineWidget->show();
}

void MainWindow::dragEnterEvent(QDragEnterEvent* event) {
    if (event->mimeData()->hasUrls()) {
        event->acceptProposedAction();
    }
}

void MainWindow::dropEvent(QDropEvent* event) {
    if (event->mimeData()->hasUrls()) {
        QStringList paths;
        for (const QUrl& url : event->mimeData()->urls()) {
            if (url.isLocalFile()) {
                paths.append(url.toLocalFile());
            }
        }
        if (!paths.isEmpty()) {
            addMediaFiles(paths);
        }
    }
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
        track.trackName = tObj["trackName"].toString(tObj["name"].toString()); // fallback to old "name" key
        track.locked = tObj["locked"].toBool(tObj["isLocked"].toBool()); // fallback to old "isLocked" key
        track.muted = tObj["muted"].toBool(tObj["isMutedOrHidden"].toBool()); // fallback to old "isMutedOrHidden" key
        track.volume = tObj["volume"].toDouble(1.0);

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
        item->setText(0, track.trackName);
        item->setText(1, QString::number(track.clips.size()));
        double totalDuration = 0.0;
        for (const auto& clip : track.clips) {
            totalDuration = qMax(totalDuration, clip.timelineOut);
        }
        item->setText(2, QString::number(totalDuration, 'f', 2) + "s");
        item->setText(3, track.locked ? "Yes" : "No");
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
        item->setText(0, track.trackName);
        item->setText(1, QString::number(track.clips.size()));
        double totalDuration = 0.0;
        for (const auto& clip : track.clips) {
            totalDuration = qMax(totalDuration, clip.timelineOut);
        }
        item->setText(2, QString::number(totalDuration, 'f', 2) + "s");
        item->setText(3, track.locked ? "Yes" : "No");
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
        tObj["trackName"] = track.trackName;
        tObj["locked"] = track.locked;
        tObj["muted"] = track.muted;
        tObj["volume"] = track.volume;

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
    // Initialize FFmpeg process
    ffmpegProcess = new QProcess(this);
    connect(ffmpegProcess, &QProcess::readyReadStandardError, this, &MainWindow::handleFFmpegOutput);
    connect(ffmpegProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, &MainWindow::handleFFmpegFinished);

    // Set total cut duration
    totalCutDurationSeconds = m_timelineWidget->getSelectedDuration();

    // Reset progress bar
    progressBar->setValue(0);

    // Update command preview
    updateCommandPreview();

    // Show file save dialog
    QString outputFilePath = QFileDialog::getSaveFileName(
        this,
        tr("Save Rendered Video"),
        QFileInfo(m_currentInputFilePath).baseName() + "_rendered",
        tr("Video Files (*.mp4 *.mkv *.avi)")
    );

    if (!outputFilePath.isEmpty()) {
        // Create and store render dialog
        m_renderDialog = new RenderDialog(m_project, this);
        m_renderDialog->setOutputFilePath(outputFilePath);

        // Connect the dialog's finished signal to a slot that will handle the result
        connect(m_renderDialog, &QDialog::finished, this, [this](int result) {
            if (result == QDialog::Accepted) {
                // Handle successful render
                handleFFmpegFinished(0);
            } else {
                // Handle render cancellation or failure
                handleFFmpegFinished(1);
            }
        });

        m_renderDialog->exec();
    }
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
        item->setText(0, track.trackName);
        item->setText(1, QString::number(track.clips.size()));
        double totalDuration = 0.0;
        for (const auto& clip : track.clips) {
            totalDuration = qMax(totalDuration, clip.timelineOut);
        }
        item->setText(2, QString::number(totalDuration, 'f', 2) + "s");
        item->setText(3, track.locked ? "Yes" : "No");
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

void MainWindow::updatePlayPauseButton() {
    // Check if the current state is playing
    // (Qt6 uses QMediaPlayer::PlaybackState::PlayingState, Qt5 uses QMediaPlayer::PlayingState)
    if (m_mediaPlayer->playbackState() == QMediaPlayer::PlayingState) {
        // Update your button icon or text to show "Pause"
        m_playPauseButton->setIcon(QIcon(":/icons/pause.png"));
    } else {
        // Update your button icon or text to show "Play"
        m_playPauseButton->setIcon(QIcon(":/icons/play.png"));
    }

    // Update command preview
    updateCommandPreview();
}

void MainWindow::handleFFmpegOutput() {
    QByteArray output = ffmpegProcess->readAllStandardError();
    QString logString = QString::fromUtf8(output);

    // Log to terminal/console view text box if necessary
    // logTextEdit->appendPlainText(logString);

    // Extract time parameter using a quick search profile or RegExp
    // Example match format: time=00:01:23.45
    if (logString.contains("time=")) {
        int index = logString.indexOf("time=") + 5;
        QString timeStr = logString.mid(index, 11); // Pulls "HH:MM:SS.xx"

        QStringList parts = timeStr.split(":");
        if (parts.size() == 3) {
            double hours = parts[0].toDouble();
            double minutes = parts[1].toDouble();
            double seconds = parts[2].toDouble();

            double currentProgressSeconds = (hours * 3600.0) + (minutes * 60.0) + seconds;

            if (totalCutDurationSeconds > 0.0) {
                int percentage = static_cast<int>((currentProgressSeconds / totalCutDurationSeconds) * 100.0);
                progressBar->setValue(qBound(0, percentage, 100));
            }
        }
    }
}

void MainWindow::handleFFmpegFinished(int exitCode) {
    if (exitCode != 0) {
        // Task failed, style the progress bar or throw an alert box
        progressBar->setStyleSheet("QProgressBar::chunk { background-color: red; }");

        QMessageBox::critical(this,
                              "Render Failed",
                              QString("FFmpeg exited with error code %1.\nPlease check your settings or source file.").arg(exitCode));
    } else {
        // Task succeeded
        progressBar->setValue(100);
        progressBar->setStyleSheet(""); // Reset to default look
        QMessageBox::information(this, "Success", "Video processing completed successfully!");

        // Add log entry to history list
        QString timestamp = QDateTime::currentDateTime().toString("hh:mm:ss");
        QString outputFile = m_renderDialog->getOutputFilePath(); // Assuming RenderDialog has a method to get the output file path
        QString logEntry = QString("[%1] Exported: %2").arg(timestamp, QFileInfo(outputFile).fileName());

        QListWidgetItem* item = new QListWidgetItem(logEntry);
        item->setToolTip(outputFile); // Store full path in tooltip for user convenience
        historyListWidget->addItem(item);

// Connect double-click handler to open the file or folder
connect(historyListWidget, &QListWidget::itemDoubleClicked, this, [this](QListWidgetItem* item) {
    QString filePath = item->toolTip();
    QFileInfo fileInfo(filePath);
    if (fileInfo.exists()) {
        QDesktopServices::openUrl(QUrl::fromLocalFile(fileInfo.absoluteFilePath()));
    } else {
        QMessageBox::warning(this, "File Not Found", "The file could not be found at the specified location.");
    }
});
    }

    if (ffmpegProcess) {
        ffmpegProcess->deleteLater();
        ffmpegProcess = nullptr;
    }
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
        m_currentInputFilePath = QFileInfo(filePath).absoluteFilePath();
        m_mediaPlayer->setSource(QUrl::fromLocalFile(filePath));
        m_mediaPlayer->play();
        statusBar()->showMessage("Playing: " + QFileInfo(filePath).fileName());
    }
}

void MainWindow::onPreferences() {
    QDialog dialog(this);
    dialog.setWindowTitle(tr("Preferences"));
    dialog.setMinimumWidth(320);

    auto* layout = new QVBoxLayout(&dialog);
    auto* formLayout = new QFormLayout();
    auto* themeCombo = new QComboBox(&dialog);
    themeCombo->addItems({tr("Arch Stealth"), tr("Kdenlive Dark"), tr("Nordic Frost")});

    int currentTheme = this->property("currentThemeIndex").toInt();
    themeCombo->setCurrentIndex(currentTheme);

    formLayout->addRow(tr("Interface Theme:"), themeCombo);
    layout->addLayout(formLayout);

    auto* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
    layout->addWidget(buttonBox);

    connect(buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    if (dialog.exec() == QDialog::Accepted) {
        int selectedIndex = themeCombo->currentIndex();
        this->setProperty("currentThemeIndex", selectedIndex);
        applyThemeString(selectedIndex);
    }
}

void MainWindow::applyThemeString(int themeIndex) {
    QString baseStyle = R"(
        QMainWindow, QDialog, QWidget, QTabWidget, QStackedWidget { background-color: %1; color: %2; font-family: 'Segoe UI', Arial, sans-serif; }
        QSplitter::handle { background-color: %3; }
        QSplitter::handle:horizontal { width: 4px; }
        QSplitter::handle:vertical { height: 4px; }
        QLineEdit, QComboBox, QTextEdit, QListWidget, QTreeWidget, QTabWidget, QStackedWidget { background-color: %4; color: %2; border: 1px solid %3; border-radius: 4px; padding: 4px; }
        QListWidget::item:selected, QTreeWidget::item:selected { background-color: %5; color: %6; border: 1px solid %7; font-weight: bold; }
        QListWidget::item:hover, QTreeWidget::item:hover { background-color: %8; }
        QPushButton { background-color: %9; color: %2; border: 1px solid %3; border-radius: 4px; padding: 5px 12px; }
        QPushButton:hover { background-color: %5; border: 1px solid %7; color: #ffffff; }
        QTabWidget::panel { border: 1px solid %3; background-color: %1; }
        QTabBar::tab { background-color: %4; color: %10; border: 1px solid %3; padding: 6px 14px; }
        QTabBar::tab:selected { background-color: %1; color: %6; border-bottom-color: %1; font-weight: bold; }
        QStatusBar, QMenuBar, QToolBar { background-color: %4; border: none; color: %10; }
        QMenuBar::item:selected { background-color: %5; color: %6; }
    )";

    QString bg, text, border, inputBg, selBg, selText, selBorder, hoverBg, btnBg, tabText;

    if (themeIndex == 0) { // Arch Stealth
        bg = "#0d0e11"; text = "#d1d5db"; border = "#1f232a"; inputBg = "#050607";
        selBg = "#172a3a"; selText = "#38bdf8"; selBorder = "#0284c7"; hoverBg = "#0f172a";
        btnBg = "#111827"; tabText = "#6b7280";
    }
    else if (themeIndex == 1) { // Kdenlive Dark
        bg = "#2a2a2a"; text = "#eff0f1"; border = "#31363b"; inputBg = "#1d2023";
        selBg = "#3daee9"; selText = "#ffffff"; selBorder = "#297fa6"; hoverBg = "#2a3642";
        btnBg = "#31363b"; tabText = "#bdc3c7";
    }
    else { // Nordic Frost
        bg = "#2e3440"; text = "#d8dee9"; border = "#4c566a"; inputBg = "#242933";
        selBg = "#88c0d0"; selText = "#2e3440"; selBorder = "#81a1c1"; hoverBg = "#3b4252";
        btnBg = "#434c5e"; tabText = "#e5e9f0";
    }

    this->setStyleSheet(baseStyle.arg(bg, text, border, inputBg, selBg, selText, selBorder, hoverBg, btnBg, tabText));

    for (QWidget* w : findChildren<QWidget*>()) {
        w->style()->unpolish(w);
        w->style()->polish(w);
        w->update();
    }

    QFile configFile(".config");
    if (configFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&configFile);
        out << "theme=" << themeIndex << "\n";
    }
}

void MainWindow::onClipDropped(const QString& clipId, double playheadTime, bool isAudio, int trackIndex) {
    if (!m_project) return;

    // Create a new timeline clip
    TimelineClip newClip;
    newClip.id = QUuid::createUuid().toString(QUuid::WithoutBraces);
    newClip.mediaClipId = clipId;
    newClip.timelineIn = playheadTime;
    newClip.sourceIn = 0.0;

    // Find the media clip to get duration
    MediaClip* mediaClip = nullptr;
    for (auto& clip : m_project->mediaClips) {
        if (clip.id == clipId) {
            mediaClip = &clip;
            break;
        }
    }

    if (mediaClip) {
        newClip.duration = mediaClip->duration;
        newClip.timelineOut = playheadTime + mediaClip->duration;

        // Add the clip to the appropriate track
        for (auto& track : m_project->tracks) {
            if (track.id == trackIndex + 1) { // Track IDs start at 1
                track.clips.append(newClip);
                break;
            }
        }

        // Update the timeline
        if (m_timelineWidget) {
            m_timelineWidget->updateTimeline();
        }

        // Update the timeline tracks tree
        m_timelineTracksTree->clear();
        for (const auto& track : m_project->tracks) {
            auto* item = new QTreeWidgetItem(m_timelineTracksTree);
        item->setText(0, track.trackName);
        item->setText(1, QString::number(track.clips.size()));
        double totalDuration = 0.0;
        for (const auto& clip : track.clips) {
            totalDuration = qMax(totalDuration, clip.timelineOut);
        }
        item->setText(2, QString::number(totalDuration, 'f', 2) + "s");
        item->setText(3, track.locked ? "Yes" : "No");
        }

        statusBar()->showMessage("Inserted clip onto Timeline.");
    }
}

void MainWindow::setupFormatComboBox() {
    // Stub implementation to satisfy linker
}

void MainWindow::onRazorToolTriggered() {
    qDebug() << "Razor tool triggered";
}

void MainWindow::onRazorToolClicked(QPoint pos) {
    qDebug() << "Razor tool clicked at position:" << pos;
}

void MainWindow::onClipDropped(const QString& clipId, double playheadTime, bool isAudio) {
    qDebug() << "Clip dropped:" << clipId << "at time:" << playheadTime << "isAudio:" << isAudio;
}

void MainWindow::updateCommandPreview() {
    // Stub implementation to satisfy linker
}