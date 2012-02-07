#include "book.h"
#include "page.h"
#include "pagetemplate.h"
#include "layouttemplate.h"
#include "mainwindow.h"
#include "booktemplate.h"

#include <QtXml>
#include <QtGui>
#include <QCoreApplication>
#include <QSvgGenerator>
#include <QProcess>

Book::Book(QString fileName, QString inkscape, QString ghostscript, MainWindow *parent)
{
    setValid(true);

    strFilename = fileName;

    setPathToInkscape(inkscape);
    setPathToGhostscript(ghostscript);

    this->pMainWindow = parent;
    this->bkTemplate = 0;
    if(!readFromFile(fileName))
        setValid(false);
    setChanged(false);
}

Book::Book(BookTemplate *bookTemplate, LanguageTemplate *lgtemplate, QString fontfamily, QString inkscape, QString ghostscript, MainWindow *parent)
{
    setValid(true);

    strFilename = "";

    setPathToInkscape(inkscape);
    setPathToGhostscript(ghostscript);

    setFontFamily(fontfamily);

    this->pMainWindow = parent;
    this->bkTemplate = bookTemplate;
    this->lgTemplate = lgtemplate;

    // this is sort of redundant since there's a page count
    // we just need to decide about numbering front and back matter
    int pageCount = 0;

    // add the front matter
    for(int i=0; i<bookTemplate->frontMatter()->count(); i++)
    {
        Page *newPage = new Page( this , bookTemplate->frontMatter()->at(i), getPageSideFromPageNumber(pageCount), lgTemplate );
	newPage->populateFromTemplate();
	aPages << newPage;
	pageCount++;
    }

    // add the middle matter
    for(int i=0; i<bookTemplate->middleMatter()->count(); i++)
    {
        Page *newPage = new Page( this , bookTemplate->middleMatter()->at(i), getPageSideFromPageNumber(pageCount), lgTemplate );
	newPage->populateFromTemplate();
	aPages << newPage;
	pageCount++;
    }

    // add the back matter
    for(int i=0; i<bookTemplate->backMatter()->count(); i++)
    {
        Page *newPage = new Page( this , bookTemplate->backMatter()->at(i), getPageSideFromPageNumber(pageCount), lgTemplate );
	newPage->populateFromTemplate();
	aPages << newPage;
	pageCount++;
    }

    pPageTemplate = bookTemplate->pageTemplate();
    eTemplateMode = ConformityMode;

    setChanged(false);
}
Book::Book(PageTemplate *pageTemplate, LanguageTemplate *lgtemplate, QString fontfamily, QString inkscape, QString ghostscript, MainWindow *parent)
{
    setValid(true);

    pPageTemplate = pageTemplate;

    strFilename = "";

    setPathToInkscape(inkscape);
    setPathToGhostscript(ghostscript);

    setFontFamily(fontfamily);

    this->pMainWindow = parent;
    this->bkTemplate = 0;
    this->lgTemplate = lgtemplate;

    eTemplateMode = BreakoutMode;

    setChanged(false);
}

Book::~Book()
{
    qDeleteAll(aPages.begin(), aPages.end());
}

