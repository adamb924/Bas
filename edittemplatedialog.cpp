#include "edittemplatedialog.h"

#include "languagetemplate.h"
#include "abstracttemplate.h"

#include <QtGui>

EditTemplateDialog::EditTemplateDialog(QList<AbstractTemplate*> *templates, NewTemplateMethod nt, MainWindow *parent) :
    QDialog(parent)
{
    pMW = parent;

    aTemplates = templates;

    newTemplate = nt;

    createLayout();

    for(int i=0; i < templates->count(); i++)
    {
        list->addItem( templates->at(i)->name() );
        stack->addWidget( templates->at(i)->createSettingsPage() );
        connect( templates->at(i) , SIGNAL(nameChanged(QString)) , this, SLOT(setCurrentText(QString)) );
    }

    connect( list, SIGNAL(currentRowChanged(int)), stack, SLOT(setCurrentIndex(int)) );

}

void EditTemplateDialog::createLayout()
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

void EditTemplateDialog::setCurrentText(QString string)
{
    list->currentItem()->setText( string );
}

void EditTemplateDialog::add()
{
    aTemplates->append( (pMW->*newTemplate)() );
    list->addItem( aTemplates->last()->name() );
    stack->addWidget( aTemplates->last()->createSettingsPage() );
    connect( aTemplates->last() , SIGNAL(nameChanged(QString)) , this, SLOT(setCurrentText(QString)) );

    list->setCurrentItem( list->item(list->count()-1) );
}

void EditTemplateDialog::remove()
{
    QList<QListWidgetItem*> selection = list->selectedItems();
    if(selection.count() < 1)
        return;
    int index = list->row(selection.at(0));

    delete aTemplates->takeAt(index);
    delete selection.at(0);
}

void EditTemplateDialog::redoCurrentWidget()
{
    qDebug() << "bam";
    int index = stack->currentIndex();
    qDebug() << "bam2";
    delete stack->widget(index);
    qDebug() << "bam3";
    stack->insertWidget(index, aTemplates->at(index)->createSettingsPage() );
    qDebug() << "bam4";
}
