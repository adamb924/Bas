#ifndef NUMBEREDIT_H
#define NUMBEREDIT_H

#include <QLineEdit>

class NumberEdit : public QLineEdit
{
    Q_OBJECT
public:
    explicit NumberEdit(double initial, QWidget *parent = 0);

    inline double value() const { return text().toDouble(); }

signals:
    void valueChanged(double value);

public slots:

private slots:
    void calculateValue();

};

#endif // NUMBEREDIT_H
