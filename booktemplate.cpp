#include "booktemplate.h"
#include "mainwindow.h"
#include "pagetemplate.h"
#include "layouttemplate.h"
#include "xmatterlistwidget.h"

#include <QtGui>
#include <QtXml>

BookTemplate::BookTemplate(MainWindow *parent)
{
    this->pMW = parent;

    nFillerPagesBefore = 9999999;
    eFillerPagesPosition = BackMatter;

    pPageTemplate = 0;
}

QString BookTemplate::read(QXmlStreamReader *xml)
{
    QXmlStreamAttributes attr;

    Position parsePosition = FrontMatter;

    //    qDebug() << "BookTemplate::read BEGIN";

    attr = xml->attributes();
    if( !attr.value("name").isNull() )
	strName = attr.value("name").toString();
    if( !attr.value("page-template").isNull() )
    {
	QString pt = attr.value("page-template").toString();
        PageTemplate *tmp = pMW->pageTemplateFromName(pt);
	if( tmp != 0 )
	    pPageTemplate = tmp;
	else
	    return "Book template '" + pt + "' not found.";
    }

    QLocale locale;
    QString thisLanguage = locale.name().split('_').at(0);

    bool insertFiller = false;

    while (!xml->atEnd() )
    {
        //        qDebug() << insertFiller;
	if(xml->readNext() == QXmlStreamReader::StartElement)
	{
	    QString name = xml->name().toString();

            //	    qDebug() << "BookTemplate::read" << name;

            if(name == "page") {
		attr = xml->attributes();
		QString lt = attr.value("layout-template").toString();
                LayoutTemplate *tmp = pMW->layoutTemplateFromName(lt, pPageTemplate->name() );

                //                qDebug() << "read a page" << insertFiller;

                if( insertFiller )
                {
                    insertFiller = false;

                    //                    qDebug() << "BookTemplate::read" << parsePosition;
                    /*
*/
                }

		if( tmp == 0 )
		{
		    return "Layout template '" + lt + "' not found.";
		}

		switch(parsePosition)
		{
		case FrontMatter:
		    aFrontMatter << tmp;
		    break;
		case MiddleMatter:
		    aMiddleMatter << tmp;
		    break;
		case BackMatter:
		    aBackMatter << tmp;
		    break;
		default:
		    break;
		}
	    }
            else if(name == "front-matter") {
                parsePosition = FrontMatter;
            }
            else if(name == "middle-matter") {
		parsePosition = MiddleMatter;
	    }
            else if(name == "back-matter") {
                parsePosition = BackMatter;
            }
            else if(name == "long-name") {
                attr = xml->attributes();
                if( !attr.value("lang").isNull() )
                {
                    if( attr.value("lang").toString()==thisLanguage )
                        strLongName = xml->readElementText();
                    else
                        aOtherLongNames << IString(attr.value("lang").toString(),xml->readElementText());
                }
	    }
            else if( name == "filler-pages-here" )
            {
                insertFiller = true;

                switch(parsePosition)
                {
                case FrontMatter:
                    setFillerPagesBefore( parsePosition , aFrontMatter.count() );
                    break;
                case MiddleMatter:
                    setFillerPagesBefore( parsePosition , aMiddleMatter.count() );
                    break;
                case BackMatter:
                    setFillerPagesBefore( parsePosition , aBackMatter.count() );
                    break;
                default:
                    break;
                }
            }
	}
	else if(xml->tokenType() == QXmlStreamReader::EndElement)
	{
	    QString name = xml->name().toString();
            if(name == "book-template") {
                xml->readNext();
                return "";
	    }
	}

    }

    return "";
}

