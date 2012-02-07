#ifndef EDITTEMPLATEDIALOG_H
#define EDITTEMPLATEDIALOG_H

#include <QDialog>
#include <QList>

#include <QDebug>

class AbstractTemplate;
class QListWidget;
class QStackedWidget;

#include "mainwindow.h"

class EditTemplateDialog : public QDialog
{
    Q_OBJECT
public:
    explicit EditTemplateDialog(QList<AbstractTemplate*> *templates, NewTemplateMethod nt, MainWindow *parent);

signals:

private:
    QList<AbstractTemplate*> *aTemplates;

    QListWidget *list;
    QStackedWidget *stack;

    MainWindow *pMW;

    // pointer to the new widget thing
    NewTemplateMethod newTemplate;

    void createLayout();

private slots:
    void setCurrentText(QString string);
    void redoCurrentWidget();

    void add();
    void remove();

};

#endif // LANGUAGETEMPLATEDIALOG_H
