#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QHostAddress>
#include <QMainWindow>
#include "traceview.h"
#include "searchdock.h"

QT_BEGIN_NAMESPACE
class QHostAddress;
class QAction;
class QActionGroup;
class QMenu;
class QTextEdit;
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow();
    TraceView* getLiveView() const { return m_liveView; }
    void hightlightAllOccurrences();
    bool isOccurrencesHighlighted() { return m_isOccurrencesHighlighted; }

protected:
    void closeEvent(QCloseEvent* event) override;
    void dropEvent(QDropEvent* event) override;
    void dragEnterEvent(QDragEnterEvent* event) override;

public slots:
    void open();
    void save();
    void showSearchDock(bool advanced = false);
    void copy();
    void clear();
    void about();
    void onCopyAvailable(bool);
    void onSearchDockHidden();

private:
    void createActions();
    void createMenus();
    void onTabCloseRequested(int);
    void onCurrentTabChanged(int);
    void onSearchRequested(bool, bool);
    void openFile(QString&);
    void clearOccurrencesHighlight();
    void normalSearch(bool);
    void advancedSearch();
    void onSearchResultSelected(const QTextCursor);

    //! [Actions]
    QMenu* m_fileMenu;
    QMenu* m_editMenu;
    QMenu* m_helpMenu;
    QAction* m_openAct;
    QAction* m_saveAct;
    QAction* m_searchAct;
    QAction* m_copyAct;
    QAction* m_clearAct;
    QAction* m_exitAct;
    QAction* m_aboutAct;
    //! [Actions]

    //! [Widgets]
    QTabWidget*   m_tabWidget {nullptr};
    TraceView*    m_liveView {nullptr};
    SearchDock*   m_searchDock {nullptr};

    //! [Attr]
    bool          m_isOccurrencesHighlighted {false};
    QTextCursor   m_lastSearchCursor;
};
#endif // MAINWINDOW_H
