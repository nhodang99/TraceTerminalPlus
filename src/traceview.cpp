#include "inc/traceview.h"
#include "inc/mainwindow.h"
#include "inc/customhighlightdialog.h"
#include "inc/tracehighlighter.h"
#include "inc/constants.h"
#include <QtWidgets>
#include <QDialog>
#include <QSettings>
#include <QtConcurrent/QtConcurrentRun>
#include <QGuiApplication>

TraceView::TraceView()
{
    setReadOnly(true);
    setAcceptRichText(true);
    setLineWrapMode(QTextEdit::NoWrap);
    QFont font("Consolas", 10, QFont::Medium);
    setFont(font);

    m_highlighter = new TraceHighlighter(this->document());

    createTraceActions();
    createNetworkActions();
}

TraceView::~TraceView()
{
    if (m_highlighter)
    {
        m_highlighter->deleteLater();
        m_highlighter = nullptr;
    }
}

#ifndef QT_NO_CONTEXTMENU
void TraceView::contextMenuEvent(QContextMenuEvent *event)
{
    QMenu* menu = createStandardContextMenu();
    menu->addAction(m_saveAct);
    menu->addAction(m_searchAct);
    menu->addAction(m_clearAct);
    menu->addAction(m_clearUntilHereAct);
    menu->addSeparator();
    menu->addAction(m_setAutoScrollAct);
    menu->addSeparator();
    menu->addAction(m_setCustomHighlightAct);
    menu->addSeparator();
    menu->addAction(m_setAnyItfAct);
    menu->addAction(m_setAnyItfIpv6Act);
    menu->addAction(m_setLocalItfAct);
    menu->addAction(m_setRemoteItfAct);
    menu->addAction(m_setSerialItfAct);
    menu->addSeparator();
    menu->addAction(m_setPortAct);

    menu->exec(event->globalPos());
    delete menu;
}
#endif // QT_NO_CONTEXTMENU

///
/// \brief TraceView::mousePressEvent override
/// \param event
///
void TraceView::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::RightButton)
    {
        // Need to save the current cursor when right-click
        // to use for action Clear until here
        m_clearUntilCursor = cursorForPosition(event->pos());
    }
    QTextEdit::mousePressEvent(event);
}

///
/// \brief TraceView::createTraceActions
///
void TraceView::createTraceActions()
{
    m_saveAct = new QAction("Save", this);
    m_saveAct->setShortcuts(QKeySequence::Save);
    m_saveAct->setStatusTip("Save the document to disk");
    m_saveAct->setEnabled(false);
    connect(m_saveAct, &QAction::triggered, this, &TraceView::save);
    connect(this, &QTextEdit::textChanged, m_saveAct, [this](){
        m_saveAct->setEnabled(!this->document()->isEmpty());
    });

    m_searchAct = new QAction("Search...", this);
    m_searchAct->setShortcuts(QKeySequence::Find);
    m_searchAct->setStatusTip("Search text in the current view. Shortcut Ctrl+F for normal search, Ctrl+Shift+F for advanced search. "
                              "Search multiple texts with syntax <text1 | text2 + text3...>");
    connect(m_searchAct, &QAction::triggered, this, [=](){
        auto mainWindow = (MainWindow*)this->nativeParentWidget();
        mainWindow->showSearchDock();
    });

    m_clearAct = new QAction("Clear", this);
    m_clearAct->setShortcuts(QKeySequence::Refresh);
    m_clearAct->setStatusTip("Clear all the traces");
    connect(m_clearAct, &QAction::triggered, this, &TraceView::clear);

    m_clearUntilHereAct = new QAction("Clear until here", this);
    m_clearUntilHereAct->setStatusTip("Clear all the traces until the pointer");
    connect(m_clearUntilHereAct, &QAction::triggered, this, &TraceView::clearUntilHere);

    m_setAutoScrollAct = new QAction("Autoscroll", this);
    m_setAutoScrollAct->setEnabled(false);
    m_setAutoScrollAct->setIconVisibleInMenu(false);
    m_setAutoScrollAct->setStatusTip("If false, the trace is not automatically scroll if new trace comes");
    m_setAutoScrollAct->setIcon(QIcon(":/img/checkmark.png"));

    m_setCustomHighlightAct = new QAction("Custom Highlights", this);
    m_setCustomHighlightAct->setStatusTip("Set custom highlights for traces. Avoid changing custom highlight when "
                                          "receving traces, it can cause lagging to the tool.");
    connect(m_setCustomHighlightAct, &QAction::triggered, this, &TraceView::setCustomHighlights);
}

