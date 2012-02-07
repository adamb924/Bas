#include "textblocklayoutitem.h"

#include <QCoreApplication>
#include <QtGui>

TextBlockLayoutItem::TextBlockLayoutItem(qreal vp, QString s)
{
    verticalPosition = vp;
    itemstyle = s;

    strDefaultText = QCoreApplication::translate("TextBlockLayoutItem","Write text here");
}

int TextBlockLayoutItem::fontSize() const
{
    int fontSizePixels = 12;

    QRegExp fontSizeRE("font-size:[ ]*(\\d+)px"); // interpret as points
    if( fontSizeRE.indexIn(itemstyle ) > -1)
        fontSizePixels = fontSizeRE.cap(1).toInt() * 0.352777778; // convert to mm

    return fontSizePixels;
}

QString TextBlockLayoutItem::strippedStyle() const
{
    QRegExp fontSizeRE("font-size:[ ]*(\\d+)px(;)"); // interpret as points
    QString ret = itemstyle;
    ret = ret.replace(fontSizeRE,"");
    return ret;
}

Qt::AlignmentFlag TextBlockLayoutItem::alignment() const
{
    QRegExp alignmentRE("text-align:[ ]*(\\w+)(;)"); // interpret as points
    if( alignmentRE.indexIn(itemstyle ) == -1)
        return (Qt::AlignmentFlag)0;

    QString a = alignmentRE.cap(1);
    if(a == "left")
        return Qt::AlignLeft;
    else if(a == "right")
        return Qt::AlignRight;
    else if(a == "center")
        return Qt::AlignHCenter;
    else if(a == "justify")
        return Qt::AlignJustify;
    else
        return (Qt::AlignmentFlag)0;
}
