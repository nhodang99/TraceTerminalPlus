#include "inc/traceview.h"
#include "inc/mainwindow.h"
#include "inc/customhighlightdialog.h"
#include "inc/constants.h"
#include <QtWidgets>
#include <QDialog>
#include <QSettings>
#include <QtConcurrent/QtConcurrentRun>
#include <QGuiApplication>

TraceView::TraceView(bool live, bool autoscroll)
    : m_liveview(live)
    , m_autoScroll(autoscroll)
{
    setReadOnly(true);
    setAcceptRichText(true);
    setLineWrapMode(QTextEdit::NoWrap);
    setStyleSheet("white-space: pre");
    QFont font("Consolas", 10, QFont::Medium);
    setFont(font);

    QSettings settings(Config::CONFIG_DIR, QSettings::IniFormat);
    m_remoteAddress = settings.value(Config::REMOTE_ADDRESS, QString("192.168.137.1")).toString();

    m_highlighter = new TraceHighlighter(this->document());

    createActions();
}

TraceView::~TraceView()
{
    m_highlighter->deleteLater();
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
/// \brief TraceView::createActions
///
void TraceView::createActions()
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
                              "Search multiple texts with syntax <text1 -AND text2...>");
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
    m_setAutoScrollAct->setStatusTip("If false, the trace is not automatically scroll if new trace comes");
    m_setAutoScrollAct->setIcon(QIcon(":/img/checkmark.png"));
    m_setAutoScrollAct->setEnabled(m_liveview);
    m_setAutoScrollAct->setIconVisibleInMenu(m_autoScroll);
    connect(m_setAutoScrollAct, &QAction::triggered, this, &TraceView::toggleAutoScroll);

    m_setPortAct = new QAction(QString("Configure Port - [%1]")
                                   .arg(QString::number(m_currentPort)), this);
    m_setPortAct->setStatusTip("Set port to connect to UDP connection. Default 911.");
    m_setPortAct->setEnabled(m_liveview);
    connect(m_setPortAct, &QAction::triggered, this, &TraceView::changePort);

    m_setCustomHighlightAct = new QAction("Custom Highlights", this);
    m_setCustomHighlightAct->setStatusTip("Set custom highlights for traces. Avoid changing custom highlight when "
                                          "receving traces, it can cause lagging to the tool.");
    connect(m_setCustomHighlightAct, &QAction::triggered, this, &TraceView::setCustomHighlights);

    // Set interface and port action
    createSetHostActions();
}