///
/// \brief TraceView::createNetworkActions
///
void TraceView::createNetworkActions()
{
    m_setAnyItfAct = new QAction("Any Interface", this);
    m_setAnyItfAct->setEnabled(false);
    m_setAnyItfAct->setStatusTip("Listen to any interface. Equivalent to custom address 0.0.0.0");
    m_setAnyItfAct->setIcon(QIcon(":/img/checkmark.png"));

    m_setAnyItfIpv6Act = new QAction("Any Interface IPv6", this);
    m_setAnyItfIpv6Act->setEnabled(false);
    m_setAnyItfIpv6Act->setStatusTip("Listen to any IPv6 interface. Equivalent to custom address ::");
    m_setAnyItfIpv6Act->setIcon(QIcon(":/img/checkmark.png"));

    m_setLocalItfAct = new QAction("Local Interface", this);
    m_setLocalItfAct->setEnabled(false);
    m_setLocalItfAct->setStatusTip("Listen to local host only. Equivalent to custom address 127.0.0.1");
    m_setLocalItfAct->setIcon(QIcon(":/img/checkmark.png"));

    m_setRemoteItfAct = new QAction("Remote Interface", this);
    m_setRemoteItfAct->setEnabled(false);
    m_setRemoteItfAct->setStatusTip("Connect to an interface set by user. Default 192.168.137.1");
    m_setRemoteItfAct->setIcon(QIcon(":/img/checkmark.png"));

    m_setSerialItfAct = new QAction("Serial interface", this);
    m_setSerialItfAct->setEnabled(false);
    m_setSerialItfAct->setStatusTip("Listen to serial interface only");
    m_setSerialItfAct->setIcon(QIcon(":/img/checkmark.png"));

    m_setPortAct = new QAction("Configure Port", this);
    m_setPortAct->setEnabled(false);
    m_setPortAct->setStatusTip("Set port to connect to UDP connection. Default 911.");
}

///
/// \brief TraceView::toAction
/// \param addr
/// \return
///
QAction* TraceView::toAction(QString& addr) const
{
    QHash<QString, QAction*> hash = {
        { SpecialInterface::ANY_INTERFACE,       m_setAnyItfAct },
        { SpecialInterface::ANY_IPV6_INTERFACE,  m_setAnyItfIpv6Act },
        { SpecialInterface::LOCALHOST_INTERFACE, m_setLocalItfAct },
        { SpecialInterface::SERIAL_INTERFACE,    m_setSerialItfAct }
    };
    auto action = hash[addr];
    if (!action)
    {
        return m_setRemoteItfAct;
    }
    return action;
}

///
/// \brief TraceView::setCustomHighlights
///
void TraceView::setCustomHighlights()
{
    bool ok;
    QStringList highlights = CustomHighlightDialog::getStrings(nullptr, &ok);
    if (!ok)
    {
        return;
    }

    auto mainWindow = (MainWindow*)this->nativeParentWidget();
    mainWindow->setCustomHighlights(highlights);
}

