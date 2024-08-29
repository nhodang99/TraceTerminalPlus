#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "livetraceview.h"
#include "searchdock.h"

QT_BEGIN_NAMESPACE
class QAction;
class QActionGroup;
class QMenu;
class QTextEdit;
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(LiveTraceView*, SearchDock*);
    inline bool isOccurrencesHighlighted() const;
    void hightlightAllOccurrences();
    void setCustomHighlights(const QStringList&);

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

signals:
    void highlightChanged();

private:
    void createActions();
    void createMenus();
    void onTabCloseRequested(int);
    void onCurrentTabChanged(int);
    void onSearchRequested(bool, bool);
    void openFile(const QString&);
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
    QTabWidget*    m_tabWidget {nullptr};
    LiveTraceView* m_liveView {nullptr};
    SearchDock*    m_searchDock {nullptr};

    //! [Attr]
    bool           m_isOccurrencesHighlighted {false};
    QTextCursor    m_lastSearchCursor;
    TraceView*     m_viewInAdvSearch{nullptr};
    int            m_lastTabIndex{0};
};


inline bool MainWindow::isOccurrencesHighlighted() const
{
    return m_isOccurrencesHighlighted;
}

#endif // MAINWINDOW_H
