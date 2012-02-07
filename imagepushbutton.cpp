#include "imagepushbutton.h"

#include <QtGui>
#include <QtDebug>

ImagePushButton::ImagePushButton(QImage *active, QImage *pressed, QImage *inactive, QWidget *parent) :
    QAbstractButton(parent)
{
    imgActive = *active;
    imgInactive = *inactive;
    imgPressed = *pressed;

    this->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);
}

ImagePushButton::~ImagePushButton()
{
}

void ImagePushButton::setImages(QImage *active, QImage *pressed, QImage *inactive)
{
    imgActive = *active;
    imgInactive = *inactive;
    imgPressed = *pressed;
}

void ImagePushButton::paintEvent ( QPaintEvent * e )
{
    QPainter painter(this);

    if( this->isEnabled() )
	if( this->isDown() )
	    painter.drawImage(0,0,imgPressed);
	else
	    painter.drawImage(0,0,imgActive);
    else
	painter.drawImage(0,0,imgInactive);
}

QSize ImagePushButton::sizeHint() const
{
    return imgActive.size();
}
