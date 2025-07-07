#pragma once
#include <QPainter>
#include <QColor>
#include <QPointF>

class Brush
{
public:
    Brush(QColor color, float size, int opacity);
    void setColor(const QColor& color);
    void setSize(float size);
    void setOpacity(int opacity);
    void draw(QPainter& painter, const QPointF& pos, float angle = 0.0f);

private:
    QColor mColor;
    float mSize;
    int mOpacity;
};