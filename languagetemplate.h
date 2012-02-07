#ifndef LANGUAGETEMPLATE_H
#define LANGUAGETEMPLATE_H

#include <QString>

#include "abstracttemplate.h"

class QXmlStreamReader;
class QXmlStreamWriter;

class LanguageTemplate : public AbstractTemplate
{
    Q_OBJECT
public:
    enum TextDirection { RTL, LTR };

    LanguageTemplate();

    void read(QXmlStreamReader *xml);
    void write(QXmlStreamWriter *xml);

    QWidget* createSettingsPage();

    inline QString iso() const { return strIso; }
    inline QString defaultFont() const { return strDefaultFont; }
    inline TextDirection textDirection() const { return eTextDirection; }

public slots:
    inline void setIso(QString s) { strIso = s; }
    inline void setDefaultFont(QString s) { strDefaultFont = s; }
    inline void setTextDirection(TextDirection d) { eTextDirection = d; }
    void setTextDirectionFromIndex(int i);

private:
    QString strIso, strDefaultFont;
    TextDirection eTextDirection;
};

#endif // LANGUAGETEMPLATE_H
