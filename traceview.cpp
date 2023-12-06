#include <QtWidgets>
#include "traceview.h"

TraceView::TraceView(bool live, bool autoscroll)
    : m_liveview(live)
    , m_autoScroll(autoscroll)
{
    setLineWrapMode(QTextEdit::NoWrap);
    setAcceptRichText(true);
    setReadOnly(true);
    QFont font("Helvetica");
    font.setPointSize(10);
    setFont(font);

    createActions();
}

#ifndef QT_NO_CONTEXTMENU
void TraceView::contextMenuEvent(QContextMenuEvent *event)
{
    QMenu *menu = createStandardContextMenu();
    menu->addAction(saveAct);
    menu->addAction(clearAct);
    menu->addAction(clearUntilHereAct);
    menu->addSeparator();
    menu->addAction(setAutoScrollAct);
    menu->addSeparator();
    menu->addAction(setAnyItfAct);
    menu->addAction(setAnyItfIpv6Act);
    menu->addAction(setLocalItfAct);
    menu->addAction(setRemoteItfAct);
    menu->addAction(setSerialItfAct);
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
    saveAct = new QAction(tr("&Save"), this);
    saveAct->setShortcuts(QKeySequence::Save);
    saveAct->setStatusTip(tr("Save the document to disk"));
    saveAct->setEnabled(false);
    connect(saveAct, &QAction::triggered, this, &TraceView::save);
    connect(this, &QTextEdit::textChanged, saveAct, [this](){
        saveAct->setEnabled(!this->document()->isEmpty());
    });

    clearAct = new QAction(tr("&Clear"), this);
    clearAct->setShortcuts(QKeySequence::Refresh);
    clearAct->setStatusTip(tr("Clear all the traces"));
    connect(clearAct, &QAction::triggered, this, &TraceView::clear);

    clearUntilHereAct = new QAction(tr("&Clear until here"), this);
    clearAct->setStatusTip(tr("Clear all the traces until the pointer"));
    connect(clearUntilHereAct, &QAction::triggered, this, &TraceView::clearUntilHere);

    setAutoScrollAct = new QAction(tr("&Autoscroll"), this);
    setAutoScrollAct->setStatusTip(tr("If false, the trace is not automatically scroll if new trace comes"));
    setAutoScrollAct->setIcon(QIcon(":/img/checkmark.png"));
    setAutoScrollAct->setEnabled(m_liveview);
    setAutoScrollAct->setIconVisibleInMenu(m_autoScroll);
    connect(setAutoScrollAct, &QAction::triggered, this, &TraceView::toggleAutoScroll);

    createSetHostActions();
}

///
/// \brief TraceView::createSetHostActions
///
void TraceView::createSetHostActions()
{
    setAnyItfAct = new QAction(tr("&Any interface"), this);
    setAnyItfAct->setStatusTip(tr("Listen to any interface"));
    setAnyItfAct->setIcon(QIcon(":/img/checkmark.png"));
    setAnyItfAct->setEnabled(m_liveview);
    connect(setAnyItfAct, &QAction::triggered, this, [=](){
        setInterface(setAnyItfAct);
    });

    setAnyItfIpv6Act = new QAction(tr("&Any interface IPv6"), this);
    setAnyItfIpv6Act->setStatusTip(tr("Listen to any IPv6 interface"));
    setAnyItfIpv6Act->setIcon(QIcon(":/img/checkmark.png"));
    setAnyItfIpv6Act->setEnabled(m_liveview);
    connect(setAnyItfIpv6Act, &QAction::triggered, this, [=](){
        setInterface(setAnyItfIpv6Act);
    });

    setLocalItfAct = new QAction(tr("&Local interface"), this);
    setLocalItfAct->setStatusTip(tr("Equivalent to 127.0.0.1"));
    setLocalItfAct->setIcon(QIcon(":/img/checkmark.png"));
    setLocalItfAct->setEnabled(m_liveview);
    connect(setLocalItfAct, &QAction::triggered, this, [=](){
        setInterface(setLocalItfAct);
    });

    //@todo: How to get the remote interface and serial interface
    setRemoteItfAct = new QAction(tr("&Remote interface"), this);
    setRemoteItfAct->setStatusTip(tr("Listen to remote interace only"));
//    setRemoteItfAct->setIcon(QIcon(":/img/checkmark.png"));
    setRemoteItfAct->setIcon(QIcon(":/img/warning.png"));
    setRemoteItfAct->setIconVisibleInMenu(true);
    setRemoteItfAct->setEnabled(m_liveview);
    connect(setRemoteItfAct, &QAction::triggered, this, &TraceView::setIncompletedFunction);

    setSerialItfAct = new QAction(tr("&Serial interface"), this);
    setSerialItfAct->setStatusTip(tr("Listen to serial interace only"));
//    setSerialItfAct->setIcon(QIcon(":/img/checkmark.png"));
    setSerialItfAct->setIcon(QIcon(":/img/warning.png"));
    setSerialItfAct->setIconVisibleInMenu(true);
    setSerialItfAct->setEnabled(m_liveview);
    connect(setSerialItfAct, &QAction::triggered, this, &TraceView::setIncompletedFunction);
}

