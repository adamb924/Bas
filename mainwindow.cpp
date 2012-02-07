#include "mainwindow.h"
#include "pageview.h"
#include "page.h"
#include "multipagelayout.h"
#include "newbookwidget.h"

#include "edittemplatedialog.h"

#include "book.h"
#include "languagetemplate.h"
#include "layouttemplate.h"
#include "pagetemplate.h"
#include "booktemplate.h"
#include "textitem.h"
#include "textblocklayoutitem.h"

#include <QGraphicsScene>
#include <QGraphicsView>
#include <QSvgGenerator>
#include <QPainter>
#include <QPaintEngine>
#include <QtGui>
#include <QtDebug>
#include <QKeyEvent>
#include <QList>
#include <QtXml>
#include <QtXmlPatterns>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    initialized = true;
    pBook = 0;
    settings = 0;

    pHardwiredLanguage = 0;
    bShowAdvancedPageOptions = false;

    if( !loadLanguageTemplates() )
    {
        initialized = false;
        return;
    }
    if( !loadPageTemplates() )
    {
        initialized = false;
        return;
    }
    if( !loadLayoutTemplates() )
    {
        initialized = false;
        return;
    }
    if( !loadBookTemplates() )
    {
        initialized = false;
        return;
    }

    setupMenus();

    getSettings();


    this->setWindowTitle(tr("Bas"));
}

MainWindow::~MainWindow()
{
    setSettings();
    delete settings;

    qDeleteAll(aPageTemplates.begin(), aPageTemplates.end());
    qDeleteAll(aLayoutTemplates.begin(), aLayoutTemplates.end());
    qDeleteAll(aBookTemplates.begin(), aBookTemplates.end());
    qDeleteAll(aLanguageTemplates.begin(), aLanguageTemplates.end());
}

void MainWindow::closeEvent ( QCloseEvent * event )
{
    savePageTemplates();
    saveLayoutTemplates();
    saveBookTemplates();
    saveLanguageTemplates();

    if( pBook == 0 )
    {
        event->accept();
        return;
    }
    if( promptToSave() )
        event->accept();
    else
        event->ignore();
}

void MainWindow::getSettings()
{
    settings = new QSettings("AdamBaker", "Bas");

    // the hardwired language
    pHardwiredLanguage = languageTemplateFromIso( settings->value("languages/hardwired").toString() );

    // advanced page option
    bShowAdvancedPageOptions = settings->value("editting/advancedpageoptions").toInt();
    allowAdvancedPageOptions->setChecked(bShowAdvancedPageOptions);

    QString defaultGS = "C:/Program Files/gs/gs8.63/bin/gswin32.exe";
    char *pPath;
    pPath = getenv("GS_PROG");
    if( pPath != NULL )
        defaultGS = QString::fromUtf8(pPath);

    // external apps
    strPathToInkscape = settings->value("external/inkscape","C:/Program Files/Inkscape/inkscape.exe").toString();
    strPathToGhostscript = settings->value("external/ghostscript",defaultGS).toString();
}

void MainWindow::setSettings()
{
    if( settings == 0 )
        return;

    // the hardwired language
    if( pHardwiredLanguage != 0 )
        settings->setValue("languages/hardwired",pHardwiredLanguage->iso());
    else
        settings->setValue("languages/hardwired","");

    // advanced page options
    settings->setValue("editting/advancedpageoptions",(int)bShowAdvancedPageOptions);

    // external apps
    settings->setValue("external/inkscape",strPathToInkscape);
    settings->setValue("external/ghostscript",strPathToGhostscript);
}

QFile* MainWindow::openOrCopyFromResource(QString filename)
{
    if( !QFile::exists(filename) )
        return copyFromResource(filename);

    QFile *file = new QFile(filename);
    if(! file->open(QFile::ReadOnly | QFile::Text) )
        return 0;
    else
        return file;
}

