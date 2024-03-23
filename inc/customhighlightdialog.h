#ifndef CUSTOMHIGHLIGHTDIALOG_H
#define CUSTOMHIGHLIGHTDIALOG_H

#include <QDialog>

QT_BEGIN_NAMESPACE
class QLineEdit;
QT_END_NAMESPACE

class CustomHighlightDialog : public QDialog
{
    Q_OBJECT
public:
    CustomHighlightDialog(QWidget* parent = nullptr);

    static QStringList getStrings(QWidget *parent, bool *ok = nullptr);

private:
    QList<QLineEdit*> m_fields;
    static QStringList m_lastHighlights;
};

#endif // CUSTOMHIGHLIGHTDIALOG_H
