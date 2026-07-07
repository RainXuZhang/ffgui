#include <QMimeData>
#include <QPainter>
#include <QMouseEvent>
#include "TimelineWidget.h"

static constexpr int TRACK_HEADER_WIDTH = 120;
static constexpr int TRACK_HEIGHT = 50;
static constexpr int BUTTON_MARGIN = 4;
static constexpr int BUTTON_WIDTH = 28;

TimelineWidget::TimelineWidget(QWidget* parent)
    : QWidget(parent)
{
    setMinimumHeight(100);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    setAcceptDrops(true);
}

void TimelineWidget::setProject(Project* project) {
    m_project = project;
    update();
}

void TimelineWidget::setPosition(double seconds) {
    m_currentPosition = seconds;
    update();
}

void TimelineWidget::updateTimeline() {
    update();
}

// TimelineWidget implementation

void TimelineWidget::setDuration(double seconds) {
    totalDurationSeconds = seconds;
    update();
}

void TimelineWidget::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // Draw track header column on the left (120px)
    QRect headersRect(0, 0, TRACK_HEADER_WIDTH, height());
    painter.fillRect(headersRect, QColor(25, 25, 40));

    // Draw a vertical separator line
    painter.setPen(QPen(QColor(60, 60, 80), 1));
    painter.drawLine(TRACK_HEADER_WIDTH, 0, TRACK_HEADER_WIDTH, height());

    // Draw timeline area background (right of headers)
    QRect timelineRect(TRACK_HEADER_WIDTH, 0, width() - TRACK_HEADER_WIDTH, height());
    painter.fillRect(timelineRect, QColor(30, 30, 46));

    // Draw track headers and timeline area rows
    if (m_project) {
        for (int i = 0; i < m_project->tracks.size(); ++i) {
            const auto& track = m_project->tracks[i];
            int y = i * TRACK_HEIGHT;

            // Track header background
            QRect headerRect(0, y, TRACK_HEADER_WIDTH - 1, TRACK_HEIGHT - 1);
            painter.fillRect(headerRect, QColor(35, 35, 50));

            // Track name
            painter.setPen(QPen(QColor(205, 214, 244)));
            QFont nameFont = painter.font();
            nameFont.setPointSize(8);
            nameFont.setBold(true);
            painter.setFont(nameFont);
            painter.drawText(headerRect.adjusted(BUTTON_MARGIN, BUTTON_MARGIN, 0, -TRACK_HEIGHT / 2),
                             Qt::AlignLeft | Qt::AlignVCenter, track.trackName);

            // Lock button area
            QRect lockBtnRect(BUTTON_MARGIN, y + TRACK_HEIGHT / 2 + BUTTON_MARGIN,
                              BUTTON_WIDTH, TRACK_HEIGHT / 2 - BUTTON_MARGIN * 2);
            QColor lockColor = track.locked ? QColor(239, 120, 120) : QColor(100, 100, 120);
            painter.setBrush(lockColor);
            painter.setPen(QPen(QColor(80, 80, 100), 1));
            painter.drawRoundedRect(lockBtnRect, 3, 3);
            painter.setPen(QPen(Qt::white));
            painter.drawText(lockBtnRect, Qt::AlignCenter, track.locked ? "🔒" : "🔓");

            // Mute button area
            QRect muteBtnRect(BUTTON_MARGIN + BUTTON_WIDTH + BUTTON_MARGIN, y + TRACK_HEIGHT / 2 + BUTTON_MARGIN,
                              BUTTON_WIDTH, TRACK_HEIGHT / 2 - BUTTON_MARGIN * 2);
            QColor muteColor = track.muted ? QColor(239, 120, 120) : QColor(100, 100, 120);
            painter.setBrush(muteColor);
            painter.setPen(QPen(QColor(80, 80, 100), 1));
            painter.drawRoundedRect(muteBtnRect, 3, 3);
            painter.setPen(QPen(Qt::white));
            painter.drawText(muteBtnRect, Qt::AlignCenter, track.muted ? "🔇" : "🔊");

            // Timeline row background
            QRect trackTimelineRect(TRACK_HEADER_WIDTH, y, width() - TRACK_HEADER_WIDTH, TRACK_HEIGHT - 1);
            painter.fillRect(trackTimelineRect, (i % 2 == 0) ? QColor(35, 35, 55) : QColor(40, 40, 60));

            // Horizontal separator line
            painter.setPen(QPen(QColor(50, 50, 70), 1));
            painter.drawLine(0, y + TRACK_HEIGHT - 1, width(), y + TRACK_HEIGHT - 1);
        }
    }

    // Draw timeline clocks for each second (adjusted for header offset)
    int timelineWidth = width() - TRACK_HEADER_WIDTH;
    if (timelineWidth > 0 && totalDurationSeconds > 0) {
        painter.setPen(QPen(QColor(205, 214, 244)));

        // Draw time markers in the timeline area
        int numMarkers = qMin(10, qMax(1, timelineWidth / 80));
        for (int i = 0; i <= numMarkers; ++i) {
            int x = TRACK_HEADER_WIDTH + i * timelineWidth / numMarkers;
            painter.drawLine(x, height() - 10, x, height());
            double time = (i * totalDurationSeconds) / numMarkers;
            painter.drawText(x - 20, height() - 20, 40, 15, Qt::AlignCenter,
                             QString::number(time, 'f', 1) + "s");
        }
    }

    // Draw selection highlight
    if (timelineWidth > 0 && totalDurationSeconds > 0 && inPointSeconds < outPointSeconds) {
        int inX = TRACK_HEADER_WIDTH + static_cast<int>((inPointSeconds / totalDurationSeconds) * timelineWidth);
        int outX = TRACK_HEADER_WIDTH + static_cast<int>((outPointSeconds / totalDurationSeconds) * timelineWidth);
        painter.fillRect(inX, 0, outX - inX, height(), QColor(137, 180, 250, 50));
    }

    // Draw playhead
    if (timelineWidth > 0 && totalDurationSeconds > 0) {
        int playheadX = TRACK_HEADER_WIDTH + static_cast<int>((m_currentPosition / totalDurationSeconds) * timelineWidth);
        painter.setPen(QPen(QColor(137, 180, 250), 2));
        painter.drawLine(playheadX, 0, playheadX, height());
    }
}

