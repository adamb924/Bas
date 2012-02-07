#include "textitem.h"

#include "textblocklayoutitem.h"
#include "languagetemplate.h"
#include "page.h"
#include "book.h"
#include "layouttemplate.h"
#include "pagetemplate.h"

#include <QApplication>
#include <QClipboard>
#include <QMimeData>
#include <QGraphicsSceneDragDropEvent>
#include <QtDebug>
#include <QKeyEvent>
#include <QtXml>
#include <QTextDocument>
#include <QXmlStreamWriter>

TextItem::TextItem(Page *page, TextBlockLayoutItem *layoutItem, LanguageTemplate *lgTemplate)
{
    this->pPage = page;
    this->layoutItem = layoutItem;
    this->lgTemplate = lgTemplate;

    setTextInteractionFlags(Qt::TextEditorInteraction);

    QFont fnt( page->book()->fontFamily() );
    fnt.setPixelSize( layoutItem->fontSize() );
    setFont( fnt );
}

void TextItem::dropEvent ( QGraphicsSceneDragDropEvent * event )
{
    if( event->mimeData()->hasText() )
    {
        QMimeData *mime = new QMimeData;
        mime->setData("text/html",event->mimeData()->text().toUtf8());
        event->setMimeData(mime);
    }
    QGraphicsTextItem::dropEvent(event);
    setHtml( toHtml() );
}

void TextItem::keyPressEvent ( QKeyEvent * event )
{
    if( event->matches( QKeySequence::Paste ) )
    {
        QClipboard *clipboard = QApplication::clipboard();
        if(clipboard->mimeData()->hasText())
        {
            QMimeData *mime = new QMimeData;
            mime->setData("text/html",clipboard->text().toUtf8());
            clipboard->setMimeData(mime);
        }
        QGraphicsTextItem::keyPressEvent(event);
        setHtml( toHtml() );
    }
    else
    {
        pPage->setChanged(true);
        QGraphicsTextItem::keyPressEvent(event);
    }
}

void TextItem::inputMethodEvent ( QInputMethodEvent * event )
{
    QGraphicsTextItem::inputMethodEvent(event);
    setHtml( toHtml() );
}

void TextItem::setHtml ( const QString & text )
{
//    qDebug() << "TextItem::setHtml" << text << pPage->book()->fontFamily();
    QString revised = text;
    fixHtml(revised);
    QGraphicsTextItem::setHtml(revised);

    pPage->setChanged(true);
}

void TextItem::fixHtml(QString &in)
{
    QString output;
    QXmlStreamReader reader("<html>"+in+"</html>");
    QXmlStreamWriter writer( &output );

    writer.writeDTD("<!DOCTYPE html PUBLIC \"-//W3C//DTD HTML 4.01//EN\" \"http://www.w3.org/TR/html4/strict.dtd\">");
    writer.writeStartElement("html");
    writer.writeStartElement("head");
    writer.writeEmptyElement("meta");
    writer.writeAttribute("name","qrichtext");
    writer.writeAttribute("content","1");
    writer.writeEndElement(); // head
    writer.writeStartElement("body");
    if( lgTemplate->textDirection() == LanguageTemplate::LTR )
        writer.writeAttribute("dir", "LTR" );
    else
        writer.writeAttribute("dir", "RTL" );

    while (!reader.atEnd())
    {
        reader.readNext();
        if(reader.tokenType() == QXmlStreamReader::StartElement && "p" == reader.name().toString() )
        {
            QString text = reader.readElementText();
            writer.writeStartElement("p");
            writer.writeAttribute("style", QString("font-family: %1; %2").arg(pPage->book()->fontFamily()).arg(layoutItem->strippedStyle()) );

            switch( layoutItem->alignment() )
            {
            case Qt::AlignLeft:
                writer.writeAttribute("align","left");
                break;
            case Qt::AlignRight:
                writer.writeAttribute("align","right");
                break;
            case Qt::AlignHCenter:
                writer.writeAttribute("align","center");
                break;
            case Qt::AlignJustify:
                writer.writeAttribute("align","justify");
                break;
            default:
                break;
            }

            writer.writeCharacters(text);
            writer.writeEndElement();
        }
    }

    writer.writeEndElement(); // html

    if (reader.hasError())
        qDebug() << "TextItem::fixHtml error:" << reader.errorString();

    in = output;
}

bool TextItem::toSvg(QXmlStreamWriter &w, QString id)
{
    w.writeStartElement("flowRoot");
    w.writeAttribute("transform","translate(0,0)");
    w.writeAttribute("id",id);
    w.writeAttribute("style", QString("%1; font-family: %2;").arg( layoutItem->style() ).arg( pPage->book()->fontFamily() ) );

    w.writeStartElement("flowRegion");

    w.writeEmptyElement("rect");
    w.writeAttribute("width", QString("%1mm").arg(pPage->pageTemplate()->getPrintAreaWidth( pPage->side() ) ) );
    w.writeAttribute("height", "1000mm");
    w.writeAttribute("x", QString("%1mm").arg( pPage->pageTemplate()->leftMargin( pPage->side() ) ) );
    w.writeAttribute("y", QString("%1mm").arg( layoutItem->y() + pPage->pageTemplate()->topMargin( pPage->side() ) ) );
    w.writeEndElement(); // flowRegion

    QXmlStreamReader r(toHtml());
    while( !r.atEnd() )
    {
        r.readNext();
        if( r.tokenType() == QXmlStreamReader::StartElement && r.name().toString() == "p" )
        {
            w.writeStartElement("flowPara");
            w.writeCharacters( r.readElementText() );
            w.writeEndElement(); // flowPara
        }
    }

    w.writeEndElement(); // flowRoot

    return true;
}
