/*! \class MultiPageLayout
    \brief Provides the main workspace of the application, displaying pages and buttons

    This class handles the display of page displays, as well as appropriate navigation buttons.
  */

#ifndef MULTIPAGELAYOUT_H
#define MULTIPAGELAYOUT_H

#include <QGridLayout>

#include "book.h"

class Page;
class PageView;
class ImagePushButton;
class QPushButton;

class MultiPageLayout : public QGridLayout
{
    Q_OBJECT
public:
    enum PagePosition { SinglePage, DoublePageFirst, DoublePageSecond };

    MultiPageLayout(Book *book, QWidget *parent);

    void setPages(int from, int to);

    void setZoom(double zoom);
    double getZoom();

public slots:
    void updatePages();

signals:
    void zoomChanged(double zoom);
    void forwardEnabled(bool enabled);
    void backwardEnabled(bool enabled);

private slots:
    void goForward();
    void goBackward();
    void addPageAfterFirst();
    void addPageAfterSecond();
    void addPageAfterFirst(LayoutTemplate *lt);
    void addPageAfterSecond(LayoutTemplate *lt);

    void removeFirstPage();
    void removeSecondPage();

    void removeAndDeleteWidget(int row, int col);

    void addPage( Page * page, int pageIndex, PagePosition position );

private:
    Book *book;
    double zoom;
    int nFirstPage;

    bool bSwapDirection;

    ImagePushButton *backward;
    ImagePushButton *forward;

    QPushButton *firstAddPage, *secondAddPage;
};

#endif // MULTIPAGELAYOUT_H