void Book::createPdf(QString filename)
{
    QStringList pdfFilenames;
    QDir dir;

    if( QFile::exists( filename ) )
        QFile::remove( filename );

    // this gets set somehow
    //    PrintMode printMode = Book::Single;
    PrintMode printMode = Book::Duplex;

    filename = dir.absoluteFilePath(filename);

    QString tmpDirName;
    do {
        tmpDirName = QString("tmp%1").arg( QTime::currentTime().toString("hhmmsszzz") );
    }
    while( dir.exists(tmpDirName) );

    if(!dir.mkdir(tmpDirName))
    {
        qDebug() << "Could not make directory:" << tmpDirName;
        return;
    }
    dir.cd(tmpDirName);


    if( printMode == Book::Single )
    {
        QProgressDialog progress(QCoreApplication::translate("Book","Saving pages..."), QCoreApplication::translate("Book","Cancel"), 0, pages()->count() );
        progress.setWindowModality(Qt::WindowModal);
        progress.setValue(1);
        for(int i=0; i<pages()->count(); i++)
        {
            progress.setValue(i + 1);

            QString filename = dir.absoluteFilePath(QString("page%1.svg").arg(i+1));
            createSinglePage( pages()->at(i) , filename );
            if( convertSvgToPdf( filename ) == false )
            {
                QMessageBox::critical(0,QCoreApplication::translate("Book","Bas"),QCoreApplication::translate("Book","There was an error trying to create the PDF file. (The call to Inkscape failed.) Sorry!"));
                qDebug() << "About to return...";
                return;
            }
            pdfFilenames << filename;
            pdfFilenames.last().replace(QRegExp("\\.svg$"),".pdf");
        }
    }
    else if( printMode == Book::Duplex )
    {
        int nOutputPages = nextMultipleOfN( pages()->count() , 4 );
        int fillerDifference = nOutputPages - pages()->count();

        BookTemplate::Position fillerPosition;
        int fillerBefore;
        if(bkTemplate != 0)
        {
            bkTemplate->fillerPagesBefore(fillerPosition, fillerBefore);
        }
        else
        {
            fillerPosition = BookTemplate::FrontMatter;
            fillerBefore = aPages.count();
        }

        QList<Page*> aOutputPages;

        for(int i=0; i < aPages.count(); i++)
        {
            int thisIndex = i;
            BookTemplate::Position thisPagePosition = BookTemplate::FrontMatter;
            if(bkTemplate != 0 )
            {
                bkTemplate->positionOfPage(i,aPages.count());
                bkTemplate->relativeIndexOfPage( i , aPages.count() );
            }

            // not happy with this; can't think of another way
            if( thisPagePosition == BookTemplate::FrontMatter || thisPagePosition == BookTemplate::BackMatter )
                if(thisPagePosition == fillerPosition && thisIndex == fillerBefore)
            {
                for(int j=0; j < fillerDifference; j++ )
                    aOutputPages << new Page(this, pMainWindow->layoutTemplateFromName("blank",aPages.at(i)->pageTemplate()->name()), Book::Left, lgTemplate);

            }

            aOutputPages << aPages.at(i);
        }
        // if nothing else, add in the extra pages at the end
        while( aOutputPages.count() < nOutputPages )
            aOutputPages << new Page(this, pMainWindow->layoutTemplateFromName("blank",aPages.last()->pageTemplate()->name()), Book::Left, lgTemplate);

        QProgressDialog progress(QCoreApplication::translate("Book","Saving pages..."), QCoreApplication::translate("Book","Cancel"), 0, aOutputPages.count() );
        progress.setWindowModality(Qt::WindowModal);
        progress.setValue(1);
        for(int i=0; i < aOutputPages.count()/4; i++ )
        {
            progress.setValue( 4*i + 1 );

            int leftPage, rightPage;
            int n = aOutputPages.count();

            // these left/right interpretations are for RTL layout books
            // also they are 1-indexed

            // OBVERSE

            leftPage = 1 + 2*i;
            rightPage = n - 2*i;
            //            qDebug() << "obverse" << leftPage << rightPage;
            leftPage--; rightPage--; // now they are zero-indexed

            QString filename = dir.absoluteFilePath(QString("page-%1-%2.svg").arg(leftPage+1).arg(rightPage+1));
            if( lgTemplate->textDirection() == LanguageTemplate::RTL )
                createDuplexPage( aOutputPages.at(leftPage), aOutputPages.at(rightPage), filename);
            else if( lgTemplate->textDirection() == LanguageTemplate::LTR )
                createDuplexPage( aOutputPages.at(rightPage), aOutputPages.at(leftPage), filename);
            if( convertSvgToPdf( filename ) == false )
            {
                QMessageBox::critical(0,QCoreApplication::translate("Book","Bas"),QCoreApplication::translate("Book","There was an error trying to create the PDF file. (The call to Inkscape failed.) Sorry!"));
                return;
            }
            pdfFilenames << filename;
            pdfFilenames.last().replace(QRegExp("\\.svg$"),".pdf");

            // REVERSE

            leftPage = n - 1 - 2*i;
            rightPage = 2 + 2*i;
            //            qDebug() << "reverse" << leftPage << rightPage;
            leftPage--; rightPage--; // now they are zero-indexed

            filename = dir.absoluteFilePath(QString("page-%1-%2.svg").arg(leftPage+1).arg(rightPage+1));
            if( lgTemplate->textDirection() == LanguageTemplate::RTL )
                createDuplexPage( aOutputPages.at(leftPage), aOutputPages.at(rightPage), filename, true);
            else if( lgTemplate->textDirection() == LanguageTemplate::LTR )
                createDuplexPage( aOutputPages.at(rightPage), aOutputPages.at(leftPage), filename, true);

            if( convertSvgToPdf( filename ) == false )
            {
                QMessageBox::critical(0,QCoreApplication::translate("Book","Bas"),QCoreApplication::translate("Book","There was an error trying to create the PDF file. (The call to Inkscape failed.) Sorry!"));
                return;
            }
            pdfFilenames << filename;
            pdfFilenames.last().replace(QRegExp("\\.svg$"),".pdf");
        }
    }

    //    return;

    // concatenate the pages
    QProcess *myProcess = new QProcess;
    myProcess->setStandardErrorFile("concatenation-output.txt");
    QStringList arguments;
    // gs -dBATCH -dNOPAUSE -q -sDEVICE=pdfwrite -sOutputFile=finished.pdffile1.pdf file2.pdf
    arguments << "-dBATCH" << "-dNOPAUSE" << "-q" << "-sDEVICE=pdfwrite" << QString("-sOutputFile=%1").arg(filename);
    arguments.append(pdfFilenames);

    myProcess->start(strPathToGhostscript, arguments);

    if( !myProcess->waitForFinished() )
    {
        qDebug() << "Done waiting for return of gswin32";
        qDebug() << myProcess->errorString();
        return;
    }
    if( myProcess->exitCode() )
    {
        qDebug() << "gs exit code: " << myProcess->exitCode() << myProcess->errorString();
    }
    else
        QMessageBox::information(0,QApplication::tr("Bas"),QApplication::tr("The PDF file has been created at %1").arg(filename));
    delete myProcess;

    if( !QFile::exists(filename) )
    {
        QMessageBox::critical(0,QApplication::tr("Bas"),QApplication::tr("There was an error and the PDF file was not created.\n\nThis is probably a problem with Ghostscript."));
    }

    // clean up
    QStringList files = dir.entryList();
    for(int i=0; i<files.count(); i++)
    {
//        QFile::remove( files.at(i) );
        QFile f(dir.absoluteFilePath(files.at(i)));
        f.open(QIODevice::WriteOnly);
        f.setPermissions(QFile::ReadOther|QFile::WriteOther|QFile::ExeOther);
//        qDebug() << files.at(i) << f.remove();
    }
    dir.cdUp();
    dir.rmdir(tmpDirName);
}

