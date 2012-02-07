#include "layouttemplate.h"

#include "graphicslayoutitem.h"
#include "textblocklayoutitem.h"
#include "mainwindow.h"
#include "pagetemplate.h"

#include <QtXml>
#include <QtGui>

LayoutTemplate::LayoutTemplate(MainWindow *parent)
{
    this->pMW = parent;
    pPageTemplate = 0;
    strBackgroundImage = "";
}

void LayoutTemplate::read(QXmlStreamReader *xml)
{
    QXmlStreamAttributes attr;
    attr = xml->attributes();
    if( !attr.value("name").isNull() )
	strName = attr.value("name").toString();

    QString strPT;
    if( !attr.value("page-template").isNull() )
	strPT = attr.value("page-template").toString();
    if( strPT.length() > 0 )
        pPageTemplate = pMW->pageTemplateFromName(strPT);

    if( pPageTemplate == 0 )
    {
	qDebug() << "LayoutTemplate::read Null page template.";
        while( !( xml->tokenType() != QXmlStreamReader::EndElement && xml->name() == "page-template" ) )
            xml->readNext();

	return;
    }

    QLocale locale;
    QString thisLanguage = locale.name().split('_').at(0);

    while (!xml->atEnd() )
    {
	if(xml->readNext() == QXmlStreamReader::StartElement)
	{
	    QString name = xml->name().toString();

	    if(name == "text-block-layout") {
		attr = xml->attributes();
                aTextblockLayouts << new TextBlockLayoutItem(attr.value("vertical-position").toString().toDouble(), attr.value("style").toString());
	    }
	    else if(name == "graphic-layout") {
		attr = xml->attributes();
		aGraphicLayouts << new GraphicsLayoutItem(attr.value("vertical-position").toString().toDouble(), attr.value("height").toString().toDouble(), attr.value("width").toString().toDouble());
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
            else if(name == "background-image") {
                attr = xml->attributes();
                if( !attr.value("href").isNull() )
                {
                    strBackgroundImage = attr.value("href").toString();

                    QByteArray decoded = QByteArray::fromBase64(xml->attributes().value("href").toString().remove("data:image/png;base64,").toAscii());
                    imgBackground.loadFromData( decoded, "PNG" );
                }
            }
	    else if(name == "default-text") {
		if(aTextblockLayouts.count() == 0 || aTextblockLayouts.last() == 0 )
		    return;
		attr = xml->attributes();
                if( !attr.value("lang").isNull() )
                {
                    if( attr.value("lang").toString()==thisLanguage )
                        aTextblockLayouts.last()->setDefaultText(xml->readElementText());
                    else
                        aTextblockLayouts.last()->otherDefaultTexts()->append( IString(attr.value("lang").toString(),xml->readElementText()) );
                }
	    }
	}
	else if (xml->tokenType() == QXmlStreamReader::EndElement && xml->name() == "layout-template" )
	{
	    xml->readNext();
	    return;
	}
    }
}

void LayoutTemplate::write(QXmlStreamWriter *xml)
{
    QLocale locale;
    QString thisLanguage = locale.name().split('_').at(0);

    xml->writeStartElement("layout-template");
    xml->writeAttribute("name",name());
    if( pPageTemplate != 0 )
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

    if( !strBackgroundImage.isEmpty() )
    {
        xml->writeEmptyElement("background-image");
        xml->writeAttribute("href",strBackgroundImage);
    }

    for(int i=0; i<aTextblockLayouts.count(); i++)
    {
        xml->writeStartElement("text-block-layout");
        xml->writeAttribute("vertical-position",QString::number(aTextblockLayouts.at(i)->y()));
        xml->writeAttribute("style",aTextblockLayouts.at(i)->style());

        xml->writeStartElement("default-text");
        xml->writeAttribute("lang",thisLanguage);
        xml->writeCharacters(aTextblockLayouts.at(i)->defaultText());
        xml->writeEndElement();
        for(int j=0; j<aTextblockLayouts.at(i)->otherDefaultTexts()->count(); j++)
        {
            xml->writeStartElement("default-text");
            xml->writeAttribute("lang",aTextblockLayouts.at(i)->otherDefaultTexts()->at(j).lang);
            xml->writeCharacters(aTextblockLayouts.at(i)->otherDefaultTexts()->at(j).string);
            xml->writeEndElement();
        }

        xml->writeEndElement();
    }

    for(int i=0; i<aGraphicLayouts.count(); i++)
    {
        xml->writeEmptyElement("graphic-layout");
        xml->writeAttribute("vertical-position",QString::number(aGraphicLayouts.at(i)->y()));
        xml->writeAttribute("height",QString::number(aGraphicLayouts.at(i)->height()));
        xml->writeAttribute("width",QString::number(aGraphicLayouts.at(i)->width()));
    }

    xml->writeEndElement(); // layout-template
}

bool LayoutTemplate::compatibleWith(const LayoutTemplate& other)
{
    bool compatible = true;
    if( this->aGraphicLayouts.count() != other.aGraphicLayouts.count() )
	compatible = false;
    if( this->aTextblockLayouts.count() != other.aTextblockLayouts.count() )
	compatible = false;
    return compatible;
}

QWidget* LayoutTemplate::createSettingsPage()
{
    QWidget *w = new QWidget;
    QVBoxLayout *vlayout = new QVBoxLayout;

    QLineEdit *name = new QLineEdit(strName);
    connect(name,SIGNAL(textChanged(QString)),this,SLOT(setName(QString)));

    QLineEdit *longname = new QLineEdit(strLongName);
    connect(longname,SIGNAL(textChanged(QString)),this,SLOT(setLongName(QString)));

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
    QVBoxLayout *textitemlayout = new QVBoxLayout;
    QVBoxLayout *graphicsitemlayout = new QVBoxLayout;
    hlayout->addLayout(textitemlayout);
    hlayout->addLayout(graphicsitemlayout);
    vlayout->addLayout(hlayout);

    vlayout->addWidget(new QLabel("Below, 'position' vertical position, and units are in mm."));
    vlayout->addSpacing(10);

    QTreeWidget *textBlocks = new QTreeWidget;
    QStringList textBlockHeaders;
    textBlockHeaders << tr("Position") << tr("Default text") << tr("Style");
    textBlocks->setHeaderLabels(textBlockHeaders);
    textBlocks->setEditTriggers(QAbstractItemView::DoubleClicked|QAbstractItemView::SelectedClicked|QAbstractItemView::EditKeyPressed);
    for(int i=0; i < aTextblockLayouts.count();i++)
    {
        QStringList entries;
        entries << QString::number(aTextblockLayouts.at(i)->y()) << aTextblockLayouts.at(i)->defaultText() << aTextblockLayouts.at(i)->style();
        QTreeWidgetItem *tmp = new QTreeWidgetItem(entries);
        tmp->setData(0,Qt::UserRole,i);
        tmp->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEditable|Qt::ItemIsEnabled|Qt::ItemIsDragEnabled);
        textBlocks->addTopLevelItem(tmp);
    }
    textBlocks->resizeColumnToContents(0);
    textBlocks->resizeColumnToContents(1);
    textBlocks->resizeColumnToContents(2);
    connect(textBlocks,SIGNAL(itemChanged(QTreeWidgetItem*,int)),this,SLOT(setTextLayoutItemFromWidgetItem(QTreeWidgetItem*,int)));

    QTreeWidget *graphicsBlocks = new QTreeWidget;
    QStringList graphicBlockHeaders;
    graphicBlockHeaders << tr("Position") << tr("Height") << tr("Width");
    graphicsBlocks->setHeaderLabels(graphicBlockHeaders);
    graphicsBlocks->setEditTriggers(QAbstractItemView::DoubleClicked|QAbstractItemView::SelectedClicked|QAbstractItemView::EditKeyPressed);
    for(int i=0; i < aGraphicLayouts.count();i++)
    {
        QStringList entries;
        entries << QString::number(aGraphicLayouts.at(i)->y()) << QString::number(aGraphicLayouts.at(i)->height()) << QString::number(aGraphicLayouts.at(i)->width());
        QTreeWidgetItem *tmp = new QTreeWidgetItem(entries);
        tmp->setData(0,Qt::UserRole,i);
        tmp->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEditable|Qt::ItemIsEnabled|Qt::ItemIsDragEnabled);
        graphicsBlocks->addTopLevelItem(tmp);
    }
    graphicsBlocks->resizeColumnToContents(0);
    graphicsBlocks->resizeColumnToContents(1);
    graphicsBlocks->resizeColumnToContents(2);
    connect(graphicsBlocks,SIGNAL(itemChanged(QTreeWidgetItem*,int)),this,SLOT(setGraphicLayoutItemFromWidgetItem(QTreeWidgetItem*,int)));

    QGridLayout *grdLayout = new QGridLayout;
    grdLayout->setColumnMinimumWidth(2,10);
    grdLayout->addWidget(new QLabel("Text blocks"), 1, 1);
    grdLayout->addWidget(new QLabel("Graphic"), 1, 3);
    grdLayout->addWidget(textBlocks, 2, 1);
    grdLayout->addWidget(graphicsBlocks, 2, 3);
    grdLayout->setColumnStretch(1,4);
    grdLayout->setColumnStretch(3,2);
    vlayout->addLayout(grdLayout);

    w->setLayout(vlayout);
    return w;
}

void LayoutTemplate::setTextLayoutItemFromWidgetItem(QTreeWidgetItem * item, int column )
{
    int index = item->data(0,Qt::UserRole).toInt();

    if(index >= 0 && index < aTextblockLayouts.count() )
    {
        switch(column)
        {
        case 0:
            aTextblockLayouts.at(index)->setY(item->text(column).toDouble());
            break;
        case 1:
            aTextblockLayouts.at(index)->setDefaultText(item->text(column));
            break;
        case 2:
            aTextblockLayouts.at(index)->setStyle(item->text(column));
            break;
        default:
            break;
        }
    }
}

void LayoutTemplate::setGraphicLayoutItemFromWidgetItem(QTreeWidgetItem * item, int column )
{
    int index = item->data(0,Qt::UserRole).toInt();

    if(index >= 0 && index < aGraphicLayouts.count() )
    {
        switch(column)
        {
        case 0:
            aGraphicLayouts.at(index)->setY(item->text(column).toDouble());
            break;
        case 1:
            aGraphicLayouts.at(index)->setHeight(item->text(column).toDouble());
            break;
        case 2:
            aGraphicLayouts.at(index)->setWidth(item->text(column).toDouble());
            break;
        default:
            break;
        }
    }
}
