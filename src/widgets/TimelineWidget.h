#ifndef TIMELINEWIDGET_H
#define TIMELINEWIDGET_H

#include <QWidget>
#include <QScrollBar>
#include <QSlider>
#include <QToolButton>
#include <QLabel>
#include <QMouseEvent>
#include <QPaintEvent>
#include "../core/ProjectModel.h"

class TimelineWidget : public QWidget {
    Q_OBJECT

public:
    explicit TimelineWidget(QWidget* parent = nullptr);
    ~TimelineWidget() override = default;

    void setProject(Project* project);
    void updateTimeline();

signals:
    void clipSelected(const QString& clipId);
    void playheadChanged(double seconds);

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void dragEnterEvent(QDragEnterEvent* event) override;
    void dragMoveEvent(QDragMoveEvent* event) override;
    void dropEvent(QDropEvent* event) override;

private slots:
    void onZoomChanged(int value);
    void onScrollChanged(int value);
    void onToolSelectToggled(bool checked);
    void onToolRazorToggled(bool checked);

private:
    struct DragState {
        enum Mode { None, MoveClip, ResizeLeft, ResizeRight, MovePlayhead };
        Mode mode = None;
        QString activeClipId;
        int activeTrackIndex = -1;
        double dragStartPlayhead = 0.0;
        double dragStartClipIn = 0.0;
        double dragStartClipOut = 0.0;
        QPoint dragStartMousePos;
    };

    QRect getClipRect(int trackIndex, const TimelineClip& clip) const;
    double xToTime(int x) const;
    int timeToX(double t) const;
    int getTrackY(int trackIndex) const;
    int getTrackUnderMouse(const QPoint& pos) const;
    QString getClipUnderMouse(const QPoint& pos, int* trackIndexOut = nullptr) const;
    int getClipEdgeUnderMouse(const QPoint& pos, const QString& clipId) const; // -1: none, 0: left, 1: right

    void drawRuler(QPainter& painter);
    void drawTrackHeaders(QPainter& painter);
    void drawTracksAndClips(QPainter& painter);
    void drawPlayhead(QPainter& painter);

    Project* m_project = nullptr;
    
    // Tools
    enum Tool { SelectionTool, RazorTool };
    Tool m_activeTool = SelectionTool;

    // View settings
    double m_pixelsPerSecond = 20.0; // Zoom level
    int m_headerWidth = 120;
    int m_trackHeight = 50;
    int m_rulerHeight = 25;
    int m_scrollOffset = 0; // horizontal scroll in pixels

    DragState m_dragState;
    QString m_selectedClipId;

    QScrollBar* m_scrollBar;
    QSlider* m_zoomSlider;
    QToolButton* m_selectButton;
    QToolButton* m_razorButton;
};

#endif // TIMELINEWIDGET_H
