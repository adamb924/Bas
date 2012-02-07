/*! \class BookTemplate
    \brief A data class holding book template information

    The book template controls the structure of the book, e.g., determining how pages can be added.
  */

#ifndef BOOKTEMPLATE_H
#define BOOKTEMPLATE_H

#include <QString>
#include <QList>

#include "abstracttemplate.h"
#include "mainwindow.h"

class PageTemplate;
class LayoutTemplate;
class MainWindow;

class QXmlStreamReader;
class QXmlStreamWriter;

#include <QMessageBox>

class BookTemplate : public AbstractTemplate
{
    Q_OBJECT
public:
    enum Position { FrontMatter, MiddleMatter, BackMatter };

    BookTemplate(MainWindow *parent);

    QWidget* createSettingsPage();

    bool canAddPageAfter(int page, int ofPage);
    bool canRemovePageAt(int page, int ofPage);
    BookTemplate::Position positionOfPage(int page, int ofPage);
    int relativeIndexOfPage(int page, int ofPage);

    //! \brief Return the layout template for adding a page at the specified position
    // assumes that a page can be added at this position
    LayoutTemplate* layoutTemplateForNewPageAt(int position);

    QString read(QXmlStreamReader *xml);
    void write(QXmlStreamWriter *xml);

    bool isMiddleMatterReflowable() const;

    inline QString longName() const { return strLongName; }

    inline QList<LayoutTemplate*>* frontMatter() { return &aFrontMatter; }
    inline QList<LayoutTemplate*>* middleMatter() { return &aMiddleMatter; }
    inline QList<LayoutTemplate*>* backMatter() { return &aBackMatter; }

    inline PageTemplate* pageTemplate() const { return pPageTemplate; }
    inline MainWindow* mainWindow() const { return pMW; }

    inline void fillerPagesBefore( Position &pos , int &after ) const { pos = eFillerPagesPosition; after = nFillerPagesBefore; }
    inline void setFillerPagesBefore( Position pos, int after) { eFillerPagesPosition = pos; nFillerPagesBefore = after; }

public slots:
    inline void setLongName(QString s) { strLongName = s; }
    inline void setPageTemplateFromIndex(int i) { pPageTemplate = pMW->pageTemplates()->at(i); QMessageBox::information(0,tr("Bas"),tr("After changing the page template you need to close this window and then open it again. Hopefully this will change in the future.")); }

private:
    QString strLongName;

    Position eFillerPagesPosition;
    int nFillerPagesBefore;

    PageTemplate *pPageTemplate;
    MainWindow *pMW;

    QList<LayoutTemplate*> aFrontMatter;
    QList<LayoutTemplate*> aMiddleMatter;
    QList<LayoutTemplate*> aBackMatter;
};

#endif // BOOKTEMPLATE_H
