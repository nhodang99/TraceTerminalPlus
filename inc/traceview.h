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
    void setCurrentSpecificAddress(QString&);
    QString getCurrentSpecificAddress() const;

protected:
    void contextMenuEvent(QContextMenuEvent *event) override;
    void mousePressEvent(QMouseEvent* event) override;

public slots:
    void save();
    void clear();
    void clearUntilHere();
    void toggleAutoScroll();
    void setCustomHiglights();
    void setSpecificHost();

    void setIncompletedFunction();
    void onSocketBindResult(QString, quint16, bool);
    void onNewTracesReady(QStringList);

signals:
    void hostChangeRequested(QString);
    void portChangeRequested(quint16);

private:
    void createActions();
    void createSetHostActions();

    QAction* toAction(QString&) const;
    void changeHost(const QString&);
    void changePort();

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
//    QAction* m_setRemoteItfAct;
//    QAction* m_setSerialItfAct;
    QAction* m_setSpecificItfAct;
    QAction* m_setPortAct;
    //! [Actions]

    //! [Attr]
    QAction*     m_lastSetItfAct{nullptr};
    bool         m_liveview{false};
    bool         m_autoScroll{false};
    QTextCursor  m_clearUntilCursor;
    quint16      m_currentPort{911}; // for context menu
    QString      m_currentSpecificHost{"192.168.137.1"}; // For context menu, managed on gui
    QString      m_waitingStep{"oooo0"};
};

#endif // TRACEVIEW_H
