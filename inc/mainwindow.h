#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QHostAddress>
#include <QMainWindow>
#include "traceview.h"

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

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    void open();
    void save();
    void copy();
    void clear();
    void about();
    void onCopyAvailable(bool);

private:
    void createActions();
    void createMenus();
    void onTabCloseRequested(int);
    void onCurrentTabChanged(int);

    //! [Actions]
    QMenu *fileMenu;
    QMenu *editMenu;
    QMenu *settingMenu;
    QMenu *helpMenu;
    QAction *openAct;
    QAction *saveAct;
    QAction *copyAct;
    QAction *clearAct;
    QAction *exitAct;
    QAction *aboutAct;
    //! [Actions]

    //! [Widgets]
    QTabWidget *m_tabWidget {nullptr};
    TraceView *m_liveView {nullptr};
};
#endif // MAINWINDOW_H
