/*! \class PageView
    \brief The class for displaying pages, subclassed from QGraphicsView.

    This class displays formatted pages, according to the content of an associated Page object (itself a subclass of QGraphicsScene).
  */

#ifndef PAGEVIEW_H
#define PAGEVIEW_H

#include <QGraphicsView>

class Page;

class PageView : public QGraphicsView
{
    Q_OBJECT

public:

    //! \brief Construct the object, eliminating scroll bars and ensuring that the whole of the Page scene/page is visible.
    PageView(Page *scene, QWidget *parent = 0);
    ~PageView();

    //! \brief Return the zoom level of the page
    double getZoom() const;

    //! \brief Return an absurdly large size for the widget
    QSize sizeHint() const;

public slots:
    //! \brief Set the zoom variable and update the page geometry
    void setZoom(double z);

private:
    double zoom;
    Page *page;
    int w,h;

    //! \brief Set the maximum width so as to maintain the aspect ratio of the associated Page scene, and ensure that the Page scene fits within the view
    void resizeEvent( QResizeEvent * event );

    //! \brief Reimplemented to do nothing with scroll events
    void scrollContentsBy( int dx, int dy );
};

#endif // PAGEVIEW_H
