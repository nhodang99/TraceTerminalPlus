#include "inc/traceview.h"
#include "inc/mainwindow.h"
#include "inc/customhighlightdialog.h"
#include "inc/tracemanager.h"
#include <QtWidgets>
#include <QDialog>

TraceView::TraceView(bool live, bool autoscroll)
    : m_liveview(live)
    , m_autoScroll(autoscroll)
{
    setReadOnly(true);
    setAcceptRichText(true);
    setLineWrapMode(QTextEdit::NoWrap);
    setStyleSheet("white-space: pre");
    QFont font("Helvetica");
    font.setPointSize(10);
    setFont(font);

    createActions();
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
    m_saveAct = new QAction(tr("&Save"), this);
    m_saveAct->setShortcuts(QKeySequence::Save);
    m_saveAct->setStatusTip(tr("Save the document to disk"));
    m_saveAct->setEnabled(false);
    connect(m_saveAct, &QAction::triggered, this, &TraceView::save);
    connect(this, &QTextEdit::textChanged, m_saveAct, [this](){
        m_saveAct->setEnabled(!this->document()->isEmpty());
    });

    m_searchAct = new QAction(tr("&Search..."), this);
    m_searchAct->setShortcuts(QKeySequence::Find);
    m_searchAct->setStatusTip(tr("Search in the document. Use shortcut Ctrl+F for normal search, Ctrl+Shift+F for advanced search."));
    connect(m_searchAct, &QAction::triggered, this, [=](){
        auto mainWindow = (MainWindow*)this->nativeParentWidget();
        mainWindow->showSearchDock();
    });

    m_clearAct = new QAction(tr("&Clear"), this);
    m_clearAct->setShortcuts(QKeySequence::Refresh);
    m_clearAct->setStatusTip(tr("Clear all the traces"));
    connect(m_clearAct, &QAction::triggered, this, &TraceView::clear);

    m_clearUntilHereAct = new QAction(tr("&Clear until here"), this);
    m_clearUntilHereAct->setStatusTip(tr("Clear all the traces until the pointer"));
    connect(m_clearUntilHereAct, &QAction::triggered, this, &TraceView::clearUntilHere);

    m_setAutoScrollAct = new QAction(tr("&Autoscroll"), this);
    m_setAutoScrollAct->setStatusTip(tr("If false, the trace is not automatically scroll if new trace comes"));
    m_setAutoScrollAct->setIcon(QIcon(":/img/checkmark.png"));
    m_setAutoScrollAct->setEnabled(m_liveview);
    m_setAutoScrollAct->setIconVisibleInMenu(m_autoScroll);
    connect(m_setAutoScrollAct, &QAction::triggered, this, &TraceView::toggleAutoScroll);

    m_setPortAct = new QAction("Configure Port", this);
    m_setPortAct->setStatusTip("Set port to connect to UDP connection");
    m_setPortAct->setEnabled(m_liveview);
    connect(m_setPortAct, &QAction::triggered, this, &TraceView::setPort);

    m_setCustomHighlightAct = new QAction("Custom Highlights", this);
    m_setCustomHighlightAct->setStatusTip("Set custom highlights for traces");
    connect(m_setCustomHighlightAct, &QAction::triggered, this, &TraceView::setCustomHiglights);

    createSetHostActions();
}

