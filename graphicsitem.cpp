#include "graphicsitem.h"

#include <QtDebug>
#include <QtGui>
#include <QFileDialog>
#include <QCoreApplication>

#include "page.h"
#include "pagetemplate.h"
#include "graphicslayoutitem.h"

GraphicsItem::GraphicsItem(Page *page, QPixmap pixmap, GraphicsLayoutItem *layout)
{
    setAcceptDrops(true);

    this->layout = layout;

    pPage = page;

    fromPixmap(pixmap);
}

GraphicsItem::GraphicsItem(Page *page, const QString& base64, GraphicsLayoutItem *layout)
{
    setAcceptDrops(true);

    this->layout = layout;

    pPage = page;

    fromBase64(base64);
}

void GraphicsItem::dropEvent ( QGraphicsSceneDragDropEvent * event )
{
    if (event->mimeData()->hasImage())
    {
	 QImage image = qvariant_cast<QImage>(event->mimeData()->imageData());
	 QPixmap pixmap = QPixmap::fromImage(image);
	 fromPixmap(pixmap);
    }
    else if( event->mimeData()->hasUrls() )
    {
	QImage image(event->mimeData()->urls().at(0).toLocalFile());
	QPixmap pixmap = QPixmap::fromImage(image);
	fromPixmap(pixmap);
    }
}

void GraphicsItem::fromPixmap(QPixmap &pixmap)
{
    setPixmap(pixmap);
    setScalePosition();

    QByteArray ba;
    QBuffer buf(&ba);
    QImageWriter writer(&buf,"PNG");
    writer.write(pixmap.toImage());
    strBase64 = "data:image/png;base64," + ba.toBase64();

    pPage->setChanged(true);
}

void GraphicsItem::setScalePosition()
{
    QPixmap pm = this->pixmap();

    qreal sc = layout->height() / pm.height(); // by default, scale to fit the height
    if( pm.width()*sc > pPage->pageTemplate()->getPrintAreaWidth(pPage->side()) ) // if it's too wide, scale to the width instead
        sc = pPage->pageTemplate()->getPrintAreaWidth(pPage->side()) / pm.width();

    qreal top = pPage->pageTemplate()->topMargin(pPage->side()) + layout->y() + 0.5*( layout->height() - pm.height()*sc );
    qreal left = pPage->pageTemplate()->leftMargin(pPage->side()) + 0.5*(pPage->pageTemplate()->getPrintAreaWidth(pPage->side()) - pm.width()*sc);

    setPos(left,top);
    setScale(sc);
}

void GraphicsItem::fromBase64(const QString& base64)
{
    strBase64 = base64;

    QByteArray decoded = QByteArray::fromBase64(base64.toAscii());
    QPixmap pixmap;
    if(!pixmap.loadFromData( decoded, "PNG" ))
        pixmap = QPixmap::fromImage(QImage(":/resources/nopicture.png"));
    setPixmap(pixmap);
    setScalePosition();
}

bool GraphicsItem::toSvg(QXmlStreamWriter &w, QString id)
{
    w.writeEmptyElement("image");

    w.writeAttribute("id",id);
    w.writeAttribute("width", QString("%1mm").arg( pixmap().width() * this->scale() ) );
    w.writeAttribute("height", QString("%1mm").arg( pixmap().height() * this->scale() ) );
    w.writeAttribute("x", QString("%1mm").arg( this->pos().x() ) );
    w.writeAttribute("y", QString("%1mm").arg( this->pos().y() ) );

    QByteArray ba;
    QBuffer buf(&ba);
    QImageWriter writer(&buf,"PNG");
    writer.write(pixmap().toImage());
    w.writeAttribute("http://www.w3.org/1999/xlink","href","data:image/png;base64," + ba.toBase64() );

    return true;
}

void GraphicsItem::mouseDoubleClickEvent ( QGraphicsSceneMouseEvent * event )
{
    QStringList formats;
    QList<QByteArray> baFormats = QImageReader::supportedImageFormats();
    for(int i=0; i<baFormats.count(); i++)
        formats << QString("*.%1").arg(QString(baFormats.at(i)));

    QString filename;
    filename = QFileDialog::getOpenFileName(0, QCoreApplication::translate("GraphicsItem","Select a file"), QString(), QCoreApplication::translate("GraphicsItem","Images (%1)").arg(formats.join(" ")) );

    if(filename.isEmpty())
        return;

    QPixmap p(filename);
    fromPixmap( p );

    pPage->setChanged(true);
}
