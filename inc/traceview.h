#ifndef TRACEVIEW_H
#define TRACEVIEW_H

#include <QTextEdit>
#include <QHostAddress>

class TraceView : public QTextEdit
{
    Q_OBJECT

public:
    ~TraceView();
    TraceView(bool live = false, bool autoscroll = false);

    bool isAutoscroll() const { return m_autoScroll; }
    void setRemoteAddress(QString&);
    QString getRemoteAddress() const;

protected:
    void contextMenuEvent(QContextMenuEvent *event) override;
    void mousePressEvent(QMouseEvent* event) override;

public slots:
    void save();
    void clear();
    void clearUntilHere();
    void toggleAutoScroll();
    void setCustomHiglights();
    void promptAndSetRemoteInterface();

    void onSocketBindResult(QString, quint16, bool);
    void onNewTracesReady(QStringList);

signals:
    void interfaceChangeRequested(QString);
    void portChangeRequested(quint16);

private:
    void createActions();
    void createSetHostActions();

    QAction* toAction(QString&) const;
    void changeInterface(const QString&);
    void changePort();

    //! [Actions]
    QAction* m_saveAct{nullptr};
    QAction* m_searchAct{nullptr};
    QAction* m_clearAct{nullptr};
    QAction* m_clearUntilHereAct{nullptr};
    QAction* m_setAutoScrollAct{nullptr};
    QAction* m_setCustomHighlightAct{nullptr};

    QAction* m_setAnyItfAct{nullptr};
    QAction* m_setAnyItfIpv6Act{nullptr};
    QAction* m_setLocalItfAct{nullptr};
    QAction* m_setRemoteItfAct{nullptr};
    QAction* m_setSerialItfAct{nullptr};
    QAction* m_setPortAct{nullptr};
    //! [Actions]

    //! [Attr]
    QAction*     m_lastSetItfAct{nullptr};
    bool         m_liveview{false};
    bool         m_autoScroll{false};
    QTextCursor  m_clearUntilCursor;
    quint16      m_currentPort{911}; // for context menu
    QString      m_remoteAddress{"192.168.137.1"}; // For context menu, managed on gui
    QString      m_waitingStep{"oooo0"};
};

#endif // TRACEVIEW_H