///
/// \brief TraceView::createSetHostActions
///
void TraceView::createSetHostActions()
{
    m_setAnyItfAct = new QAction(tr("&Any interface"), this);
    m_setAnyItfAct->setStatusTip(tr("Listen to any interface"));
    m_setAnyItfAct->setIcon(QIcon(":/img/checkmark.png"));
    m_setAnyItfAct->setEnabled(m_liveview);
    connect(m_setAnyItfAct, &QAction::triggered, this, [=](){
        setHost(m_setAnyItfAct);
    });

    m_setAnyItfIpv6Act = new QAction(tr("&Any interface IPv6"), this);
    m_setAnyItfIpv6Act->setStatusTip(tr("Listen to any IPv6 interface"));
    m_setAnyItfIpv6Act->setIcon(QIcon(":/img/checkmark.png"));
    m_setAnyItfIpv6Act->setEnabled(m_liveview);
    connect(m_setAnyItfIpv6Act, &QAction::triggered, this, [=](){
        setHost(m_setAnyItfIpv6Act);
    });

    m_setLocalItfAct = new QAction(tr("&Local interface"), this);
    m_setLocalItfAct->setStatusTip(tr("Equivalent to 127.0.0.1"));
    m_setLocalItfAct->setIcon(QIcon(":/img/checkmark.png"));
    m_setLocalItfAct->setEnabled(m_liveview);
    connect(m_setLocalItfAct, &QAction::triggered, this, [=](){
        setHost(m_setLocalItfAct);
    });

    //@TODO: How to get the remote interface and serial interface
    m_setRemoteItfAct = new QAction(tr("&Remote interface"), this);
    m_setRemoteItfAct->setStatusTip(tr("Listen to remote interace only"));
//    setRemoteItfAct->setIcon(QIcon(":/img/checkmark.png"));
    m_setRemoteItfAct->setIcon(QIcon(":/img/warning.png"));
    m_setRemoteItfAct->setIconVisibleInMenu(true);
    m_setRemoteItfAct->setEnabled(m_liveview);
    connect(m_setRemoteItfAct, &QAction::triggered, this, &TraceView::setIncompletedFunction);

    m_setSerialItfAct = new QAction(tr("&Serial interface"), this);
    m_setSerialItfAct->setStatusTip(tr("Listen to serial interace only"));
//    setSerialItfAct->setIcon(QIcon(":/img/checkmark.png"));
    m_setSerialItfAct->setIcon(QIcon(":/img/warning.png"));
    m_setSerialItfAct->setIconVisibleInMenu(true);
    m_setSerialItfAct->setEnabled(m_liveview);
    connect(m_setSerialItfAct, &QAction::triggered, this, &TraceView::setIncompletedFunction);
}

///
/// \brief TraceView::actionFromHostAddress
/// \param addr
/// \return
///
QAction* TraceView::actionFromHostAddress(QHostAddress& addr) const
{
    // @TODO: Any have string representation is 0.0.0.0 the same as AnyIPv4!!!
    QHash<QHostAddress, QAction*> hash = {
        { QHostAddress("0.0.0.0"),               m_setAnyItfAct },
        { QHostAddress(QHostAddress::Any),       m_setAnyItfAct },
        { QHostAddress(QHostAddress::AnyIPv6),   m_setAnyItfIpv6Act },
        { QHostAddress(QHostAddress::LocalHost), m_setLocalItfAct }
    };
    return hash[addr];
}

///
/// \brief TraceView::hostAddressFromAction
/// \param act
/// \return
///
QHostAddress TraceView::hostAddressFromAction(QAction* act) const
{
    QHash<QAction*, QHostAddress> hash = {
        { m_setAnyItfAct,     QHostAddress::Any },
        { m_setAnyItfIpv6Act, QHostAddress::AnyIPv6 },
        { m_setLocalItfAct,   QHostAddress::LocalHost }
    };
    return hash[act];
}

void TraceView::setHost(QAction* act)
{
    if (act == nullptr || !m_liveview)
    {
        return;
    }
    auto addr = hostAddressFromAction(act);
    emit changeHost(addr);
}

///
/// \brief TraceView::setPort
///
void TraceView::setPort()
{
    bool ok;
    int port = QInputDialog::getInt(this, tr("Set port"),
                                    tr("Port number (1 - 65535)"),
                                    m_currentPort, 1, 65535, 1, &ok,
                                    Qt::MSWindowsFixedSizeDialogHint);
    if (ok)
    {
        emit changePort(port);
    }
}

