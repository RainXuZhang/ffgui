#include "TimelineWidget.h"
#include <QPainter>
#include <QPaintEvent>
#include <QTime>
#include <QMouseEvent>
#include <QScrollBar>
#include <QDebug>

TimelineWidget::TimelineWidget(QWidget* parent)
    : QAbstractScrollArea(parent)
{
    setMinimumSize(400, 200);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
}

void TimelineWidget::setProject(Project* project) {
    m_project = project;
    if (m_project) {
        m_duration = m_project->duration;
        m_playheadPos = m_project->currentPlayhead;
    }
    updateTimeline();
}

void TimelineWidget::updateTimeline() {
    updateScrollbars();
    viewport()->update();
}

void TimelineWidget::setDuration(double seconds) {
    m_duration = seconds;
    updateTimeline();
}

void TimelineWidget::setPixelsPerSecond(double pps) {
    m_pixelsPerSecond = pps;
    updateTimeline();
}

void TimelineWidget::setTrackCount(int count) {
    m_trackCount = count;
    updateTimeline();
}

void TimelineWidget::updateScrollbars() {
    // Calculate content size
    int contentWidth = static_cast<int>(m_duration * m_pixelsPerSecond);
    int contentHeight = m_rulerHeight + m_trackCount * m_trackHeight;
    
    // Set viewport size
    setMinimumSize(contentWidth, contentHeight);
}

void TimelineWidget::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event);
    QPainter painter(viewport());
    painter.setRenderHint(QPainter::Antialiasing);
    
    // Fill background
    painter.fillRect(rect(), QColor("#252525"));
    
    // Draw time ruler
    drawTimeRuler(painter);
    
    // Draw tracks
    drawTracks(painter);
    
    // Draw playhead
    int playheadX = static_cast<int>(m_playheadPos * m_pixelsPerSecond);
    painter.setPen(QPen(QColor("#ff4444"), 2));
    painter.drawLine(playheadX, 0, playheadX, height());
}

void TimelineWidget::drawTimeRuler(QPainter& painter) {
    QFont font = painter.font();
    font.setPointSize(8);
    painter.setFont(font);
    
    painter.setPen(QColor("#d2d2d2"));
    
    // Draw ruler background
    painter.fillRect(0, 0, width(), m_rulerHeight, QColor("#353535"));
    
    // Draw time markers
    int secondsPerMarker = 10;
    for (int s = 0; s <= static_cast<int>(m_duration); s += secondsPerMarker) {
        int x = static_cast<int>(s * m_pixelsPerSecond);
        painter.drawLine(x, m_rulerHeight - 10, x, m_rulerHeight);
        
        QString timeText = QTime(0, 0, 0).addSecs(s).toString("hh:mm:ss");
        painter.drawText(x + 2, 0, 50, m_rulerHeight - 10, Qt::AlignLeft, timeText);
    }
}

void TimelineWidget::drawTracks(QPainter& painter) {
    QFont font = painter.font();
    font.setPointSize(9);
    painter.setFont(font);
    
    for (int i = 0; i < m_trackCount; ++i) {
        int y = m_rulerHeight + i * m_trackHeight;
        int trackBottom = y + m_trackHeight;
        
        // Track background
        QColor bgColor = (i % 2 == 0) ? QColor("#2a2a2a") : QColor("#252525");
        painter.fillRect(0, y, width(), m_trackHeight, bgColor);
        
        // Track separator
        painter.setPen(QColor("#1a1a1a"));
        painter.drawLine(0, trackBottom, width(), trackBottom);
        
        // Track name
        QString trackName = QString("Track %1").arg(i + 1);
        if (m_project && i < m_project->tracks.size()) {
            trackName = m_project->tracks[i].name;
        }
        painter.setPen(QColor("#a0a0a0"));
        painter.drawText(4, y + 4, m_headerWidth - 8, m_trackHeight - 8, Qt::AlignLeft, trackName);
        
        // Draw clips if project is set
        if (m_project && i < m_project->tracks.size()) {
            const auto& track = m_project->tracks[i];
            for (const auto& clip : track.clips) {
                int clipX = static_cast<int>(clip.timelineIn * m_pixelsPerSecond);
                int clipWidth = static_cast<int>(clip.duration * m_pixelsPerSecond);
                
                // Clip rectangle
                QColor clipColor = track.isAudio ? QColor("#4a86e8") : QColor("#7aa74a");
                painter.fillRect(m_headerWidth + clipX, y + 4, clipWidth, m_trackHeight - 8, clipColor);
                
                // Clip border
                painter.setPen(QColor("#2a82da"));
                painter.drawRect(m_headerWidth + clipX, y + 4, clipWidth, m_trackHeight - 8);
                
                // Clip text
                painter.setPen(QColor("#ffffff"));
                QString clipText = QString("Clip %1").arg(clip.timelineIn);
                painter.drawText(m_headerWidth + clipX + 4, y + 4, clipWidth - 8, m_trackHeight - 8, Qt::AlignLeft, clipText);
            }
        }
    }
}

void TimelineWidget::resizeEvent(QResizeEvent* event) {
    QAbstractScrollArea::resizeEvent(event);
    updateScrollbars();
}

void TimelineWidget::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        m_draggingPlayhead = true;
        m_lastMouseX = event->pos().x();

        // Calculate millisecond position based on X coordinate.
        // Scale rule: 100 pixels = 1 second = 1000 ms => 1 pixel = 10 ms.
        int x = event->pos().x();
        if (x < 0) {
            x = 0;
        }
        double ms = static_cast<double>(x) * 10.0; // milliseconds

        // Emit seek request and update view.
        emit seekRequested(ms);
        update();
    }
}

void TimelineWidget::mouseMoveEvent(QMouseEvent* event) {
    if (m_draggingPlayhead) {
        m_lastMouseX = event->pos().x();

        // Calculate millisecond position based on X coordinate.
        int x = event->pos().x();
        if (x < 0) {
            x = 0;
        }
        double ms = static_cast<double>(x) * 10.0; // milliseconds

        // Emit seek request and update view.
        emit seekRequested(ms);
        update();
    }
}

void TimelineWidget::mouseReleaseEvent(QMouseEvent* event) {
    Q_UNUSED(event);
    m_draggingPlayhead = false;
}

void TimelineWidget::setPosition(double seconds) {
    m_playheadPos = seconds;
    update();
}
