#ifndef SEARCHDOCK_H
#define SEARCHDOCK_H

#include <QDockWidget>
#include <QTextCursor>

QT_BEGIN_NAMESPACE
class QLineEdit;
class QListWidget;
class QListWidgetItem;
class QCheckBox;
class QPushButton;
QT_END_NAMESPACE

class SearchDock : public QDockWidget
{
    Q_OBJECT
public:
    SearchDock(QWidget* parent = nullptr);

    QString getQuery() const;
    void setQuery(QString&);

    bool isCaseSensitiveChecked() const;
    bool isLoopSearchChecked() const;

    void addAdvSearchResult(QTextCursor&);
    void show(bool advanced = false);
    void sortAdvSearchResult();

protected:
    void hideEvent(QHideEvent*) override;

public slots:
    void onSearchClicked();
    void onAdvSearchClicked();
    void onResultDoubleClicked(QListWidgetItem*);

signals:
    void search(bool advanced = false, bool newSearch = true);
    void searchDockHidden();
    void searchResultSelected(const QTextCursor);
    void clearHighlight();

private:
    void clearAdvSearchList();

    QPushButton*        m_searchButton;
    QPushButton*        m_advSearchButton;
    QPushButton*        m_clearHighlightButton;
    QLineEdit*          m_lineEdit;
    QListWidget*        m_advSearchList;
    QCheckBox*          m_caseSensitiveCheck;
    QCheckBox*          m_loopCheck;
    QString             m_lastQuery;
};

#endif // SEARCHDOCK_H
