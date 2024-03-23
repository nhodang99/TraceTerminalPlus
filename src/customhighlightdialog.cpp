#include "inc/customhighlightdialog.h"
#include "inc/constants.h"
#include <QSettings>
#include <QLabel>
#include <QLineEdit>
#include <QDialogButtonBox>
#include <QFormLayout>

QStringList CustomHighlightDialog::m_lastHighlights = {};

CustomHighlightDialog::CustomHighlightDialog(QWidget *parent)
    : QDialog(parent, Qt::WindowSystemMenuHint | Qt::WindowCloseButtonHint)
{
    setWindowTitle("Custom Highlights");
    setMinimumSize(420, 190);
    QFormLayout* mainLayout = new QFormLayout(this);

    QSettings settings(Config::CONFIG_DIR, QSettings::IniFormat);
    auto highlights = settings.value(Config::HIGHLIGHTS, QStringList()).toStringList();

    for (int i = 0; i < Highlight::CUSTOM_COLOR_NUMBER; ++i)
    {
        QLabel* label = new QLabel(this);
        QImage image(22, 22, QImage::Format_RGB32);
        image.fill(QColor(Highlight::defaultCustomHighlights[i]));
        QLineEdit* line = new QLineEdit(this);
        line->setStyleSheet(QString("border: 1px solid %1;"
                                    "color: %1;"
                                    "font: 15px 'Roboto';").arg(Highlight::defaultCustomHighlights[i]));
        if (i < highlights.length())
        {
            line->setText(highlights[i]);
        }

        label->setPixmap(QPixmap::fromImage(image));
        mainLayout->addRow(label, line);
        m_fields << line;
    }

    QLabel* infoLabel = new QLabel(this);
    infoLabel->setText("<i>Please note that apply new custom highlight rule might cause laggy <br>"
                       " for a few seconds while applying to large file</i>");
    QFont font;
    font.setPointSize(10);
    infoLabel->setFont(font);
    mainLayout->addRow(infoLabel);

    QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok |
                                                           QDialogButtonBox::Cancel,
                                                       Qt::Horizontal, this );
    mainLayout->addWidget(buttonBox);

    connect(buttonBox, &QDialogButtonBox::accepted, this, &CustomHighlightDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &CustomHighlightDialog::reject);

    setLayout(mainLayout);
    m_lastHighlights = highlights;
}

QStringList CustomHighlightDialog::getStrings(QWidget *parent, bool *ok)
{
    CustomHighlightDialog* dialog = new CustomHighlightDialog(parent);
    QStringList list;

    const int ret = dialog->exec();
    if (ok)
    {
        *ok = !!ret;
    }

    if (ret)
    {
        for (auto& field : dialog->m_fields) {
            list << field->text();
        }
    }

    dialog->deleteLater();

    if (list == m_lastHighlights)
    {
        if (ok) *ok = false;
        return QStringList();
    }
    m_lastHighlights = list;
    return list;
}
