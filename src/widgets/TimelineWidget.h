#ifndef TIMELINEWIDGET_H
#define TIMELINEWIDGET_H

#include <QAbstractScrollArea>
#include <QPaintEvent>
#include <QPainter>
#include <QMouseEvent>
#include <QResizeEvent>
#include "../core/ProjectModel.h"

class TimelineWidget : public QAbstractScrollArea
{
    Q_OBJECT

public:
    explicit TimelineWidget(QWidget* parent = nullptr);
    ~TimelineWidget() override = default;

    // Public API for timeline configuration
    void setDuration(double seconds);
    void setPixelsPerSecond(double pps);
    void setTrackCount(int count);
    
    // Project integration
    void setProject(Project* project);
    void updateTimeline();

signals:
    void clipSelected(const QString& clipId);
    void playheadChanged(double seconds);

protected:
    void paintEvent(QPaintEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;

private:
    void drawTimeRuler(QPainter& painter);
    void drawTracks(QPainter& painter);
    void updateScrollbars();

    // Timeline configuration
    double m_duration = 300.0;           // Total timeline duration in seconds (default 5 minutes)
    double m_pixelsPerSecond = 50.0;     // Zoom scale: logical pixels per second
    int m_trackCount = 3;                // Number of tracks to display
    int m_rulerHeight = 30;              // Height of the time ruler area
    int m_trackHeight = 60;              // Height of each track
    int m_headerWidth = 120;             // Width of track header area (left side)
    
    // Project data
    Project* m_project = nullptr;
    
    // Playhead position
    double m_playheadPos = 0.0;
    
    // Mouse interaction
    bool m_draggingPlayhead = false;
    int m_lastMouseX = 0;
};

#endif // TIMELINEWIDGET_H