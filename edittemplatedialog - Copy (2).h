#ifndef EDITTEMPLATEDIALOG_H
#define EDITTEMPLATEDIALOG_H

#include <QDialog>
#include <QList>

#include <QDebug>

class AbstractTemplate;
class QListWidget;
class QStackedWidget;

#include "languagetemplate.h"
#include "abstracttemplate.h"

#include <QtGui>

class EditTemplateDialogBase
{
    Q_OBJECT
public:
  EditTemplateDialogBase( QWidget* parent = 0 );
  ~EditTemplateDialogBase();
};

template <class U>
class EditTemplateDialog : public EditTemplateDialogBase
{
//    Q_OBJECT
public:
    explicit EditTemplateDialog(QList<U*> *templates, QWidget *parent = 0);

signals:

private:
    QListWidget *list;
    QStackedWidget *stack;

    void createLayout();

private slots:
    void setCurrentText(QString string);

    void add();
    void remove();

};

template <class U>
EditTemplateDialog<U>::EditTemplateDialog(QList<U*> *templates, QWidget *parent) :
    QDialog(parent)
{
    createLayout();

    for(int i=0; i < templates->count(); i++)
    {
        list->addItem( templates->at(i)->name() );
        stack->addWidget( templates->at(i)->createSettingsPage() );
        connect( templates->at(i) , SIGNAL(nameChanged(QString)) , this, SLOT(setCurrentText(QString)) );
    }

    connect( list, SIGNAL(currentRowChanged(int)), stack, SLOT(setCurrentIndex(int)) );

}

template <class U>
void EditTemplateDialog<U>::createLayout()
{
    QHBoxLayout *hlayout = new QHBoxLayout;

    QPushButton *bAdd = new QPushButton(tr("Add"));
    QPushButton *bRemove = new QPushButton(tr("Remove"));
    QHBoxLayout *buttonlayout = new QHBoxLayout;
    buttonlayout->addWidget(bAdd,0);
    buttonlayout->addWidget(bRemove,0);
    connect(bAdd,SIGNAL(clicked()),this,SLOT(add()));
    connect(bRemove,SIGNAL(clicked()),this,SLOT(remove()));

    QVBoxLayout *vlayout = new QVBoxLayout;

    list = new QListWidget;
    list->setMaximumWidth(200);
    list->setSpacing(3);

    vlayout->addWidget(list);
    vlayout->addLayout(buttonlayout);

    hlayout->addLayout(vlayout);

    stack = new QStackedWidget;
    hlayout->addWidget(stack);

    setLayout(hlayout);
}

template <class U>
void EditTemplateDialog<U>::setCurrentText(QString string)
{
    list->currentItem()->setText( string );
}

template <class U>
void EditTemplateDialog<U>::add()
{

}

template <class U>
void EditTemplateDialog<U>::remove()
{

}


#endif // LANGUAGETEMPLATEDIALOG_H
