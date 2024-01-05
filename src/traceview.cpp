#include "inc/traceview.h"
#include "inc/mainwindow.h"
#include "inc/customhighlightdialog.h"
#include "inc/tracemanager.h"
#include <QtWidgets>
#include <QDialog>
#include <QSettings>

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

TraceView::~TraceView()
{

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
    // @TODO: add these 2 actions.
    // At the moment we can manually use them by set specific address
//    menu->addAction(m_setRemoteItfAct);
//    menu->addAction(m_setSerialItfAct);
    menu->addAction(m_setSpecificItfAct);
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
    m_searchAct->setStatusTip("Search in the document. Use shortcut Ctrl+F for normal search, Ctrl+Shift+F for advanced search.");
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
    connect(m_setPortAct, &QAction::triggered, this, &TraceView::setPort);

    m_setCustomHighlightAct = new QAction("Custom Highlights", this);
    m_setCustomHighlightAct->setStatusTip("Set custom highlights for traces");
    connect(m_setCustomHighlightAct, &QAction::triggered, this, &TraceView::setCustomHiglights);

    // Set interface and port action
    createSetHostActions();
}

///
/// \brief TraceView::createSetHostActions
///
void TraceView::createSetHostActions()
{
    m_setAnyItfAct = new QAction("Any Interface", this);
    m_setAnyItfAct->setStatusTip("Listen to any interface");
    m_setAnyItfAct->setIcon(QIcon(":/img/checkmark.png"));
    m_setAnyItfAct->setEnabled(m_liveview);
    connect(m_setAnyItfAct, &QAction::triggered, this, [=](){
        setHost(m_setAnyItfAct);
    });

    m_setAnyItfIpv6Act = new QAction("Any Interface IPv6", this);
    m_setAnyItfIpv6Act->setStatusTip("Listen to any IPv6 interface");
    m_setAnyItfIpv6Act->setIcon(QIcon(":/img/checkmark.png"));
    m_setAnyItfIpv6Act->setEnabled(m_liveview);
    connect(m_setAnyItfIpv6Act, &QAction::triggered, this, [=](){
        setHost(m_setAnyItfIpv6Act);
    });

    m_setLocalItfAct = new QAction("Local Interface", this);
    m_setLocalItfAct->setStatusTip("Equivalent to 127.0.0.1");
    m_setLocalItfAct->setIcon(QIcon(":/img/checkmark.png"));
    m_setLocalItfAct->setEnabled(m_liveview);
    connect(m_setLocalItfAct, &QAction::triggered, this, [=](){
        setHost(m_setLocalItfAct);
    });

    //@TODO: How to get the remote interface and serial interface
    /*
    m_setRemoteItfAct = new QAction("Remote interface", this);
    m_setRemoteItfAct->setStatusTip("Listen to remote interface only");
//    setRemoteItfAct->setIcon(QIcon(":/img/checkmark.png"));
    m_setRemoteItfAct->setIcon(QIcon(":/img/warning.png"));
    m_setRemoteItfAct->setIconVisibleInMenu(true);
    m_setRemoteItfAct->setEnabled(m_liveview);
    connect(m_setRemoteItfAct, &QAction::triggered, this, &TraceView::setIncompletedFunction);

    m_setSerialItfAct = new QAction("Serial interface", this);
    m_setSerialItfAct->setStatusTip("Listen to serial interface only");
//    setSerialItfAct->setIcon(QIcon(":/img/checkmark.png"));
    m_setSerialItfAct->setIcon(QIcon(":/img/warning.png"));
    m_setSerialItfAct->setIconVisibleInMenu(true);
    m_setSerialItfAct->setEnabled(m_liveview);
    connect(m_setSerialItfAct, &QAction::triggered, this, &TraceView::setIncompletedFunction);
    */

    m_setSpecificItfAct = new QAction(QString("Other Interface... - [%1]")
                                          .arg(m_currentSpecificHost.toString()), this);
    m_setSpecificItfAct->setStatusTip("Connect to an interface set by user. Default 192.168.137.1");
    m_setSpecificItfAct->setIcon(QIcon(":/img/checkmark.png"));
    m_setSpecificItfAct->setEnabled(m_liveview);
    connect(m_setSpecificItfAct, &QAction::triggered, this, &TraceView::setSpecificHost);
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
    auto action = hash[addr];
    if (action == nullptr)
    {
        return m_setSpecificItfAct;
    }
    return action;
}

