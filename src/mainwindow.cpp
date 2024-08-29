#include "inc/mainwindow.h"
#include "inc/constants.h"
#include "inc/searchdock.h"
#include "inc/tracemanager.h"
#include "inc/tracehighlighter.h"
#include <QtWidgets>
#include <QSettings>
#include <QMessageBox>
#include <QtConcurrent/QtConcurrentRun>
#include <QGuiApplication>

///
/// \brief Helper function
///        Create regex to match the query, with operator | take precedence over +
///        E.g: Syntax: text1 | text2 + text3..., we find line containing (text1) or line containing (both text2 and text3)
/// \param query query to match
/// \param caseSensitive whether the search is case sensitive
/// \return list of regexes to match the block for the query
///
static QList<QRegularExpression> getRegexsForQuery(const QString& query, bool caseSensitive)
{
    const char* orSeparator = " | ";
    const char* andSeparator = " + ";

    QList<QRegularExpression> regexs;
    QRegularExpression::PatternOption patternOption = caseSensitive ? QRegularExpression::NoPatternOption
                                                                    : QRegularExpression::CaseInsensitiveOption;
    auto orClauses = query.split(orSeparator);
    foreach (const auto& orClause, orClauses)
    {
        QRegularExpression regex;
        if (orClause.contains(andSeparator))
        {
            QStringList andKeywords = orClause.split(andSeparator);
            QString keywordsAtSameLinePattern = andKeywords.join(".*?");
            regex.setPattern(keywordsAtSameLinePattern);
        }
        else
        {
            regex.setPattern(orClause);
        }
        regex.setPatternOptions(patternOption);
        regexs.append(regex);
    }
    return regexs;
}

MainWindow::MainWindow(LiveTraceView* liveView, SearchDock* searchDock)
    : m_liveView(liveView)
    , m_searchDock(searchDock)
{
    QSettings settings(Config::CONFIG_DIR, QSettings::IniFormat);

    // Main tab widget
    m_tabWidget = new QTabWidget(this);
    m_tabWidget->setTabsClosable(true);
    setCentralWidget(m_tabWidget);

    // Live traceview
    m_tabWidget->addTab(m_liveView, "Live Trace");
    // Hide the close button of live view
    if (m_tabWidget->tabBar()->tabButton(0, QTabBar::RightSide))
    {
        m_tabWidget->tabBar()->tabButton(0, QTabBar::RightSide)->hide();
    }
    else if (m_tabWidget->tabBar()->tabButton(0, QTabBar::RightSide))
    {
        m_tabWidget->tabBar()->tabButton(0, QTabBar::LeftSide)->hide();
    }

    // Search dock
    m_searchDock->setParent(this);
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

    connect(this,         &MainWindow::highlightChanged,     m_liveView, &TraceView::onHighlightingChanged);
    connect(m_tabWidget,  &QTabWidget::tabCloseRequested,    this, &MainWindow::onTabCloseRequested);
    connect(m_tabWidget,  &QTabWidget::currentChanged,       this, &MainWindow::onCurrentTabChanged);
    connect(m_liveView,   &TraceView::copyAvailable,         this, &MainWindow::onCopyAvailable);
    connect(m_searchDock, &SearchDock::search,               this, &MainWindow::onSearchRequested);
    connect(m_searchDock, &SearchDock::searchDockHidden,     this, &MainWindow::onSearchDockHidden);
    connect(m_searchDock, &SearchDock::searchResultSelected, this, &MainWindow::onSearchResultSelected);
    connect(m_searchDock, &SearchDock::clearHighlight,       this, [=](){
        this->clearOccurrencesHighlight();
    });
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

    m_guidelineAct = new QAction("How to use", this);
    m_guidelineAct->setStatusTip("Show the application's guideline");
    connect(m_guidelineAct, &QAction::triggered, this, &MainWindow::guideline);

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
    m_helpMenu->addAction(m_guidelineAct);
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
    settings.setValue(Config::TRACEVIEW_AUTOSCROLL, m_liveView->isAutoScrollEnabled());
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
        if (m_viewInAdvSearch == pView)
        {
            m_viewInAdvSearch = nullptr;
        }
        pView->deleteLater();
    }
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
/// \brief MainWindow::guideline
///
void MainWindow::guideline()
{
    auto text = QString("<ul>"
                        "  <li>Access menu by right-clicking.</li>"
                        "  <li>Set desired network interface and port that you want the tool to receive traces from.</li>"
                        "  <li>To open a file:"
                        "    <ul>"
                        "      <li>File -> Open or simply Drag and drop trace file to app view.</li>"
                        "      <li>File format .txt and .html is supported.</li>"
                        "    </ul>"
                        "  </li>"
                        "  <li>To save a file:"
                        "    <ul>"
                        "      <li>File -> Save or use shortcut Ctrl+S.</li>"
                        "      <li>File can be saved in .txt or .html format.</li>"
                        "    </ul>"
                        "  </li>"
                        "  <li>Search shortcuts:"
                        "    <ul>"
                        "      <li>Ctrl+F to open normal search mode.</li>"
                        "      <li>Ctrl+Shift+F to open advanced search mode.</li>"
                        "    </ul>"
                        "  </li>"
                        "  <li>Multiple texts search:"
                        "    <ul>"
                        "      <li>Use separator \" + \" and \" | \" between texts (text1 | text2 + text3...) to search for multiple texts at once.</li>"
                        "      <li>Operator | takes precedence over +.</li>"
                        "      <li>Example: Syntax: text1 | text2 + text3, we find lines containing (text1) or lines containing (both text2 and text3).</li>"
                        "    </ul>"
                        "  </li>"
                        "</ul>");

    QMessageBox* msgBox = new QMessageBox("How to use", text,
                                          QMessageBox::Information, 0, 0, 0, this);
    msgBox->setAttribute(Qt::WA_DeleteOnClose);
    QIcon icon = msgBox->windowIcon();
    QSize size = icon.actualSize(QSize(64, 64));
    msgBox->setIconPixmap(icon.pixmap(size));
    msgBox->show();
}

