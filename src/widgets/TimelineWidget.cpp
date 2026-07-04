#include "TimelineWidget.h"
#include <QPainter>
#include <QPainterPath>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QIcon>
#include <QCursor>
#include <QUuid>
#include <QDebug>
#include <cmath>

TimelineWidget::TimelineWidget(QWidget* parent)
    : QWidget(parent)
{
    setAcceptDrops(true);
    setMouseTracking(true);
    setMinimumHeight(200);

    // External layout is expected to embed us along with scrollbars and buttons,
    // or we can build our sub-widgets inline. Let's build a nice controls layout
    // that wraps the main canvas and is contained in a vertical layout.
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // Timeline Toolbar
    auto* toolbar = new QWidget(this);
    toolbar->setStyleSheet("background-color: #252525; border-bottom: 1px solid #1a1a1a;");
    toolbar->setFixedHeight(32);
    auto* toolLayout = new QHBoxLayout(toolbar);
    toolLayout->setContentsMargins(4, 2, 4, 2);
    toolLayout->setSpacing(4);

    m_selectButton = new QToolButton(toolbar);
    m_selectButton->setCheckable(true);
    m_selectButton->setChecked(true);
    m_selectButton->setText("Selection Tool");
    m_selectButton->setToolTip("Normal selection & move tool (S)");
    connect(m_selectButton, &QToolButton::toggled, this, &TimelineWidget::onToolSelectToggled);
    toolLayout->addWidget(m_selectButton);

    m_razorButton = new QToolButton(toolbar);
    m_razorButton->setCheckable(true);
    m_razorButton->setText("Razor Tool");
    m_razorButton->setToolTip("Cut clips in the timeline (X)");
    connect(m_razorButton, &QToolButton::toggled, this, &TimelineWidget::onToolRazorToggled);
    toolLayout->addWidget(m_razorButton);

    toolLayout->addStretch();

    // Zoom slider label & slider
    auto* zoomLabel = new QLabel("Zoom:", toolbar);
    zoomLabel->setStyleSheet("color: #a0a0a0; font-size: 11px;");
    toolLayout->addWidget(zoomLabel);

    m_zoomSlider = new QSlider(Qt::Horizontal, toolbar);
    m_zoomSlider->setRange(5, 100);
    m_zoomSlider->setValue(20);
    m_zoomSlider->setFixedWidth(120);
    connect(m_zoomSlider, &QSlider::valueChanged, this, &TimelineWidget::onZoomChanged);
    toolLayout->addWidget(m_zoomSlider);

    mainLayout->addWidget(toolbar);

    // Content container
    auto* contentContainer = new QWidget(this);
    auto* contentLayout = new QVBoxLayout(contentContainer);
    contentLayout->setContentsMargins(0, 0, 0, 0);
    contentLayout->setSpacing(0);

    // Timeline main interactive canvas is THIS widget's drawing. We'll add a stretch
    // and layout the scrollbar at the bottom.
    contentLayout->addStretch(1);

    m_scrollBar = new QScrollBar(Qt::Horizontal, contentContainer);
    m_scrollBar->setRange(0, 1000);
    connect(m_scrollBar, &QScrollBar::valueChanged, this, &TimelineWidget::onScrollChanged);
    contentLayout->addWidget(m_scrollBar);

    mainLayout->addWidget(contentContainer, 1);
}

void TimelineWidget::setProject(Project* project) {
    m_project = project;
    updateTimeline();
}

void TimelineWidget::updateTimeline() {
    if (m_project) {
        // Adjust scrollbar range
        double projWidth = m_project->duration * m_pixelsPerSecond;
        int maxScroll = static_cast<int>(projWidth) - (width() - m_headerWidth);
        m_scrollBar->setRange(0, qMax(0, maxScroll));
    }
    update();
}

double TimelineWidget::xToTime(int x) const {
    int contentX = x - m_headerWidth + m_scrollOffset;
    return static_cast<double>(contentX) / m_pixelsPerSecond;
}

int TimelineWidget::timeToX(double t) const {
    return static_cast<int>(t * m_pixelsPerSecond) + m_headerWidth - m_scrollOffset;
}

int TimelineWidget::getTrackY(int trackIndex) const {
    return m_rulerHeight + trackIndex * m_trackHeight + 32; // add toolbar offset
}

