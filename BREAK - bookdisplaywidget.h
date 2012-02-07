#ifndef BOOKDISPLAYWIDGET_H
#define BOOKDISPLAYWIDGET_H

#include <QWidget>

#include "book.h"
#include "multipagelayout.h"

class BookDisplayWidget : public QWidget
{
    Q_OBJECT
public:
    explicit BookDisplayWidget(Book *book, QWidget *parent = 0);
    ~BookDisplayWidget();

    inline Book* book() const { return pBook; }

signals:

public slots:
    void saveBook();
    void saveBookAs();

private:
    Book *pBook;

    //! \brief Layout widget for displaying the pages
    MultiPageLayout *mpl;

};

#endif // BOOKDISPLAYWIDGET_H
