#ifndef PAGECONTROLWIDGET_H
#define PAGECONTROLWIDGET_H

#include <QWidget>
class QHBoxLayout;
class QPushButton;
class Book;
class Page;
class QMenu;
class LayoutTemplate;

class PageControlWidget : public QWidget
{
    Q_OBJECT
public:
    explicit PageControlWidget(Book *book, Page *page, QWidget *parent = 0);
    ~PageControlWidget();

signals:
    void addPage();
    void addPageFromTemplate(LayoutTemplate *lt);
    void removePage();

public slots:
    void setCanAddPage(bool possible);
    void setCanRemovePage(bool possible);

private slots:
    void pageTemplateChanged(QAction *action);
    void aboutThisPage();
    void addPageFromAction(QAction *action);

private:
    QHBoxLayout *layout;
    QPushButton *bAddPage;
    QPushButton *bRemovePage;
    QPushButton *bChangeTemplate;
    QPushButton *bAdvancedOptions;

    QMenu* templateMenu();

    Book *pBook;
    Page *pPage;
};

#endif // PAGECONTROLWIDGET_H
