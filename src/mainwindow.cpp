#include "inc/mainwindow.h"
#include "inc/constants.h"
#include "inc/searchdock.h"
#include "inc/tracemanager.h"
#include <QtWidgets>
#include <QSettings>

MainWindow::MainWindow()
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
    // @TODO: How to detect icon position?
    m_tabWidget->tabBar()->tabButton(0, QTabBar::RightSide)->hide();

    // Search dock
    auto caseSensitive = settings.value(Config::SEARCH_CASESENSITIVE, false).toBool();
    auto loopSearch = settings.value(Config::SEARCH_LOOPSEARCH, false).toBool();
    m_searchDock = new SearchDock(this, caseSensitive, loopSearch);
    addDockWidget(Qt::BottomDockWidgetArea, m_searchDock);
    m_searchDock->hide();

    // Create app's menus and actions
    createActions();
    createMenus();

    QString message = tr("A context menu is available by right-clicking");
    statusBar()->setStyleSheet("color: indigo");
    statusBar()->showMessage(message);

    // Init the main window property
    setWindowTitle(tr("TraceTerminal++ - Live View"));
    setWindowIcon(QIcon(":/img/favicon.ico"));
    setAcceptDrops(true);
    setMinimumSize(480, 360);
    restoreGeometry(settings.value(Config::MAINWINDOW_GEOMETRY).toByteArray());

    connect(m_tabWidget, &QTabWidget::tabCloseRequested,
            this, &MainWindow::onTabCloseRequested);
    connect(m_tabWidget, &QTabWidget::currentChanged,
            this, &MainWindow::onCurrentTabChanged);
    connect(m_liveView, &TraceView::copyAvailable, this, &MainWindow::onCopyAvailable);
    connect(m_searchDock, &SearchDock::search, this, &MainWindow::onSearchRequested);
    connect(m_searchDock, &SearchDock::searchDockHidden,
            this, &MainWindow::onSearchDockHidden);
    connect(m_searchDock, &SearchDock::searchResultSelected, this, &MainWindow::onSearchResultSelected);
}

///
/// \brief MainWindow::createActions
///
void MainWindow::createActions()
{
    m_openAct = new QAction(tr("&Open..."), this);
    m_openAct->setShortcuts(QKeySequence::Open);
    m_openAct->setStatusTip(tr("Open an existing file"));
    connect(m_openAct, &QAction::triggered, this, &MainWindow::open);

    m_saveAct = new QAction(tr("&Save"), this);
    m_saveAct->setShortcuts(QKeySequence::Save);
    m_saveAct->setStatusTip(tr("Save the document to disk"));
    connect(m_saveAct, &QAction::triggered, this, &MainWindow::save);

    m_searchAct = new QAction(tr("&Search..."), this);
    m_searchAct->setShortcuts(QKeySequence::Find);
    m_searchAct->setStatusTip(tr("Search in the document. Use shortcut Ctrl+F for normal search, Ctrl+Shift+F for advanced search."));
    connect(m_searchAct, &QAction::triggered, this, [=](){
        this->showSearchDock();
    });
    QShortcut* advSearchShortcut = new QShortcut(QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_F), this);
    connect(advSearchShortcut, &QShortcut::activated, this, [=](){
        this->showSearchDock(true);
    });

    m_exitAct = new QAction(tr("&Exit"), this);
    m_exitAct->setShortcuts(QKeySequence::Quit);
    m_exitAct->setStatusTip(tr("Exit the application"));
    connect(m_exitAct, &QAction::triggered, this, &QWidget::close);

    m_copyAct = new QAction(tr("&Copy"), this);
    m_copyAct->setShortcuts(QKeySequence::Copy);
    m_copyAct->setStatusTip(tr("Copy the current selection's contents to the "
                               "clipboard"));
    m_copyAct->setEnabled(false);
    connect(m_copyAct, &QAction::triggered, this, &MainWindow::copy);
    connect(m_liveView, &TraceView::copyAvailable, this, &MainWindow::onCopyAvailable);

    m_clearAct = new QAction(tr("&Clear"), this);
    m_clearAct->setShortcuts(QKeySequence::Refresh);
    m_clearAct->setStatusTip(tr("Clear all the traces"));
    connect(m_clearAct, &QAction::triggered, this, &MainWindow::clear);

    m_aboutAct = new QAction(tr("&About"), this);
    m_aboutAct->setStatusTip(tr("Show the application's About box"));
    connect(m_aboutAct, &QAction::triggered, this, &MainWindow::about);
}

