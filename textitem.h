/*! \class TextItem
    \brief A subclass of QGraphicsTextItem that restricts how MIME data is inserted

    This class is used to add text blocks to the page/scene. The behavior of the class is identical in behavior to QGraphicsTextItem, except that rich text formatting is stripped out of MIME data inserted upon drag-and-drop operations, and paste operations.
  */

#ifndef TEXTITEM_H
#define TEXTITEM_H

#include <QGraphicsTextItem>

class QGraphicsSceneDragDropEvent;
class TextBlockLayoutItem;
class LanguageTemplate;
class Page;
class QXmlStreamWriter;

class TextItem : public QGraphicsTextItem
{
    Q_OBJECT
public:
    TextItem(Page *page, TextBlockLayoutItem *layoutItem, LanguageTemplate *lgTemplate);

    void setHtml ( const QString & text );

    bool toSvg(QXmlStreamWriter &w, QString id);

public slots:

protected:
    TextBlockLayoutItem *layoutItem;
    LanguageTemplate *lgTemplate;
    Page *pPage;

    void fixHtml(QString &in);


    //! \brief Strip the rich text formatting out of text-drop events, and then proceed as usual
    void dropEvent ( QGraphicsSceneDragDropEvent * event );

    //! \brief Strip the rich text formatting out of paste events, and then proceed as usual
    void keyPressEvent ( QKeyEvent * event );

    void inputMethodEvent ( QInputMethodEvent * event );
};

#endif // TEXTITEM_H