void BookTemplate::write(QXmlStreamWriter *xml)
{
    QLocale locale;
    QString thisLanguage = locale.name().split('_').at(0);

    xml->writeStartElement("book-template");

    xml->writeAttribute("name",name());
    if(pPageTemplate != 0)
        xml->writeAttribute("page-template",pageTemplate()->name());

    xml->writeStartElement("long-name");
    xml->writeAttribute("lang",thisLanguage);
    xml->writeCharacters(longName());
    xml->writeEndElement(); // long-name
    for(int i=0; i<aOtherLongNames.count(); i++)
    {
        xml->writeStartElement("long-name");
        xml->writeAttribute("lang",aOtherLongNames.at(i).lang);
        xml->writeCharacters(aOtherLongNames.at(i).string);
        xml->writeEndElement(); // long-name
    }

    xml->writeStartElement("front-matter");
    for(int i=0; i<frontMatter()->count(); i++)
    {
        if( eFillerPagesPosition == FrontMatter && i == nFillerPagesBefore )
            xml->writeEmptyElement("filler-pages-here");
        xml->writeEmptyElement("page");
        xml->writeAttribute("layout-template",frontMatter()->at(i)->name());
    }
    if( eFillerPagesPosition == FrontMatter && frontMatter()->count() == nFillerPagesBefore )
        xml->writeEmptyElement("filler-pages-here");
    xml->writeEndElement(); // front-matter

    xml->writeStartElement("middle-matter");
    for(int i=0; i<middleMatter()->count(); i++)
    {
        xml->writeEmptyElement("page");
        xml->writeAttribute("layout-template",middleMatter()->at(i)->name());
    }
    xml->writeEndElement(); // middle-matter

    xml->writeStartElement("back-matter");
    for(int i=0; i<backMatter()->count(); i++)
    {
        if( eFillerPagesPosition == BackMatter && i == nFillerPagesBefore )
            xml->writeEmptyElement("filler-pages-here");
        xml->writeEmptyElement("page");
        xml->writeAttribute("layout-template",backMatter()->at(i)->name());
    }
    if( eFillerPagesPosition == BackMatter && backMatter()->count() == nFillerPagesBefore )
        xml->writeEmptyElement("filler-pages-here");
    xml->writeEndElement(); // back-matter

    xml->writeEndElement(); // book-template
}

bool BookTemplate::canAddPageAfter(int page, int ofPage)
{
    if( positionOfPage(page,ofPage) == BookTemplate::MiddleMatter )
	return true;
    else
	return false;
}

bool BookTemplate::canRemovePageAt(int page, int ofPage)
{
    if( positionOfPage(page,ofPage) == BookTemplate::MiddleMatter )
        return true;
    else
	return false;
}

BookTemplate::Position BookTemplate::positionOfPage(int page, int ofPage)
{
    if( page < aFrontMatter.count() )
        return BookTemplate::FrontMatter;
    if( page >= aFrontMatter.count() && page < (ofPage - aBackMatter.count() ) )
        return BookTemplate::MiddleMatter;
    else
        return BookTemplate::BackMatter;
}

int BookTemplate::relativeIndexOfPage(int page, int ofPage)
{
    switch( positionOfPage(page,ofPage) )
    {
    case FrontMatter:
        return page;
        break;
    case MiddleMatter:
        return (page - aFrontMatter.count()) % aMiddleMatter.count();
        break;
    case BackMatter:
        return aBackMatter.count() - ( ofPage - page );
        break;
    default:
        return -1;
        break;
    }
}

LayoutTemplate* BookTemplate::layoutTemplateForNewPageAt(int position)
{
    if( position < aFrontMatter.count() )
	return 0;
    position -= aFrontMatter.count();
    return aMiddleMatter.at(position % aMiddleMatter.count() );
}

bool BookTemplate::isMiddleMatterReflowable() const
{
    if(aMiddleMatter.count() < 1)
	return false;

    int textCount = aMiddleMatter.at(0)->textblockLayouts()->count();
    int graphicsCount = aMiddleMatter.at(0)->graphicsLayouts()->count();

    for(int i=0; i<aMiddleMatter.count(); i++)
	if( aMiddleMatter.at(i)->textblockLayouts()->count() != textCount || aMiddleMatter.at(0)->graphicsLayouts()->count() != graphicsCount )
	    return false;
    return true;
}