///
/// \brief MainWindow::createMenus
///
void MainWindow::createMenus()
{
    m_fileMenu = menuBar()->addMenu(tr("&File"));
    m_fileMenu->addAction(m_openAct);
    m_fileMenu->addAction(m_saveAct);
    m_fileMenu->addAction(m_searchAct);
    m_fileMenu->addSeparator();
    m_fileMenu->addAction(m_exitAct);

    m_editMenu = menuBar()->addMenu(tr("&Edit"));
    m_editMenu->addAction(m_copyAct);
    m_editMenu->addSeparator();
    m_editMenu->addAction(m_clearAct);

    m_helpMenu = menuBar()->addMenu(tr("&Help"));
    m_helpMenu->addAction(m_aboutAct);
}

///
/// \brief MainWindow::closeEvent override
/// \param event
///
void MainWindow::closeEvent(QCloseEvent* event)
{
    QSettings settings(Config::CONFIG_DIR, QSettings::IniFormat);
    settings.setValue(Config::MAINWINDOW_GEOMETRY, saveGeometry());
    settings.setValue(Config::TRACEVIEW_AUTOSCROLL, m_liveView->isAutoscroll());
    settings.setValue(Config::SEARCH_CASESENSITIVE, m_searchDock->isCaseSensitiveChecked());
    settings.setValue(Config::SEARCH_LOOPSEARCH, m_searchDock->isLoopSearchChecked());
    event->accept();
}

///
/// \brief MainWindow::dragEnterEvent
/// \param event
///
void MainWindow::dragEnterEvent(QDragEnterEvent* event)
{
    if (event->mimeData()->hasUrls()) {
        event->acceptProposedAction();
    }
}

///
/// \brief MainWindow::dropEvent
/// \param event
///
void MainWindow::dropEvent(QDropEvent* event)
{
    auto mimeData = event->mimeData();
    if (mimeData->hasUrls())
    {
        foreach (auto& url, mimeData->urls())
        {
            if (url.toString().endsWith("html") || url.toString().endsWith("txt"))
            {
                auto filename = url.toLocalFile();
                openFile(filename);
            }
        }
    }
}

///
/// \brief MainWindow::onTabCloseRequested override
/// \param index
///
void MainWindow::onTabCloseRequested(int index)
{
    auto pView = m_tabWidget->widget(index);
    if (pView)
    {
        pView->deleteLater();
    }
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
/// \brief MainWindow::onSearchDockHidden
///
void MainWindow::onSearchDockHidden()
{
    clearOccurrencesHighlight();
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
    openFile(filename[0]);
}

///
/// \brief MainWindow::openFile
/// \param url
///
void MainWindow::openFile(QString& url)
{
    QFileInfo fileInfo(url);
    for (auto i = 1; i < m_tabWidget->count(); ++i) // exclude live view
    {
        if (fileInfo.fileName() == m_tabWidget->tabText(i))
        {
            m_tabWidget->setCurrentIndex(i);
            return;
        }
    }

    auto offlineView = new TraceView();
    offlineView->setDocumentTitle(fileInfo.fileName());
    m_tabWidget->addTab(offlineView, fileInfo.fileName());
    m_tabWidget->setCurrentIndex(m_tabWidget->count() - 1);
    connect(offlineView, &TraceView::copyAvailable, this, &MainWindow::onCopyAvailable);

    QProgressDialog progress("Opening files...", "Abort", 0, 100, this);
    progress.setWindowModality(Qt::WindowModal);
    progress.setWindowFlags(Qt::Window | Qt::WindowTitleHint | Qt::CustomizeWindowHint);
    progress.open();

    QFile file(url);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qDebug() << "<span style=\"color:red\">Cannot open trace file</span>";
        progress.close();
        offlineView->append("Problem in opening the file.");
        return;
    }
    progress.setValue(50);

    // @TODO: process html line by line produce 2 redundant first line (html formatting line)
    bool isTxt = fileInfo.suffix() == "txt";
    QTextStream in(&file);
    while (!in.atEnd())
    {
        if (progress.wasCanceled())
        {
            offlineView->append("<span style=\"color:orange\">Process aborted.</span>");
            break;
        }
        auto line = in.readLine();
        if (isTxt)
        {
            TraceManager::instance().processTraceLine(line);
        }
        offlineView->append(line);
        // Process event loop so that gui thread can be updated while appending the text still occuring
        QApplication::processEvents();
    }
    progress.setValue(100);
}

