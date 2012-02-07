/*! \class MainWindow
    \brief The class for the main window.

    This class provides the main window. One instance of it is created in the main() function. Notably, it contains all of the pages of the present document, as well as the MultiPageLayout which displays the pages.
  */


#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtGui/QMainWindow>
#include <QList>

class MultiPageLayout;
class Book;
class Page;
class PageView;
class QAction;
class QSettings;
class LanguageTemplate;
class QFile;
class BookTemplate;
class PageTemplate;
class LayoutTemplate;
class LanguageTemplate;
class AbstractTemplate;

class MainWindow : public QMainWindow
{
    Q_OBJECT

signals:

public:
    bool initialized;

    //! \brief Constructs and displays the window
    MainWindow(QWidget *parent = 0);
    //! \brief Deconstructs the window
    ~MainWindow();

    PageTemplate* pageTemplateFromName(QString name);
    LayoutTemplate* layoutTemplateFromName(QString name, QString pagelayoutName);
    LayoutTemplate* layoutTemplateFromLongName(QString longName, QString pagelayoutName);
    BookTemplate* bookTemplateFromName(QString name);
    LanguageTemplate* languageTemplateFromIso(QString iso);

    QList<LayoutTemplate*> getLayoutTemplatesForPageTemplate(PageTemplate *pt);

    QList<LayoutTemplate*> getCompatibleLayoutTemplates(LayoutTemplate *first);

    inline QList<PageTemplate*>* pageTemplates() { return &aPageTemplates; }
    inline QList<BookTemplate*>* bookTemplates() { return &aBookTemplates; }
    inline QList<LanguageTemplate*>* languageTemplates() { return &aLanguageTemplates; }
    inline QList<LayoutTemplate*>* layoutTemplates() { return &aLayoutTemplates; }


    inline LanguageTemplate* hardwiredLanguage() const { return pHardwiredLanguage; }

    inline bool showAdvancedPageOptions() const { return bShowAdvancedPageOptions; }

public slots:

private:
    QSettings *settings;

    QString strPathToInkscape;
    QString strPathToGhostscript;

    QFile* openOrCopyFromResource(QString filename);
    QFile* copyFromResource(QString filename);

    void createBookDisplay();

    QAction *allowAdvancedPageOptions;

    void getSettings();
    void setSettings();

    bool promptToSave();
    void closeEvent ( QCloseEvent * event );

    bool requireExternalApps();

    //! \brief The available page templates
    QList<PageTemplate*> aPageTemplates;
    QList<LayoutTemplate*> aLayoutTemplates;
    QList<BookTemplate*> aBookTemplates;
    QList<LanguageTemplate*> aLanguageTemplates;


    void savePageTemplates();
    void saveLayoutTemplates();
    void saveBookTemplates();
    void saveLanguageTemplates();

    //! \brief Process keyboard events
    void keyPressEvent ( QKeyEvent * event );

    //! \brief Create the menus and associated actions
    void setupMenus();

    //! \brief Load the page templates from the XML file
    bool loadPageTemplates();

    //! \brief Load the layout templates from the XML file
    bool loadLayoutTemplates();

    //! \brief Load the book templates from the XML file
    bool loadBookTemplates();

    //! \brief Load the language templates from the XML file
    bool loadLanguageTemplates();

    QMenu *lgMenu;
    LanguageTemplate *pHardwiredLanguage;

    bool bShowAdvancedPageOptions;

    Book *pBook;

private slots:
    void newBook();
    void newBookCustomDesign();
    void newBookOk();
    void newBookCancel();
    void openBook();
    void saveBook();
    void saveBookAs();
    void closeBook();
    void createPDF();
    void selectInterfaceLanguage(QAction *action);
    void selectHardwiredLanguage(QAction *action);

    void setShowAdvancedPageOptions(bool allow);

    void aboutThisBook();

    void lookForInkscape();
    void lookForGhostscript();

    void showInkscapePath();
    void showGhostscriptPath();

    // template modifications
    void editBookTemplates();
    void editLanguageTemplates();
    void editLayoutTemplates();
    void editPageTemplates();

    AbstractTemplate* newPageTemplate();
    AbstractTemplate* newBookTemplate();
    AbstractTemplate* newLanguageTemplate();
    AbstractTemplate* newLayoutTemplate();
};

typedef  AbstractTemplate* (MainWindow::*NewTemplateMethod)();

#endif // MAINWINDOW_H
