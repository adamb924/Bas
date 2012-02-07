#include "pageview.h"
#include "page.h"

#include <QtDebug>
#include <QtGui>
#include <QWidget>

PageView::PageView(Page *scene, QWidget *parent) : QGraphicsView(scene,parent)
{
    zoom = 1.0f;
    page = scene;

    // since we will be keeping everything within the scene, disable the scroll bars
    this->setVerticalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
    this->setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOff );

    // the horizontal dimension will be set in resizeEvent, the vertical dimension should be as large as possible
    this->setSizePolicy(QSizePolicy(QSizePolicy::Fixed,QSizePolicy::Expanding));
}

PageView::~PageView()
{
}

double PageView::getZoom() const
{
    return zoom;
}

void PageView::setZoom(double z)
{
    zoom = z;
    this->updateGeometry();
}

QSize PageView::sizeHint() const
{
    return QSize(1000,1000);
}

void PageView::resizeEvent ( QResizeEvent * event )
{
    this->setMaximumWidth( page->aspectRatio() * this->height() );
    this->fitInView(page->sceneRect(), Qt::KeepAspectRatio);
}

void PageView::scrollContentsBy( int dx, int dy )
{
    return;
}
