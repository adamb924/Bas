#include "page.h"
#include <QtDebug>
#include <QtXml>
#include <QtGui>
#include <QRegExp>
#include <QByteArray>
#include <QImageWriter>

#include "pagetemplate.h"
#include "layouttemplate.h"
#include "textblocklayoutitem.h"
#include "graphicslayoutitem.h"
#include "graphicsitem.h"
#include "textitem.h"
#include "languagetemplate.h"
#include "mainwindow.h"

Page::Page(Book *pBook, LayoutTemplate *layoutTemplate, Book::Side s, LanguageTemplate *lgTemplate, QObject *parent) :
	QGraphicsScene(parent)
{
    pBook = pBook;
    lgTemplate = lgTemplate;
    pageSide = s;

    pLayoutTemplate = layoutTemplate;
    addBackgroundImage();
}

QRectF Page::sceneRect() const
{
    return QRectF(0,0,pLayoutTemplate->pageTemplate()->width(),pLayoutTemplate->pageTemplate()->height());
}

qreal Page::aspectRatio() const
{
    return pLayoutTemplate->pageTemplate()->width() / pLayoutTemplate->pageTemplate()->height();
}


void Page::read(QXmlStreamReader *xml)
{
    QString textBlock="";
    int textblockCounter = 0;
    int graphicCounter = 0;

    QXmlStreamAttributes attr = xml->attributes();
    if( attr.hasAttribute("language-template") )
        lgTemplate = pBook->mainWindow()->languageTemplateFromIso( attr.value("language-template").toString() );

    while (!xml->atEnd() )
    {
	if(xml->readNext() == QXmlStreamReader::StartElement)
	{
	    QString name = xml->name().toString();

	    if(name == "text-block")
	    {
		textBlock = "";
	    }
	    else if(name == "graphic-block")
	    {
		// are there now more graphics than slots in the template?
		if( graphicCounter+1 > pLayoutTemplate->graphicsLayouts()->count() )
		{
		    qDebug() << "Too many graphics on this page";
		    continue; // oops, too many
		}

		GraphicsLayoutItem *t = pLayoutTemplate->graphicsLayouts()->at(graphicCounter);

		// process the graphic-block here
                GraphicsItem *item;

		if( xml->attributes().hasAttribute("href") )
                    item = new GraphicsItem(this, xml->attributes().value("href").toString().remove("data:image/png;base64,"), t);
		else
                    item = new GraphicsItem(this, QPixmap::fromImage(QImage(":/resources/nopicture.png")), t);

		aGraphics << item;
		this->addItem(item);

		graphicCounter++;
	    }
            else {
                textBlock += "<" + xml->name().toString() + ">";
            }
	}
	else if(xml->tokenType() == QXmlStreamReader::EndElement )
	{
	    QString name = xml->name().toString();
	    //	    qDebug() << "Page::read END OF" << name;

	    // we parse <text-block>s at the end in order to capture all intervening <p>s
	    if(name == "text-block")
	    {
		// are there now more text-blocks than slots in the template?
		if( textblockCounter+1 > pLayoutTemplate->textblockLayouts()->count() )
		{
		    qDebug() << "Too many text-blocks on this page";
		    continue; // oops, too many
		}
//		aTextblocks << formatTextItem( textBlock, pLayoutTemplate->textblockLayouts()->at(textblockCounter) );
                aTextblocks <<  new TextItem( this, pLayoutTemplate->textblockLayouts()->at(textblockCounter),lgTemplate);

                qreal top = pLayoutTemplate->pageTemplate()->topMargin(pageSide) + pLayoutTemplate->textblockLayouts()->at(textblockCounter)->y();
                qreal left = pLayoutTemplate->pageTemplate()->leftMargin(pageSide);
                aTextblocks.last()->setPos(left,top);
                aTextblocks.last()->setTextWidth(pLayoutTemplate->pageTemplate()->getPrintAreaWidth(pageSide));

//                qDebug() << "Page::read";
                aTextblocks.last()->setHtml(textBlock);

                this->addItem( aTextblocks.last() );
		textblockCounter++;
	    }
	    else if(name == "page")
	    {
		this->addRect(this->sceneRect(),QPen(Qt::NoPen));
		return;
	    }
	    else { // remember other tags as well
		textBlock += "</" + xml->name().toString() + ">";
	    }
	}
	else if(xml->tokenType() == QXmlStreamReader::Characters )
	{
	    QString str = xml->text().toString();
	    if( xml->text().toString().trimmed().length() > 0 )
		textBlock += str;
	}

    }
}

bool Page::isValid() const
{
    bool valid = true;

    if(pLayoutTemplate->pageTemplate() == 0)
	valid = false;
    if(pLayoutTemplate == 0)
	valid = false;

    return valid;
}

QString Page::error() const
{
    if(isValid())
	return "";
    else
    {
	QString errormessage = "";
	if(pLayoutTemplate->pageTemplate() == 0)
	    errormessage += "Page template is null. ";
	if(pLayoutTemplate == 0)
	    errormessage += "Layout template is null. ";
	return errormessage;
    }
}

