#include "numberedit.h"

#include <QDoubleValidator>

NumberEdit::NumberEdit(double initial, QWidget *parent) :
    QLineEdit(parent)
{
    QDoubleValidator *v = new QDoubleValidator;
    this->setValidator(v);

    this->setText(QString::number(initial));

    connect(this, SIGNAL(textChanged(QString)), this, SLOT(calculateValue()));
}

void NumberEdit::calculateValue()
{
    emit valueChanged(text().toDouble());
}