int TimelineWidget::getTrackUnderMouse(const QPoint& pos) const {
    if (pos.x() < m_headerWidth || !m_project) return -1;
    
    int y = pos.y() - m_rulerHeight - 32;
    if (y < 0) return -1;
    
    int trackIndex = y / m_trackHeight;
    if (trackIndex >= 0 && trackIndex < m_project->tracks.size()) {
        return trackIndex;
    }
    return -1;
}

QString TimelineWidget::getClipUnderMouse(const QPoint& pos, int* trackIndexOut) const {
    if (pos.x() < m_headerWidth || !m_project) return "";

    double t = xToTime(pos.x());
    int trackIdx = getTrackUnderMouse(pos);
    if (trackIdx != -1) {
        const auto& track = m_project->tracks[trackIdx];
        for (const auto& clip : track.clips) {
            if (t >= clip.timelineIn && t <= clip.timelineOut) {
                if (trackIndexOut) *trackIndexOut = trackIdx;
                return clip.id;
            }
        }
    }
    return "";
}

int TimelineWidget::getClipEdgeUnderMouse(const QPoint& pos, const QString& clipId) const {
    if (!m_project || clipId.isEmpty()) return -1;
    
    int trackIdx;
    TimelineClip* clip = m_project->findTimelineClip(clipId, &trackIdx);
    if (!clip) return -1;

    int leftX = timeToX(clip->timelineIn);
    int rightX = timeToX(clip->timelineOut);

    const int threshold = 6; // pixels from edge to count as trim handle
    if (std::abs(pos.x() - leftX) < threshold) return 0;
    if (std::abs(pos.x() - rightX) < threshold) return 1;

    return -1;
}

QRect TimelineWidget::getClipRect(int trackIndex, const TimelineClip& clip) const {
    int x1 = timeToX(clip.timelineIn);
    int x2 = timeToX(clip.timelineOut);
    int y = getTrackY(trackIndex);
    return QRect(x1, y + 2, x2 - x1, m_trackHeight - 4);
}

void TimelineWidget::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    // Draw background
    painter.fillRect(rect(), QColor(30, 30, 30));

    if (!m_project) {
        painter.setPen(QColor(120, 120, 120));
        painter.drawText(rect(), Qt::AlignCenter, "No active project. Load or import media.");
        return;
    }

    drawTracksAndClips(painter);
    drawRuler(painter);
    drawTrackHeaders(painter);
    drawPlayhead(painter);
}

void TimelineWidget::drawRuler(QPainter& painter) {
    painter.fillRect(QRect(0, 32, width(), m_rulerHeight), QColor(45, 45, 45));
    painter.fillRect(QRect(0, 32, m_headerWidth, m_rulerHeight), QColor(55, 55, 55));
    
    painter.setPen(QColor(150, 150, 150));
    painter.setFont(QFont("monospace", 8));

    // Draw grid ticks
    double duration = m_project->duration;
    double step = 1.0; // 1 second steps default
    if (m_pixelsPerSecond < 5.0) step = 10.0;
    else if (m_pixelsPerSecond < 12.0) step = 5.0;
    else if (m_pixelsPerSecond > 45.0) step = 0.5;

    for (double t = 0; t <= duration; t += step) {
        int x = timeToX(t);
        if (x < m_headerWidth || x > width()) continue;

        // Draw major or minor tick
        bool isMajor = (std::fmod(t, step * 5.0) < 0.001);
        int tickHeight = isMajor ? 12 : 6;
        painter.drawLine(x, 32 + m_rulerHeight - tickHeight, x, 32 + m_rulerHeight);

        if (isMajor) {
            int minutes = static_cast<int>(t) / 60;
            int seconds = static_cast<int>(t) % 60;
            double fraction = t - std::floor(t);
            int frames = static_cast<int>(fraction * 25.0); // assume 25 fps timecode display
            QString tc = QString("%1:%2:%3")
                             .arg(minutes, 2, 10, QChar('0'))
                             .arg(seconds, 2, 10, QChar('0'))
                             .arg(frames, 2, 10, QChar('0'));
            painter.drawText(x + 3, 32 + 15, tc);
        }
    }
}