QWidget* BookTemplate::createSettingsPage()
{
    QWidget *w = new QWidget;

    QVBoxLayout *vlayout = new QVBoxLayout;

    QLineEdit *name = new QLineEdit(strName);
    connect(name,SIGNAL(textChanged(QString)),this,SLOT(setName(QString)));

    QLineEdit *longname = new QLineEdit(strLongName);
    connect(longname,SIGNAL(textChanged(QString)),this,SLOT(setLongName(QString)));

    // for selecting the page template
    QComboBox *pagetemplate = new QComboBox;
    for(int i=0; i < pMW->pageTemplates()->count(); i++)
        pagetemplate->addItem( pMW->pageTemplates()->at(i)->name() );
    pagetemplate->setCurrentIndex( pMW->pageTemplates()->indexOf( pPageTemplate ) );
    connect(pagetemplate,SIGNAL(currentIndexChanged(int)),this,SLOT(setPageTemplateFromIndex(int)));

    QFormLayout *formLayout = new QFormLayout;
    formLayout->addRow(tr("&Name:"), name);
    formLayout->addRow(tr("&Long name:"), longname);
    formLayout->addRow(tr("&Page Template:"), pagetemplate);
    vlayout->addLayout(formLayout);
    vlayout->addSpacing(10);

    QHBoxLayout *hlayout = new QHBoxLayout;
    QVBoxLayout *booklayout = new QVBoxLayout;
    QVBoxLayout *layoutlayout = new QVBoxLayout;
    hlayout->addLayout(booklayout);
    hlayout->addLayout(layoutlayout);
    vlayout->addLayout(hlayout);

    QListWidgetItem *fillerpages = new QListWidgetItem( tr("[[Filler Pages]]") );
    fillerpages->setData(0,tr("[[Filler Pages]]"));
    fillerpages->setData(1,tr("[[Filler Pages]]"));
    fillerpages->setData(2,tr("[[Filler Pages]]"));
    fillerpages->setToolTip( tr("Bas will add pages at the position of this \"page\" to make enough pages for a printed book. Just add this at one place.") );
    fillerpages->setBackgroundColor(Qt::yellow);

    booklayout->addWidget(new QLabel(tr("Front Matter")));
    XMatterListWidget *frontMatter = new XMatterListWidget(FrontMatter,&aFrontMatter,this);
    booklayout->addWidget(frontMatter);
    if( eFillerPagesPosition == FrontMatter )
        frontMatter->insertItem(nFillerPagesBefore,fillerpages);

    booklayout->addWidget(new QLabel(tr("Middle Matter")));
    XMatterListWidget *middleMatter = new XMatterListWidget(MiddleMatter,&aMiddleMatter,this);
    booklayout->addWidget(middleMatter);

    booklayout->addWidget(new QLabel(tr("Back Matter")));
    XMatterListWidget *backMatter = new XMatterListWidget(BackMatter,&aBackMatter,this);
    booklayout->addWidget(backMatter);
    if( eFillerPagesPosition == BackMatter )
        backMatter->insertItem(nFillerPagesBefore,fillerpages);


    layoutlayout->addWidget(new QLabel(tr("Layout Templates")));
    QListWidget *layoutTemplates = new QListWidget;
    layoutTemplates->setSpacing(3);
    layoutTemplates->setDragDropMode(QAbstractItemView::DragOnly);
    QList<LayoutTemplate*> aLT = pMW->getLayoutTemplatesForPageTemplate(pPageTemplate);
    for(int i=0; i < aLT.count(); i++ )
    {
        QListWidgetItem *tmp = new QListWidgetItem( aLT.at(i)->name() );
        tmp->setToolTip( aLT.at(i)->longName() );
        layoutTemplates->addItem( tmp );
    }
    layoutlayout->addWidget(layoutTemplates);

    QListWidgetItem *fillerpagestwo = new QListWidgetItem( tr("[[Filler Pages]]") );
    fillerpagestwo->setData(0,tr("[[Filler Pages]]"));
    fillerpagestwo->setData(1,tr("[[Filler Pages]]"));
    fillerpagestwo->setData(2,tr("[[Filler Pages]]"));
    fillerpagestwo->setToolTip( tr("Bas will add pages at the position of this \"page\" to make enough pages for a printed book. Just add this at one place.") );
    fillerpagestwo->setBackgroundColor(Qt::yellow);
    layoutTemplates->addItem( fillerpagestwo );


    connect(frontMatter,SIGNAL(fillerAdded()),backMatter,SLOT(removeFiller()));
    connect(backMatter,SIGNAL(fillerAdded()),frontMatter,SLOT(removeFiller()));

    w->setLayout(vlayout);

    return w;
}
