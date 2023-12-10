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

    void setHost(QAction*);
    void setPort();
    void setIncompletedFunction();
    void onNewDataReady(QString);

signals:
    void changeHost(QHostAddress);
    void changePort(quint16);

private:
    void createActions();
    void createSetHostActions();

    QAction* actionFromHostAddress(QHostAddress&) const;
    QHostAddress hostAddressFromAction(QAction*) const;

    //! [Actions]
    QAction *saveAct;
    QAction *clearAct;
    QAction *clearUntilHereAct;
    QAction *setAutoScrollAct;

    QAction *setAnyItfAct;
    QAction *setAnyItfIpv6Act;
    QAction *setLocalItfAct;
    QAction *setRemoteItfAct;
    QAction *setSerialItfAct;
    QAction *setPortAct;
    //! [Actions]

    //! [Attr]
    QAction*    m_lastSetItfAct{ nullptr };
    bool        m_liveview{ false };
    bool        m_autoScroll{ false };
    QTextCursor m_clearUntilCursor;
    quint16     m_currentPort {911}; // for context menu
};

#endif // TRACEVIEW_H