///
/// \brief TraceView::hostAddressFromAction
/// \param act
/// \return
///
QHostAddress& TraceView::hostAddressFromAction(QAction* act)
{
    if (act == m_setSpecificItfAct)
    {
        return m_currentSpecificHost;
    }
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
    auto& addr = hostAddressFromAction(act);
    emit changeHost(addr);
}

///
/// \brief TraceView::setSpecificHost
///
void TraceView::setSpecificHost()
{
    if (!m_liveview)
    {
        return;
    }
    bool ok;
    auto addr = QInputDialog::getText(this, "Set Specific Interface",
                                      "Address:", QLineEdit::Normal,
                                      m_currentSpecificHost.toString(), &ok,
                                      Qt::MSWindowsFixedSizeDialogHint);
    if (ok && !addr.isEmpty())
    {
        m_currentSpecificHost = QHostAddress(addr);
        emit changeHost(m_currentSpecificHost);
    }
}

///
/// \brief TraceView::setCurrentSpecificAddress
/// \param addr
///
void TraceView::setCurrentSpecificAddress(QString& addr)
{
    m_currentSpecificHost = QHostAddress(addr);
    auto setHostActTitle = QString("Other interface... - [%1]")
                               .arg(addr);
    m_setSpecificItfAct->setText(setHostActTitle);
}

///
/// \brief getCurrentSpecificHost
/// \return
///
QString TraceView::getCurrentSpecificAddress() const
{
    return m_currentSpecificHost.toString();
}

///
/// \brief TraceView::setPort
///
void TraceView::setPort()
{
    bool ok;
    int port = QInputDialog::getInt(this, "Set port",
                                    "Port number (1 - 65535)",
                                    m_currentPort, 1, 65535, 1, &ok,
                                    Qt::MSWindowsFixedSizeDialogHint);
    if (ok)
    {
        emit changePort(port);
    }
}

///
/// \brief TraceView::setCustomHiglights
///
void TraceView::setCustomHiglights()
{
    bool ok;
    QStringList list = CustomHighlightDialog::getStrings(this, &ok);
    if (ok)
    {
        TraceManager::instance().setCustoms(list);
    }
    // @TODO: update previous trace...
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
    auto filename = QFileDialog::getSaveFileName(this, "TraceTerminal++ - Save file",
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
    if (!m_autoScroll)
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
    for (const auto& trace : traces)
    {
        append(trace);
    }
//    qDebug() << traces;
//    qDebug() << "-------------------------";
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
    auto act = actionFromHostAddress(addr);
    if (act == nullptr)
    {
        qDebug() << "Something wrong";
        return;
    }

    if (act == m_setSpecificItfAct)
    {
        // Display current specific host right in the action
        // Eg.: Other interface... - [192.168.137.1]
        auto setHostActTitle = QString("Other interface... - [%1]")
                                   .arg(m_currentSpecificHost.toString());
        m_setSpecificItfAct->setText(setHostActTitle);
    }

    m_currentPort = port;
    // Display current port number right in the action. Eg.: Configure Port - [800]
    auto setPortActTitle = QString("Configure Port - [%1]").arg(QString::number(m_currentPort));
    m_setPortAct->setText(setPortActTitle);

    QString msg;
    if (success)
    {
        qDebug() << QString("%1:%2").arg(addr.toString(), QString::number(port));
        if (m_lastSetItfAct != nullptr)
        {
            m_lastSetItfAct->setIconVisibleInMenu(false);
        }
        act->setIconVisibleInMenu(true);
        m_lastSetItfAct = act;

        msg = QString("<span style=\"color:black\">>Binding to %1:%2 interface OK</span>")
                  .arg(addr.toString(), QString::number(port));
    }
    else
    {
        // If bind fail, don't active the old option too
        if (m_lastSetItfAct != nullptr)
        {
            m_lastSetItfAct->setIconVisibleInMenu(false);
        }

        msg = QString("<span style=\"color:red\">>Binding to %1:%2 interface failed. ")
                  .arg(addr.toString(), QString::number(port));
        msg += "Please check if other application is taking over the address.";
        msg += "</span>";
    }
    append(msg);
    if (m_autoScroll)
    {
        moveCursor(QTextCursor::End);
    }
}
