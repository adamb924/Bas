#include "newbookwidget.h"

#include "book.h"
#include "booktemplate.h"
#include "languagetemplate.h"
#include "pagetemplate.h"
#include "mainwindow.h"

#include <QtDebug>
#include <QtGui>
#include <QList>

NewBookWidget::NewBookWidget(bool fromTemplate, MainWindow *parent) :
        QWidget(parent)
{
    pMW = parent;
    bFromTemplate = fromTemplate;

    setStyleSheet("font-size: 14pt;  "
                  "QFrame { margin: 40pt; color: red; border: 2px solid green; }"
                  "QPushButton { font-size: 12pt; }"
                  "QListView { font-size: 16pt; border-radius: 5px; }"
                  );

    QVBoxLayout *layout = new QVBoxLayout;

    if(bFromTemplate)
    {
        bookTemplatesModel = new QStandardItemModel;
        QModelIndex first;
        for(int i = 0; i < parent->bookTemplates()->count(); i++)
        {
            QStandardItem *item = new QStandardItem( parent->bookTemplates()->at(i)->longName() );
            item->setData(i);
            bookTemplatesModel->appendRow( item );
            if(i == 0)
                first = bookTemplatesModel->indexFromItem(item);
        }
        bookTemplatesView = new QListView;
        bookTemplatesView->setSpacing(3);
        bookTemplatesView->setModel(bookTemplatesModel);
        bookTemplatesView->setWordWrap(true);
        bookTemplatesView->setSelectionMode(QAbstractItemView::SingleSelection);
        bookTemplatesView->selectionModel()->select( first , QItemSelectionModel::Select );

        layout->addWidget(new QLabel(tr("Select what kind of book you want to create...")));
        layout->addWidget(bookTemplatesView);
    }
    else
    {
        pageTemplatesModel = new QStandardItemModel;
        QModelIndex first;

        for(int i = 0; i < parent->pageTemplates()->count(); i++)
        {
            QStandardItem *item = new QStandardItem( parent->pageTemplates()->at(i)->longName() );
            item->setData(i);
            pageTemplatesModel->appendRow( item );
            if(i == 0)
                first = pageTemplatesModel->indexFromItem(item);
        }
        pageTemplatesView = new QListView;
        pageTemplatesView->setSpacing(3);
        pageTemplatesView->setModel(pageTemplatesModel);
        pageTemplatesView->setWordWrap(true);
        pageTemplatesView->setSelectionMode(QAbstractItemView::SingleSelection);
        pageTemplatesView->selectionModel()->select( first , QItemSelectionModel::Select );

        layout->addWidget(new QLabel(tr("Select what page size the book should use...")));
        layout->addWidget(pageTemplatesView);
    }

    if( pMW->hardwiredLanguage() == 0 )
    {
        QHBoxLayout *languageLayout = new QHBoxLayout;
        languageCombo = new QComboBox;
        QStringList languages;
        for(int i = 0; i < parent->languageTemplates()->count(); i++)
            languages << parent->languageTemplates()->at(i)->name();
        languagesModel = new QStringListModel(languages);
        languageCombo->setModel( languagesModel );
        languageLayout->addWidget(new QLabel(tr("Language of the book")),0);
        languageLayout->addWidget(languageCombo,1);

        QHBoxLayout *fontfamilyLayout = new QHBoxLayout;
        QFontDatabase database;
        fontFamilyCombo = new QComboBox;
        fontFamilyCombo->setModel( new QStringListModel(database.families() ));
        fontfamilyLayout->addWidget(new QLabel(tr("Font family")),0);
        fontfamilyLayout->addWidget(fontFamilyCombo,1);

        setFontFromLanguage(languageCombo->currentText());

        layout->addLayout(languageLayout);
        layout->addLayout(fontfamilyLayout);

        connect( languageCombo, SIGNAL(currentIndexChanged(QString)), this, SLOT(setFontFromLanguage(QString)) );
    }

    QPushButton *ok = new QPushButton(tr("OK"));
    QPushButton *cancel = new QPushButton(tr("Cancel"));
    QHBoxLayout *okcancelLayout = new QHBoxLayout;
    okcancelLayout->addStretch(1);
    okcancelLayout->addWidget(ok,0);
    okcancelLayout->addWidget(cancel,0);
    okcancelLayout->addStretch(1);

    layout->addLayout(okcancelLayout);
    this->setLayout(layout);

    connect(ok,SIGNAL(clicked()),this,SIGNAL(ok()));
    connect(cancel,SIGNAL(clicked()),this,SIGNAL(cancel()));
}

void NewBookWidget::setFontFromLanguage(QString lg)
{
    for(int i=0; i < pMW->languageTemplates()->count(); i++)
    {
        if( pMW->languageTemplates()->at(i)->name() == lg )
        {
            int index = fontFamilyCombo->findText( pMW->languageTemplates()->at(i)->defaultFont() );
            if( index != -1 )
            {
                fontFamilyCombo->setCurrentIndex(index);
                return;
            }
        }
    }
}

QString NewBookWidget::fontFamily() const
{
    if( pMW->hardwiredLanguage() != 0 )
        return pMW->hardwiredLanguage()->defaultFont();
    else
        return fontFamilyCombo->currentText();
}

BookTemplate* NewBookWidget::bookTemplate() const
{
    if( !bFromTemplate )
        return false;

    QModelIndexList indices = bookTemplatesView->selectionModel()->selectedIndexes();
    if(indices.count() < 1)
        return 0;
    return pMW->bookTemplates()->at( bookTemplatesModel->itemFromIndex(indices.at(0))->data().toInt() );
}

LanguageTemplate* NewBookWidget::languageTemplate() const
{
    if( pMW->hardwiredLanguage() != 0 )
        return pMW->hardwiredLanguage();
    else
        return pMW->languageTemplateFromIso(pMW->languageTemplates()->at(languageCombo->currentIndex())->iso());
}

PageTemplate* NewBookWidget::pageTemplate() const
{
    if( bFromTemplate )
        return 0;

    QModelIndexList indices = pageTemplatesView->selectionModel()->selectedIndexes();
    if(indices.count() < 1)
        return 0;
    return pMW->pageTemplates()->at( pageTemplatesModel->itemFromIndex(indices.at(0))->data().toInt() );
}
