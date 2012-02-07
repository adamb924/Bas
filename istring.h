#ifndef ISTRING_H
#define ISTRING_H

class IString {
public:
    IString(QString l, QString s)
    {
        lang = l;
        string = s;
    }
    IString(const IString& other)
    {
        lang = other.lang;
        string = other.string;
    }
    QString lang;
    QString string;
};

#endif // ISTRING_H