///
/// \brief MainWindow::save
///
void MainWindow::save()
{
    auto currentView = (TraceView*)m_tabWidget->currentWidget();
    currentView->save();
}

///
/// \brief MainWindow::showSearchDock
///
void MainWindow::showSearchDock(bool advanced)
{
    m_searchDock->show(advanced);
    m_searchDock->setFocus();

    auto currentView = (TraceView*)m_tabWidget->currentWidget();
    if (currentView->textCursor().hasSelection())
    {
        auto selectedText = currentView->textCursor().selectedText();
        m_searchDock->setSearchText(selectedText);
        if (!advanced)
        {
            hightlightAllOccurrences();
        }
    }
}

///
/// \brief MainWindow::onSearchRequested
/// \param advanced bool: using advanced or normal search
///
void MainWindow::onSearchRequested(bool advanced, bool newSearch)
{
    if (advanced)
    {
        m_isOccurrencesHighlighted = false;
        m_lastSearchCursor = QTextCursor();
        advancedSearch();
    }
    else
    {
        normalSearch(newSearch);
    }
}

///
/// \brief MainWindow::normalSearch
///
void MainWindow::normalSearch(bool newSearch)
{
    // Find and highlight the next occurrence
    auto currentView = (TraceView*)m_tabWidget->currentWidget();
    auto document = currentView->document();
    auto text = m_searchDock->getSearchText();
    auto isCaseSensitive = m_searchDock->isCaseSensitiveChecked();
    auto isLoopSearch = m_searchDock->isLoopSearchChecked();

    // If new word searched: start finding from current cursor in current view
    // If continue to search the next result, search from the last found cursor
    auto foundCursor = document->find(text, newSearch ? currentView->textCursor() : m_lastSearchCursor,
                                      isCaseSensitive ? QTextDocument::FindCaseSensitively : QTextDocument::FindFlag());
    if (foundCursor.isNull())
    {
        statusBar()->setStyleSheet("color: indigo");
        // If loop search is not checked, keep the current word hightlight
        if (!isLoopSearch)
        {
            statusBar()->showMessage("The end of document has been reached", 2000);
            return;
        }

        // Maybe the end of file, loop search checked
        currentView->moveCursor(QTextCursor::Start);
        foundCursor = document->find(text, currentView->textCursor(),
                                     isCaseSensitive ? QTextDocument::FindCaseSensitively : QTextDocument::FindFlag());
        if (!foundCursor.isNull())
        {
            statusBar()->showMessage("The end of document has been reached, searching from the start", 2000);
        }
        else
        {
            return;
        }

    }

    // Save the found cursor so that we can continue to search from there later
    m_lastSearchCursor = foundCursor;
    // Highlight again the found text and extra highlight the next text found
    hightlightAllOccurrences();
    auto extraSelections = currentView->extraSelections();

    int index = 0;
    for (auto& extra : extraSelections)
    {
        if (extra.cursor == foundCursor)
        {
            break;
        }
        index++;
    }
    extraSelections[index].format.setBackground(QColor(Qt::green).lighter());
    currentView->setExtraSelections(extraSelections);
    currentView->setTextCursor(foundCursor);
    currentView->moveCursor(QTextCursor::EndOfWord);
    currentView->moveCursor(QTextCursor::Right);
}

