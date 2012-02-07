#ifndef GRAPHICSLAYOUTITEM_H
#define GRAPHICSLAYOUTITEM_H

#include <QObject>

class GraphicsLayoutItem
{
public:
    GraphicsLayoutItem(qreal vp, qreal h, qreal w);

    inline void setY(qreal y) { verticalPosition = y; }
    inline void setWidth(qreal width) { imgwidth = width; }
    inline void setHeight(qreal height) { imgheight = height; }

    inline qreal y() const { return verticalPosition; }
    inline qreal width() const { return imgwidth; }
    inline qreal height() const { return imgheight; }

private:
    qreal verticalPosition;
    qreal imgwidth;
    qreal imgheight;
};

#endif // GRAPHICSLAYOUTITEM_H
