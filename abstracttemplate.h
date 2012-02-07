#ifndef ABSTRACTTEMPLATE_H
#define ABSTRACTTEMPLATE_H

#include <QObject>
#include "istring.h"

class AbstractTemplate : public QObject
{
    Q_OBJECT
public:
    explicit AbstractTemplate(QObject *parent = 0) {}

    virtual QWidget* createSettingsPage() = 0;

    inline QString name() const { return strName; }

signals:
    void nameChanged(QString name);
    void redoWidget();

public slots:
    inline void setName(const QString& s) { strName = s; emit nameChanged(strName); }

protected:
    QString strName;
    QList<IString> aOtherNames;
    QList<IString> aOtherLongNames;
};

#endif // ABSTRACTTEMPLATE_H
