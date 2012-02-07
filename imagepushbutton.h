/*! \class ImagePushButton
    \brief A class that provides an image button

    This class provides a button with different images for active, depressed, and inactive states
  */

#ifndef IMAGEPUSHBUTTON_H
#define IMAGEPUSHBUTTON_H

#include <QAbstractButton>

class QImage;

class ImagePushButton : public QAbstractButton
{
    Q_OBJECT
public:
    //! \brief Construct the button, storing deep copies of the three QImage parameters
    explicit ImagePushButton(QImage *active, QImage *pressed, QImage *inactive, QWidget *parent = 0);

    //! \brief Currently does nothing
    ~ImagePushButton();

    //! \brief Return as the suggested the size of the active button
    QSize sizeHint() const;

    //! \brief Display the proper image for the state of the button
    void paintEvent ( QPaintEvent * e );

    //! \brief Set the images for the button states
    void setImages(QImage *active, QImage *pressed, QImage *inactive);

private:
    QImage imgActive, imgInactive, imgPressed;

signals:

public slots:

};

#endif // IMAGEPUSHBUTTON_H
