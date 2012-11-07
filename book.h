/*! \class Book
    \brief A data class for creating books

    This is the top level data class for a book. Book objects would typically be owned by the MainWindow. Notable members are the list of pages of the book, and a pointer to the book's template.
  */

#ifndef BOOK_H
#define BOOK_H

class PageTemplate;
class LayoutTemplate;
class BookTemplate;
class MainWindow;
class Page;

#include <QString>
#include <QList>

#include "languagetemplate.h"

class Book
{
public:
    enum Side { Left, Right };
    enum TemplateMode { ConformityMode, BreakoutMode };
    enum PrintMode { Single, Duplex };

    //! \brief Reads the book stored in \a fileName
    Book(QString fileName, QString inkscape, QString ghostscript, MainWindow *parent);

    //! \brief Creates a book from the book template \a bookTemplate using text direction \a dir
    Book(BookTemplate *bookTemplate, LanguageTemplate *lgtemplate, QString fontfamily, QString inkscape, QString ghostscript, MainWindow *parent);

    //! \brief Creates a book in breakout mode
    Book(PageTemplate *pageTemplate, LanguageTemplate *lgtemplate, QString fontfamily, QString inkscape, QString ghostscript, MainWindow *parent);

    ~Book();

    //! \brief Create a PDF file of the book
    void createPdf(QString filename);

    //! \brief Return what side of the book page \a p
    Book::Side getPageSideFromPageNumber(int p) const;

    //! \brief Insert a page at \a position (0-indexed, relative to the first page of the book)
    bool addPage(int position);

    //! \brief Insert a page at \a position with the \a layoutTemplate
    bool addPage(int position, LayoutTemplate *layoutTemplate);

    //! \brief Remove a page at \a position (0-indexed, relative to the first page of the book)
    bool removePage(int position);

    //! \brief Write the book to \a fileName
    void writeToFile(QString fileName = QString());

    //! \brief
    bool readFromFile(QString fileName);

    //! \brief
    void reflowMiddleMatter();

    //! \brief Returns a value indicating how the book relates to the book template
    TemplateMode templateMode() const;

    //! \brief Returns the text direction of the book
    inline LanguageTemplate::TextDirection textDirection() const { return lgTemplate->textDirection(); }

    //! \brief Returns the filename of the book
    inline QString filename() const { return strFilename; }

    //! \brief Returns a pointer to the list of pages in the book
    inline QList<Page*>* pages() { return &aPages; }

    //! \brief Returns the book template
    inline BookTemplate* bookTemplate() const { return bkTemplate; }

    //! \brief Returns the page template
    PageTemplate* pageTemplate() const;

    //! \brief Returns a pointer to the MainWindow associated with the book
    inline MainWindow* mainWindow() const { return pMainWindow; }

    inline QString fontFamily() const { return strFontFamily; }

    inline void setFontFamily(QString family) { strFontFamily = family; }

    bool convertSvgToPdf(const QString& filename) const;

    int nextMultipleOfN( int number, int multiple = 4 );

    inline void setChanged(bool changed) { bChanged = changed; }
    inline bool changed() const { return bChanged; }

    inline void setValid(bool valid) { bValid = valid; }
    inline bool valid() const { return bValid; }

    inline void setPathToInkscape(QString path) { strPathToInkscape = path; }
    inline void setPathToGhostscript(QString path) { strPathToGhostscript = path; }

    void updateLayoutItemFormats();

    inline LanguageTemplate* languageTemplate() const { return lgTemplate; }

    void populateFromTemplate();

private:
    //! \brief The pages of the book
    QList<Page*> aPages;


    QString strPathToInkscape;
    QString strPathToGhostscript;

    //! \brief How the book relates to its template
    TemplateMode eTemplateMode;

    bool bChanged;

    bool bValid;

    void createSinglePage( Page *p , const QString& filename );
    void createDuplexPage( Page *left, Page *right, const QString& filename, bool flip = false );

    //! \brief The book template of the book
    BookTemplate *bkTemplate;

    PageTemplate *pPageTemplate;

    //! \brief The language template of the book
    LanguageTemplate *lgTemplate;

    //! \brief The book filename
    QString strFilename;

    //! \brief The book template of the book
    MainWindow *pMainWindow;

    QString strFontFamily;
};

#endif // BOOK_H
