#ifndef TRACEVIEW_H
#define TRACEVIEW_H

#include <QTextEdit>
#include <QHostAddress>

class TraceView : public QTextEdit
{
    Q_OBJECT

public:
    TraceView(bool live = false, bool autoscroll = false);

    bool isAutoscroll() const { return m_autoScroll; }
    void onSocketBindResult(QHostAddress, quint16, bool);

protected:
    void contextMenuEvent(QContextMenuEvent *event) override;
    void mousePressEvent(QMouseEvent* event) override;

public slots:
    void save();
    void clear();
    void clearUntilHere();
    void toggleAutoScroll();
    void setCustomHiglights();

    void setHost(QAction*);
    void setPort();
    void setIncompletedFunction();
    void onNewTraceReady();

signals:
    void changeHost(QHostAddress);
    void changePort(quint16);

private:
    void createActions();
    void createSetHostActions();

    QAction* actionFromHostAddress(QHostAddress&) const;
    QHostAddress hostAddressFromAction(QAction*) const;

    //! [Actions]
    QAction* m_saveAct;
    QAction* m_searchAct;
    QAction* m_clearAct;
    QAction* m_clearUntilHereAct;
    QAction* m_setAutoScrollAct;
    QAction* m_setCustomHighlightAct;

    QAction* m_setAnyItfAct;
    QAction* m_setAnyItfIpv6Act;
    QAction* m_setLocalItfAct;
    QAction* m_setRemoteItfAct;
    QAction* m_setSerialItfAct;
    QAction* m_setPortAct;
    //! [Actions]

    //! [Attr]
    QAction*    m_lastSetItfAct{ nullptr };
    bool        m_liveview{ false };
    bool        m_autoScroll{ false };
    QTextCursor m_clearUntilCursor;
    quint16     m_currentPort{911}; // for context menu
};

#endif // TRACEVIEW_H