///
/// \brief TraceView::actionFromHostAddress
/// \param addr
/// \return
///
QAction* TraceView::actionFromHostAddress(QHostAddress& addr) const
{
    // @todo: Any have string representation is 0.0.0.0
    // the same as AnyIPv4???
    if (addr == QHostAddress("0.0.0.0")
        || addr == QHostAddress(QHostAddress::Any))    return setAnyItfAct;
    if (addr == QHostAddress(QHostAddress::AnyIPv6))   return setAnyItfIpv6Act;
    if (addr == QHostAddress(QHostAddress::LocalHost)) return setLocalItfAct;
    return                                             nullptr;
}

///
/// \brief TraceView::hostAddressFromAction
/// \param act
/// \return
///
QHostAddress TraceView::hostAddressFromAction(QAction* act) const
{
    std::map<QAction*, QHostAddress> hostMap = {
        { setAnyItfAct,     QHostAddress::Any },
        { setAnyItfIpv6Act, QHostAddress::AnyIPv6 },
        { setLocalItfAct,   QHostAddress::LocalHost }
    };

    return hostMap[act];
}

void TraceView::setInterface(QAction* act)
{
    if (act == nullptr || !m_liveview)
    {
        return;
    }
    auto addr = hostAddressFromAction(act);
    emit changeInterface(addr);
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
    setAutoScrollAct->setIconVisibleInMenu(m_autoScroll);
}

///
/// \brief Temporary function
///
void TraceView::setIncompletedFunction()
{
    QMessageBox msgBox;
    msgBox.setWindowTitle("Under construction!");
    msgBox.setWindowIcon(QIcon(":/img/favicon"));
    msgBox.setText("We are working on it.");
    msgBox.setIcon(QMessageBox::Information);
    msgBox.exec();
}

///
/// \brief TraceView::onNewDataReady
/// \param data
///
void TraceView::onNewDataReady(QString data)
{
    if (!m_liveview)
    {
        return;
    }
    append(data);
    if (m_autoScroll)
    {
        moveCursor(QTextCursor::End);
    }
}

///
/// \brief TraceView::onSocketBindResult
/// \param success
///
void TraceView::onSocketBindResult(QHostAddress& itf, bool success)
{
    if (!m_liveview)
    {
        return;
    }

    QString msg;
    if (success)
    {
        qDebug() << itf;
        auto act = actionFromHostAddress(itf);
        if (act == nullptr)
            return;

        if (m_lastSetItfAct != nullptr)
        {
            m_lastSetItfAct->setIconVisibleInMenu(false);
        }
        act->setIconVisibleInMenu(true);
        m_lastSetItfAct = act;

        msg = "<font color=Black>Binding to " + itf.toString() + ":911 interface OK</font>";
    }
    else
    {
        // If bind fail, don't active any option
        if (m_lastSetItfAct != nullptr)
        {
            m_lastSetItfAct->setIconVisibleInMenu(false);
        }

        msg = "<font color=Red>Binding to " + itf.toString() + ":911 interface failed. ";
        msg += "Please check if other application is taking over the address.";
        msg += "</font>";
    }
    append(msg);
    if (m_autoScroll)
    {
        moveCursor(QTextCursor::End);
    }
}
