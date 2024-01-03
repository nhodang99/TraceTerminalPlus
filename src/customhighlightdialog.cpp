#include "inc/customhighlightdialog.h"
#include "inc/constants.h"
#include <QSettings>
#include <QLabel>
#include <QLineEdit>
#include <QDialogButtonBox>
#include <QFormLayout>

CustomHighlightDialog::CustomHighlightDialog(QWidget *parent)
    : QDialog(parent, Qt::MSWindowsFixedSizeDialogHint)
{
    setWindowTitle("Custom Highlights");
    setMinimumSize(400, 180);
    QFormLayout* mainLayout = new QFormLayout(this);

    QSettings settings(Config::CONFIG_DIR, QSettings::IniFormat);
    auto customList = settings.value(Config::CUSTOMS, QStringList()).toStringList();

    for (int i = 0; i < 5; ++i)
    {
        QLabel *label = new QLabel(this);
        QImage image(22, 22, QImage::Format_RGB32);
        image.fill(QColor(Highlight::customs[i]));
        QLineEdit *line = new QLineEdit(this);
        line->setStyleSheet(QString("border: 1px solid %1;"
                                    "color: %1;"
                                    "font: 15px 'Roboto';").arg(Highlight::customs[i]));
        if (i < customList.length())
        {
            line->setText(customList[i]);
        }

        label->setPixmap(QPixmap::fromImage(image));
        mainLayout->addRow(label, line);
        fields << line;
    }

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok |
                                                           QDialogButtonBox::Cancel,
                                                       Qt::Horizontal, this );
    mainLayout->addWidget(buttonBox);

    bool conn = connect(buttonBox, &QDialogButtonBox::accepted,
                        this, &CustomHighlightDialog::accept);
    Q_ASSERT(conn);
    conn = connect(buttonBox, &QDialogButtonBox::rejected,
                   this, &CustomHighlightDialog::reject);
    Q_ASSERT(conn);
    setLayout(mainLayout);
}

QStringList CustomHighlightDialog::getStrings(QWidget *parent, bool *ok)
{
    CustomHighlightDialog* dialog = new CustomHighlightDialog(parent);
    QStringList list;

    const int ret = dialog->exec();
    if (ok != nullptr)
        *ok = !!ret;

    if (ret) {
        for (auto& field : dialog->fields) {
            list << field->text();
        }
    }

    dialog->deleteLater();
    return list;
}