int Book::nextMultipleOfN( int number, int multiple )
{
    while( number % multiple != 0 )
        number++;
    return number;
}

bool Book::convertSvgToPdf(const QString& filename) const
{
    bool bReturnValue = true;

    QString pdfFilename = filename;
    pdfFilename.replace(QRegExp("\\.svg$"),".pdf");
    QProcess *convertProcess = new QProcess;
    convertProcess->setStandardErrorFile("inkscape-output");
    QStringList arguments;
    arguments << "-A" << pdfFilename << filename;
    convertProcess->start(strPathToInkscape, arguments);
    if( !convertProcess->waitForFinished() )
    {
        qDebug() << "Book::convertSvgToPdf Done waiting for return" << convertProcess->exitCode() << convertProcess->exitStatus();
        bReturnValue = false;
    }
    if( convertProcess->exitCode() != 0 )
        bReturnValue = false;
    delete convertProcess;
    return bReturnValue;
}

void Book::createSinglePage( Page *p , const QString& filename )
{
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

    w.writeStartElement("svg");
    w.writeDefaultNamespace("http://www.w3.org/2000/svg");
    w.writeNamespace ( "http://www.w3.org/1999/xlink", "xlink" );
    w.writeAttribute("width",QString("%1mm").arg( p->layoutTemplate()->pageTemplate()->width() ));
    w.writeAttribute("height",QString("%1mm").arg( p->layoutTemplate()->pageTemplate()->height() ));
    w.writeAttribute("version","1.1");

    if( !p->toSvg( w ) )
        qDebug() << "Failed to save page";

    w.writeEndElement(); // svg
}

void Book::createDuplexPage( Page *left, Page *right, const QString& filename , bool flip )
{
    int width = 297;
    int height = 210;

    qreal horizontalOffset = 3.543307 * left->pageTemplate()->width();
    qreal verticalOffset = 0;

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

    w.writeStartElement("svg");
    w.writeDefaultNamespace("http://www.w3.org/2000/svg");
    w.writeNamespace ( "http://www.w3.org/1999/xlink", "xlink" );
    w.writeAttribute("width",QString("%1mm").arg( width ));
    w.writeAttribute("height",QString("%1mm").arg( height ));
    w.writeAttribute("version","1.1");

    if( flip )
    {
        w.writeStartElement("g");
        w.writeAttribute("transform",QString("rotate(180,%1,%2)").arg((width*3.543307)/2.0).arg((height*3.543307)/2.0) );
    }

    if( !left->toSvg( w ) )
        qDebug() << "Failed to save left page";
    if( !right->toSvg( w , QPoint( horizontalOffset , verticalOffset ) ) )
        qDebug() << "Failed to save right page";

    if( flip )
        w.writeEndElement();; // g for flipping


    w.writeEndElement(); // svg

}