void TimelineWidget::drawTrackHeaders(QPainter& painter) {
    painter.setPen(QPen(QColor(26, 26, 26), 1));
    
    for (int i = 0; i < m_project->tracks.size(); ++i) {
        const auto& track = m_project->tracks[i];
        int y = getTrackY(i);
        
        // Header background
        painter.fillRect(QRect(0, y, m_headerWidth, m_trackHeight), QColor(40, 40, 40));
        
        // Track Name
        painter.setPen(QColor(210, 210, 210));
        painter.setFont(QFont("Arial", 9, QFont::Bold));
        painter.drawText(QRect(10, y + 5, m_headerWidth - 20, 20), Qt::AlignLeft | Qt::AlignVCenter, track.name);

        // Subtitle (Video/Audio indicator)
        painter.setFont(QFont("Arial", 7));
        painter.setPen(QColor(130, 130, 130));
        painter.drawText(QRect(10, y + 25, m_headerWidth - 20, 20), Qt::AlignLeft | Qt::AlignVCenter, track.isAudio ? "AUDIO TRACK" : "VIDEO TRACK");

        // Track border separator
        painter.setPen(QPen(QColor(26, 26, 26), 1));
        painter.drawLine(0, y + m_trackHeight, width(), y + m_trackHeight);
    }

    // Vertical separator between headers and tracks
    painter.setPen(QPen(QColor(20, 20, 20), 2));
    painter.drawLine(m_headerWidth, 32, m_headerWidth, height());
}

void TimelineWidget::drawTracksAndClips(QPainter& painter) {
    // Fill track canvas background
    int canvasY = getTrackY(0);
    int canvasHeight = m_project->tracks.size() * m_trackHeight;
    painter.fillRect(QRect(m_headerWidth, canvasY, width() - m_headerWidth, canvasHeight), QColor(22, 22, 22));

    // Draw clips
    for (int i = 0; i < m_project->tracks.size(); ++i) {
        const auto& track = m_project->tracks[i];
        int trackY = getTrackY(i);

        // Draw track horizontal grid
        painter.setPen(QPen(QColor(33, 33, 33), 1));
        painter.drawLine(m_headerWidth, trackY + m_trackHeight, width(), trackY + m_trackHeight);

        for (const auto& clip : track.clips) {
            QRect rect = getClipRect(i, clip);
            if (rect.right() < m_headerWidth || rect.left() > width()) continue;

            // Clip rect clipping to start at headers
            QRect clipRect = rect;
            if (clipRect.left() < m_headerWidth) {
                clipRect.setLeft(m_headerWidth);
            }

            // Clip colors
            QColor baseColor = track.isAudio ? QColor(46, 125, 50) : QColor(25, 118, 210); // Green for audio, Blue for video
            if (clip.id == m_selectedClipId) {
                baseColor = baseColor.lighter(130);
            }

            // Draw rounded clip block
            QPainterPath path;
            path.addRoundedRect(clipRect, 4, 4);
            painter.fillPath(path, baseColor);

            // Highlights and borders
            if (clip.id == m_selectedClipId) {
                painter.setPen(QPen(QColor(255, 170, 0), 2)); // Gold border for selection
                painter.drawPath(path);
            } else {
                painter.setPen(QPen(baseColor.darker(150), 1));
                painter.drawPath(path);
            }

            // Clip label text
            painter.setPen(Qt::white);
            painter.setFont(QFont("Arial", 8, QFont::Bold));
            MediaClip* mc = m_project->findMediaClip(clip.mediaClipId);
            QString label = mc ? mc->fileName : "Unknown Clip";
            
            // Add indicator if clip has text/effects
            if (!clip.effects.isEmpty()) {
                label += " [FX]";
            }

            painter.drawText(clipRect.adjusted(8, 2, -8, -2), Qt::AlignLeft | Qt::AlignVCenter, label);
        }
    }
}

void TimelineWidget::drawPlayhead(QPainter& painter) {
    if (!m_project) return;

    int x = timeToX(m_project->currentPlayhead);
    if (x < m_headerWidth || x > width()) return;

    // Draw playhead vertical line
    painter.setPen(QPen(QColor(230, 33, 33), 1)); // Red line
    painter.drawLine(x, 32 + m_rulerHeight, x, height());

    // Draw playhead triangle/handle at the ruler
    QPolygon triangle;
    triangle << QPoint(x - 6, 32 + m_rulerHeight - 12)
             << QPoint(x + 6, 32 + m_rulerHeight - 12)
             << QPoint(x + 6, 32 + m_rulerHeight - 4)
             << QPoint(x, 32 + m_rulerHeight)
             << QPoint(x - 6, 32 + m_rulerHeight - 4);
    
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(230, 33, 33));
    painter.drawPolygon(triangle);
}