QFile* MainWindow::copyFromResource(QString filename)
{
    if( QFile::exists(filename) )
        QFile::remove(filename);

    QFile tmp(QString(":/resources/%1").arg(filename));
    tmp.open(QIODevice::ReadWrite);
    tmp.setPermissions( QFile::ReadOther | QFile::WriteOther );
    if( !tmp.copy(filename) )
    {
        QMessageBox::critical(this,tr("Bas"),tr("The file '%1' cannot be found. Bas cannot work without this file, so it will close now.").arg(filename));
        return false;
    }
    tmp.close();
    QFile tmp2(filename);
    tmp2.setPermissions(QFile::WriteUser);
    tmp2.close();

    QFile *file = new QFile(filename);
    if(! file->open(QFile::ReadOnly | QFile::Text) )
        return 0;
    else
        return file;
}

bool MainWindow::loadPageTemplates()
{
    QFile *file = openOrCopyFromResource("page-templates.xml");
    if( file == 0 )
        return false;
    QXmlStreamReader xml(file);

    while (!xml.atEnd())
    {
	if(xml.readNext() == QXmlStreamReader::StartElement)
        {
	    while( xml.name().toString() == "page-template" )
	    {
                aPageTemplates << new PageTemplate(this);
                aPageTemplates.last()->read(&xml);
	    }
	}
    }
    file->close();

    if( aPageTemplates.count() == 0 )
    {
        copyFromResource("page-templates.xml");
        QMessageBox::critical(this,tr("Bas"),tr("Something seems to be wrong with the file '%1'. I have tried to fix the problem, and if you start Bas again it will probably work.").arg("page-templates.xml"));
        return false;
    }
    else
    {
        return true;
    }
}

bool MainWindow::loadLayoutTemplates()
{
    QFile *file = openOrCopyFromResource("layout-templates.xml");
    if( file == 0 )
        return false;
    QXmlStreamReader xml(file);

    while (!xml.atEnd())
    {
	if(xml.readNext() == QXmlStreamReader::StartElement)
	{
	    while( xml.name().toString() == "layout-template" )
	    {
                aLayoutTemplates << new LayoutTemplate(this);
                aLayoutTemplates.last()->read(&xml);
	    }
	}
    }

    if( aLayoutTemplates.count() == 0 )
    {
        copyFromResource("layout-templates.xml");
        QMessageBox::critical(this,tr("Bas"),tr("Something seems to be wrong with the file '%1'. I have tried to fix the problem, and if you start Bas again it will probably work.").arg("layout-templates.xml"));
        return false;
    }
    else
    {
        return true;
    }
}

bool MainWindow::loadBookTemplates()
{
    QFile *file = openOrCopyFromResource("book-templates.xml");
    if( file == 0 )
        return false;
    QXmlStreamReader xml(file);

    while (!xml.atEnd())
    {
	if(xml.readNext() == QXmlStreamReader::StartElement)
	{
	    while( xml.name().toString() == "book-template" )
	    {
                aBookTemplates << new BookTemplate(this);
                QString returnValue = aBookTemplates.last()->read(&xml);
		if(returnValue != "")
		    QMessageBox::critical(this,tr("Error Message"),tr("There has been an error in reading a book template. The error message is: ") + returnValue );
	    }
	}
    }

    if( aBookTemplates.count() == 0 )
    {
        copyFromResource("book-templates.xml");
        QMessageBox::critical(this,tr("Bas"),tr("Something seems to be wrong with the file '%1'. I have tried to fix the problem, and if you start Bas again it will probably work.").arg("book-templates.xml"));
        return false;
    }
    else
    {
        return true;
    }
}

bool MainWindow::loadLanguageTemplates()
{
    QFile *file = openOrCopyFromResource("language-templates.xml");
    if( file == 0 )
        return false;
    QXmlStreamReader xml(file);

    while (!xml.atEnd())
    {
        if(xml.readNext() == QXmlStreamReader::StartElement)
        {
            while( xml.name().toString() == "language-template" )
            {
                aLanguageTemplates << new LanguageTemplate;
                aLanguageTemplates.last()->read(&xml);
            }
        }
    }

    if( aLanguageTemplates.count() == 0 )
    {
        copyFromResource("language-templates.xml");
        QMessageBox::critical(this,tr("Bas"),tr("Something seems to be wrong with the file '%1'. I have tried to fix the problem, and if you start Bas again it will probably work.").arg("language-templates.xml"));
        return false;
    }
    else
    {
        return true;
    }
}

