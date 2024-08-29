#ifndef LIVETRACEVIEW_H
#define LIVETRACEVIEW_H

#include "traceview.h"

class LiveTraceView : public TraceView
{
    Q_OBJECT
public:
    LiveTraceView();

    inline bool isAutoScrollEnabled() const override;
    inline QString getRemoteAddress() const;

signals:
    void interfaceChangeRequested(QString);
    void portChangeRequested(quint16);

public slots:
    void toggleAutoScroll();
    void promptAndSetRemoteInterface();
    void onSocketBindResult(QString, quint16, bool);
    void onNewTracesReady(QStringList);

private:
    void createTraceActions(); // Not an override method due to calling in constructor
    void createNetworkActions(); // Not an override method due to calling in constructor

    void changeInterface(const QString&);
    void changePort();

    //! [Attr]
    bool         m_autoScroll{false};

    QAction*     m_lastSetItfAct{nullptr};
    QString      m_waitingStep{"oooo0"};
    quint16      m_currentPort{911}; // for context menu
    QString      m_remoteAddress{"192.168.137.1"}; // For context menu, managed on gui
};

inline bool LiveTraceView::isAutoScrollEnabled() const
{
    return m_autoScroll;
}

inline QString LiveTraceView::getRemoteAddress() const
{
    return m_remoteAddress;
}

#endif // LIVETRACEVIEW_H
