#ifndef GRAPHICSITEM_H
#define GRAPHICSITEM_H

#include <QGraphicsPixmapItem>

class PageTemplate;
class GraphicsLayoutItem;
class QXmlStreamWriter;
class Page;

class GraphicsItem : public QGraphicsPixmapItem
{
public:
    GraphicsItem(Page *page, QPixmap pixmap, GraphicsLayoutItem *layout);
    GraphicsItem(Page *page, const QString& base64, GraphicsLayoutItem *layout);

    void fromPixmap(QPixmap &pixmap);
    void fromBase64(const QString& base64);

    void setScalePosition();

    bool toSvg(QXmlStreamWriter &w, QString id);

    inline const QString& base64() const { return strBase64; }

private:
    GraphicsLayoutItem *layout;
    Page *pPage;

    QString strBase64;

    void mouseDoubleClickEvent ( QGraphicsSceneMouseEvent * event );

    void dropEvent ( QGraphicsSceneDragDropEvent * event );
};

#endif // GRAPHICSITEM_H