void MainWindow::savePageTemplates()
{
    QString filename = "page-templates.xml";

    QFile file(filename);
    if( ! file.open( QFile::WriteOnly | QFile::Text ) )
    {
        qDebug() << "Could not open: " << filename;
        return;
    }
    QXmlStreamWriter w(&file);
    w.setCodec("utf-8");
    w.writeStartDocument("1.0");
    w.setAutoFormatting(true);

    w.writeComment("all dimensions are in millimeters");

    w.writeStartElement("page-templates");

    w.writeDefaultNamespace("/bas/page-template");

    for(int i=0; i<aPageTemplates.count(); i++)
        aPageTemplates.at(i)->write(&w);

    w.writeEndElement(); // page-templates
}

void MainWindow::saveLayoutTemplates()
{
    QString filename = "layout-templates.xml";

    QFile file(filename);
    if( ! file.open( QFile::WriteOnly | QFile::Text ) )
    {
        qDebug() << "Could not open: " << filename;
        return;
    }
    QXmlStreamWriter w(&file);
    w.setCodec("utf-8");
    w.writeStartDocument("1.0");
    w.setAutoFormatting(true);

    w.writeStartElement("layout-templates");

    w.writeDefaultNamespace("/bas/layout-template");

    for(int i=0; i<aLayoutTemplates.count(); i++)
        aLayoutTemplates.at(i)->write(&w);

    w.writeEndElement(); // layout-templates
}

void MainWindow::saveBookTemplates()
{
    QString filename = "book-templates.xml";

    QFile file(filename);
    if( ! file.open( QFile::WriteOnly | QFile::Text ) )
    {
        qDebug() << "Could not open: " << filename;
        return;
    }
    QXmlStreamWriter w(&file);
    w.setCodec("utf-8");
    w.writeStartDocument("1.0");
    w.setAutoFormatting(true);

    w.writeStartElement("book-templates");

    w.writeDefaultNamespace("/bas/book-template");

    for(int i=0; i<aBookTemplates.count(); i++)
        aBookTemplates.at(i)->write(&w);

    w.writeEndElement(); // book-templates
}

void MainWindow::saveLanguageTemplates()
{
    QString filename = "language-templates.xml";

    QFile file(filename);
    if( ! file.open( QFile::WriteOnly | QFile::Text ) )
    {
        qDebug() << "Could not open: " << filename;
        return;
    }
    QXmlStreamWriter w(&file);
    w.setCodec("utf-8");
    w.writeStartDocument("1.0");
    w.setAutoFormatting(true);

    w.writeStartElement("language-templates");

    w.writeDefaultNamespace("/bas/language-template");

    for(int i=0; i<aLanguageTemplates.count(); i++)
        aLanguageTemplates.at(i)->write(&w);

    w.writeEndElement(); // language-templates
}

void MainWindow::keyPressEvent( QKeyEvent * event )
{
    /*
    if(event->key() == Qt::Key_A)
	mpl->setZoom( mpl->getZoom() * 0.9 );
    if(event->key() == Qt::Key_S)
	mpl->setZoom( mpl->getZoom() * 1.1 );
*/
}

