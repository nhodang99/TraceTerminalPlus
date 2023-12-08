#ifndef TRACEVIEW_H
#define TRACEVIEW_H

#include <QTextEdit>
#include <QHostAddress>
#include <QMutex>

class TraceView : public QTextEdit
{
    Q_OBJECT

public:
    TraceView(bool live = false, bool autoscroll = false);

    bool isAutoscroll() const { return m_autoScroll; }
    void onSocketBindResult(QHostAddress&, bool);

protected:
    void contextMenuEvent(QContextMenuEvent *event) override;
    void mousePressEvent(QMouseEvent* event) override;

public slots:
    void save();
    void clear();
    void clearUntilHere();
    void toggleAutoScroll();

    void setInterface(QAction*);
    void setIncompletedFunction();
    void onNewDataReady(QString);

signals:
    void changeInterface(QHostAddress);

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
    //! [Actions]

    //! [Attr]
    QAction* m_lastSetItfAct{ nullptr };
    bool m_liveview{ false };
    bool m_autoScroll{ false };
    QTextCursor m_clearUntilCursor;
};

#endif // TRACEVIEW_H