void TimelineWidget::mousePressEvent(QMouseEvent* event) {
    if (!m_project) return;

    QPoint pos = event->pos();

    // Check if clicked in the ruler area
    if (pos.y() >= 32 && pos.y() < 32 + m_rulerHeight && pos.x() >= m_headerWidth) {
        double newTime = xToTime(pos.x());
        m_project->currentPlayhead = qMax(0.0, qMin(m_project->duration, newTime));
        m_dragState.mode = DragState::MovePlayhead;
        emit playheadChanged(m_project->currentPlayhead);
        update();
        return;
    }

    // Check if clicked a clip
    int trackIdx = -1;
    QString clipId = getClipUnderMouse(pos, &trackIdx);
    if (!clipId.isEmpty() && trackIdx != -1) {
        TimelineClip* clip = m_project->findTimelineClip(clipId);
        if (clip) {
            m_selectedClipId = clipId;
            emit clipSelected(clipId);

            if (m_activeTool == RazorTool) {
                // Perform RAZOR CUT!
                double cutTime = xToTime(pos.x());
                if (cutTime > clip->timelineIn && cutTime < clip->timelineOut) {
                    // Split the clip!
                    double leftDuration = cutTime - clip->timelineIn;
                    double rightDuration = clip->timelineOut - cutTime;

                    TimelineClip secondPart = *clip;
                    secondPart.id = QUuid::createUuid().toString(QUuid::WithoutBraces);
                    secondPart.timelineIn = cutTime;
                    secondPart.timelineOut = clip->timelineOut;
                    secondPart.sourceIn = clip->sourceIn + leftDuration;
                    secondPart.duration = rightDuration;

                    clip->timelineOut = cutTime;
                    clip->duration = leftDuration;

                    m_project->tracks[trackIdx].clips.append(secondPart);
                    m_selectedClipId = secondPart.id;
                    emit clipSelected(secondPart.id);
                    update();
                }
            } else {
                // Selection / Move/ Resize mode
                int edge = getClipEdgeUnderMouse(pos, clipId);
                if (edge == 0) {
                    m_dragState.mode = DragState::ResizeLeft;
                } else if (edge == 1) {
                    m_dragState.mode = DragState::ResizeRight;
                } else {
                    m_dragState.mode = DragState::MoveClip;
                }

                m_dragState.activeClipId = clipId;
                m_dragState.activeTrackIndex = trackIdx;
                m_dragState.dragStartClipIn = clip->timelineIn;
                m_dragState.dragStartClipOut = clip->timelineOut;
                m_dragState.dragStartPlayhead = m_project->currentPlayhead;
                m_dragState.dragStartMousePos = pos;
            }
            update();
            return;
        }
    }

    // Clicked empty area
    m_selectedClipId = "";
    emit clipSelected("");
    update();
}

void TimelineWidget::mouseMoveEvent(QMouseEvent* event) {
    if (!m_project) return;

    QPoint pos = event->pos();

    // Hover mouse cursor update
    if (m_dragState.mode == DragState::None) {
        int trackIdx;
        QString clipId = getClipUnderMouse(pos, &trackIdx);
        if (!clipId.isEmpty()) {
            int edge = getClipEdgeUnderMouse(pos, clipId);
            if (edge != -1) {
                setCursor(Qt::SplitHCursor);
            } else if (m_activeTool == RazorTool) {
                setCursor(Qt::CrossCursor);
            } else {
                setCursor(Qt::ArrowCursor);
            }
        } else {
            setCursor(Qt::ArrowCursor);
        }
    }

    // Handle Active Dragging
    if (m_dragState.mode == DragState::MovePlayhead) {
        double newTime = xToTime(pos.x());
        m_project->currentPlayhead = qMax(0.0, qMin(m_project->duration, newTime));
        emit playheadChanged(m_project->currentPlayhead);
        update();
    } else if (m_dragState.mode == DragState::MoveClip) {
        TimelineClip* clip = m_project->findTimelineClip(m_dragState.activeClipId);
        if (clip) {
            double deltaT = (pos.x() - m_dragState.dragStartMousePos.x()) / m_pixelsPerSecond;
            double targetIn = m_dragState.dragStartClipIn + deltaT;
            targetIn = qMax(0.0, targetIn);

            clip->timelineIn = targetIn;
            clip->timelineOut = targetIn + clip->duration;

            // Track change (dragging vertically between tracks)
            int targetTrackIdx = getTrackUnderMouse(pos);
            if (targetTrackIdx != -1 && targetTrackIdx != m_dragState.activeTrackIndex) {
                // Move the clip from original track to new track
                auto& oldTrack = m_project->tracks[m_dragState.activeTrackIndex];
                auto& newTrack = m_project->tracks[targetTrackIdx];

                for (int i = 0; i < oldTrack.clips.size(); ++i) {
                    if (oldTrack.clips[i].id == clip->id) {
                        newTrack.clips.append(oldTrack.clips.takeAt(i));
                        m_dragState.activeTrackIndex = targetTrackIdx;
                        break;
                    }
                }
            }
            update();
        }
    } else if (m_dragState.mode == DragState::ResizeLeft) {
        TimelineClip* clip = m_project->findTimelineClip(m_dragState.activeClipId);
        if (clip) {
            double deltaT = (pos.x() - m_dragState.dragStartMousePos.x()) / m_pixelsPerSecond;
            double targetIn = m_dragState.dragStartClipIn + deltaT;
            targetIn = qMax(0.0, qMin(m_dragState.dragStartClipOut - 0.1, targetIn)); // min 0.1s duration

            double extraSource = targetIn - clip->timelineIn;
            clip->sourceIn += extraSource;
            clip->timelineIn = targetIn;
            clip->duration = clip->timelineOut - clip->timelineIn;
            update();
        }
    } else if (m_dragState.mode == DragState::ResizeRight) {
        TimelineClip* clip = m_project->findTimelineClip(m_dragState.activeClipId);
        if (clip) {
            double deltaT = (pos.x() - m_dragState.dragStartMousePos.x()) / m_pixelsPerSecond;
            double targetOut = m_dragState.dragStartClipOut + deltaT;
            targetOut = qMax(m_dragState.dragStartClipIn + 0.1, targetOut);

            clip->timelineOut = targetOut;
            clip->duration = clip->timelineOut - clip->timelineIn;
            update();
        }
    }
}