void MainWindow::setupMenus()
{
    QMenu *file = new QMenu(tr("File"));
    this->menuBar()->addMenu(file);

    file->addAction(QIcon::fromTheme("document-new"),tr("New book from a template"),this,SLOT(newBook()),QKeySequence::New);
    file->addAction(QIcon::fromTheme("document-new"),tr("New book from my own design"),this,SLOT(newBookCustomDesign()));
    file->addAction(QIcon::fromTheme("document-open"),tr("Open..."),this,SLOT(openBook()),QKeySequence::Open);
    file->addAction(QIcon::fromTheme("document-save"),tr("Save"),this,SLOT(saveBook()),QKeySequence::Save);
    file->addAction(QIcon::fromTheme("document-save-as"),tr("Save As..."),this,SLOT(saveBookAs()),QKeySequence::SaveAs);
    file->addAction(QIcon::fromTheme("document-close"),tr("Close"),this,SLOT(closeBook()),QKeySequence::Close);
    file->addAction(QIcon::fromTheme("document-print"),tr("Create PDF..."),this,SLOT(createPDF()),QKeySequence::Print);
    file->addSeparator();
    file->addAction(QIcon::fromTheme("application-close"),tr("Quit"),this,SLOT(close()));

    QMenu *advanced = new QMenu(tr("Options"));
    this->menuBar()->addMenu(advanced);

    advanced->addAction(tr("About this book"),this,SLOT(aboutThisBook()));
    advanced->addSeparator();

    lgMenu = new QMenu(tr("Default language for new books"));
    advanced->addMenu(lgMenu);
    QActionGroup *languageGroup = new QActionGroup(this);
    QAction *none = new QAction(tr("None"),lgMenu);
    none->setData(-1);
    none->setCheckable(true);
    if(pHardwiredLanguage == 0)
        none->setChecked(true);

    languageGroup->addAction(none);
    lgMenu->addAction(none);
    lgMenu->addSeparator();

    for(int i=0; i < aLanguageTemplates.count(); i++)
    {
        QAction *tmp = new QAction( aLanguageTemplates.at(i)->name() , lgMenu );
        tmp->setData(i);
        tmp->setCheckable(true);
        if(pHardwiredLanguage == aLanguageTemplates.at(i))
            tmp->setChecked(true);

        languageGroup->addAction(tmp);
        lgMenu->addAction(tmp);
    }
    connect(languageGroup,SIGNAL(triggered(QAction*)),this,SLOT(selectHardwiredLanguage(QAction*)));

    allowAdvancedPageOptions = new QAction(tr("Show advanced page options"),this);
    allowAdvancedPageOptions->setCheckable(true);
    allowAdvancedPageOptions->setChecked(bShowAdvancedPageOptions);
    advanced->addAction(allowAdvancedPageOptions);

    connect(allowAdvancedPageOptions,SIGNAL(triggered(bool)),this,SLOT(setShowAdvancedPageOptions(bool)));

    QMenu *lgUiMenu = new QMenu(tr("Language of \"Bas\""));
    advanced->addMenu(lgUiMenu);
    QActionGroup *languageUiGroup = new QActionGroup(this);

    QLocale thisLocale;

    QAction *english = new QAction("English",lgUiMenu);
    english->setData("en_US");
    english->setCheckable(true);
    lgUiMenu->addAction(english);
    languageUiGroup->addAction(english);
    if( thisLocale.name() == "en_US" )
        english->setChecked(true);

    QDir dir(":/translations");
    QStringList fileNames = dir.entryList(QStringList("*.qm"), QDir::Files,
                                          QDir::Name);
    QMutableStringListIterator i(fileNames);
    while (i.hasNext())
    {
        i.next();
        i.setValue(dir.filePath(i.value()));

        QTranslator translator;
        translator.load(i.value());
        QString lgName = translator.translate("MainWindow", "English");
        QString locale = translator.translate("MainWindow","en_US");

        QAction *tmp = new QAction(lgName,lgUiMenu);
        tmp->setData(locale);
        tmp->setCheckable(true);
        lgUiMenu->addAction(tmp);
        languageUiGroup->addAction(tmp);
        if( thisLocale.name() == locale )
            tmp->setChecked(true);
    }

    connect(languageUiGroup,SIGNAL(triggered(QAction*)),this,SLOT(selectInterfaceLanguage(QAction*)));

    advanced->addSeparator();

    QMenu *gsMenu = new QMenu(tr("Ghostscript"),advanced);
    QMenu *iMenu = new QMenu(tr("Inkscape"),advanced);
    advanced->addMenu(gsMenu);
    advanced->addMenu(iMenu);

    iMenu->addAction(tr("Set Inkscape path"),this,SLOT(lookForInkscape()));
    iMenu->addAction(tr("Get Inkscape path"),this,SLOT(showInkscapePath()));
    gsMenu->addAction(tr("Set Ghostscript path"),this,SLOT(lookForGhostscript()));
    gsMenu->addAction(tr("Get Ghostscript path"),this,SLOT(showGhostscriptPath()));

    // template editting
    advanced->addSeparator();
    QMenu *templateMenu = new QMenu(tr("Templates"),advanced);
    advanced->addMenu(templateMenu);
/*
    templateMenu->addAction(tr("New Book Templates..."),this,SLOT(newBookTemplate()));
    templateMenu->addAction(tr("New Page Template..."),this,SLOT(newPageTemplate()));
    templateMenu->addSeparator();
*/
    templateMenu->addAction(tr("Edit Book Templates..."),this,SLOT(editBookTemplates()));
    templateMenu->addAction(tr("Edit Language Templates..."),this,SLOT(editLanguageTemplates()));
    templateMenu->addAction(tr("Edit Layout Templates..."),this,SLOT(editLayoutTemplates()));
    templateMenu->addAction(tr("Edit Page Templates..."),this,SLOT(editPageTemplates()));
}

