#include <QMimeData>
#include <QPainter>
#include <QMouseEvent>
#include "TimelineWidget.h"
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

// Draw timeline background
painter.fillRect(rect(), QColor(30, 30, 46));

// Draw playhead
if (totalDurationSeconds > 0) {
    int playheadX = static_cast<int>((m_currentPosition / totalDurationSeconds) * width());
    painter.setPen(QPen(QColor(137, 180, 250), 2));
    painter.drawLine(playheadX, 0, playheadX, height());
}

// Draw time markers
painter.setPen(QPen(QColor(205, 214, 244)));
for (int i = 0; i <= 10; ++i) {
    int x = i * width() / 10;
    painter.drawLine(x, height() - 10, x, height());
    if (totalDurationSeconds > 0) {
        double time = (i * totalDurationSeconds) / 10;
        painter.drawText(x, height() - 20, QString::number(time, 'f', 1) + "s");
    }
}

// Draw selection highlight
if (totalDurationSeconds > 0 && inPointSeconds < outPointSeconds) {
    int inX = static_cast<int>((inPointSeconds / totalDurationSeconds) * width());
    int outX = static_cast<int>((outPointSeconds / totalDurationSeconds) * width());
    painter.fillRect(inX, 0, outX - inX, height(), QColor(137, 180, 250, 50));
}
}

void TimelineWidget::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton && totalDurationSeconds > 0) {
        m_isDragging = true;
        double newPosition = static_cast<double>(event->pos().x()) / width() * totalDurationSeconds;
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

        // Calculate track index based on vertical position
        int trackHeight = height() / 4; // Assuming 4 tracks
        int trackIndex = static_cast<int>(event->position().y()) / trackHeight;

        // Determine if track is audio or video
        bool isAudio = (trackIndex == 2 || trackIndex == 3); // AudioTrack1 or AudioTrack2

        // Convert horizontal position to timestamp
        double playheadTime = static_cast<double>(event->position().x()) / width() * totalDurationSeconds;

        emit clipDropped(clipId, playheadTime, isAudio, trackIndex);
        event->acceptProposedAction();
    } else if (event->mimeData()->hasUrls()) {
        // Handle file URLs if needed
        event->ignore();
    }
}
void TimelineWidget::mouseReleaseEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        m_isDragging = false;
        emit razorToolClicked(event->pos());
    }
}
