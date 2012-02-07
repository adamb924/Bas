#ifndef PAGETEMPLATE_H
#define PAGETEMPLATE_H

#include <QtGlobal>
#include <QString>
#include <QList>

#include "book.h"

#include "abstracttemplate.h"

class QXmlStreamReader;
class QXmlStreamWriter;
class MainWindow;

class PageTemplate : public AbstractTemplate
{
    Q_OBJECT
public:
    PageTemplate(MainWindow *parent);
    PageTemplate(const PageTemplate& pt);

    QWidget* createSettingsPage();

    void read(QXmlStreamReader *xml);
    void write(QXmlStreamWriter *xml);

    qreal getPrintAreaWidth(Book::Side side);
    qreal leftMargin(Book::Side side);
    qreal rightMargin(Book::Side side);
    qreal topMargin(Book::Side side);
    qreal bottomMargin(Book::Side side);


    inline QString longName() const { return strLongName; }
    inline qreal width() const { return dWidth; }
    inline qreal height() const { return dHeight; }
    inline qreal header() const { return dHeader; }
    inline qreal footer() const { return dFooter; }
    inline QString isHalfOf() const { return strIsHalfOf; }

    inline QString toString() const { return QString("%1 %2 %3 %4 %5 %6 %7 %8 %9 %10 %11 %12 %13 %14 %15")
                                      .arg(strName)
                                      .arg(strLongName)
                                      .arg(strIsHalfOf)
                                      .arg(dWidth)
                                      .arg(dHeight)
                                      .arg(dHeader)
                                      .arg(dFooter)
                                      .arg(lLeftMargin)
                                      .arg(lRightMargin)
                                      .arg(lTopMargin)
                                      .arg(lBottomMargin)
                                      .arg(rLeftMargin)
                                      .arg(rRightMargin)
                                      .arg(rTopMargin)
                                      .arg(rBottomMargin); }

public slots:
    inline void setWidth(double v) { dWidth = v; }
    inline void setHeight(double v) { dHeight = v; }
    inline void setFooter(double v) { dFooter = v; }
    inline void setHeader(double v) { dHeader = v; }

    inline void setLLeftMargin(double v) { lLeftMargin = v; }
    inline void setLRightMargin(double v) { lRightMargin = v; }
    inline void setLTopMargin(double v) { lTopMargin = v; }
    inline void setLBottomMargin(double v) { lBottomMargin = v; }

    inline void setRLeftMargin(double v) { rLeftMargin = v; }
    inline void setRRightMargin(double v) { rRightMargin = v; }
    inline void setRTopMargin(double v) { rTopMargin = v; }
    inline void setRBottomMargin(double v) { rBottomMargin = v; }

    inline void setLongName(QString s) { strLongName = s; }
    inline void setIsHalfOf(QString s) { strIsHalfOf = s; }

private:
    qreal dWidth, dHeight, dHeader, dFooter;
    qreal lLeftMargin, lRightMargin, lTopMargin, lBottomMargin;
    qreal rLeftMargin, rRightMargin, rTopMargin, rBottomMargin;

    QString strLongName, strIsHalfOf;

    MainWindow *pMW;

};

#endif // PAGETEMPLATE_H