// File Operations

void MainWindow::newBook()
{
//    if( promptToSave() != false )
//        promptToSave();
    closeBook();

    if( centralWidget() != 0 )
        delete centralWidget();

    NewBookWidget *nbw = new NewBookWidget(true, this);

    connect(nbw,SIGNAL(ok()),this,SLOT(newBookOk()));
    connect(nbw,SIGNAL(cancel()),this,SLOT(newBookCancel()));

    setCentralWidget(nbw);
}

void MainWindow::newBookCustomDesign()
{
    closeBook();

    if( centralWidget() != 0 )
        delete centralWidget();

    NewBookWidget *nbw = new NewBookWidget(false, this);

    connect(nbw,SIGNAL(ok()),this,SLOT(newBookOk()));
    connect(nbw,SIGNAL(cancel()),this,SLOT(newBookCancel()));

    setCentralWidget(nbw);
}

void MainWindow::newBookOk()
{
    NewBookWidget *tmp = qobject_cast<NewBookWidget*>(centralWidget() );
    if(tmp == 0)
        return;
    if( pBook != 0 )
    {
        delete pBook;
        pBook = 0;
    }

    if( tmp->fromTemplate() )
    {
        pBook = new Book( tmp->bookTemplate() , tmp->languageTemplate(), tmp->fontFamily(), strPathToInkscape, strPathToGhostscript, this );
    }
    else
    {
        pBook = new Book( tmp->pageTemplate() , tmp->languageTemplate(), tmp->fontFamily(), strPathToInkscape, strPathToGhostscript, this );

        QList<LayoutTemplate*> lt = getLayoutTemplatesForPageTemplate(tmp->pageTemplate());
        QStringList templates;
        for(int i=0; i < lt.count(); i++ )
            templates << lt.at(i)->longName();
        QString pt = QInputDialog::getItem( this, tr("Bas"), tr("Select a template for the first page"), templates, 0, false );
        if( layoutTemplateFromLongName(pt,tmp->pageTemplate()->name()) != 0 )
            pBook->addPage( 0, layoutTemplateFromLongName(pt,tmp->pageTemplate()->name()) );
    }

    createBookDisplay();
}

void MainWindow::newBookCancel()
{
    NewBookWidget *tmp = qobject_cast<NewBookWidget*>(centralWidget() );
    if(tmp == 0)
        return;
    delete centralWidget();
}

void MainWindow::openBook()
{
//    if( promptToSave() != false )
//        promptToSave();
    closeBook();

    QString fileName = QFileDialog::getOpenFileName(this, tr("Open"),
						    "",
                                                    tr("Books (*.ktb)"));
    if(fileName.length() != 0)
    {
        pBook = new Book(fileName, strPathToInkscape, strPathToGhostscript, this);
        if( pBook->valid() )
        {
            delete centralWidget();
            createBookDisplay();
        }
    }
}