///
/// \brief TraceView::createSetHostActions
///
void TraceView::createSetHostActions()
{
    m_setAnyItfAct = new QAction("Any Interface", this);
    m_setAnyItfAct->setStatusTip("Listen to any interface. Equivalent to custom address 0.0.0.0");
    m_setAnyItfAct->setIcon(QIcon(":/img/checkmark.png"));
    m_setAnyItfAct->setEnabled(m_liveview);
    connect(m_setAnyItfAct, &QAction::triggered, this, [=](){
        changeInterface(SpecialInterface::ANY_INTERFACE);
    });

    m_setAnyItfIpv6Act = new QAction("Any Interface IPv6", this);
    m_setAnyItfIpv6Act->setStatusTip("Listen to any IPv6 interface. Equivalent to custom address ::");
    m_setAnyItfIpv6Act->setIcon(QIcon(":/img/checkmark.png"));
    m_setAnyItfIpv6Act->setEnabled(m_liveview);
    connect(m_setAnyItfIpv6Act, &QAction::triggered, this, [=](){
        changeInterface(SpecialInterface::ANY_IPV6_INTERFACE);
    });

    m_setLocalItfAct = new QAction("Local Interface", this);
    m_setLocalItfAct->setStatusTip("Listen to local host only. Equivalent to custom address 127.0.0.1");
    m_setLocalItfAct->setIcon(QIcon(":/img/checkmark.png"));
    m_setLocalItfAct->setEnabled(m_liveview);
    connect(m_setLocalItfAct, &QAction::triggered, this, [=](){
        changeInterface(SpecialInterface::LOCALHOST_INTERFACE);
    });

    m_setRemoteItfAct = new QAction(QString("Remote Interface... - [%1]")
                                        .arg(m_remoteAddress), this);
    m_setRemoteItfAct->setStatusTip("Connect to an interface set by user. Default 192.168.137.1");
    m_setRemoteItfAct->setIcon(QIcon(":/img/checkmark.png"));
    m_setRemoteItfAct->setEnabled(m_liveview);
    connect(m_setRemoteItfAct, &QAction::triggered, this, &TraceView::promptAndSetRemoteInterface);

    m_setSerialItfAct = new QAction("Serial interface", this);
    m_setSerialItfAct->setStatusTip("Listen to serial interface only");
    m_setSerialItfAct->setIcon(QIcon(":/img/checkmark.png"));
    m_setSerialItfAct->setEnabled(m_liveview);
    connect(m_setSerialItfAct, &QAction::triggered, this, [=](){
        changeInterface(SpecialInterface::SERIAL_INTERFACE);
    });
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
/// \brief TraceView::setHost
/// \param addr
///
void TraceView::changeInterface(const QString& addr)
{
    if (addr.isEmpty() || !m_liveview)
    {
        return;
    }
    emit interfaceChangeRequested(addr);
}

///
/// \brief TraceView::promptAndSetRemoteInterface
///
void TraceView::promptAndSetRemoteInterface()
{
    if (!m_liveview)
    {
        return;
    }
    bool ok;
    auto addr = QInputDialog::getText(this, "Set Remote Interface",
                                      "Remote Address:", QLineEdit::Normal,
                                      m_remoteAddress, &ok,
                                      Qt::WindowSystemMenuHint | Qt::WindowCloseButtonHint);
    addr = addr.trimmed();
    if (ok && !addr.isEmpty())
    {
        if (toAction(addr) == m_setRemoteItfAct)
        {
            m_remoteAddress = addr;
        }
        // Else in case user set the address of special interface,
        // We active special action instead of Remote Interface and keep m_remoteAddress
        emit interfaceChangeRequested(addr);
    }
}

///
/// \brief getRemoteAddress
/// \return
///
QString TraceView::getRemoteAddress() const
{
    return m_remoteAddress;
}

///
/// \brief TraceView::setPort
///
void TraceView::changePort()
{
    bool ok;
    int port = QInputDialog::getInt(this, "Set port",
                                    "Port number (1 - 65535)",
                                    m_currentPort, 1, 65535, 1, &ok,
                                    Qt::WindowSystemMenuHint | Qt::WindowCloseButtonHint);
    if (ok)
    {
        emit portChangeRequested((quint16)port);
    }
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
/// \brief TraceView::setAutoScroll
///
void TraceView::toggleAutoScroll()
{
    m_autoScroll = !m_autoScroll;
    m_setAutoScrollAct->setIconVisibleInMenu(m_autoScroll);
    // @CHECK: up one line so that append does not auto scroll the view when cursor at the end of view
    if (!m_autoScroll)
    {
        moveCursor(QTextCursor::Up);
    }
}

///
/// \brief TraceView::onNewTracesReady
/// \param traces
///
void TraceView::onNewTracesReady(QStringList traces)
{
    if (!m_liveview)
    {
        return;
    }

    // Append line by line help us using blockNumber() to get the line number when searching
//    int count = 0;
    for (const auto& trace : traces)
    {
        append(trace);
        if (m_autoScroll)
        {
            moveCursor(QTextCursor::End);
        }

//        // The view only updated when control return to event loop
//        // so we need to call processEvents every 2 lines to create a scroll effect for the incoming traces
//        // Tested, the count reset when reach 2 produces the best effect
//        count++;
//        if (count == 2)
//        {
//            QGuiApplication::processEvents(QEventLoop::ExcludeUserInputEvents |
//                                           QEventLoop::ExcludeSocketNotifiers);
//            count = 0;
//        }
    }

    // If user is searching text, highlight also incoming text
    auto mainWindow = (MainWindow*)this->nativeParentWidget();
    if (mainWindow->isOccurrencesHighlighted())
    {
        mainWindow->hightlightAllOccurrences();
    }
}

///
/// \brief TraceView::onSocketBindResult
/// \param success
///
void TraceView::onSocketBindResult(QString addr, quint16 port, bool success)
{
    if (!m_liveview)
    {
        return;
    }

    auto act = toAction(addr);
    if (!act)
    {
        QMessageBox::critical(this, "ERROR!!!", "Please restart the application");
        return;
    }
    if (m_lastSetItfAct)
    {
        m_lastSetItfAct->setIconVisibleInMenu(false);
    }
    act->setIconVisibleInMenu(true);

    if (act == m_setRemoteItfAct)
    {
        // Display current remote address right in the action
        // Eg.: Remote interface... - [192.168.137.1]
        QString setHostActTitle = QString("Remote interface... - [%1]")
                                      .arg(m_remoteAddress);
        m_setRemoteItfAct->setText(setHostActTitle);
    }
    m_currentPort = port;
    // Display current port number right in the action. Eg.: Configure Port - [800]
    QString setPortActTitle = QString("Configure Port - [%1]").arg(QString::number(m_currentPort));
    m_setPortAct->setText(setPortActTitle);

    QString msg;
    if (success)
    {
        msg = QString("<span style=\"color:black\">>Binding to %1").arg(addr);
        if (act != m_setSerialItfAct)
        {
            msg += QString(":%1").arg(QString::number(m_currentPort));
        }
        msg += " interface OK</span>";
    }
    else
    {
        // For remote interface, maybe it is not avaiable in the network at the moment
        // But maybe it will later, so we need to continuously retry binding to it
        if (act == m_setRemoteItfAct)
        {
            // Find next step of waiting bar
            int idx = m_waitingStep.indexOf("0");
            int nextIdx = (idx + 1) % m_waitingStep.length();
            m_waitingStep.replace(idx, 1, "o");
            m_waitingStep.replace(nextIdx, 1, "0");

            // Remove the old step to replace by new step
            if (m_lastSetItfAct && m_lastSetItfAct == act)
            {
                auto cursor = textCursor();
                cursor.movePosition(QTextCursor::End);
                cursor.movePosition(QTextCursor::StartOfLine, QTextCursor::KeepAnchor);
                QString interfaceAndPort = cursor.selectedText().split(" ").at(2);

                if (interfaceAndPort == QString("%1:%2").arg(m_remoteAddress,
                                                             QString::number(m_currentPort)))
                {
                    cursor.movePosition(QTextCursor::End);
                    cursor.movePosition(QTextCursor::Left, QTextCursor::KeepAnchor, m_waitingStep.length());
                    // Replace old waiting step by the new one
                    cursor.insertHtml(QString("<span style=\"color:blue\">%1</span>").arg(m_waitingStep));
                    m_lastSetItfAct = act;
                    return;
                }
            }
            // In other case, we append a full message
            msg = QString("<span style=\"color:blue\">>Binding to %1:%2 interface...   %3</span>")
                      .arg(addr, QString::number(m_currentPort), m_waitingStep);
        }
        else
        {
            msg = QString("<span style=\"color:red\">>Binding to %1").arg(addr);
            if (act != m_setSerialItfAct)
            {
                msg += QString(":%1").arg(QString::number(m_currentPort));
            }
            msg += " interface failed. Please check if other application is taking over the address.</span>";
        }
    }

    m_lastSetItfAct = act;
    append(msg);
    if (m_autoScroll)
    {
        moveCursor(QTextCursor::End);
    }
}

///
/// \brief TraceView::onHighlightingChanged
///
void TraceView::onHighlightingChanged()
{
    m_highlightUpdated = false;
    m_askedToUpdateHighlight = false;
}

void TraceView::updateHighlighting()
{
    if (!m_highlighter) return;

    m_highlightUpdated = true;
    m_askedToUpdateHighlight = true;
    m_highlighter->rehighlight();
}

void TraceView::setAskedToUpdateHighlight()
{
    m_askedToUpdateHighlight = true;
}

void TraceView::disableCustomHighlighting()
{
    delete m_highlighter;
    m_highlighter = nullptr;
}
