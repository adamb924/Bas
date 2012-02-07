#include "pagetemplate.h"

#include "numberedit.h"
#include "mainwindow.h"

#include <QStringList>
#include <QtDebug>
#include <QXmlStreamReader>
#include <QtGui>

PageTemplate::PageTemplate(MainWindow *parent)
{
    pMW = parent;
}

PageTemplate::PageTemplate(const PageTemplate& pt)
{
    dWidth = pt.dWidth;
    dHeight = pt.dHeight;
    dHeader = pt.dHeader;
    dFooter = pt.dFooter;
    lLeftMargin = pt.lLeftMargin;
    lRightMargin = pt.lRightMargin;
    lTopMargin = pt.lTopMargin;
    lBottomMargin = pt.lBottomMargin;
    rLeftMargin = pt.rLeftMargin;
    rRightMargin = pt.rRightMargin;
    rTopMargin = pt.rTopMargin;
    rBottomMargin = pt.rBottomMargin;

    strName = pt.strName;
    strLongName = pt.strLongName;
}

void PageTemplate::read(QXmlStreamReader *xml)
{
    bool inRight = false;
    QXmlStreamAttributes attr;

    attr = xml->attributes();
    if( !attr.value("name").isNull() )
	strName = attr.value("name").toString();
    if( !attr.value("is-half-of").isNull() )
        strIsHalfOf = attr.value("is-half-of").toString();

    QLocale locale;
    QString thisLanguage = locale.name().split('_').at(0);

    while (!xml->atEnd() )
    {
        xml->readNext();

        if( xml->tokenType() == QXmlStreamReader::StartElement)
	{
	    QString name = xml->name().toString();

	    if(name == "width") {
		dWidth = xml->readElementText().toDouble();
	    }
	    else if(name == "height") {
		dHeight = xml->readElementText().toDouble();
	    }
	    else if(name == "header") {
                dHeader = xml->readElementText().toDouble();
	    }
	    else if(name == "footer") {
                dFooter = xml->readElementText().toDouble();
	    }
	    else if(name == "margins") {
		attr = xml->attributes();
		if( attr.value("side").toString() == "right" )
		    inRight = true;
		else
		    inRight = false;
	    }
	    else if(name == "left-margin") {
                if(!inRight)
		    lLeftMargin = xml->readElementText().toDouble();
		else
		    rLeftMargin = xml->readElementText().toDouble();
	    }
	    else if(name == "right-margin") {
                if(!inRight)
		    lRightMargin = xml->readElementText().toDouble();
		else
		    rRightMargin = xml->readElementText().toDouble();
	    }
	    else if(name == "top-margin") {
                if(!inRight)
		    lTopMargin = xml->readElementText().toDouble();
		else
		    rTopMargin = xml->readElementText().toDouble();
	    }
	    else if(name == "bottom-margin") {
                if(!inRight)
		    lBottomMargin = xml->readElementText().toDouble();
		else
		    rBottomMargin = xml->readElementText().toDouble();
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
	}
        else if ( xml->tokenType() ==  QXmlStreamReader::EndElement && xml->name() == "page-template" )
        {
            xml->readNext();
            return;
        }
    }
}

void PageTemplate::write(QXmlStreamWriter *xml)
{
    QLocale locale;
    QString thisLanguage = locale.name().split('_').at(0);

    xml->writeStartElement("page-template");
    xml->writeAttribute("name",name());
    if( !isHalfOf().isEmpty() )
        xml->writeAttribute("is-half-of",isHalfOf());

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

    xml->writeTextElement("width",QString::number(width()));
    xml->writeTextElement("height",QString::number(height()));
    xml->writeTextElement("header",QString::number(header()));
    xml->writeTextElement("footer",QString::number(footer()));

    xml->writeStartElement("margins");
    xml->writeAttribute("side","right");
    xml->writeTextElement("left-margin",QString::number(leftMargin(Book::Right)));
    xml->writeTextElement("right-margin",QString::number(rightMargin(Book::Right)));
    xml->writeTextElement("top-margin",QString::number(topMargin(Book::Right)));
    xml->writeTextElement("bottom-margin",QString::number(bottomMargin(Book::Right)));
    xml->writeEndElement(); // margins

    xml->writeStartElement("margins");
    xml->writeAttribute("side","left");
    xml->writeTextElement("left-margin",QString::number(leftMargin(Book::Left)));
    xml->writeTextElement("right-margin",QString::number(rightMargin(Book::Left)));
    xml->writeTextElement("top-margin",QString::number(topMargin(Book::Left)));
    xml->writeTextElement("bottom-margin",QString::number(bottomMargin(Book::Left)));
    xml->writeEndElement(); // margins

    xml->writeEndElement(); // page-template
}

qreal PageTemplate::getPrintAreaWidth(Book::Side side)
{
    if( side == Book::Left )
	return dWidth - lRightMargin - lLeftMargin;
    else
	return dWidth - rRightMargin - rLeftMargin;
}

qreal PageTemplate::leftMargin(Book::Side side)
{
    if( side == Book::Left )
	return lLeftMargin;
    else
	return rLeftMargin;
}

qreal PageTemplate::rightMargin(Book::Side side)
{
    if( side == Book::Left )
	return lRightMargin;
    else
	return rRightMargin;
}

qreal PageTemplate::topMargin(Book::Side side)
{
    if( side == Book::Left )
	return lTopMargin;
    else
	return rTopMargin;
}

qreal PageTemplate::bottomMargin(Book::Side side)
{
    if( side == Book::Left )
	return lBottomMargin;
    else
	return rBottomMargin;
}

QWidget* PageTemplate::createSettingsPage()
{
    QWidget *w = new QWidget;


    QLineEdit *name = new QLineEdit(strName);
    connect(name,SIGNAL(textChanged(QString)),this,SLOT(setName(QString)));

    QLineEdit *longname = new QLineEdit(strLongName);
    connect(longname,SIGNAL(textChanged(QString)),this,SLOT(setLongName(QString)));

    NumberEdit *width = new NumberEdit(dWidth);
    connect(width,SIGNAL(valueChanged(double)),this,SLOT(setWidth(double)));

    NumberEdit *height = new NumberEdit(dHeight);
    connect(height,SIGNAL(valueChanged(double)),this,SLOT(setHeight(double)));

    NumberEdit *header = new NumberEdit(dHeader);
    connect(header,SIGNAL(valueChanged(double)),this,SLOT(setHeader(double)));

    NumberEdit *footer = new NumberEdit(dFooter);
    connect(footer,SIGNAL(valueChanged(double)),this,SLOT(setFooter(double)));

    NumberEdit *llmargin = new NumberEdit(lLeftMargin);
    connect(llmargin,SIGNAL(valueChanged(double)),this,SLOT(setLLeftMargin(double)));

    NumberEdit *rlmargin = new NumberEdit(rLeftMargin);
    connect(rlmargin,SIGNAL(valueChanged(double)),this,SLOT(setRLeftMargin(double)));

    NumberEdit *lrmargin = new NumberEdit(lRightMargin);
    connect(lrmargin,SIGNAL(valueChanged(double)),this,SLOT(setLRightMargin(double)));

    NumberEdit *rrmargin = new NumberEdit(rRightMargin);
    connect(rrmargin,SIGNAL(valueChanged(double)),this,SLOT(setRRightMargin(double)));

    NumberEdit *ltmargin = new NumberEdit(lTopMargin);
    connect(ltmargin,SIGNAL(valueChanged(double)),this,SLOT(setLTopMargin(double)));

    NumberEdit *rtmargin = new NumberEdit(rTopMargin);
    connect(rtmargin,SIGNAL(valueChanged(double)),this,SLOT(setRTopMargin(double)));

    NumberEdit *lbmargin = new NumberEdit(lBottomMargin);
    connect(lbmargin,SIGNAL(valueChanged(double)),this,SLOT(setLBottomMargin(double)));

    NumberEdit *rbmargin = new NumberEdit(rBottomMargin);
    connect(rbmargin,SIGNAL(valueChanged(double)),this,SLOT(setRBottomMargin(double)));

    QComboBox *ishalfof = new QComboBox;
    ishalfof->addItem("");
    connect(ishalfof,SIGNAL(currentIndexChanged(QString)),this,SLOT(setIsHalfOf(QString)));
    for(int i=0; i < pMW->pageTemplates()->count(); i++)
        ishalfof->addItem(pMW->pageTemplates()->at(i)->name());
    ishalfof->setCurrentIndex( pMW->pageTemplates()->indexOf(pMW->pageTemplateFromName(strIsHalfOf)) + 1 );

    QVBoxLayout *vlayout = new QVBoxLayout;

    QFormLayout *formLayout = new QFormLayout;
    formLayout->addWidget(new QLabel(tr("All numbers are millimeters")));
    formLayout->addRow(tr("&Name:"), name);
    formLayout->addRow(tr("Long Name"),longname);
    formLayout->addRow(tr("Is half the size of:"),ishalfof);
    formLayout->addRow(tr("&Width:"), width);
    formLayout->addRow(tr("&Height:"), height);
    formLayout->addRow(tr("&Header:"), header);
    formLayout->addRow(tr("&Footer:"), footer);

    QGroupBox *leftSide = new QGroupBox(tr("Left-hand pages"));
    QFormLayout *leftSideLayout = new QFormLayout;
    leftSide->setLayout(leftSideLayout);

    leftSideLayout->addRow(tr("Left Margin:"),llmargin);
    leftSideLayout->addRow(tr("Right Margin:"),lrmargin);
    leftSideLayout->addRow(tr("Top Margin:"),ltmargin);
    leftSideLayout->addRow(tr("Bottom Margin:"),lbmargin);

    QGroupBox *rightSide = new QGroupBox(tr("Right-hand pages"));
    QFormLayout *rightSideLayout = new QFormLayout;
    rightSide->setLayout(rightSideLayout);

    rightSideLayout->addRow(tr("Left Margin:"),rlmargin);
    rightSideLayout->addRow(tr("Right Margin:"),rrmargin);
    rightSideLayout->addRow(tr("Top Margin:"),rtmargin);
    rightSideLayout->addRow(tr("Bottom Margin:"),rbmargin);

    vlayout->addLayout(formLayout);
    vlayout->addWidget(leftSide);
    vlayout->addWidget(rightSide);

    w->setLayout(vlayout);

    return w;
}