void MainWindow::saveBook()
{
    if(pBook == 0)
        return;
    if(pBook->filename().length() == 0)
        saveBookAs();
    else
        pBook->writeToFile(pBook->filename());
}

void MainWindow::saveBookAs()
{
    if(pBook == 0)
        return;

    QString fileName = QFileDialog::getSaveFileName(this, tr("Save"),
                                                    pBook->filename(),
                                                    tr("Books (*.ktb)"));
    if( fileName.length() != 0 )
        pBook->writeToFile(fileName);
}

void MainWindow::closeBook()
{
    if( promptToSave() != false )
    {
        delete centralWidget();
        pBook = 0;
    }
}

bool MainWindow::promptToSave()
{
    if( pBook == 0 )
        return false;

    if( pBook->changed() )
    {
        QMessageBox::StandardButton response = QMessageBox::question( this, tr("Bas"), tr("The book has changed. Do you want to save it before closing it?"), QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel, QMessageBox::Yes );
        if( response == QMessageBox::Cancel )
            return false;
        else if( response == QMessageBox::Yes )
            saveBook();
    }
    return true;
}

void MainWindow::createPDF()
{
    if( pBook == 0 )
        return;

    if(!requireExternalApps())
        return;

    QString filename = QFileDialog::getSaveFileName(this, tr("Save"),
                                                    "",
                                                    tr("PDF Files (*.pdf)"));
    if( filename.length() != 0 )
        pBook->createPdf(filename);
}

PageTemplate* MainWindow::pageTemplateFromName(QString name)
{
    for(int i=0; i<aPageTemplates.count(); i++)
        if( aPageTemplates.at(i)->name() == name )
            return aPageTemplates.at(i);
    return 0;
}

LayoutTemplate* MainWindow::layoutTemplateFromName(QString name, QString pagelayoutName)
{
    for(int i=0; i<aLayoutTemplates.count(); i++)
        if( aLayoutTemplates.at(i)->name() == name && aLayoutTemplates.at(i)->pageTemplate()->name() == pagelayoutName )
            return aLayoutTemplates.at(i);
    return 0;
}

LayoutTemplate* MainWindow::layoutTemplateFromLongName(QString longName, QString pagelayoutName)
{
    for(int i=0; i<aLayoutTemplates.count(); i++)
        if( aLayoutTemplates.at(i)->longName() == longName && aLayoutTemplates.at(i)->pageTemplate()->name() == pagelayoutName )
            return aLayoutTemplates.at(i);
    return 0;
}

BookTemplate* MainWindow::bookTemplateFromName(QString name)
{
    for(int i=0; i<aBookTemplates.count(); i++)
        if( aBookTemplates.at(i)->name() == name )
            return aBookTemplates.at(i);
    return 0;
}

LanguageTemplate* MainWindow::languageTemplateFromIso(QString iso)
{
    for(int i=0; i<aLanguageTemplates.count(); i++)
        if( aLanguageTemplates.at(i)->iso() == iso )
            return aLanguageTemplates.at(i);
    return 0;
}

QList<LayoutTemplate*> MainWindow::getCompatibleLayoutTemplates(LayoutTemplate *first)
{
    QList<LayoutTemplate*> resultList;
    for(int i=0; i<aLayoutTemplates.count(); i++)
        if( first->compatibleWith( *(aLayoutTemplates.at(i)) ) )
            resultList << aLayoutTemplates.at(i);
    return resultList;
}

QList<LayoutTemplate*> MainWindow::getLayoutTemplatesForPageTemplate(PageTemplate *pt)
{
    QList<LayoutTemplate*> resultList;
    for(int i=0; i<aLayoutTemplates.count(); i++)
        if( aLayoutTemplates.at(i)->pageTemplate() == pt )
            resultList << aLayoutTemplates.at(i);
    return resultList;
}