void Page::populateFromTemplate()
{
    for(int i=0; i<pLayoutTemplate->graphicsLayouts()->count(); i++)
    {
	QPixmap pixmap;
	pixmap = QPixmap::fromImage(QImage(":/resources/nopicture.png"));
        GraphicsItem *item = new GraphicsItem(this, pixmap, pLayoutTemplate->graphicsLayouts()->at(i));
	this->addItem(item);

	aGraphics << item;
    }
    for(int i=0; i<pLayoutTemplate->textblockLayouts()->count(); i++)
    {
        QString defaultText = "<p>" + pLayoutTemplate->textblockLayouts()->at(i)->defaultText() + "</p>";
//	aTextblocks << formatTextItem( defaultText, pLayoutTemplate->textblockLayouts()->at(i) );
        aTextblocks <<  new TextItem( this, pLayoutTemplate->textblockLayouts()->at(i),lgTemplate);

        qreal top = pLayoutTemplate->pageTemplate()->topMargin(pageSide) + pLayoutTemplate->textblockLayouts()->at(i)->y();
        qreal left = pLayoutTemplate->pageTemplate()->leftMargin(pageSide);
        aTextblocks.last()->setPos(left,top);
        aTextblocks.last()->setTextWidth(pLayoutTemplate->pageTemplate()->getPrintAreaWidth(pageSide));

//        qDebug() << "Page::populateFromTemplate()";
        aTextblocks.last()->setHtml( defaultText );
//        aTextblocks.last()->setHtml("<p>" + defaultText + "</p>");

	this->addItem( aTextblocks.last() );
    }

    this->addRect(this->sceneRect(),QPen(Qt::NoPen));
}

void Page::write(QXmlStreamWriter *xml) const
{
    xml->writeStartElement("page");
    xml->writeAttribute("layout-template",pLayoutTemplate->name());

    // this is an awkward way to parse out from the textblocks everything but the body tokens
    for(int i=0; i<aTextblocks.count(); i++)
    {
	xml->writeStartElement("text-block");

	xml->setAutoFormatting(false);
        QXmlStreamReader reader(aTextblocks.at(i)->toHtml());
        bool begin = false;
	while( !reader.atEnd() )
	{
	    reader.readNext();

	    if( reader.tokenType() == QXmlStreamReader::StartElement && reader.name().toString() == "body")
		begin = true;
	    if( reader.tokenType() == QXmlStreamReader::EndElement && reader.name().toString() == "body")
	    {
		xml->writeEndElement();
		begin = false;
	    }
	    if( begin &&  reader.tokenType() != QXmlStreamReader::DTD && reader.tokenType() != QXmlStreamReader::StartDocument && reader.tokenType() != QXmlStreamReader::EndDocument  )
            {
		xml->writeCurrentToken(reader);
            }
	}
	xml->setAutoFormatting(true);

	xml->writeEndElement(); // text-block
    }

    for(int i=0; i<aGraphics.count(); i++)
    {
	xml->writeEmptyElement("graphic-block");
/*
	QByteArray encoded, unencoded;
	QBuffer buffer( &unencoded );
	QImageWriter writer(&buffer,"png");
	writer.write(aGraphics.at(i)->pixmap().toImage());
	encoded = unencoded.toBase64();
	QString strEncoded(encoded);
	strEncoded.prepend("data:image/png;base64,");
*/
        xml->writeAttribute("href",aGraphics.at(i)->base64());
    }

    xml->writeEndElement(); // page
}

void Page::updateLayoutItemFormats()
{
    for(int i=0; i < aTextblocks.count(); i++ )
    {
        QString html = aTextblocks.at(i)->toHtml();
        delete aTextblocks.at(i); // doesn't change the array, just deletes the object
        aTextblocks[i] = new TextItem( this, pLayoutTemplate->textblockLayouts()->at(i), lgTemplate );
//        qDebug() << "Page::updateLayoutItemFormats";
        aTextblocks.at(i)->setHtml(html);

        qreal top = pLayoutTemplate->pageTemplate()->topMargin(pageSide) + pLayoutTemplate->textblockLayouts()->at(i)->y();
        qreal left = pLayoutTemplate->pageTemplate()->leftMargin(pageSide);
        aTextblocks.at(i)->setPos(left,top);
        aTextblocks.at(i)->setTextWidth(pLayoutTemplate->pageTemplate()->getPrintAreaWidth(pageSide));

	this->addItem(aTextblocks.at(i));
    }
    for(int i=0; i < aGraphics.count(); i++ )
    {
	QPixmap pixmap = aGraphics.at(i)->pixmap();
	delete aGraphics.at(i); // doesn't change the array, just deletes the object
        aGraphics[i] = new GraphicsItem(this, pixmap, pLayoutTemplate->graphicsLayouts()->at(i) );
	this->addItem(aGraphics.at(i));
    }
}

bool Page::toSvg(QXmlStreamWriter &w, QPoint location) const
{
    w.writeStartElement("g");
    w.writeAttribute("transform",QString("translate(%1,%2)").arg(location.x()).arg(location.y()));

    for(int i=0; i < aTextblocks.count(); i++ )
        if( !aTextblocks.at(i)->toSvg(w , QString("text-%1").arg(i) ) )
            return false;

    for(int i=0; i < aGraphics.count(); i++ )
        if( !aGraphics.at(i)->toSvg(w , QString("picture-%1").arg(i) ) )
            return false;

    w.writeEndElement(); // g

    return true;
}

void Page::setChanged(bool changed)
{
    bChanged = changed;
    if(bChanged)
        book()->setChanged(true);
}

void Page::addBackgroundImage()
{
    qDebug() << "Page::addBackgroundImage()";
    QPixmap bg = pLayoutTemplate->backgroundImage();
    qDebug() << "Page::addBackgroundImage()";
    if( bg.isNull() )
        return;

    qDebug() << "Page::addBackgroundImage()";
    QGraphicsPixmapItem *item = new QGraphicsPixmapItem(bg);
    qDebug() << "Page::addBackgroundImage()";
    item->setPos(0,0);
    qDebug() << "Page::addBackgroundImage()";
    addItem(item);
    qDebug() << "Page::addBackgroundImage()";
}