void TraceView::setCustomHiglights()
{
    bool ok;
    QStringList list = CustomHighlightDialog::getStrings(this, &ok);

    if (ok)
    {
        TraceManager::instance().setCustoms(list);
    }
}

///
/// \brief TraceView::save
///
void TraceView::save()
{
    static QRegularExpression noHyphensAndColons("[-:]");
    auto strDateTime = QDateTime::currentDateTime().toString(Qt::ISODate)
                                                   .remove(noHyphensAndColons);
    auto defaultName = "FMTrace_" + strDateTime + "_TraceName.html";
    auto filename = QFileDialog::getSaveFileName(this, tr("TraceTerminal++ - Save file"),
                                                 defaultName, "Rich text (*.html);;Plain text(*.txt)");
    if (filename.isEmpty())
    {
        return;
    }

    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        qDebug() << "There is a problem creating the file";
        return;
    }
    QTextStream out(&file);
    if (filename.endsWith(".html"))
    {
        out << toHtml();
    } else if (filename.endsWith(".txt"))
    {
        out << toPlainText();
    }
    file.close();

    // Open saved directory
    QFileInfo newFileDir(file);
    QProcess::startDetached("explorer.exe", {"/select,", QDir::toNativeSeparators(newFileDir.absoluteFilePath())});
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

    // Select the text block from start to the end of this line (including \n)
    cursor.movePosition(QTextCursor::Start);
    cursor.setPosition(mousePos, QTextCursor::KeepAnchor);
    cursor.movePosition(QTextCursor::EndOfLine, QTextCursor::KeepAnchor);
    cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor);

    setTextCursor(cursor);
    cursor.removeSelectedText();
}

///
/// \brief TraceView::setAutoScroll
///
void TraceView::toggleAutoScroll()
{
    m_autoScroll = !m_autoScroll;
    m_setAutoScrollAct->setIconVisibleInMenu(m_autoScroll);
    // up one line so that append does not auto scroll the view when cursor at the end of view
    if (m_liveview)
    {
        moveCursor(QTextCursor::Up);
    }
}

///
/// \brief Temporary function
///
void TraceView::setIncompletedFunction()
{
    QMessageBox msgBox;
    msgBox.setWindowTitle("Under construction!");
    msgBox.setText("We are working on it.");
    msgBox.setIcon(QMessageBox::Information);
    msgBox.exec();
}

///
/// \brief TraceView::onNewTraceReady
/// \param data
///
void TraceView::onNewTraceReady()
{
    if (!m_liveview)
    {
        return;
    }

    if (m_autoScroll)
    {
        moveCursor(QTextCursor::End);
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
void TraceView::onSocketBindResult(QHostAddress addr, quint16 port, bool success)
{
    if (!m_liveview)
    {
        return;
    }

    QString msg;
    if (success)
    {
        qDebug() << addr;
        auto act = actionFromHostAddress(addr);
        if (act == nullptr)
            return;

        if (m_lastSetItfAct != nullptr)
        {
            m_lastSetItfAct->setIconVisibleInMenu(false);
        }
        act->setIconVisibleInMenu(true);
        m_lastSetItfAct = act;

        m_currentPort = port;
        // Display current port number right in the action. Eg.: Configure Port - [800]
        auto setPortActTitle = QString("Configure Port - [%1]").arg(QString::number(m_currentPort));
        m_setPortAct->setText(setPortActTitle);

        msg = QString("<font color=Black>Binding to %1:%2 interface OK</font>")
                  .arg(addr.toString(), QString::number(port));
    }
    else
    {
        // If bind fail, don't active the old option too
        if (m_lastSetItfAct != nullptr)
        {
            m_lastSetItfAct->setIconVisibleInMenu(false);
        }

        msg = QString("<font color=Red>Binding to %1:%2 interface failed. ")
                  .arg(addr.toString(), QString::number(port));
        msg += "Please check if other application is taking over the address.";
        msg += "</font>";
    }
    append(msg);
    if (m_autoScroll)
    {
        moveCursor(QTextCursor::End);
    }
}