void TimelineWidget::mouseReleaseEvent(QMouseEvent* event) {
    Q_UNUSED(event);
    m_dragState.mode = DragState::None;
    setCursor(Qt::ArrowCursor);
    update();
}

void TimelineWidget::dragEnterEvent(QDragEnterEvent* event) {
    if (event->mimeData()->hasText() || event->mimeData()->hasFormat("application/x-qabstractitemmodeldatalist")) {
        event->acceptProposedAction();
    }
}

void TimelineWidget::dragMoveEvent(QDragMoveEvent* event) {
    event->acceptProposedAction();
}

void TimelineWidget::dropEvent(QDropEvent* event) {
    if (!m_project) return;

    // Get the dragged clip ID. Since ProjectBinWidget sets standard QListWidget drag,
    // we can retrieve the selected item's clip ID from bin.
    // For simplicity, we can fetch the clip details from bin based on the selected item.
    // Let's assume the mime text matches the ID of the clip or we can fallback to the current bin clip.
    // To be perfectly robust, we can broadcast or query our media bin.
    // Let's extract clip ID from text or fallback to active media clips.
    QString clipId = event->mimeData()->text();
    
    // If MIME doesn't directly expose clipId, check if there's any media clip in the project at all
    if (m_project->mediaClips.isEmpty()) return;

    MediaClip* mc = nullptr;
    if (!clipId.isEmpty()) {
        mc = m_project->findMediaClip(clipId);
    }
    
    if (!mc) {
        // Fallback to the first media clip
        mc = &m_project->mediaClips.first();
    }

    int trackIdx = getTrackUnderMouse(event->position().toPoint());
    if (trackIdx != -1 && mc) {
        double dropTime = xToTime(event->position().toPoint().x());
        dropTime = qMax(0.0, dropTime);

        TimelineClip newClip;
        newClip.id = QUuid::createUuid().toString(QUuid::WithoutBraces);
        newClip.mediaClipId = mc->id;
        newClip.timelineIn = dropTime;
        newClip.timelineOut = dropTime + mc->duration;
        newClip.sourceIn = 0.0;
        newClip.duration = mc->duration;

        m_project->tracks[trackIdx].clips.append(newClip);
        m_selectedClipId = newClip.id;
        emit clipSelected(newClip.id);
        update();
        event->acceptProposedAction();
    }
}

void TimelineWidget::onZoomChanged(int value) {
    m_pixelsPerSecond = static_cast<double>(value);
    updateTimeline();
}

void TimelineWidget::onScrollChanged(int value) {
    m_scrollOffset = value;
    update();
}

void TimelineWidget::onToolSelectToggled(bool checked) {
    if (checked) {
        m_activeTool = SelectionTool;
        m_razorButton->setChecked(false);
    }
}

void TimelineWidget::onToolRazorToggled(bool checked) {
    if (checked) {
        m_activeTool = RazorTool;
        m_selectButton->setChecked(false);
    }
}
