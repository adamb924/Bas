#include "multipagelayout.h"
#include "pageview.h"
#include "page.h"
#include "imagepushbutton.h"
#include "booktemplate.h"
#include "pagecontrolwidget.h"

#include <QtDebug>
#include <QList>
#include <QtGui>

MultiPageLayout::MultiPageLayout(Book *book, QWidget *parent)
{
    this->book = book;

    // Qt automatically sets the layout to be RTL or LTR, based on the application's QLocale
    // this works, unless a person working in an LTR interface is working on an RTL-text book
    // or unless a person working in an RTL interface is working on an LTR-text book
    // in either of those cases we need to switch the layout interface back, manually
    bSwapDirection = false;
    if( (parent->layoutDirection() == Qt::LeftToRight && book->textDirection() == LanguageTemplate::RTL) ||
        (parent->layoutDirection() == Qt::RightToLeft && book->textDirection() == LanguageTemplate::LTR) )
        bSwapDirection = true;

    nFirstPage = 0;

    if( book->textDirection() == LanguageTemplate::LTR )
    {
        backward = new ImagePushButton(new QImage(":/resources/left-active.png"), new QImage(":/resources/left-pressed.png"), new QImage(":/resources/left-inactive.png"));
        forward = new ImagePushButton(new QImage(":/resources/right-active.png"), new QImage(":/resources/right-pressed.png"), new QImage(":/resources/right-inactive.png"));
    }
    else
    {
        forward = new ImagePushButton(new QImage(":/resources/left-active.png"), new QImage(":/resources/left-pressed.png"), new QImage(":/resources/left-inactive.png"));
        backward = new ImagePushButton(new QImage(":/resources/right-active.png"), new QImage(":/resources/right-pressed.png"), new QImage(":/resources/right-inactive.png"));
    }

    if( bSwapDirection )
    {
        addWidget(backward,1,4,1,1,Qt::AlignRight);
        addWidget(forward,1,1,1,1,Qt::AlignLeft);
    }
    else
    {
        addWidget(backward,1,1,1,1,Qt::AlignRight);
        addWidget(forward,1,4,1,1,Qt::AlignLeft);
    }

    backward->setToolTip(tr("Backward"));
    forward->setToolTip(tr("Forward"));

    connect(this,SIGNAL(backwardEnabled(bool)),backward,SLOT(setEnabled(bool)));
    connect(this,SIGNAL(forwardEnabled(bool)),forward,SLOT(setEnabled(bool)));

    connect(backward,SIGNAL(clicked()),this,SLOT(goBackward()));
    connect(forward,SIGNAL(clicked()),this,SLOT(goForward()));

    if(book->pages()->count() > 0)
    {
        setPages(0,0);
    }
    else
    {
	backward->setEnabled(false);
	forward->setEnabled(false);
    }
}

void MultiPageLayout::updatePages()
{
    setPages(nFirstPage,qMin(nFirstPage+1,book->pages()->count()-1));
}

void MultiPageLayout::setPages(int from, int to)
{
    if( book->pages()->count() == 0 ||  from < 0 || to >= book->pages()->count() )
	return;

    if( from == 0 )
        to = 0;

//    qDebug() << "MultiPageLayout::setPages" << from << to;


    nFirstPage = from;

    if(from == 0)
	emit backwardEnabled(false);
    if(to == book->pages()->count()-2)
	emit forwardEnabled(false);

    // remove and delete any widgets that might be on the layout already
    removeAndDeleteWidget(1,2);
    removeAndDeleteWidget(1,3);
    removeAndDeleteWidget(2,2);
    removeAndDeleteWidget(2,3);

    if(from==to)
    {
	addPage( book->pages()->at(from), from, SinglePage );

    }
    else
    {
        if(bSwapDirection)
        {
            addPage( book->pages()->at(from), from, DoublePageSecond );
            addPage( book->pages()->at(to), to, DoublePageFirst );
        }
        else
        {
            addPage( book->pages()->at(from), from, DoublePageFirst );
            addPage( book->pages()->at(to), to, DoublePageSecond );
        }
    }

    if( from == 0 )
	emit backwardEnabled(false);
    else
	emit backwardEnabled(true);

    if( to == book->pages()->count()-1)
	emit forwardEnabled(false);
    else
	emit forwardEnabled(true);
}

