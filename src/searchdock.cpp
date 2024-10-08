#include "inc/searchdock.h"
#include "inc/constants.h"
#include "inc/advancedsearchitem.h"
#include <QDebug>
#include <QSettings>
#include <QDialog>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QListWidget>
#include <QHideEvent>
#include <QCheckBox>
#include <QMessageBox>

SearchDock::SearchDock(QWidget* parent)
    : QDockWidget(parent, Qt::Widget)
{
    setWindowTitle("Search");
    setAllowedAreas(Qt::BottomDockWidgetArea);

    QSettings settings(Config::CONFIG_DIR, QSettings::IniFormat);
    bool caseSensitive = settings.value(Config::SEARCH_CASESENSITIVE, false).toBool();
    bool loopSearch = settings.value(Config::SEARCH_LOOPSEARCH, false).toBool();

    auto findLabel = new QLabel("Enter text:");
    m_lineEdit = new QLineEdit;
    m_lineEdit->setClearButtonEnabled(true);
    m_caseSensitiveCheck = new QCheckBox("Case sensitive", this);
    m_caseSensitiveCheck->setChecked(caseSensitive);
    m_loopCheck = new QCheckBox("Loop", this);
    m_loopCheck->setChecked(loopSearch);
    m_searchButton = new QPushButton("Search");
    m_advSearchButton = new QPushButton("Advanced search");
    m_clearHighlightButton = new QPushButton("Clear Highlights");

    QHBoxLayout* searchRowLayout = new QHBoxLayout;
    searchRowLayout->setSpacing(5);
    searchRowLayout->addWidget(findLabel);
    searchRowLayout->addWidget(m_lineEdit);
    searchRowLayout->addWidget(m_caseSensitiveCheck);
    searchRowLayout->addWidget(m_loopCheck);
    searchRowLayout->addWidget(m_searchButton);
    searchRowLayout->addWidget(m_advSearchButton);
    searchRowLayout->addWidget(m_clearHighlightButton);

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
    m_searchButton->setDefault(true);

    connect(m_lineEdit, &QLineEdit::returnPressed, this, [=](){
        if (m_searchButton->isDefault())
            this->onSearchClicked();
        else if (m_advSearchButton->isDefault())
            this->onAdvSearchClicked();
    });
    connect(m_searchButton, &QPushButton::clicked, this, &SearchDock::onSearchClicked);
    connect(m_advSearchButton, &QPushButton::clicked, this, &SearchDock::onAdvSearchClicked);
    connect(m_advSearchList, &QListWidget::itemDoubleClicked,
            this, &SearchDock::onResultDoubleClicked);
    connect(m_clearHighlightButton, &QPushButton::clicked, this, [=](){
        emit clearHighlight();
    });
    connect(m_caseSensitiveCheck, &QCheckBox::stateChanged, this, [=](int /*state*/){
        // Case sensitive option changed, search the keyword again
        m_lastQuery.clear();
    });
}

void SearchDock::hideEvent(QHideEvent* event)
{
    m_lastQuery.clear();
    emit searchDockHidden();
    event->accept();
}

void SearchDock::onSearchClicked()
{
    QString text = m_lineEdit->text();
    if (text.isEmpty())
    {
        m_lastQuery.clear();
        return;
    }
    emit search(false, m_lastQuery != text);
    m_searchButton->setDefault(true);
    m_advSearchButton->setDefault(false);
    m_lastQuery = text;
}

void SearchDock::onAdvSearchClicked()
{
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
    m_searchButton->setDefault(false);
    m_advSearchButton->setDefault(true);
    // We don't care about last query in advanced search, always search all again
    m_lastQuery.clear();
}

QString SearchDock::getQuery() const
{
    if (!m_lineEdit)
    {
        return "";
    }
    return m_lineEdit->text();
}

void SearchDock::setQuery(QString& text)
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

void SearchDock::addAdvSearchResult(QTextCursor& cursor)
{
    auto item = new AdvancedSearchItem(cursor);
    m_advSearchList->addItem(item);
}

void SearchDock::sortAdvSearchResult()
{
    m_advSearchList->sortItems();
}

void SearchDock::clearAdvSearchList()
{
    m_advSearchList->clear();
}

void SearchDock::onResultDoubleClicked(QListWidgetItem* item)
{
    const auto cursor = item->data(Qt::UserRole).value<QTextCursor>();
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

    if (advanced)
    {
        if (m_advSearchList->isHidden())
        {
            m_advSearchList->show();
        }
        m_searchButton->setDefault(false);
        m_advSearchButton->setDefault(true);
    }
    else
    {
        if (!m_advSearchList->isHidden())
        {
            m_advSearchList->hide();
        }
        m_searchButton->setDefault(true);
        m_advSearchButton->setDefault(false);
    }
}