void MainWindow::selectInterfaceLanguage(QAction *action)
{
    settings->setValue("interface/locale",action->data().toString());
    QMessageBox::information(this,tr("Bas"),tr("The language will change the next time you start Bas."));
}

void MainWindow::selectHardwiredLanguage(QAction *action)
{
    int index = action->data().toInt();
    if( index < 0 || index >= aLanguageTemplates.count() )
        pHardwiredLanguage = 0;
    else
        pHardwiredLanguage = aLanguageTemplates.at(index);
}

bool MainWindow::requireExternalApps()
{
    if( QFile::exists(strPathToInkscape) && QFile::exists(strPathToGhostscript) )
        return true;

    if( !QFile::exists(strPathToInkscape) )
    {
        if( QMessageBox::information(this,tr("Bas"),tr("I can't find Inkscape, which is needed for creating PDF files. Click OK to look for this file yourself."),QMessageBox::Ok|QMessageBox::Cancel,QMessageBox::Ok) == QMessageBox::Ok )
            lookForInkscape();
        if( !strPathToInkscape.isEmpty() && pBook != 0 )
        {
            pBook->setPathToInkscape(strPathToInkscape);
            return true;
        }
        else
        {
            return false;
        }
    }

    if( !QFile::exists(strPathToGhostscript) )
    {
        if( QMessageBox::information(this,tr("Bas"),tr("I can't find Ghostscript, which is needed for creating PDF files. Click OK to look for this file yourself."),QMessageBox::Ok|QMessageBox::Cancel,QMessageBox::Ok) == QMessageBox::Ok )
            lookForGhostscript();
        if( !strPathToGhostscript.isEmpty() && pBook != 0 )
        {
            pBook->setPathToGhostscript(strPathToGhostscript);
            return true;
        }
        else
        {
            return false;
        }
    }

    return true;
}

void MainWindow::lookForInkscape()
{
    strPathToInkscape = QFileDialog::getOpenFileName(this,tr("Please locate Inkscape"));
}

void MainWindow::lookForGhostscript()
{
    QString pathGS = "";
    char *pPath;
    pPath = getenv("GS_PROG");
    if( pPath != NULL )
    {
        pathGS = QString::fromUtf8(pPath);
        if( QFile::exists(pathGS) )
            if( QMessageBox::question(this,tr("Bas"),tr("It seems that Ghostscript is installed at %1. Would you like to try that location? If this is your first time seeing this message, you should click 'Yes'.").arg(pathGS),QMessageBox::Yes|QMessageBox::No,QMessageBox::Yes) == QMessageBox::Yes )
            {
            strPathToGhostscript = pathGS;
            return;
        }
    }
    strPathToGhostscript = QFileDialog::getOpenFileName(this,tr("Please locate Ghostscript"));
}

void MainWindow::showInkscapePath()
{
    QMessageBox::information(this,tr("Bas"),strPathToInkscape);
}

void MainWindow::showGhostscriptPath()
{
    QMessageBox::information(this,tr("Bas"),strPathToGhostscript);
}

void MainWindow::createBookDisplay()
{
    if( pBook == 0 )
        return;

    MultiPageLayout *mpl = new MultiPageLayout(pBook,this);

    mpl->setZoom(1);

    // the hassle with the extra widgets here is to get the proper scrolling behavior
    QHBoxLayout *hlayout = new QHBoxLayout;
    hlayout->addLayout(mpl,0);

    QWidget *cw = new QWidget(this);
    cw->setLayout(hlayout);
    cw->setSizePolicy(QSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding));

    QScrollArea *sa = new QScrollArea;
    sa->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    sa->setWidgetResizable(true);
    sa->setWidget(cw);

    setCentralWidget(sa);
}

void MainWindow::editBookTemplates()
{
    QList<AbstractTemplate*> list;
    for(int i=0; i<aBookTemplates.count(); i++)
        list << aBookTemplates.at(i);

    EditTemplateDialog dlg(&list,&MainWindow::newBookTemplate,this);
    dlg.setWindowTitle(tr("Edit Book Templates"));
    dlg.exec();

    aBookTemplates.clear();
    for(int i=0; i<list.count(); i++)
        aBookTemplates.append( (BookTemplate*)list.at(i) );
}