void MultiPageLayout::setZoom(double zoom)
{
    if(this->zoom != zoom )
    {
	this->zoom = zoom;
	emit zoomChanged(zoom);
    }
}

double MultiPageLayout::getZoom()
{
    return zoom;
}

void MultiPageLayout::goForward()
{
/*
    if(nFirstPage == secondPage)
    {
//	secondPage = firstPage + 2;
//	firstPage = firstPage + 1;
        secondPage = nFirstPage + 1;
   }
    else
    {
	nFirstPage = nFirstPage + 2;
	secondPage = secondPage + 2;
    }

    nFirstPage = qMin(nFirstPage,book->pages()->count()-1);
    secondPage = qMin(secondPage,book->pages()->count()-1);
*/
    if(nFirstPage == 0)
    {
        nFirstPage++;
    }
    else
    {
        nFirstPage += 2;
    }
//    qDebug() << "MultiPageLayout::goForward" << nFirstPage << qMin(nFirstPage+1,book->pages()->count()-1);
    setPages(nFirstPage,qMin(nFirstPage+1,book->pages()->count()-1));
}

void MultiPageLayout::goBackward()
{
/*
    if(nFirstPage == secondPage)
    {
	secondPage = nFirstPage - 1;
	nFirstPage = secondPage - 1;
    }
    else
    {
	nFirstPage = nFirstPage - 2;
	secondPage = secondPage - 2;
    }

    nFirstPage = qMax(nFirstPage,0);
    secondPage = qMax(secondPage,0);
*/
    nFirstPage = qMax(nFirstPage-2,0);
//    qDebug() << "goBackward" << nFirstPage << qMin(nFirstPage+1,book->pages()->count()-1 );
    setPages(nFirstPage, qMin(nFirstPage+1,book->pages()->count()-1 ) );
}

void MultiPageLayout::addPageAfterFirst()
{
    // in this case the second page should be new, so it will be refreshed
    if( book->addPage(nFirstPage) )
    {
        if( nFirstPage == 0 )
            goForward();
        else
            setPages(nFirstPage,nFirstPage+1);
    }
}

void MultiPageLayout::addPageAfterSecond()
{
    // in this case the page after the second page is new, so move forward to view that page
    if( book->addPage(nFirstPage+1) )
	goForward();
}

void MultiPageLayout::addPageAfterFirst(LayoutTemplate *lt)
{
    // in this case the second page should be new, so it will be refreshed
//    qDebug() << "MultiPageLayout::addPageAfterFirst(LayoutTemplate *lt)";
    if( book->addPage(nFirstPage,lt) )
    {
        if( nFirstPage == 0 )
            goForward();
        else
            setPages(nFirstPage,nFirstPage+1);
    }
}

void MultiPageLayout::addPageAfterSecond(LayoutTemplate *lt)
{
    // in this case the page after the second page is new, so move forward to view that page
    qDebug() << "MultiPageLayout::addPageAfterSecond(LayoutTemplate *lt)";
    if( book->addPage(nFirstPage+1,lt) )
        goForward();
}

void MultiPageLayout::removeFirstPage()
{
    if( QMessageBox::question(this->parentWidget(), tr("Bas"), tr("Are you sure you want to delete this page?\n\nYou cannot undo this."),QMessageBox::Yes | QMessageBox::No, QMessageBox::No) == QMessageBox::Yes )
        if( book->removePage(nFirstPage) )
        {
            qDebug() << "MultiPageLayout::removeFirstPage()";
            setPages( qMin(nFirstPage, book->pages()->count()-1), qMin(nFirstPage + 1, book->pages()->count()-1) );
        }
}