///
/// \brief MainWindow::advancedSearch
///
void MainWindow::advancedSearch()
{
    QProgressDialog progress("Searching files...", "Abort", 0, 100, this);
    progress.setWindowModality(Qt::WindowModal);
    progress.setWindowFlags(Qt::Window | Qt::WindowTitleHint | Qt::CustomizeWindowHint);
    progress.open();
    progress.setValue(50); // Ugly trick...
    auto text = m_searchDock->getSearchText();
    auto isCaseSensitive = m_searchDock->isCaseSensitiveChecked();
    auto currentView = (TraceView*)m_tabWidget->currentWidget();
    auto document = currentView->document();
    QTextCursor prevCursor;
    while (true)
    {
        if (progress.wasCanceled())
        {
            statusBar()->setStyleSheet("color: red");
            statusBar()->showMessage("Advanced search aborted.", 2000);
            break;
        }

        auto currentCursor = document->find(text, prevCursor, isCaseSensitive ? QTextDocument::FindCaseSensitively
                                                                              : QTextDocument::FindFlag());
        if (currentCursor.isNull())
        {
            break;
        }
        auto cursorOnLine = currentCursor;
        cursorOnLine.select(QTextCursor::LineUnderCursor);
        auto lineUnderCursor = cursorOnLine.selectedText();
        auto lineNumber = QString::number(cursorOnLine.blockNumber() + 1);
        auto resText = QString("Line %1\t%2").arg(lineNumber, lineUnderCursor);
        m_searchDock->addAdvSearchResult(resText, currentCursor);
        prevCursor = currentCursor;
        // Process event loop so that gui thread can be updated while searching
        QApplication::processEvents();
    }
    progress.setValue(100);
}

///
/// \brief MainWindow::onSearchResultSelected
/// \param cursor
///
void MainWindow::onSearchResultSelected(const QTextCursor cursor)
{
    auto currentView = (TraceView*)m_tabWidget->currentWidget();
    QList<QTextEdit::ExtraSelection> extraSelections;

    auto cursorOnLine = cursor;
    cursorOnLine.select(QTextCursor::LineUnderCursor);
    QTextEdit::ExtraSelection extraForLine;
    extraForLine.format.setBackground(QColor(Qt::gray).lighter(140));
    extraForLine.cursor = cursorOnLine;
    extraSelections.append(extraForLine);
    // extraForWord must be set after extraForLine
    QTextEdit::ExtraSelection extraForWord;
    extraForWord.format.setBackground(QColor(Qt::green).lighter());
    extraForWord.cursor = cursor;
    extraSelections.append(extraForWord);

    currentView->setExtraSelections(extraSelections);
    currentView->setTextCursor(cursor);
    currentView->moveCursor(QTextCursor::EndOfWord);
    currentView->moveCursor(QTextCursor::Right);
}

///
/// \brief MainWindow::hightlightAllOccurrences
///
void MainWindow::hightlightAllOccurrences()
{
    auto text = m_searchDock->getSearchText();
    auto isCaseSensitive = m_searchDock->isCaseSensitiveChecked();
    auto currentView = (TraceView*)m_tabWidget->currentWidget();
    auto document = currentView->document();
    QTextCursor prevCursor;
    auto lighterYellow = QColor(Qt::yellow).lighter(140);

    QList<QTextEdit::ExtraSelection> extraSelections;
    while (true)
    {
        auto currentCursor = document->find(text, prevCursor, isCaseSensitive ? QTextDocument::FindCaseSensitively
                                                                              : QTextDocument::FindFlag());
        if (currentCursor.isNull())
        {
            break;
        }
        QTextEdit::ExtraSelection extra;
        extra.format.setBackground(lighterYellow);
        extra.cursor = currentCursor;
        extraSelections.append(extra);
        prevCursor = currentCursor;
    }
    currentView->setExtraSelections(extraSelections);
    m_isOccurrencesHighlighted = true;
}

///
/// \brief MainWindow::clearOccurrencesHighlight
///
void MainWindow::clearOccurrencesHighlight()
{
    auto currentView = (TraceView*)m_tabWidget->currentWidget();
    currentView->setExtraSelections(QList<QTextEdit::ExtraSelection>());
    m_isOccurrencesHighlighted = false;
}

///
/// \brief MainWindow::copy
///
void MainWindow::copy()
{
    auto currentView = (TraceView*)m_tabWidget->currentWidget();
    currentView->copy();
}

///
/// \brief MainWindow::onCopyAvailable
/// \param a
///
void MainWindow::onCopyAvailable(bool a)
{
    m_copyAct->setEnabled(a);
}

///
/// \brief MainWindow::clear
///
void MainWindow::clear()
{
    auto currentView = (TraceView*)m_tabWidget->currentWidget();
    currentView->clear();
}

///
/// \brief MainWindow::about
///
void MainWindow::about()
{
    QMessageBox::about(this, tr("About TraceTerminal++"),
                       tr("<b>TraceTerminal++</b> offers view, search in file, export/import trace file in-place. "
                          "Inspired by TraceTerminal app.<br>"
                          "Author: Nhodang"));
}