///
/// \brief TraceView::save
///
void TraceView::save()
{
    static QRegularExpression noHyphensAndColons("[-:]");
    QString strDateTime = QDateTime::currentDateTime().toString(Qt::ISODate)
                                                      .remove(noHyphensAndColons);
    QString defaultName = "FMTrace_" + strDateTime + "_TraceName.txt";
    QString filename = QFileDialog::getSaveFileName(this, "TraceTerminal++ - Save file",
                                                    defaultName, "Plain text(*.txt);;Rich text (*.html)");
    if (filename.isEmpty())
    {
        return;
    }

    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QMessageBox::critical(this, "ERROR!!!", "Cannot creating the file");
        return;
    }
    QTextStream out(&file);
    if (filename.endsWith(".html"))
    {
        out << toHtmlWithAdditionalFormats();
    }
    else
    {
        out << toPlainText();
    }
    file.close();

    // Open saved directory
    QFileInfo newFileDir(file);
    QProcess::startDetached("explorer.exe", {"/select,", QDir::toNativeSeparators(newFileDir.absoluteFilePath())});
}

///
/// \brief get the formatted string of the text in the layout
///        QSyntaxHighlighter add additional formats to the text layout
///        that a simple QTextEdit::toHtml() cannot save it
/// \return text formatted html string
///
QString TraceView::toHtmlWithAdditionalFormats()
{
    auto text = toPlainText();

    // Create a new document from all the selected text document.
    QTextCursor cursor(document());
    cursor.select(QTextCursor::Document);
    QTextDocument* tempDocument(new QTextDocument);
    Q_ASSERT(tempDocument);
    QTextCursor tempCursor(tempDocument);

    tempCursor.insertFragment(cursor.selection());
    tempCursor.select(QTextCursor::Document);

    // Apply the additional formats set by the syntax highlighter
    QTextBlock start = document()->findBlock(cursor.selectionStart());
    QTextBlock end = document()->findBlock(cursor.selectionEnd());
    end = end.next();
    int selectionStart = cursor.selectionStart();
    int endOfDocument = tempDocument->characterCount() - 1;
    for (QTextBlock current = start; current.isValid() and current != end; current = current.next()) {
        const QTextLayout* layout(current.layout());

        foreach (const QTextLayout::FormatRange &range, layout->formats()) {
            int start = current.position() + range.start - selectionStart;
            int end = start + range.length;
            if (end <= 0 || start >= endOfDocument)
                continue;
            tempCursor.setPosition(qMax(start, 0));
            tempCursor.setPosition(qMin(end, endOfDocument), QTextCursor::KeepAnchor);
            tempCursor.setCharFormat(range.format);
        }
    }

    // Reset the user states since they are not interesting
    for(QTextBlock block = tempDocument->begin(); block.isValid(); block = block.next())
        block.setUserState(-1);

    // Make sure the text appears pre-formatted, and set the background we want.
    tempCursor.select(QTextCursor::Document);

    // Finally retreive the syntax higlighted and formatted html.
    text = tempCursor.selection().toHtml();
    delete tempDocument;

    return text;
}

///
/// \brief TraceView::clear
///
void TraceView::clear()
{
    QTextEdit::clear();
}

///
/// \brief TraceView::clearUntilHere
///
void TraceView::clearUntilHere()
{
    auto cursor = textCursor();
    int mousePos = m_clearUntilCursor.position();

    // Select the text block from start to the end of this line (including line break)
    cursor.movePosition(QTextCursor::Start);
    cursor.setPosition(mousePos, QTextCursor::KeepAnchor);
    cursor.movePosition(QTextCursor::EndOfLine, QTextCursor::KeepAnchor);
    cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor);
    cursor.removeSelectedText();
}

///
/// \brief TraceView::onHighlightingChanged
///
void TraceView::onHighlightingChanged()
{
    m_highlightUpdated = false;
    m_highlightUpdateRequested = false;
}

///
/// \brief TraceView::updateHighlighting
///
void TraceView::updateHighlighting()
{
    if (!m_highlighter) return;

    m_highlightUpdated = true;
    m_highlightUpdateRequested = true;
    m_highlighter->rehighlight();
}

///
/// \brief TraceView::setHighlightUpdateRequested
///
void TraceView::setHighlightUpdateRequested()
{
    m_highlightUpdateRequested = true;
}

///
/// \brief TraceView::disableCustomHighlighting
///
void TraceView::disableCustomHighlighting()
{
    delete m_highlighter;
    m_highlighter = nullptr;
}
