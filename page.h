/*! \class Page
    \brief The class that holds the page data

    Not really sure if it's necessary to have a subclass. Possibly for dealing with margins...
  */

#ifndef PAGE_H
#define PAGE_H

#include <QGraphicsScene>
#include "book.h"
#include "layouttemplate.h"

class PageView;
class PageTemplate;
class QXmlStreamReader;
class QXmlStreamWriter;
class GraphicsItem;
class TextItem;
class TextBlockLayoutItem;
class PageTemplate;

class Page : public QGraphicsScene
{
    Q_OBJECT

public:
    Page(Book *pBook, LayoutTemplate *layoutTemplate, Book::Side s, LanguageTemplate *lgTemplate, QObject *parent = 0);

    void populateFromTemplate();

    bool isValid() const;
    QString error() const;

//    void fixHtml(QString &in, QString style);

    QRectF sceneRect() const;

    //! \brief Return the width divided by the height
    qreal aspectRatio() const;

    void read(QXmlStreamReader *xml);
    void write(QXmlStreamWriter *xml) const;

    bool toSvg(QXmlStreamWriter &w, QPoint location = QPoint(0,0)) const;

    inline PageTemplate* pageTemplate() const { return pLayoutTemplate->pageTemplate(); }

    inline LayoutTemplate* layoutTemplate() const { return pLayoutTemplate; }
    inline void setLayoutTemplate(LayoutTemplate* t) { pLayoutTemplate = t; updateLayoutItemFormats(); }

    inline LanguageTemplate* languageTemplate() const { return lgTemplate; }
    inline void setLanguageTemplate(LanguageTemplate* t) { lgTemplate = t; updateLayoutItemFormats(); }

    inline Book* book() const { return pBook; }

    inline Book::Side side() const { return pageSide; }

    void setChanged(bool changed);
    inline bool changed() const { return bChanged; }

    void updateLayoutItemFormats();

private:
    void addBackgroundImage();


    Book *pBook;

    bool bChanged;

    QList<TextItem*> aTextblocks;
    QList<GraphicsItem*> aGraphics;

    LayoutTemplate *pLayoutTemplate;
    QString errormessage;

    LanguageTemplate *lgTemplate;
    Book::Side pageSide;

    TextItem* formatTextItem( QString &text, TextBlockLayoutItem *t );

signals:

public slots:

};

#endif // PAGE_H
