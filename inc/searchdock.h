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
    QString getSearchText() const;
    void setSearchText(QString&);
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

private:
    void clearAdvSearchList();

    QPushButton*        m_searchButton;
    QPushButton*        m_advSearchButton;
    QLineEdit*          m_lineEdit;
    QListWidget*        m_advSearchList;
    QCheckBox*          m_caseSensitiveCheck;
    QCheckBox*          m_loopCheck;
    QString             m_lastNormalSearchText;
};

#endif // SEARCHDOCK_H
