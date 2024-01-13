#include "inc/searchdock.h"
#include <QDebug>
#include <QDialog>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QMessageBox>
#include <QListWidget>
#include <QHideEvent>
#include <QCheckBox>

SearchDock::SearchDock(QWidget* parent, bool caseSensitive, bool loopSearch)
    : QDockWidget(parent, Qt::Widget)
{
    setWindowTitle("Search");
    setAllowedAreas(Qt::BottomDockWidgetArea);

    auto findLabel = new QLabel("Enter text:");
    m_lineEdit = new QLineEdit;
    m_lineEdit->setClearButtonEnabled(true);
    m_caseSensitiveCheck = new QCheckBox("Case sensitive", this);
    m_caseSensitiveCheck->setChecked(caseSensitive);
    m_loopCheck = new QCheckBox("Loop", this);
    m_loopCheck->setChecked(loopSearch);
    auto searchButton = new QPushButton("Search");
    auto advSearchButton = new QPushButton("Advanced search");

    QHBoxLayout* searchRowLayout = new QHBoxLayout;
    searchRowLayout->setSpacing(5);
    searchRowLayout->addWidget(findLabel);
    searchRowLayout->addWidget(m_lineEdit);
    searchRowLayout->addWidget(m_caseSensitiveCheck);
    searchRowLayout->addWidget(m_loopCheck);
    searchRowLayout->addWidget(searchButton);
    searchRowLayout->addWidget(advSearchButton);

    m_advSearchList = new QListWidget;
    m_advSearchList->hide();

    // We cannot set layout directly to the dock widget
    // Instead we need to add a widget containing that layout
    auto layoutWidget = new QWidget;
    QVBoxLayout* mainLayout = new QVBoxLayout;
    mainLayout->setSpacing(2);
    mainLayout->addItem(searchRowLayout);
    mainLayout->addWidget(m_advSearchList);
    layoutWidget->setLayout(mainLayout);
    setWidget(layoutWidget);

    setFocusProxy(m_lineEdit);
    m_lineEdit->setFocusPolicy(Qt::StrongFocus);
    searchButton->setDefault(true);

    connect(searchButton, &QPushButton::clicked, this, &SearchDock::onSearchClicked);
    connect(advSearchButton, &QPushButton::clicked, this, &SearchDock::onAdvSearchClicked);
    connect(m_advSearchList, &QListWidget::itemDoubleClicked,
            this, &SearchDock::onResultDoubleClicked);
}

void SearchDock::hideEvent(QHideEvent* event)
{
    m_lastNormalSearchText.clear();
    emit searchDockHidden();
    event->accept();
}

void SearchDock::onSearchClicked()
{
    QString text = m_lineEdit->text();
    if (text.isEmpty())
    {
        m_lastNormalSearchText.clear();
        return;
    }
    emit search(false, m_lastNormalSearchText != text);
    m_lastNormalSearchText = text;
}

void SearchDock::onAdvSearchClicked()
{
    m_lastNormalSearchText.clear();
    if (m_advSearchList->isHidden())
    {
        m_advSearchList->show();
    }

    if (m_lineEdit->text().isEmpty())
    {
        return;
    }
    clearAdvSearchList();
    emit search(true);
}

QString SearchDock::getSearchText() const
{
    if (m_lineEdit == nullptr)
    {
        return "";
    }
    return m_lineEdit->text();
}

void SearchDock::setSearchText(QString& text)
{
    m_lineEdit->setText(text);
}

bool SearchDock::isCaseSensitiveChecked() const
{
    return m_caseSensitiveCheck->isChecked();
}

bool SearchDock::isLoopSearchChecked() const
{
    return m_loopCheck->isChecked();
}

void SearchDock::addAdvSearchResult(QString& res, QTextCursor& cursor)
{
    m_advSearchList->addItem(res);
    m_searchResultCursors.append(cursor);
}

void SearchDock::clearAdvSearchList()
{
    m_advSearchList->clear();
    m_searchResultCursors.clear();
}

void SearchDock::onResultDoubleClicked(QListWidgetItem* item)
{
    auto row = m_advSearchList->row(item);
    const auto cursor = m_searchResultCursors.at(row);
    emit searchResultSelected(cursor);
}

void SearchDock::show(bool advanced)
{
    if (isHidden())
    {
        QDockWidget::show();
    }
    if (!m_lineEdit->text().isEmpty())
    {
        m_lineEdit->selectAll();
    }

    if (advanced && m_advSearchList->isHidden())
    {
        m_advSearchList->show();
    }
    else if (!advanced && !m_advSearchList->isHidden())
    {
        m_advSearchList->hide();
    }
}