bool Book::readFromFile(QString fileName)
{
    QFileInfo info(fileName);
    if(!info.exists())
    {
	QMessageBox::critical(0,"Error","The file "+fileName+" does not exist.");
        return false;
    }

    QFile file(fileName);
    if(! file.open(QFile::ReadOnly | QFile::Text) ) { QMessageBox::critical(0,"Error","The file "+fileName+" could not be opened."); return false; }
    QXmlStreamReader xml(&file);

    xml.readNextStartElement();

    if( xml.name().toString() == "book")
    {
        QXmlStreamAttributes attr;
        attr = xml.attributes();
        if( ! attr.value("language-template").isNull() )
        {
            this->lgTemplate = pMainWindow->languageTemplateFromIso( attr.value("language-template").toString() );

            strFontFamily = lgTemplate->defaultFont();
        }
        if( ! attr.value("book-template").isNull() )
        {
            bkTemplate = pMainWindow->bookTemplateFromName( attr.value("book-template").toString() );
            if( bkTemplate == 0 )
            {
                qDebug() << "Book::readFromFile Null book-template. Looking for: " << attr.value("book-template").toString();
                return false;
            }
        }
    }
    else
    {
        QMessageBox::critical(0,QCoreApplication::translate("Book","Bas"),QCoreApplication::translate("Book","That is not a proper book file."));
        return false;
    }


    while (!xml.atEnd())
    {
	if(xml.readNext() == QXmlStreamReader::StartElement)
	{
	    QString name = xml.name().toString();

            //            qDebug() << "Book::readFromFile" << name;

            if( name == "page")
	    {
		int pageIndex = aPages.count() + 1; // plus one because we haven't added it yet

                Book::Side theside = getPageSideFromPageNumber(pageIndex);

		LayoutTemplate *tmpLT = 0;

		QXmlStreamAttributes attr = xml.attributes();
		if( !attr.value("layout-template").isNull() )
                    tmpLT = pMainWindow->layoutTemplateFromName(attr.value("layout-template").toString(), pageTemplate()->name() );
		if( tmpLT == 0 )
                    qDebug() << "Layout template not found: " << attr.value("layout-template").toString() << pageTemplate()->name();

		if( tmpLT != 0 )
		{
                    Page *page = new Page( this , tmpLT, theside, lgTemplate );
		    aPages << page;
		    page->read(&xml);
		}
	    }
	}
    }
    return true;
}

Book::Side Book::getPageSideFromPageNumber(int p) const
{
    if( lgTemplate->textDirection() == LanguageTemplate::RTL )
	if( p % 2 )
	    return Book::Right;
    else
	return Book::Left;

    else // LTR
	if( p % 2 )
	    return Book::Left;
    else
	return Book::Right;
}

bool Book::addPage(int position)
{
    if( bkTemplate->canAddPageAfter(position, aPages.count() ))
    {
	LayoutTemplate *tmp = bkTemplate->layoutTemplateForNewPageAt(position);
	if( tmp != 0 )
	{
            aPages.insert(position + 1, new Page( this , tmp, Book::Left, lgTemplate ));
	    aPages.at(position + 1)->populateFromTemplate();

	    // this isn't always an appropriate call, but the function checks whether it is appropriate or not
            reflowMiddleMatter();

            setChanged(true);

	    return true;
	}
	else
	{
	    return false;
	}
    }
    else
    {
	return false;
    }
}

bool Book::addPage(int position, LayoutTemplate *layoutTemplate)
{
    if( templateMode() == BreakoutMode )
    {
        if( aPages.count() == 0 )
            position = -1;

        aPages.insert(position+1, new Page( this , layoutTemplate, Book::Left, lgTemplate ) );
        aPages.at(position+1)->populateFromTemplate();

        setChanged(true);
        return true;
    }
    else
    {
        return false;
    }
}

