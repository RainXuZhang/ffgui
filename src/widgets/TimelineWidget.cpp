#include "TimelineWidget.h"
#include <QPainter>
#include <QPaintEvent>
#include <QTime>

TimelineWidget::TimelineWidget(QWidget* parent)
    : QAbstractScrollArea(parent)
{
    setMinimumSize(400, 200);
}

void TimelineWidget::paintEvent(QPaintEvent* event)
{
}

void TimelineWidget::drawTimeRuler(QPainter& painter)
{
}
