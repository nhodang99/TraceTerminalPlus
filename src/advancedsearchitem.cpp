#include "inc/advancedsearchitem.h"
#include <QDebug>

AdvancedSearchItem::AdvancedSearchItem(QTextCursor cursor)
{
    auto cursorOnLine = cursor;
    cursorOnLine.select(QTextCursor::LineUnderCursor);
    QString lineUnderCursor = cursorOnLine.selectedText();
    int lineNumber = cursorOnLine.blockNumber() + 1;
    QString displayText = QString("Line %1\t%2").arg(QString::number(lineNumber), lineUnderCursor);

    setData(Qt::DisplayRole, displayText);
    setData(Qt::UserRole, QVariant::fromValue(cursor));
}

bool AdvancedSearchItem::operator<(const QListWidgetItem& other) const
{
    return this->data(Qt::UserRole).value<QTextCursor>() < other.data(Qt::UserRole).value<QTextCursor>();
}
