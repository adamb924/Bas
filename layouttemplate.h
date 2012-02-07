#ifndef LAYOUTTEMPLATE_H
#define LAYOUTTEMPLATE_H

#include <QtGlobal>
#include <QString>
#include <QList>

#include "abstracttemplate.h"
#include "mainwindow.h"

class QXmlStreamReader;
class TextBlockLayoutItem;
class GraphicsLayoutItem;
class MainWindow;
class PageTemplate;
class QTreeWidgetItem;
class QXmlStreamWriter;

class LayoutTemplate : public AbstractTemplate
{
    Q_OBJECT
public:
    LayoutTemplate(MainWindow *parent);

    bool compatibleWith(const LayoutTemplate& other);

    void read(QXmlStreamReader *xml);
    void write(QXmlStreamWriter *xml);

    QWidget* createSettingsPage();

    inline QString longName() const { return strLongName; }
    inline PageTemplate* pageTemplate() const { return pPageTemplate; }
    inline QList<TextBlockLayoutItem*>* textblockLayouts() { return &aTextblockLayouts; }
    inline QList<GraphicsLayoutItem*>* graphicsLayouts() { return &aGraphicLayouts; }
    inline QString backgroundImageBase64() const { return strBackgroundImage; }
    inline QPixmap backgroundImage() { return imgBackground; }

public slots:
    inline void setLongName(QString s) { strLongName = s; }
    inline void setPageTemplateFromIndex(int i) { pPageTemplate = pMW->pageTemplates()->at(i); }
    inline void setBackgroundImage(QString s) { strBackgroundImage = s; }

    //! \brief This is for setting TextBlockLayoutItem parameters when it changed from a QTreeWidget
    void setTextLayoutItemFromWidgetItem(QTreeWidgetItem * item, int column );

    //! \brief This is for setting GraphicsLayoutItem parameters when it changed from a QTreeWidget
    void setGraphicLayoutItemFromWidgetItem(QTreeWidgetItem * item, int column );

private:
    QPixmap imgBackground;
    QString strLongName;
    QString strBackgroundImage;
    QList<TextBlockLayoutItem*> aTextblockLayouts;
    QList<GraphicsLayoutItem*> aGraphicLayouts;
    MainWindow *pMW;
    PageTemplate *pPageTemplate;
};

#endif // LAYOUTTEMPLATE_H
