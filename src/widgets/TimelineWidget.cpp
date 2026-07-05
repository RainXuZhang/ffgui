#include "TimelineWidget.h"
#include <QPainter>
#include <QMouseEvent>

TimelineWidget::TimelineWidget(QWidget* parent)
    : QWidget(parent)
{
    setMinimumHeight(100);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
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
    painter.fillRect(rect(), QColor(40, 40, 40));

    // Draw playhead
    if (totalDurationSeconds > 0) {
        int playheadX = static_cast<int>((m_currentPosition / totalDurationSeconds) * width());
        painter.setPen(QPen(Qt::red, 2));
        painter.drawLine(playheadX, 0, playheadX, height());
    }

    // Draw time markers
    painter.setPen(QPen(Qt::white));
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
        painter.fillRect(inX, 0, outX - inX, height(), QColor(0, 150, 255, 50));
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