#ifndef TRACEHIGHLIGHTER_H
#define TRACEHIGHLIGHTER_H

#include <QSyntaxHighlighter>
#include <QTextCharFormat>
#include <QRegularExpression>

class TraceHighlighter : public QSyntaxHighlighter
{
    Q_OBJECT
public:
    explicit TraceHighlighter(QTextDocument *parent = nullptr);
    static void addHighlightingRule(const QString&, const QString&);

protected:
    void highlightBlock(const QString& text) override;

private:
    struct HighlightingRule
    {
        QRegularExpression pattern;
        QTextCharFormat format;
    };

    static QList<HighlightingRule> highlightingRules;
};

#endif // TRACEHIGHLIGHTER_H
