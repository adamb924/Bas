#ifndef NEWBOOKWIDGET_H
#define NEWBOOKWIDGET_H

#include <QWidget>

class MainWindow;
class QComboBox;
class BookTemplate;
class QListView;
class QStandardItemModel;
class LanguageTemplate;
class PageTemplate;
class QStringListModel;

class NewBookWidget : public QWidget
{
    Q_OBJECT
public:
    explicit NewBookWidget(bool fromTemplate, MainWindow *parent);

    QString fontFamily() const;
    BookTemplate* bookTemplate() const;
    LanguageTemplate* languageTemplate() const;
    PageTemplate* pageTemplate() const;

    inline bool fromTemplate() const { return bFromTemplate; }

private:
    QStandardItemModel *pageTemplatesModel;
    QStandardItemModel *bookTemplatesModel;
    QStringListModel *languagesModel;
    QListView *bookTemplatesView;
    QListView *pageTemplatesView;
    QComboBox *fontFamilyCombo;
    QComboBox *languageCombo;
    MainWindow *pMW;

    bool bFromTemplate;

signals:
    void ok();
    void cancel();

public slots:

private slots:
    void setFontFromLanguage(QString lg);

};

#endif // NEWBOOKWIDGET_H
