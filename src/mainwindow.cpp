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
    bool autoscroll = settings.value(Config::TRACEVIEW_AUTOSCROLL, true).toBool();
    QString remoteAddress = settings.value(Config::REMOTE_ADDRESS, QString("192.168.137.1")).toString();
    m_liveView = new TraceView(true, autoscroll);
    m_liveView->setRemoteAddress(remoteAddress);
    m_tabWidget->addTab(m_liveView, "Live Trace");
    // Hide the close button of live view
    // @TODO: How to detect icon position?
    m_tabWidget->tabBar()->tabButton(0, QTabBar::RightSide)->hide();

    // Search dock
    bool caseSensitive = settings.value(Config::SEARCH_CASESENSITIVE, false).toBool();
    bool loopSearch = settings.value(Config::SEARCH_LOOPSEARCH, false).toBool();
    m_searchDock = new SearchDock(this, caseSensitive, loopSearch);
    addDockWidget(Qt::BottomDockWidgetArea, m_searchDock);
    m_searchDock->hide();

    // Create app's menus and actions
    createActions();
    createMenus();

    QString message = "A context menu is available by right-clicking";
    statusBar()->setStyleSheet("color: indigo");
    statusBar()->showMessage(message);

    // Init the main window property
    setWindowTitle("TraceTerminal++ - Live View");
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
    m_openAct = new QAction("Open...", this);
    m_openAct->setShortcuts(QKeySequence::Open);
    m_openAct->setStatusTip("Open an existing file");
    connect(m_openAct, &QAction::triggered, this, &MainWindow::open);

    m_saveAct = new QAction("Save", this);
    m_saveAct->setShortcuts(QKeySequence::Save);
    m_saveAct->setStatusTip("Save the document to disk");
    connect(m_saveAct, &QAction::triggered, this, &MainWindow::save);

    m_searchAct = new QAction("Search...", this);
    m_searchAct->setShortcuts(QKeySequence::Find);
    m_searchAct->setStatusTip("Search in the document. Use shortcut Ctrl+F for normal search, Ctrl+Shift+F for advanced search.");
    connect(m_searchAct, &QAction::triggered, this, [=](){
        this->showSearchDock();
    });
    QShortcut* advSearchShortcut = new QShortcut(QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_F), this);
    connect(advSearchShortcut, &QShortcut::activated, this, [=](){
        this->showSearchDock(true);
    });

    m_exitAct = new QAction("Exit", this);
    m_exitAct->setShortcuts(QKeySequence::Quit);
    m_exitAct->setStatusTip("Exit the application");
    connect(m_exitAct, &QAction::triggered, this, &QWidget::close);

    m_copyAct = new QAction("Copy", this);
    m_copyAct->setShortcuts(QKeySequence::Copy);
    m_copyAct->setStatusTip("Copy the current selection's contents to the "
                               "clipboard");
    m_copyAct->setEnabled(false);
    connect(m_copyAct, &QAction::triggered, this, &MainWindow::copy);
    connect(m_liveView, &TraceView::copyAvailable, this, &MainWindow::onCopyAvailable);

    m_clearAct = new QAction("Clear", this);
    m_clearAct->setShortcuts(QKeySequence::Refresh);
    m_clearAct->setStatusTip("Clear all the traces");
    connect(m_clearAct, &QAction::triggered, this, &MainWindow::clear);

    m_aboutAct = new QAction("About", this);
    m_aboutAct->setStatusTip("Show the application's About box");
    connect(m_aboutAct, &QAction::triggered, this, &MainWindow::about);
}

///
/// \brief MainWindow::createMenus
///
void MainWindow::createMenus()
{
    m_fileMenu = menuBar()->addMenu("File");
    m_fileMenu->addAction(m_openAct);
    m_fileMenu->addAction(m_saveAct);
    m_fileMenu->addAction(m_searchAct);
    m_fileMenu->addSeparator();
    m_fileMenu->addAction(m_exitAct);

    m_editMenu = menuBar()->addMenu("Edit");
    m_editMenu->addAction(m_copyAct);
    m_editMenu->addSeparator();
    m_editMenu->addAction(m_clearAct);

    m_helpMenu = menuBar()->addMenu("Help");
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
    settings.setValue(Config::REMOTE_ADDRESS, m_liveView->getRemoteAddress());
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
        if (m_viewInAdvSearch == pView)
        {
            m_viewInAdvSearch = nullptr;
        }
    }
}

