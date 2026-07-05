#ifndef TIMELINEWIDGET_H
#define TIMELINEWIDGET_H

#include <QWidget>
#include <QPainter>
#include "core/ProjectModel.h"

class TimelineWidget : public QWidget {
    Q_OBJECT

// Timeline duration tracking
public:
double totalDurationSeconds = 0.0;
double inPointSeconds = 0.0;
double outPointSeconds = 0.0;
void setDuration(double seconds);
double getInPointSeconds() const { return inPointSeconds; }
void setInPointSeconds(double seconds) { inPointSeconds = seconds; emit inPointChanged(); }
double getOutPointSeconds() const { return outPointSeconds; }
void setOutPointSeconds(double seconds) { outPointSeconds = seconds; emit outPointChanged(); }
double getSelectedDuration() const { return outPointSeconds - inPointSeconds; }

public:
    explicit TimelineWidget(QWidget* parent = nullptr);
    void setProject(Project* project);
    void setPosition(double seconds);
    void updateTimeline();

signals:
    void seekRequested(double seconds);
    void clipSelected(const QString& clipId);
    void playheadChanged(double seconds);
    void inPointChanged();
    void outPointChanged();

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;

private:
    Project* m_project = nullptr;
    double m_currentPosition = 0.0;
    bool m_isDragging = false;
};

#endif // TIMELINEWIDGET_H