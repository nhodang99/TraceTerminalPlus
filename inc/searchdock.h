#ifndef SEARCHDOCK_H
#define SEARCHDOCK_H

#include <QDockWidget>
#include <QTextCursor>

QT_BEGIN_NAMESPACE
class QLineEdit;
class QListWidget;
class QListWidgetItem;
class QCheckBox;
QT_END_NAMESPACE

class SearchDock : public QDockWidget
{
    Q_OBJECT
public:
    SearchDock(QWidget*, bool, bool);
    QString getSearchText() const;
    void setSearchText(QString&);
    bool isCaseSensitiveChecked() const;
    bool isLoopSearchChecked() const;
    void addAdvSearchResult(QString&, QTextCursor&);
    void show(bool advanced = false);

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

    QLineEdit*          m_lineEdit;
    QListWidget*        m_advSearchList;
    QCheckBox*          m_caseSensitiveCheck;
    QCheckBox*          m_loopCheck;
    QList<QTextCursor>  m_searchResultCursors;
    QString             m_lastNormalSearchText;
};

#endif // SEARCHDOCK_H
