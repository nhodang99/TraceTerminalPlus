#ifndef TRACEVIEW_H
#define TRACEVIEW_H

#include <QTextEdit>
#include <QHostAddress>

QT_BEGIN_NAMESPACE
class TraceHighlighter;
QT_END_NAMESPACE

class TraceView : public QTextEdit
{
    Q_OBJECT

public:
    TraceView();
    ~TraceView();

    inline virtual bool isAutoScrollEnabled() const;

    inline bool isHighlightUpdated() const;
    inline bool wasHighlightUpdateRequested() const;
    void setHighlightUpdateRequested();
    void disableCustomHighlighting();
    void updateHighlighting();

protected:
    void contextMenuEvent(QContextMenuEvent *event) override;
    void mousePressEvent(QMouseEvent* event) override;

public slots:
    void save();
    void clear();
    void clearUntilHere();

    void setCustomHighlights();
    void onHighlightingChanged();

protected:
    void createTraceActions();
    void createNetworkActions();

    QString toHtmlWithAdditionalFormats();
    QAction* toAction(QString&) const;


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
    QTextCursor  m_clearUntilCursor;

    TraceHighlighter* m_highlighter{nullptr};
    bool         m_highlightUpdated{true};
    bool         m_highlightUpdateRequested{false};
};

inline bool TraceView::isAutoScrollEnabled() const
{
    return false;
}

inline bool TraceView::isHighlightUpdated() const
{
    return m_highlightUpdated;
}

inline bool TraceView::wasHighlightUpdateRequested() const
{
    return m_highlightUpdateRequested;
}

#endif // TRACEVIEW_H
