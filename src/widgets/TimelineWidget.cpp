#include "TimelineWidget.h"
#include <QPainter>
#include <QPaintEvent>
#include <QFontMetrics>

TimelineWidget::TimelineWidget(QWidget* parent)
    : QAbstractScrollArea(parent)
{
    // Set up sensible default size hint and minimum dimensions
    setMinimumSize(400, 200);
    
    // Configure scroll bars for the timeline
    horizontalScrollBar()->setRange(0, 1000);
    horizontalScrollBar()->setValue(0);
    
    // Accept drops for future functionality
    setAcceptDrops(true);
}

void TimelineWidget::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event);
    
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    
    // Clear viewport with dark gray/charcoal background for modern video editing suite
    painter.fillRect(rect(), QColor(45, 45, 45));
    
    // Draw the time ruler at the top
    drawTimeRuler(painter);
    
    // Draw the three track partitions below the ruler
    drawTracks(painter);
}

void TimelineWidget::drawTimeRuler(QPainter& painter)
{
    const int rulerHeight = m_rulerHeight;
    
    // Draw ruler background - dark gray for timeline ruler
    painter.fillRect(0, 0, width(), rulerHeight, QColor(55, 55, 55));
    
    // Set up fonts and colors for time labels
    QFont timeFont("Monospace", 8);
    QFontMetrics timeMetrics(timeFont);
    painter.setFont(timeFont);
    painter.setPen(QColor(230, 230, 230));
    
    // Calculate timeline display scale
    double pixelsPerSecond = m_pixelsPerSecond;
    double durationSeconds = m_duration;
    
    if (pixelsPerSecond <= 0) return;
    
    // Calculate tick marks and time labels
    const int secondsPerTick = 1; // 1-second ticks
    const double pixelStep = static_cast<double>(secondsPerTick) * pixelsPerSecond;
    
    int ticksToDraw = static_cast<int>(durationSeconds / secondsPerTick) + 1;
    
    for (int i = 0; i <= ticksToDraw; ++i)
    {
        double timeSeconds = i * secondsPerTick;
        int xPos = static_cast<int>(i * pixelStep);
        
        // Draw minor tick mark
        painter.setPen(QColor(180, 180, 180));
        painter.drawLine(xPos, rulerHeight - 5, xPos, rulerHeight);
        
        // Draw time label
        painter.setPen(QColor(220, 220, 220));
        QString timeText = QString("%1:%2").arg(
            static_cast<int>(timeSeconds) / 60, 2, 10, QChar('0')).arg(
            static_cast<int>(timeSeconds) % 60, 2, 10, QChar('0'));
        
        int textWidth = timeMetrics.horizontalAdvance(timeText);
        painter.drawText(xPos - textWidth / 2, 12, timeText);
    }
}

void TimelineWidget::drawTracks(QPainter& painter)
{
    const int rulerHeight = m_rulerHeight;
    
    for (int track = 0; track < m_trackCount; ++track)
    {
        int trackY = rulerHeight + track * m_trackHeight;
        
        // Draw track background - slightly varying colors for visual distinction
        QColor trackColor;
        if (track == 0) trackColor = QColor(40, 40, 40);      // Video track (dark blue-ish)
        else if (track == 1) trackColor = QColor(38, 38, 38);  // Video track (another shade)
        else trackColor = QColor(36, 36, 36);                    // Audio track (dark green-ish)
        
        painter.fillRect(0, trackY, width(), m_trackHeight, trackColor);
        
        // Draw track border separator
        painter.setPen(QPen(QColor(70, 70, 70), 1));
        painter.drawLine(0, trackY + m_trackHeight, width(), trackY + m_trackHeight);
    }
    
    // Draw vertical separator between track header area and track content
    painter.setPen(QPen(QColor(60, 60, 60), 2));
    painter.drawLine(m_headerWidth, 0, m_headerWidth, height());
    
    // Draw track header labels on the left side
    painter.setFont(QFont("Arial", 9, QFont::Bold));
    painter.setPen(QColor(220, 220, 220));
    
    for (int track = 0; track < m_trackCount; ++track)
    {
        int trackY = rulerHeight + track * m_trackHeight + 15;
        QString trackTitle;
        
        switch (track)
        {
            case 0: trackTitle = "Video 1"; break;
            case 1: trackTitle = "Video 2"; break;
            case 2: trackTitle = "Audio 1"; break;
            default: trackTitle = "Track"; break;
        }
        
        painter.drawText(10, trackY, trackTitle);
    }
}