#include "inc/livetraceview.h"
#include "inc/mainwindow.h"
#include "inc/constants.h"
#include <QSettings>
#include <QtWidgets>

LiveTraceView::LiveTraceView()
{
    QSettings settings(Config::CONFIG_DIR, QSettings::IniFormat);
    m_remoteAddress = settings.value(Config::REMOTE_ADDRESS, QString("192.168.137.1")).toString();
    m_autoScroll = settings.value(Config::TRACEVIEW_AUTOSCROLL, true).toBool();

    createTraceActions();
    createNetworkActions();
}

///
/// \brief TraceView::createTraceActions
///
void LiveTraceView::createTraceActions()
{
    TraceView::createTraceActions();

    m_setAutoScrollAct->setEnabled(true);
    m_setAutoScrollAct->setIconVisibleInMenu(m_autoScroll);
    connect(m_setAutoScrollAct, &QAction::triggered, this, &LiveTraceView::toggleAutoScroll);
}

///
/// \brief LiveTraceView::createNetworkActions
///
void LiveTraceView::createNetworkActions()
{
    TraceView::createNetworkActions();

    m_setAnyItfAct->setEnabled(true);
    connect(m_setAnyItfAct, &QAction::triggered, this, [=](){
        changeInterface(SpecialInterface::ANY_INTERFACE);
    });

    m_setAnyItfIpv6Act->setEnabled(true);
    connect(m_setAnyItfIpv6Act, &QAction::triggered, this, [=](){
        changeInterface(SpecialInterface::ANY_IPV6_INTERFACE);
    });

    m_setLocalItfAct->setEnabled(true);
    connect(m_setLocalItfAct, &QAction::triggered, this, [=](){
        changeInterface(SpecialInterface::LOCALHOST_INTERFACE);
    });

    m_setRemoteItfAct->setEnabled(true);
    m_setRemoteItfAct->setText(QString("Remote Interface... - [%1]")
                                   .arg(m_remoteAddress));
    connect(m_setRemoteItfAct, &QAction::triggered,
            this, &LiveTraceView::promptAndSetRemoteInterface);

    m_setSerialItfAct->setEnabled(true);
    connect(m_setSerialItfAct, &QAction::triggered, this, [=](){
        changeInterface(SpecialInterface::SERIAL_INTERFACE);
    });

    m_setPortAct->setEnabled(true);
    m_setPortAct->setText(QString("Configure Port - [%1]")
                              .arg(QString::number(m_currentPort)));
    connect(m_setPortAct, &QAction::triggered, this, &LiveTraceView::changePort);
}

///
/// \brief TraceView::setHost
/// \param addr
///
void LiveTraceView::changeInterface(const QString& addr)
{
    if (addr.isEmpty())
    {
        return;
    }
    emit interfaceChangeRequested(addr);
}

///
/// \brief TraceView::promptAndSetRemoteInterface
///
void LiveTraceView::promptAndSetRemoteInterface()
{
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
/// \brief TraceView::setPort
///
void LiveTraceView::changePort()
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
/// \brief TraceView::setAutoScroll
///
void LiveTraceView::toggleAutoScroll()
{
    m_autoScroll = !m_autoScroll;
    m_setAutoScrollAct->setIconVisibleInMenu(m_autoScroll);
}

///
/// \brief TraceView::onNewTracesReady
/// \param traces
///
void LiveTraceView::onNewTracesReady(QStringList traces)
{
    //qDebug() << traces;
    foreach (auto& trace, traces)
    {
        // Append line by line help us using blockNumber() to get the line number when searching
        append(trace);

        if (m_autoScroll)
        {
            moveCursor(QTextCursor::End);
        }
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
void LiveTraceView::onSocketBindResult(QString addr, quint16 port, bool success)
{
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
