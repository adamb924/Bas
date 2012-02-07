#include "xmatterlistwidget.h"

#include "layouttemplate.h"
#include "mainwindow.h"
#include "booktemplate.h"
#include "pagetemplate.h"

#include <QtGui>
#include <QList>

XMatterListWidget::XMatterListWidget(BookTemplate::Position type, QList<LayoutTemplate*>* templates, BookTemplate *bt, QWidget *parent) :
    QListWidget(parent)
{
    pBookTemplate = bt;

    eType = type;
    aTemplates = templates;
    setSpacing(3);
    setDragDropMode(QAbstractItemView::DragDrop);
    setSelectionMode(QAbstractItemView::ExtendedSelection);

    setDefaultDropAction(Qt::MoveAction);

    for(int i=0; i < aTemplates->count(); i++ )
        addItem( aTemplates->at(i)->name() );
}

void XMatterListWidget::contextMenuEvent ( QContextMenuEvent * event )
{
    QMenu menu(this);
    menu.addAction(tr("Remove"),this,SLOT(removeCurrent()));
    menu.exec(event->globalPos());
}

void XMatterListWidget::removeCurrent()
{
    for(int i=0; i < selectedItems().count(); i++)
    {
        QListWidgetItem *tmp = selectedItems().at(i);
        aTemplates->removeAt( this->row(tmp) );
        delete tmp;
    }
    updateTemplates();
}

void XMatterListWidget::dropEvent ( QDropEvent * event )
{
    QListWidget::dropEvent(event);

    if( eType == BookTemplate::MiddleMatter || findItems(tr("[[Filler Pages]]"),Qt::MatchExactly).count() > 1 )
        removeFiller();
    else if(findItems(tr("[[Filler Pages]]"),Qt::MatchExactly).count())
        emit fillerAdded();
    updateTemplates();
}

void XMatterListWidget::removeFiller()
{
    if( findItems(tr("[[Filler Pages]]"),Qt::MatchExactly).count() )
    {
        qDeleteAll(findItems(tr("[[Filler Pages]]"),Qt::MatchExactly));
        updateTemplates();
    }
}

void XMatterListWidget::updateTemplates()
{
//    qDebug() << "XMatterListWidget::updateTemplates()";

    aTemplates->clear();
    for(int i=0; i<count(); i++)
    {
        if( item(i)->text() != tr("[[Filler Pages]]") )
            aTemplates->append( pBookTemplate->mainWindow()->layoutTemplateFromName( item(i)->text() , pBookTemplate->pageTemplate()->name() ) );
        else
            pBookTemplate->setFillerPagesBefore(eType,i);
    }
}