bool Book::removePage(int position)
{
    if( templateMode() == BreakoutMode && position < aPages.count() )
    {
        delete aPages.takeAt(position);
        setChanged(true);
        return true;
    }

    if( bkTemplate->canRemovePageAt(position, aPages.count() ))
    {
	// perhaps some further checking here for template conformity?

	if( position < aPages.count() )
	{
	    delete aPages.takeAt(position);

	    // this isn't always an appropriate call, but the function checks whether it is appropriate or not
	    reflowMiddleMatter();

            setChanged(true);
        }
	else
	{
	    return false;
	}
    }
    else
    {
	return false;
    }
    return true;
}


void Book::writeToFile(QString fileName)
{
    if( fileName.isEmpty() )
        fileName = strFilename;
    if( fileName.isEmpty() )
        return;

    QString strFileContents;
    QXmlStreamWriter xml(&strFileContents);

    xml.setAutoFormatting(true);
    xml.writeStartDocument();
    xml.setCodec("UTF-8");

    xml.writeStartElement("book");

    xml.writeAttribute("language-template",lgTemplate->iso());
    if( bkTemplate != 0 )
        xml.writeAttribute("book-template",bkTemplate->name());

    // come back to this
    xml.writeEmptyElement("metadata");

    xml.writeStartElement("pages");

    QProgressDialog progress(QCoreApplication::translate("Book","Saving pages..."), QCoreApplication::translate("Book","Cancel"), 0, pages()->count() );
    progress.setWindowModality(Qt::WindowModal);
    progress.setValue(1);
    for(int i=0; i<aPages.count(); i++)
    {
        progress.setValue(i + 1);
        aPages.at(i)->write(&xml);
    }

    xml.writeEndElement(); // pages

    xml.writeEndElement(); // book

    xml.writeEndDocument();

    this->strFilename = fileName;

    setChanged(false);

    QFile file(fileName);
    file.open(QFile::WriteOnly | QFile::Text);
    QTextStream stream(&file);
    stream.setCodec("UTF-8");
    stream << strFileContents;
    file.close();
}

Book::TemplateMode Book::templateMode() const
{
    if( eTemplateMode == BreakoutMode )
        return BreakoutMode;

    int pagesPosition = 0;
    // check the first pages against the frontMatter templates
    for(int i=0; i < bkTemplate->frontMatter()->count(); i++ )
    {
	if( i >= aPages.count() )
	    return Book::BreakoutMode;
	if( bkTemplate->frontMatter()->at(i) != aPages.at(pagesPosition)->layoutTemplate() )
	    return Book::BreakoutMode;
	pagesPosition++;
    }

    // now check to see whether the middle matter follows the pattern
    for(int i=bkTemplate->frontMatter()->count(); i < aPages.count() - bkTemplate->backMatter()->count(); i++ )
    {
	int relativePosition = ( i - bkTemplate->frontMatter()->count() ) % bkTemplate->middleMatter()->count();
	if( bkTemplate->middleMatter()->at(relativePosition) != aPages.at(pagesPosition)->layoutTemplate() )
	    return Book::BreakoutMode;
	pagesPosition++;
    }

    // check the last pages against the backMatter templates
    pagesPosition = aPages.count() - 1;
    for(int i= bkTemplate->backMatter()->count() - 1; i >= 0; i--)
    {
	if( i >= aPages.count() )
	    return Book::BreakoutMode;
	if( bkTemplate->backMatter()->at(i) != aPages.at(pagesPosition)->layoutTemplate() )
	    return Book::BreakoutMode;
	pagesPosition--;
    }

    return Book::ConformityMode;
}

void Book::reflowMiddleMatter()
{
    if( templateMode() != ConformityMode )
        return;

    if( !bkTemplate->isMiddleMatterReflowable() )
	return;

    for(int i= bkTemplate->frontMatter()->count(); i < aPages.count() - bkTemplate->backMatter()->count(); i++ )
    {
	int relativePosition = ( i - bkTemplate->frontMatter()->count() ) % bkTemplate->middleMatter()->count();
        aPages.at(i)->setLayoutTemplate( bkTemplate->middleMatter()->at(relativePosition) );
    }
}

void Book::updateLayoutItemFormats()
{
    for(int i=0; i<aPages.count(); i++)
        aPages.at(i)->updateLayoutItemFormats();
}

PageTemplate* Book::pageTemplate() const
{
    if( bkTemplate != 0 )
        return bkTemplate->pageTemplate();
    else
        return pPageTemplate;
}