void MainWindow::editLanguageTemplates()
{
    QList<AbstractTemplate*> list;
    for(int i=0; i<aLanguageTemplates.count(); i++)
        list << aLanguageTemplates.at(i);
    EditTemplateDialog dlg(&list,&MainWindow::newLanguageTemplate,this);
    dlg.setWindowTitle(tr("Edit Language Templates"));
    dlg.exec();

    aLanguageTemplates.clear();
    for(int i=0; i<list.count(); i++)
        aLanguageTemplates.append( (LanguageTemplate*)list.at(i) );
}

void MainWindow::editLayoutTemplates()
{
    QList<AbstractTemplate*> list;
    for(int i=0; i<aLayoutTemplates.count(); i++)
        list << aLayoutTemplates.at(i);
    EditTemplateDialog dlg(&list,&MainWindow::newLayoutTemplate,this);
    dlg.setWindowTitle(tr("Edit Page Layout Templates"));
    dlg.exec();
    if(pBook)
        pBook->updateLayoutItemFormats();

    aLayoutTemplates.clear();
    for(int i=0; i<list.count(); i++)
        aLayoutTemplates.append( (LayoutTemplate*)list.at(i) );
}

void MainWindow::editPageTemplates()
{
    QList<AbstractTemplate*> list;
    for(int i=0; i<aPageTemplates.count(); i++)
        list << aPageTemplates.at(i);
    EditTemplateDialog dlg(&list,&MainWindow::newPageTemplate,this);
    dlg.setWindowTitle(tr("Edit Page Templates"));
    dlg.exec();
    if(pBook)
    {
        pBook->updateLayoutItemFormats();
        MultiPageLayout* mpl = qobject_cast<MultiPageLayout*>(centralWidget());
        if( mpl != 0)
            mpl->updatePages();
    }

    aPageTemplates.clear();
    for(int i=0; i<list.count(); i++)
        aPageTemplates.append( (PageTemplate*)list.at(i) );
}

void MainWindow::aboutThisBook()
{
    if(pBook == 0)
        return;

    QGridLayout *grdLayout = new QGridLayout;
    if(pBook->templateMode() == Book::ConformityMode)
    {
        grdLayout->addWidget(new QLabel(tr("Book template:")),1,1,Qt::AlignLeft);
        grdLayout->addWidget(new QLabel(pBook->bookTemplate()->name()),1,2,Qt::AlignLeft);
        grdLayout->setRowMinimumHeight(2,10);
    }
    grdLayout->addWidget(new QLabel(tr("Language:")),3,1,Qt::AlignLeft);
    grdLayout->addWidget(new QLabel(pBook->languageTemplate()->name()),3,2,Qt::AlignLeft);

    QDialog dlg;
    dlg.setLayout(grdLayout);
    dlg.exec();
}

void MainWindow::setShowAdvancedPageOptions(bool allow)
{
    bShowAdvancedPageOptions = allow;
    MultiPageLayout *mpl = qobject_cast<MultiPageLayout*>(centralWidget());
    if(mpl == 0)
        return;
    mpl->updatePages();
}

AbstractTemplate* MainWindow::newPageTemplate()
{
    aPageTemplates.append(new PageTemplate(this));
    return aPageTemplates.last();
}

AbstractTemplate* MainWindow::newBookTemplate()
{
    aBookTemplates.append(new BookTemplate(this));
    return aBookTemplates.last();
}

AbstractTemplate* MainWindow::newLanguageTemplate()
{
    aLanguageTemplates.append(new LanguageTemplate());
    return aLanguageTemplates.last();
}

AbstractTemplate* MainWindow::newLayoutTemplate()
{
    aLayoutTemplates.append(new LayoutTemplate(this));
    return aLayoutTemplates.last();
}
