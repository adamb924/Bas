#include "pagecontrolwidget.h"

#include <QtGui>
#include "book.h"
#include "page.h"
#include "layouttemplate.h"
#include "mainwindow.h"
#include "pagetemplate.h"
#include "booktemplate.h"

PageControlWidget::PageControlWidget(Book *book, Page *page, QWidget *parent) :
    QWidget(parent)
{
    pBook = book;
    pPage = page;

    layout = new QHBoxLayout;

    bAddPage = new QPushButton(tr("Add a page after this page"));
    bRemovePage = new QPushButton(tr("Remove this page"));
    bAdvancedOptions = new QPushButton(tr("Advanced"));
    bAdvancedOptions->setMenu(new QMenu);

    layout->addWidget(bAddPage);
    layout->addWidget(bRemovePage);
    if( book->mainWindow()->showAdvancedPageOptions() )
    {
        layout->addWidget(bAdvancedOptions);
        bAdvancedOptions->menu()->addAction(tr("About this page"),this,SLOT(aboutThisPage()));
        bAdvancedOptions->menu()->addMenu( templateMenu() );
    }

    if( book->templateMode() == Book::BreakoutMode )
    {
        QMenu *ltMenu = new QMenu;
        QList<LayoutTemplate*> lt = book->mainWindow()->getLayoutTemplatesForPageTemplate(book->pageTemplate());
        QActionGroup *ltGroup = new QActionGroup(ltMenu);
        for(int i=0; i < lt.count(); i++ )
        {
            QAction *tmp = new QAction(lt.at(i)->longName(),this);
            tmp->setData( lt.at(i)->name() );
            ltMenu->addAction(tmp);
            ltGroup->addAction(tmp);
        }
        connect(ltGroup,SIGNAL(triggered(QAction*)),this,SLOT(addPageFromAction(QAction*)));
        bAddPage->setMenu(ltMenu);
    }
    else
    {
        connect(bAddPage,SIGNAL(clicked()),this,SIGNAL(addPage()));
    }

    connect(bRemovePage,SIGNAL(clicked()),this,SIGNAL(removePage()));

    this->setLayout(layout);
}

PageControlWidget::~PageControlWidget()
{
    delete layout;
}

QMenu* PageControlWidget::templateMenu()
{
    QMenu *templateMenu = new QMenu(tr("Change template"));

    QList<LayoutTemplate*> templates = pBook->mainWindow()->getCompatibleLayoutTemplates(pPage->layoutTemplate());

//    if( templates.count() < 2 )
//        return templateMenu;

    QActionGroup *group = new QActionGroup(templateMenu);
    group->setExclusive(true);
    for(int i=0; i<templates.count(); i++)
    {
        QAction *tmp = new QAction( templates.at(i)->longName(), templateMenu );
        tmp->setData( templates.at(i)->name() );
        tmp->setCheckable(true);
        if( pPage->layoutTemplate() == templates.at(i) )
            tmp->setChecked(true);
        group->addAction(tmp);
        templateMenu->addAction(tmp);
    }
    connect(group,SIGNAL(triggered(QAction*)),this,SLOT(pageTemplateChanged(QAction*)));

    if(templateMenu->actions().count() == 0)
        templateMenu->setEnabled(false);

    return templateMenu;
}

void PageControlWidget::setCanAddPage(bool possible)
{
    bAddPage->setVisible(possible);
}

void PageControlWidget::setCanRemovePage(bool possible)
{
    bRemovePage->setVisible(possible);
}

void PageControlWidget::pageTemplateChanged(QAction *action)
{
    QString name = action->data().toString();
    LayoutTemplate *tmpl = pBook->mainWindow()->layoutTemplateFromName(name,pPage->layoutTemplate()->pageTemplate()->name());
    if(tmpl != 0)
        pPage->setLayoutTemplate(tmpl);
}

void PageControlWidget::aboutThisPage()
{
    if(pPage == 0)
        return;

    QGridLayout *grdLayout = new QGridLayout;
    if( pPage->book()->templateMode() == Book::ConformityMode)
    {
        grdLayout->addWidget(new QLabel(tr("Book template:")),7,1,Qt::AlignLeft);
        grdLayout->addWidget(new QLabel(pPage->book()->bookTemplate()->name()),7,2,Qt::AlignLeft);
        grdLayout->setRowMinimumHeight(6,10);
    }
    grdLayout->addWidget(new QLabel(tr("Language:")),5,1,Qt::AlignLeft);
    grdLayout->addWidget(new QLabel(pPage->book()->languageTemplate()->name()),5,2,Qt::AlignLeft);
    grdLayout->setRowMinimumHeight(4,10);
    grdLayout->addWidget(new QLabel(tr("Page template:")),3,1,Qt::AlignLeft);
    grdLayout->addWidget(new QLabel(pPage->pageTemplate()->name()),3,2,Qt::AlignLeft);
    grdLayout->setRowMinimumHeight(2,10);
    grdLayout->addWidget(new QLabel(tr("Layout template:")),1,1,Qt::AlignLeft);
    grdLayout->addWidget(new QLabel(pPage->layoutTemplate()->name()),1,2,Qt::AlignLeft);

    QDialog dlg;
    dlg.setLayout(grdLayout);
    dlg.exec();
}

void PageControlWidget::addPageFromAction(QAction *action)
{
    emit addPageFromTemplate( pBook->mainWindow()->layoutTemplateFromName( action->data().toString() , pBook->pageTemplate()->name() ) );
}