void TimelineWidget::mousePressEvent(QMouseEvent* event) {
    if (!m_project || event->button() != Qt::LeftButton) {
        QWidget::mousePressEvent(event);
        return;
    }

    int x = event->pos().x();
    int y = event->pos().y();

    // Check if click is in the track header area (left 120px)
    if (x < TRACK_HEADER_WIDTH && m_project) {
        int trackIndex = y / TRACK_HEIGHT;
        if (trackIndex >= 0 && trackIndex < m_project->tracks.size()) {
            auto& track = m_project->tracks[trackIndex];
            int trackY = trackIndex * TRACK_HEIGHT;

            // Lock button region
            QRect lockBtnRect(BUTTON_MARGIN, trackY + TRACK_HEIGHT / 2 + BUTTON_MARGIN,
                              BUTTON_WIDTH, TRACK_HEIGHT / 2 - BUTTON_MARGIN * 2);
            if (lockBtnRect.contains(x, y)) {
                track.locked = !track.locked;
                update();
                return;
            }

            // Mute button region
            QRect muteBtnRect(BUTTON_MARGIN + BUTTON_WIDTH + BUTTON_MARGIN, trackY + TRACK_HEIGHT / 2 + BUTTON_MARGIN,
                              BUTTON_WIDTH, TRACK_HEIGHT / 2 - BUTTON_MARGIN * 2);
            if (muteBtnRect.contains(x, y)) {
                track.muted = !track.muted;
                update();
                return;
            }
        }
        return;
    }

    // Click is in the timeline area - handle playhead positioning
    if (totalDurationSeconds > 0) {
        m_isDragging = true;
        int timelineWidth = width() - TRACK_HEADER_WIDTH;
        int timelineX = x - TRACK_HEADER_WIDTH;
        double newPosition = static_cast<double>(timelineX) / timelineWidth * totalDurationSeconds;
        newPosition = qBound(0.0, newPosition, totalDurationSeconds);
        m_currentPosition = newPosition;
        emit seekRequested(m_currentPosition);
        update();
    }
}

void TimelineWidget::keyPressEvent(QKeyEvent* event) {
    if (totalDurationSeconds > 0) {
        if (event->key() == Qt::Key_I) {
            setInPointSeconds(m_currentPosition);
            update();
        } else if (event->key() == Qt::Key_O) {
            setOutPointSeconds(m_currentPosition);
            update();
        }
    }
}

void TimelineWidget::mouseMoveEvent(QMouseEvent* event) {
    if (m_isDragging && totalDurationSeconds > 0) {
        double newPosition = static_cast<double>(event->pos().x()) / width() * totalDurationSeconds;
        newPosition = qBound(0.0, newPosition, totalDurationSeconds);
        m_currentPosition = newPosition;
        emit seekRequested(m_currentPosition);
        update();
    }
}

void TimelineWidget::dragEnterEvent(QDragEnterEvent* event) {
    if (event->mimeData()->hasFormat("application/x-ffgui-clip") || event->mimeData()->hasUrls()) {
        event->acceptProposedAction();
    }
}

void TimelineWidget::dragMoveEvent(QDragMoveEvent* event) {
    if (event->mimeData()->hasFormat("application/x-ffgui-clip") || event->mimeData()->hasUrls()) {
        event->acceptProposedAction();
    }
}

void TimelineWidget::dropEvent(QDropEvent* event) {
    if (event->mimeData()->hasFormat("application/x-ffgui-clip")) {
        QString clipId = event->mimeData()->data("application/x-ffgui-clip");

        // Calculate the drop timestamp timeline location (mouse position X coordinate / 100.0).
        double playheadTime = static_cast<double>(event->position().x()) / 100.0;

        // Identify the track channel index row (mouse position Y coordinate / 50).
        int trackIndex = static_cast<int>(event->position().y()) / 50;

        // Determine if it is an audio layer drop (track index >= 2).
        bool isAudio = (trackIndex >= 2);

        emit clipDropped(clipId, playheadTime, isAudio);
        event->acceptProposedAction();
        update();
    } else {
        event->ignore();
    }
}
void TimelineWidget::mouseReleaseEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        m_isDragging = false;
        emit razorToolClicked(event->pos());
    }
}
