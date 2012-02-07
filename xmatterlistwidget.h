#ifndef XMATTERLISTWIDGET_H
#define XMATTERLISTWIDGET_H

#include <QListWidget>
#include <QList>

class LayoutTemplate;
class BookTemplate;

#include "booktemplate.h"

class XMatterListWidget : public QListWidget
{
    Q_OBJECT
public:
    explicit XMatterListWidget(BookTemplate::Position type, QList<LayoutTemplate*>* templates, BookTemplate *bt, QWidget *parent = 0);

signals:
    void fillerAdded();

public slots:
    void removeFiller();

protected:
    void contextMenuEvent( QContextMenuEvent * event );
    void dropEvent ( QDropEvent * event );

    void updateTemplates();

private:
    BookTemplate::Position eType;
    QList<LayoutTemplate*>* aTemplates;
    BookTemplate *pBookTemplate;

private slots:
    void removeCurrent();
};

#endif // XMATTERLISTWIDGET_H
