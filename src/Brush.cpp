#include "Brush.h"

Brush::Brush(QColor color, float size, int opacity) :
    mColor(color), mSize(size), mOpacity(opacity)
{
    mColor.setAlpha(mOpacity);
}

void Brush::setColor(const QColor &color) { mColor = color; }
void Brush::setSize(float size) { mSize = size; }
void Brush::setOpacity(int opacity) { mOpacity = opacity; }

void Brush::draw(QPainter &painter, const QPointF &pos, float angle)
{
    painter.save();
    painter.translate(pos);
    painter.rotate(angle);
    painter.setBrush(mColor);
    painter.setPen(Qt::NoPen);
    painter.drawEllipse(QPointF(0, 0), mSize, mSize);
    painter.restore();
}
