#include "inc\tracehighlighter.h"
#include "inc/constants.h"
#include <QTextDocument>
#include <QDebug>
#include <QSettings>

namespace
{
static const QString containPattern = QStringLiteral("\\b.*?%1.*(?=\n|$)");
}

QList<TraceHighlighter::HighlightingRule> TraceHighlighter::highlightingRules = {};

TraceHighlighter::TraceHighlighter(QTextDocument *parent)
    : QSyntaxHighlighter(parent)
{
    HighlightingRule rule;
    QTextCharFormat format;
    // Default format
    addHighlightingRule(" ERROR - ", "red");
    addHighlightingRule(" WARNG - ", "darkorange");
    addHighlightingRule(" PANIC - ", "purple");
    addHighlightingRule(" PRINT - ", "slategrey");

    QSettings settings(Config::CONFIG_DIR, QSettings::IniFormat);
    auto highlights = settings.value(Config::HIGHLIGHTS, QStringList()).toStringList();
    for (int i = 0; i < highlights.length(); ++i)
    {
        addHighlightingRule(highlights.at(i), Highlight::defaultCustomHighlights[i]);
    }
}

///
/// \brief TraceHighlighter::highlightBlock override method
/// \param text
///
void TraceHighlighter::highlightBlock(const QString& text)
{
    foreach (const auto& rule, highlightingRules) {
        QRegularExpressionMatchIterator matchIterator = rule.pattern.globalMatch(text);
        while (matchIterator.hasNext()) {
            QRegularExpressionMatch match = matchIterator.next();
            setFormat(match.capturedStart(), match.capturedLength(), rule.format);
        }
    }
}

///
/// \brief TraceHighlighter::addHighlightingRule static method
/// \param keyword
/// \param formatColor
///
void TraceHighlighter::addHighlightingRule(const QString& keyword, const QString& formatColor)
{
    HighlightingRule newRule;
    QTextCharFormat format;
    format.setForeground(QColor(formatColor));
    newRule.format = format;
    QString escapedKeyword = QRegularExpression::escape(keyword);
    newRule.pattern = QRegularExpression(QString(containPattern).arg(escapedKeyword));

    // Compare to the existing highlight
    for (int i = 0; i < highlightingRules.count(); ++i) {
        if (highlightingRules[i].format == format)
        {
            if (!keyword.trimmed().isEmpty())
            {
                highlightingRules[i] = newRule;
            }
            else
            {
                highlightingRules.removeAt(i);
            }
            return;
        }
    }

    // If this is the new setting, append it if needed
    if (!keyword.trimmed().isEmpty())
    {
        highlightingRules.append(newRule);
    }
}