///
/// \brief MainWindow::about
///
void MainWindow::about()
{
    QMessageBox::about(this, "About TraceTerminal++",
                       QString("<b>TraceTerminal++</b> offers view, search in file, export/import trace file in-place. "
                               "Inspired by TraceTerminal app.<br>"
                               "Version %1<br>"
                               "Author: Nhodang - ZenRod Team").arg(QApplication::applicationVersion()));
}

///
/// \brief MainWindow::onCurrentTabChanged
/// \param index
///
void MainWindow::onCurrentTabChanged(int index)
{
    QString tabName = m_tabWidget->tabText(index);
    setWindowTitle(QString("TraceTerminal++ - %1").arg(tabName));
    m_tabWidget->tabBar()->setTabTextColor(m_lastTabIndex, QColor(Qt::black));
    m_lastTabIndex = index;

    auto pView = (TraceView*)m_tabWidget->widget(index);

    if (pView->isHighlightUpdated())
    {
        return;
    }
    // Always update the liveview's highlighting
    else if (!pView->isHighlightUpdated() && pView == m_liveView)
    {
        pView->updateHighlighting();
        return;
    }
    else
    {
        m_tabWidget->tabBar()->setTabTextColor(index, QColor(Qt::red));
        if (pView->wasHighlightUpdateRequested())
        {
            return;
        }
    }

    QFuture<void> future;
    int ret = QMessageBox::information(this, "Highlighting Rule",
                                       "The highlighting rule of this view is not up-to-date.<br>"
                                       "Do you want to reload it?<br>"
                                       "<br>"
                                       "<i>Note: This may take some times on large file</i>",
                                       QMessageBox::Yes | QMessageBox::No,
                                       QMessageBox::Yes);
    switch (ret)
    {
    case QMessageBox::Yes:
        future = QtConcurrent::run(pView, &TraceView::updateHighlighting);
        m_tabWidget->tabBar()->setTabTextColor(index, QColor(Qt::black));
        break;
    case QMessageBox::No:
        // Don't Save was clicked
        break;
    }
    pView->setHighlightUpdateRequested();
    future.waitForFinished();
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

    auto offlineView = new TraceView;
    offlineView->setDocumentTitle(fileInfo.fileName());
    m_tabWidget->addTab(offlineView, fileInfo.fileName());
    m_tabWidget->setCurrentIndex(m_tabWidget->count() - 1);
    connect(offlineView, &TraceView::copyAvailable, this, &MainWindow::onCopyAvailable);
    connect(this, &MainWindow::highlightChanged, offlineView, &TraceView::onHighlightingChanged);

    QString data;
    // Start read file concurrently
    QFuture<bool> future = QtConcurrent::run(&TraceManager::instance(), &TraceManager::readFile, url, std::ref(data));

    if (fileInfo.suffix() == "html")
    {
        int ret = QMessageBox::information(this, "Highlighting",
                                           "This file is in rich text format (html) which might already <br>"
                                           "have its own highlight.<br>"
                                           "Do you want to add your highlighting rule? The highlight of <br>"
                                           "the file is still kept but might be overwritten by your rule.<br>"
                                           "<br>"
                                           "<i>Note: This action cannot be undone. You'll need to reopen the file<br>"
                                           "to change it</i>",
                                           QMessageBox::Yes | QMessageBox::No,
                                           QMessageBox::Yes);
        switch (ret)
        {
        case QMessageBox::Yes:
            break;
        case QMessageBox::No:
            disconnect(this, &MainWindow::highlightChanged, offlineView, &TraceView::onHighlightingChanged);
            offlineView->disableCustomHighlighting();
            break;
        }
    }

    QProgressDialog progress("Opening files...", "Abort", 0, 100, this);
    progress.open();

    // Join the thread
    future.waitForFinished();
    if (!future.result())
    {
        QMessageBox::critical(nullptr, "ERROR!!!", "Cannot open trace file");
        return;
    }
    progress.setValue(50);
    QGuiApplication::processEvents(QEventLoop::ExcludeUserInputEvents | QEventLoop::ExcludeSocketNotifiers);

    if (fileInfo.suffix() == "html")
    {
        offlineView->setHtml(data);
    }
    else
    {
        offlineView->setPlainText(data);
    }
    progress.close();
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
        if (!m_searchDock->getQuery().isEmpty() &&
            m_searchDock->getQuery() != selectedText)
        {
            clearOccurrencesHighlight();
        }
        m_searchDock->setQuery(selectedText);
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
    bool isLoopSearch = m_searchDock->isLoopSearchChecked();
    if (newSearch)
    {
        m_lastSearchCursor = QTextCursor();
        clearOccurrencesHighlight();
    }
    hightlightAllOccurrences();

    auto extraSelections = currentView->extraSelections();
    QTextCursor foundCursor;

    if (!extraSelections.isEmpty())
    {
        if ((m_lastSearchCursor != extraSelections.last().cursor))
        {
            if (m_lastSearchCursor.isNull())
            {
                foundCursor = extraSelections.first().cursor;
                extraSelections.first().format.setBackground(QColor(Qt::green).lighter());
            }
            else
            {
                for (int i = 0; i < extraSelections.length(); ++i)
                {
                    if (extraSelections[i].cursor == m_lastSearchCursor)
                    {
                        foundCursor = extraSelections[i + 1].cursor;
                        extraSelections[i].format.setBackground(QColor(Qt::yellow).lighter(140));
                        extraSelections[i + 1].format.setBackground(QColor(Qt::green).lighter());
                        break;
                    }
                }
            }
        }
        else if (isLoopSearch)
        {
            qDebug() << "loop search";
            statusBar()->showMessage("The end of document has been reached, searching from the start",
                                     2000);
            foundCursor = extraSelections.first().cursor;
            extraSelections.last().format.setBackground(QColor(Qt::yellow).lighter(140));
            extraSelections.first().format.setBackground(QColor(Qt::green).lighter());
        }
    }

    if (foundCursor.isNull())
    {
        statusBar()->showMessage("The end of document has been reached",
                                 2000);
        return;
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

    auto currentView = (TraceView*)m_tabWidget->currentWidget();
    auto document = currentView->document();

    progress.setValue(50);

    // Get the regexes to match the search query
    auto regexs = getRegexsForQuery(m_searchDock->getQuery(),
                                    m_searchDock->isCaseSensitiveChecked());
    progress.setValue(75);

    // Match the regular expression against the block's text
    // Here we have each block is one line
    for (QTextBlock block = document->begin(); block.isValid(); block = block.next())
    {
        if (progress.wasCanceled())
        {
            break;
        }

        foreach (const auto& regex, regexs)
        {
            QRegularExpressionMatchIterator matchIterator = regex.globalMatch(block.text());
            while (matchIterator.hasNext())
            {
                // If there is a match, focus cursor the the actual word
                auto match = matchIterator.next();
                QTextCursor cursor(block);
                cursor.setPosition(block.position() + match.capturedStart());
                cursor.setPosition(block.position() + match.capturedEnd(), QTextCursor::KeepAnchor);

                m_searchDock->addAdvSearchResult(cursor);
            }
        }

        // Process event loop so that gui thread can be updated while searching
        QGuiApplication::processEvents(QEventLoop::ExcludeSocketNotifiers);
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
    // the last character of line is " ", do not count it
    // See note in TraceManager::filterIncompletedFromRawData
    cursorOnLine.movePosition(QTextCursor::PreviousCharacter);
    QTextEdit::ExtraSelection extraForLine;
    extraForLine.format.setBackground(QColor(Qt::gray).lighter(140));
    extraForLine.cursor = cursorOnLine;
    extraSelections.append(extraForLine);

    // extraForWord MUST be set after extraForLine
    QTextEdit::ExtraSelection extraForWord;
    extraForWord.format.setBackground(QColor(Qt::green).lighter());
    extraForWord.cursor = cursor;
    extraSelections.append(extraForWord);

    currentView->setExtraSelections(extraSelections);
    currentView->setTextCursor(cursor);
    currentView->moveCursor(QTextCursor::EndOfWord);
    currentView->moveCursor(QTextCursor::Left);
}

///
/// \brief MainWindow::hightlightAllOccurrences
///
void MainWindow::hightlightAllOccurrences()
{
    auto currentView = (TraceView*)m_tabWidget->currentWidget();
    auto lighterYellow = QColor(Qt::yellow).lighter(140);
    auto extraSelections = currentView->extraSelections();

    QTextCursor lastHighlightedCursor(currentView->document());
    if (!extraSelections.isEmpty())
    {
        lastHighlightedCursor = extraSelections.last().cursor;
    }

    // Get the regexes to match the search query
    auto regexs = getRegexsForQuery(m_searchDock->getQuery(),
                                    m_searchDock->isCaseSensitiveChecked());

    auto currentBlock = lastHighlightedCursor.block();
    QList<QTextEdit::ExtraSelection> tempExtraSelections;

    for (QTextBlock block = currentBlock; block.isValid(); block = block.next())
    {
        foreach (const auto& regex, regexs)
        {
            QRegularExpressionMatchIterator matchIterator = regex.globalMatch(block.text());
            while (matchIterator.hasNext())
            {
                // If there is a match, focus cursor in the actual word
                auto match = matchIterator.next();
                QTextCursor cursor(block);
                cursor.setPosition(block.position() + match.capturedStart());
                cursor.setPosition(block.position() + match.capturedEnd(), QTextCursor::KeepAnchor);

                // We find from the beginning of block so the catured might be already in the list before
                if (cursor <= lastHighlightedCursor)
                {
                    continue;
                }

                QTextEdit::ExtraSelection extra;
                extra.format.setBackground(lighterYellow);
                extra.cursor = cursor;
                tempExtraSelections.append(extra);
            }
        }
    }

    // We're sure that items of tempExtraSelections is larger than all items of current extraSelections
    // So just sort the tempExtraSelections and then merge them to the curent one
    if (!tempExtraSelections.isEmpty())
    {
        // Sort by position, so that we can continue to highlight from the last cursor in the list next time
        std::sort(tempExtraSelections.begin(), tempExtraSelections.end(), [&](const auto& a, const auto& b) {
            if (!a.cursor.isNull() && !b.cursor.isNull())
                return a.cursor < b.cursor;
            return false;
        });
        extraSelections.append(tempExtraSelections);
        // qDebug() << "Last after sort" << extraSelections.last().cursor.position()
        //                               << extraSelections.last().cursor.block().text();

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
/// \brief TraceView::setCustomHighlights
///
void MainWindow::setCustomHighlights(const QStringList& highlights)
{
    for (int i = 0; i < highlights.length(); ++i)
    {
        TraceHighlighter::addHighlightingRule(highlights.at(i),
                                              Highlight::defaultCustomHighlights[i]);
    }

    emit highlightChanged();
    auto currentView = (TraceView*)m_tabWidget->currentWidget();
    currentView->updateHighlighting();

    QSettings settings(Config::CONFIG_DIR, QSettings::IniFormat);
    settings.setValue(Config::HIGHLIGHTS, highlights);
}
