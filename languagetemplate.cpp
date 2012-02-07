#include "languagetemplate.h"

#include <QtXml>
#include <QtGui>

LanguageTemplate::LanguageTemplate()
{
    strName = "English";
    strIso = "eng";
    strDefaultFont = "Times New Roman";
    eTextDirection = LanguageTemplate::LTR;
}

void LanguageTemplate::read(QXmlStreamReader *xml)
{
    QLocale locale;
    QString thisLanguage = locale.name().split('_').at(0);

    while (!xml->atEnd() )
    {
        if(xml->readNext() == QXmlStreamReader::StartElement)
        {
            QString name = xml->name().toString();

            if(name == "text-direction") {
                QString tmp = xml->readElementText();
                if( tmp == "LTR" )
                    eTextDirection = LanguageTemplate::LTR;
                else if( tmp == "RTL" )
                    eTextDirection = LanguageTemplate::RTL;
            }
            else if(name == "iso") {
                strIso = xml->readElementText();
            }
            else if(name == "name") {
                QXmlStreamAttributes attr = xml->attributes();
                if( !attr.value("lang").isNull() )
                {
                    if(attr.value("lang").toString()==thisLanguage)
                        strName = xml->readElementText();
                    else
                        aOtherNames << IString(attr.value("lang").toString(), xml->readElementText() );
                }
            }
            else if(name == "default-font") {
                strDefaultFont = xml->readElementText();
            }
        }
        else if (xml->tokenType() == QXmlStreamReader::EndElement && xml->name() == "language-template" )
        {
            xml->readNext();
            return;
        }
    }
}

void LanguageTemplate::write(QXmlStreamWriter *xml)
{
    QLocale locale;
    QString thisLanguage = locale.name().split('_').at(0);

    xml->writeStartElement("language-template");

    xml->writeStartElement("name");
    xml->writeAttribute("lang",thisLanguage);
    xml->writeCharacters(name());
    xml->writeEndElement();

    for(int i=0; i<aOtherNames.count(); i++)
    {
        xml->writeStartElement("name");
        xml->writeAttribute("lang",aOtherNames.at(i).lang);
        xml->writeCharacters(aOtherNames.at(i).string);
        xml->writeEndElement();
    }
    if( textDirection() == LTR )
        xml->writeTextElement("text-direction","LTR");
    else
        xml->writeTextElement("text-direction","RTL");

    xml->writeTextElement("default-font",defaultFont());

    xml->writeTextElement("iso",iso());

    xml->writeEndElement(); // language-template
}

QWidget* LanguageTemplate::createSettingsPage()
{
    QWidget *w = new QWidget;

    QLineEdit *name = new QLineEdit(strName);
    connect(name,SIGNAL(textChanged(QString)),this,SLOT(setName(QString)));

    QLineEdit *iso = new QLineEdit(strIso);
    connect(iso,SIGNAL(textChanged(QString)),this,SLOT(setIso(QString)));

    QFontComboBox *font = new QFontComboBox();
    font->setCurrentFont( QFont(strDefaultFont) );
    connect(font,SIGNAL(currentIndexChanged(QString)),this,SLOT(setDefaultFont(QString)));

    QComboBox *textdirection = new QComboBox;
    textdirection->addItem("RTL");
    textdirection->addItem("LTR");
    if( eTextDirection == RTL )
        textdirection->setCurrentIndex(0);
    else
        textdirection->setCurrentIndex(1);
    connect(textdirection,SIGNAL(currentIndexChanged(int)),this,SLOT(setTextDirectionFromIndex(int)));

    QFormLayout *formLayout = new QFormLayout;
    formLayout->addRow(tr("&Name:"), name);
    formLayout->addRow(tr("&ISO 639-3:"), iso);
    formLayout->addRow(tr("&Default font:"), font);
    formLayout->addRow(tr("&Text direction:"), textdirection);
    w->setLayout(formLayout);

    return w;
}

void LanguageTemplate::setTextDirectionFromIndex(int i)
{
    switch(i)
    {
    case 0:
        eTextDirection = RTL;
        break;
    case 1:
        eTextDirection = LTR;
        break;
    default:
        break;
    }
}