///
/// \brief MainWindow::onCurrentTabChanged
/// \param index
///
void MainWindow::onCurrentTabChanged(int index)
{
    QString tabName = m_tabWidget->tabText(index);
    setWindowTitle(QString("TraceTerminal++ - %1").arg(tabName));
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
    QFileDialog fileDialog(this, "TraceTerminal++ - Open");
    fileDialog.setNameFilter("Trace files (*.txt *.html)");
    if (!fileDialog.exec())
        return;

    auto fileUrls = fileDialog.selectedFiles();
    if (fileUrls.isEmpty())
        return;

    foreach(auto& url, fileUrls)
    {
        openFile(url);
    }
}

///
/// \brief MainWindow::openFile
/// \param url
///
void MainWindow::openFile(const QString& url)
{
    QFileInfo fileInfo(url);
    for (int i = 1; i < m_tabWidget->count(); ++i) // exclude live view
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

    QTextStream in(&file);
    if (fileInfo.suffix() == "txt")
    {
        while (!in.atEnd())
        {
            if (progress.wasCanceled())
            {
                offlineView->append("<span style=\"color:orange\">Process aborted.</span>");
                break;
            }
            QString line = in.readLine();
            TraceManager::instance().processTraceLine(line);
            offlineView->append(line);
            // Process event loop so that gui thread can be updated while appending the text still occuring
            QGuiApplication::processEvents(QEventLoop::ExcludeSocketNotifiers);
        }
    }
    else
    {
        // @TODO: read html is a bit laggy because using readAll, but
        // append line by line is slower than setHtml
        offlineView->setHtml(in.readAll());
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
        QString selectedText = currentView->textCursor().selectedText();
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
    bool isCaseSensitive = m_searchDock->isCaseSensitiveChecked();
    bool isLoopSearch = m_searchDock->isLoopSearchChecked();
    if (newSearch)
    {
        m_lastSearchCursor = QTextCursor();
    }

    // Support searching multiple texts. Syntax: text1 -AND text2...
    auto texts = m_searchDock->getSearchText().split(" -AND ");
    QTextCursor foundCursor;
    foreach (const QString& text, texts)
    {
        // If new word being searched: start finding from the beginning
        // If continue to search the next result, search from the last found cursor
        auto cursor = document->find(text,
                                     m_lastSearchCursor,
                                     isCaseSensitive ? QTextDocument::FindCaseSensitively : QTextDocument::FindFlag());
        if (!cursor.isNull()
            && (foundCursor.isNull() || (!foundCursor.isNull() && cursor < foundCursor)))
        {
            foundCursor = cursor;
        }
    }

    if (foundCursor.isNull() && isLoopSearch)
    {
        statusBar()->showMessage("The end of document has been reached, searching from the start",
                                 2000);
        // Maybe the end of file, loop search checked, search from the beginning
        foreach (const QString& text, texts)
        {
            auto tmpCursor = document->find(text,
                                            QTextCursor(),
                                            isCaseSensitive ? QTextDocument::FindCaseSensitively : QTextDocument::FindFlag());
            if (!tmpCursor.isNull()
                && (foundCursor.isNull() || (!foundCursor.isNull() && tmpCursor < foundCursor)))
            {
                foundCursor = tmpCursor;
            }
        }
    }

    if (foundCursor.isNull())
    {
        statusBar()->showMessage("The end of document has been reached",
                                 2000);
        return;
    }

    // Highlight again the found text and extra highlight the next text found
    if (newSearch)
    {
        clearOccurrencesHighlight();
    }
    hightlightAllOccurrences();
    auto extraSelections = currentView->extraSelections();

    bool lastExtraFound = false;
    bool newExtraFound = false;
    // Must use the STL foreach here to modify the element
    for (auto& extra : extraSelections)
    {
        if (lastExtraFound && newExtraFound) break;

        if (!newExtraFound && extra.cursor == foundCursor)
        {
            extra.format.setBackground(QColor(Qt::green).lighter());
            newExtraFound = true;
        }
        if (!lastExtraFound && !m_lastSearchCursor.isNull())
        {
            if (extra.cursor == m_lastSearchCursor)
            {
                extra.format.setBackground(QColor(Qt::yellow).lighter(140));
                lastExtraFound = true;
            }
        }
    }

    currentView->setExtraSelections(extraSelections);
    currentView->setTextCursor(foundCursor);
    currentView->moveCursor(QTextCursor::EndOfWord);
    currentView->moveCursor(QTextCursor::Right);

    // Save the found cursor so that we can continue to search from there later
    m_lastSearchCursor = foundCursor;
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
    progress.setValue(0); // Ugly trick...

    bool isCaseSensitive = m_searchDock->isCaseSensitiveChecked();
    auto currentView = (TraceView*)m_tabWidget->currentWidget();
    auto document = currentView->document();

    progress.setValue(50);
    auto texts = m_searchDock->getSearchText().split(" -AND ");
    QTextCursor prevCursor;
    foreach (const auto& text, texts)
    {
        if (progress.wasCanceled())
        {
            break;
        }
        prevCursor = QTextCursor();
        while (true)
        {
            if (progress.wasCanceled())
            {
                statusBar()->showMessage("Advanced search aborted.", 2000);
                break;
            }

            auto currentCursor = document->find(text, prevCursor, isCaseSensitive ? QTextDocument::FindCaseSensitively
                                                                                  : QTextDocument::FindFlag());
            if (currentCursor.isNull())
            {
                break;
            }
            m_searchDock->addAdvSearchResult(currentCursor);
            prevCursor = currentCursor;
            // Process event loop so that gui thread can be updated while searching
            QGuiApplication::processEvents(QEventLoop::ExcludeSocketNotifiers);
        }
    }

    m_searchDock->sortAdvSearchResult();
    m_viewInAdvSearch = currentView;
    progress.setValue(100);
}

///
/// \brief MainWindow::onSearchResultSelected
/// \param cursor
///
void MainWindow::onSearchResultSelected(const QTextCursor cursor)
{
    if (!m_viewInAdvSearch)
    {
        return;
    }

    auto currentView = (TraceView*)m_tabWidget->currentWidget();
    if (currentView != m_viewInAdvSearch)
    {
        m_tabWidget->setCurrentWidget(m_viewInAdvSearch);
        currentView = m_viewInAdvSearch;
    }
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
    auto isCaseSensitive = m_searchDock->isCaseSensitiveChecked();
    auto currentView = (TraceView*)m_tabWidget->currentWidget();
    auto document = currentView->document();
    auto lighterYellow = QColor(Qt::yellow).lighter(140);
    QTextCursor lastHighlightedCursor;
    auto extraSelections = currentView->extraSelections();

    // If there are extra selection before, start from the last cursor of the list
    if (!extraSelections.isEmpty())
    {
//        qDebug() << "Last Highlight: " << extraSelections.last().cursor.position()
//                                       << extraSelections.last().cursor.block().text();
        lastHighlightedCursor = extraSelections.last().cursor;
    }

    // Support searching multiple texts. Syntax: text1 -AND text2...
    auto texts = m_searchDock->getSearchText().split(" -AND ");
    foreach (const QString& text, texts)
    {
        auto prevCursor = lastHighlightedCursor;
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
    }

    if (!extraSelections.isEmpty())
    {
        // Sort by position, so that we can continue to highlight from the last cursor in the list next time
        std::sort(extraSelections.begin(), extraSelections.end(), [&](const auto& a, const auto& b) {
            if (!a.cursor.isNull() && !b.cursor.isNull())
                return a.cursor < b.cursor;
            return false;
        });
//        qDebug() << "Last after sort" << extraSelections.last().cursor.position()
//                                      << extraSelections.last().cursor.block().text();

        currentView->setExtraSelections(extraSelections);
        m_isOccurrencesHighlighted = true;
    }
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
    QMessageBox::about(this, "About TraceTerminal++",
                       "<b>TraceTerminal++</b> offers view, search in file, export/import trace file in-place. "
                          "Inspired by TraceTerminal app.<br>"
                          "Author: Nhodang");
}
