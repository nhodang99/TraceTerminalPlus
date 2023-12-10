#include "inc/mainwindow.h"
#include "inc/utils.h"
#include <QtWidgets>
#include <QSettings>

MainWindow::MainWindow()
    : QMainWindow()
{
    QSettings settings(Config::CONFIG_DIR, QSettings::IniFormat);

    // Main tab widget
    m_tabWidget = new QTabWidget(this);
    m_tabWidget->setTabsClosable(true);
    setCentralWidget(m_tabWidget);

    // Live traceview
    auto autoscroll = settings.value(Config::TRACEVIEW_AUTOSCROLL, true).toBool();
    m_liveView = new TraceView(true, autoscroll);
    m_tabWidget->addTab(m_liveView, "Live Trace");
    // Hide the close button of live view
    // @todo: How to detect icon position?
    m_tabWidget->tabBar()->tabButton(0, QTabBar::RightSide)->hide();

    // Create app's menus and actions
    createActions();
    createMenus();

    QString message = tr("A context menu is available by right-clicking");
    statusBar()->showMessage(message);

    // Init the main window property
    setWindowTitle(tr("TraceTerminal++ - Live View"));
    setWindowIcon(QIcon(":/img/favicon.png"));
    setMinimumSize(480, 360);
    restoreGeometry(settings.value(Config::MAINWINDOW_GEOMETRY).toByteArray());

    connect(m_tabWidget, &QTabWidget::tabCloseRequested,
            this, &MainWindow::onTabCloseRequested);
    connect(m_tabWidget, &QTabWidget::currentChanged,
            this, &MainWindow::onCurrentTabChanged);
    connect(m_liveView, &TraceView::copyAvailable, this, &MainWindow::onCopyAvailable);
}

///
/// \brief MainWindow::closeEvent override
/// \param event
///
void MainWindow::closeEvent(QCloseEvent *event)
{
    QSettings settings(Config::CONFIG_DIR, QSettings::IniFormat);
    settings.setValue(Config::MAINWINDOW_GEOMETRY, saveGeometry());
    settings.setValue(Config::TRACEVIEW_AUTOSCROLL, m_liveView->isAutoscroll());
    event->accept();
}

///
/// \brief MainWindow::onTabCloseRequested override
/// \param index
///
void MainWindow::onTabCloseRequested(int index)
{
    m_tabWidget->tabBar()->removeTab(index);
    auto pView = m_tabWidget->widget(index);
    pView->deleteLater();
}

///
/// \brief MainWindow::onCurrentTabChanged
/// \param index
///
void MainWindow::onCurrentTabChanged(int index)
{
    auto tabName = m_tabWidget->tabText(index);
    setWindowTitle(tr("TraceTerminal++") + " - " + tabName);
}

///
/// \brief MainWindow::open
///
void MainWindow::open()
{
    QFileDialog fileDialog(this, tr("TraceTerminal++ - Open"));
    fileDialog.setNameFilter("Trace files (*.txt *.html)");
    if (!fileDialog.exec())
        return;

    auto filename = fileDialog.selectedFiles();
    if (filename.isEmpty())
        return;

    // Only one selected allowed
    QFileInfo fileInfo(filename[0]);
    for (auto i = 1; i < m_tabWidget->count(); ++i) // exclude default view
    {
        if (fileInfo.fileName() == m_tabWidget->tabText(i))
        {
            m_tabWidget->setCurrentIndex(i);
            return;
        }
    }

    QFile file(filename[0]);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qDebug() << "Cannot open trace file";
        return;
    }

    auto offlineView = new TraceView();
    offlineView->setDocumentTitle(fileInfo.fileName());
    QTextStream in(&file);
    if (fileInfo.suffix() == "txt")
    {
        while (!in.atEnd())
        {
            auto line = in.readLine();
            processLine(line);
            offlineView->append(line);
        }
    }
    else
    {
        offlineView->setText(in.readAll());
    }

    connect(offlineView, &TraceView::copyAvailable, this, &MainWindow::onCopyAvailable);
    m_tabWidget->addTab(offlineView, fileInfo.fileName());
    m_tabWidget->setCurrentIndex(m_tabWidget->count() - 1);
}

void MainWindow::save()
{
    auto currentView = (TraceView*)m_tabWidget->currentWidget();
    currentView->save();
}

void MainWindow::copy()
{
    auto currentView = (TraceView*)m_tabWidget->currentWidget();
    currentView->copy();
}

void MainWindow::onCopyAvailable(bool a)
{
    copyAct->setEnabled(a);
}

void MainWindow::clear()
{
    m_liveView->clear();
}

void MainWindow::about()
{
    QMessageBox::about(this, tr("About TraceTerminal++"),
                       tr("<b>TraceTerminal++</b> offers view, search in file, export/import trace file in-place. "
                          "Inspired by TraceTerminal app.<br>"
                          "Author: Nhodang"));
}

void MainWindow::createActions()
{
    openAct = new QAction(tr("&Open..."), this);
    openAct->setShortcuts(QKeySequence::Open);
    openAct->setStatusTip(tr("Open an existing file"));
    connect(openAct, &QAction::triggered, this, &MainWindow::open);

    saveAct = new QAction(tr("&Save"), this);
    saveAct->setShortcuts(QKeySequence::Save);
    saveAct->setStatusTip(tr("Save the document to disk"));
    connect(saveAct, &QAction::triggered, this, &MainWindow::save);

    exitAct = new QAction(tr("&Exit"), this);
    exitAct->setShortcuts(QKeySequence::Quit);
    exitAct->setStatusTip(tr("Exit the application"));
    connect(exitAct, &QAction::triggered, this, &QWidget::close);

    copyAct = new QAction(tr("&Copy"), this);
    copyAct->setShortcuts(QKeySequence::Copy);
    copyAct->setStatusTip(tr("Copy the current selection's contents to the "
                             "clipboard"));
    copyAct->setEnabled(false);
    connect(copyAct, &QAction::triggered, this, &MainWindow::copy);
    connect(m_liveView, &TraceView::copyAvailable, this, &MainWindow::onCopyAvailable);

    clearAct = new QAction(tr("&Clear"), this);
    clearAct->setShortcuts(QKeySequence::Refresh);
    clearAct->setStatusTip(tr("Clear all the traces"));
    connect(clearAct, &QAction::triggered, this, &MainWindow::clear);

    aboutAct = new QAction(tr("&About"), this);
    aboutAct->setStatusTip(tr("Show the application's About box"));
    connect(aboutAct, &QAction::triggered, this, &MainWindow::about);
}

void MainWindow::createMenus()
{
    fileMenu = menuBar()->addMenu(tr("&File"));
    fileMenu->addAction(openAct);
    fileMenu->addAction(saveAct);
    fileMenu->addSeparator();
    fileMenu->addAction(exitAct);

    editMenu = menuBar()->addMenu(tr("&Edit"));
    editMenu->addAction(copyAct);
    editMenu->addSeparator();
    editMenu->addAction(clearAct);

    //editMenu = menuBar()->addMenu(tr("&Setting"));

    helpMenu = menuBar()->addMenu(tr("&Help"));
    helpMenu->addAction(aboutAct);
}
