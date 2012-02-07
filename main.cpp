#include <QtGui/QApplication>
#include <QTextCodec>
#include <QLibraryInfo>
#include <QTranslator>
#include <QLocale>
#include <QDebug>
#include <QSettings>
#include <QDir>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8"));

    QApplication a(argc, argv);

    // load the QLocale from the settings file
    QSettings *settings = new QSettings("AdamBaker", "Bas");

    if( settings->value("interface/locale").isNull() )
        QLocale::setDefault( QLocale::system() );
    else
        QLocale::setDefault( QLocale( settings->value("interface/locale").toString() ) );

    QLocale thisLocale; // this should be initialized now either to the system default or to whatever was saved in the QSettings
    QTranslator myappTranslator;
    myappTranslator.load(":/translations/bas_" + thisLocale.name());
    a.installTranslator(&myappTranslator);

    a.setLayoutDirection( thisLocale.textDirection() );

    MainWindow w;
    if( w.initialized )
    {
        w.show();
        return a.exec();
    }
    else
    {
        return 1;
    }
}
