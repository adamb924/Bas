#ifndef TEXTBLOCKLAYOUTITEM_H
#define TEXTBLOCKLAYOUTITEM_H

#include <QObject>

#include <QtGlobal>
#include <QString>

#include <QList>
class QFont;
#include "istring.h"

class TextBlockLayoutItem
{
public:
    TextBlockLayoutItem(qreal vp, QString s);

    qreal y() const { return verticalPosition; }
    QString style() const { return itemstyle; }
    inline QString defaultText() const { return strDefaultText; }

    void setY(qreal y) { verticalPosition = y; }
    void setStyle(QString s) { itemstyle = s; }
    inline void setDefaultText(QString defaultText) { strDefaultText = defaultText; }
    inline QList<IString>* otherDefaultTexts() { return &aOtherDefaultTexts; }

    Qt::AlignmentFlag alignment() const;
    QString strippedStyle() const;

    int fontSize() const;

private:
    qreal verticalPosition;
    QString itemstyle;
    QString strDefaultText;
    QList<IString> aOtherDefaultTexts;
};

#endif // TEXTBLOCKLAYOUTITEM_H