void MultiPageLayout::removeSecondPage()
{
    if( QMessageBox::question(this->parentWidget(), tr("Bas"), tr("Are you sure you want to delete this page?\n\nYou cannot undo this."),QMessageBox::Yes | QMessageBox::No, QMessageBox::No) == QMessageBox::Yes )
        if( book->removePage( nFirstPage + 1 ) )
        {
            qDebug() << "MultiPageLayout::removeSecondPage()";
            setPages( qMin(nFirstPage, book->pages()->count()-1), qMin(nFirstPage+1, book->pages()->count()-1) );
        }
}


void MultiPageLayout::removeAndDeleteWidget(int row, int col)
{
    if(this->itemAtPosition(row,col) != 0)
    {
	QLayoutItem *w = this->itemAtPosition(row,col);
	QWidget *wi = w->widget();
	this->removeWidget(wi);
	this->removeItem(w);
	delete wi;
    }
}

void MultiPageLayout::addPage( Page * page, int pageIndex, PagePosition position )
{
    PageView *view = new PageView(page);
    view->setSceneRect(0,0,page->width(),page->height());

    int fromCol, columnSpan;
    switch(position)
    {
    case SinglePage:
	fromCol = 2;
	columnSpan = 2;
	break;
    case DoublePageFirst:
	fromCol = 2;
	columnSpan = 1;
	break;
    case DoublePageSecond:
	fromCol = 3;
	columnSpan = 1;
	break;
    }

    view->setZoom(zoom);
    connect(this,SIGNAL(zoomChanged(double)),view,SLOT(setZoom(double)));
    addWidget( view , 1, fromCol, 1, columnSpan, Qt::AlignCenter );

    PageControlWidget *control = new PageControlWidget(book, page);
    addWidget(control,2,fromCol,1,columnSpan, Qt::AlignCenter);


    switch(position)
    {
    case SinglePage:
    case DoublePageFirst:
        if(bSwapDirection)
        {
            connect(control,SIGNAL(addPage()),this,SLOT(addPageAfterSecond()));
            connect(control,SIGNAL(removePage()),this,SLOT(removeSecondPage()));
            connect(control,SIGNAL(addPageFromTemplate(LayoutTemplate*)),this,SLOT(addPageAfterSecond(LayoutTemplate*)));
        }
        else
        {
            connect(control,SIGNAL(addPage()),this,SLOT(addPageAfterFirst()));
            connect(control,SIGNAL(removePage()),this,SLOT(removeFirstPage()));
            connect(control,SIGNAL(addPageFromTemplate(LayoutTemplate*)),this,SLOT(addPageAfterFirst(LayoutTemplate*)));
        }
	break;
    case DoublePageSecond:
        if(bSwapDirection)
        {
            connect(control,SIGNAL(addPage()),this,SLOT(addPageAfterFirst()));
            connect(control,SIGNAL(removePage()),this,SLOT(removeFirstPage()));
            connect(control,SIGNAL(addPageFromTemplate(LayoutTemplate*)),this,SLOT(addPageAfterFirst(LayoutTemplate*)));
        }
        else
        {
            connect(control,SIGNAL(addPage()),this,SLOT(addPageAfterSecond()));
            connect(control,SIGNAL(removePage()),this,SLOT(removeSecondPage()));
            connect(control,SIGNAL(addPageFromTemplate(LayoutTemplate*)),this,SLOT(addPageAfterSecond(LayoutTemplate*)));
        }
        break;
    }

    if( book->templateMode() == Book::ConformityMode)
    {
        control->setCanAddPage(book->bookTemplate()->canAddPageAfter(pageIndex,book->pages()->count()));
        control->setCanRemovePage(book->bookTemplate()->canRemovePageAt(pageIndex,book->pages()->count()));
    }
    else
    {
        control->setCanAddPage(true);
        control->setCanRemovePage(true);
    }
}
