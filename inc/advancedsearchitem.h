#ifndef ADVANCEDSEARCHITEM_H
#define ADVANCEDSEARCHITEM_H

#include <QListWidgetItem>
#include <QTextCursor>

Q_DECLARE_METATYPE(QTextCursor);

class AdvancedSearchItem : public QListWidgetItem
{
public:
    AdvancedSearchItem(QTextCursor cursor);

    bool operator<(const QListWidgetItem& other) const override;
};

#endif // ADVANCEDSEARCHITEM_H
