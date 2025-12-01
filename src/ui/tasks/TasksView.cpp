#include "ui/tasks/TasksView.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QFrame>
#include <QToolButton>
#include <QEvent>

TasksView::TasksView(QWidget* parent)
    : QWidget(parent)
    , m_scrollArea(new QScrollArea(this))
    , m_container(new QWidget(this))
    , m_grid(new QGridLayout(m_container))
    , m_columns(5)
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setFrameShape(QFrame::NoFrame);
    m_container->setLayout(m_grid);
    m_scrollArea->setWidget(m_container);
    m_scrollArea->setAlignment(Qt::AlignTop | Qt::AlignHCenter);

    m_grid->setContentsMargins(12, 12, 12, 12);
    m_grid->setHorizontalSpacing(10);
    m_grid->setVerticalSpacing(10);

    mainLayout->addWidget(m_scrollArea);
    setLayout(mainLayout);
}

void TasksView::setTasks(const QVector<TaskInfo>& tasks)
{
    m_tasks = tasks;
    rebuildGrid();
}

void TasksView::addTaskFront(const TaskInfo& task)
{
    m_tasks.prepend(task);
    rebuildGrid();
}

void TasksView::rebuildGrid()
{
    // 清空现有卡片
    while (QLayoutItem* item = m_grid->takeAt(0)) {
        if (item->widget()) {
            item->widget()->deleteLater();
        }
        delete item;
    }

    int row = 0;
    int col = 0;

    // 第一个卡片为新建任务
    QWidget* newCard = createNewTaskCard();
    m_grid->addWidget(newCard, row, col);
    col++;

    // 添加任务卡片
    for (const TaskInfo& t : m_tasks) {
        if (col >= m_columns) { row++; col = 0; }
        QWidget* card = createTaskCard(t);
        m_grid->addWidget(card, row, col);
        col++;
    }
}

QWidget* TasksView::createNewTaskCard()
{
    QWidget* card = new QWidget(m_container);
    card->setMinimumSize(160, 110);
    card->setMaximumSize(160, 110);
    card->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    card->setStyleSheet("QWidget { border: 1px dashed #99ccff; background: #f7fbff; border-radius: 6px; }");

    QVBoxLayout* layout = new QVBoxLayout(card);
    layout->setContentsMargins(12, 12, 12, 12);
    layout->setSpacing(8);

    QLabel* icon = new QLabel("➕", card);
    icon->setAlignment(Qt::AlignCenter);
    icon->setStyleSheet("QLabel { font-size: 22pt; color: #0078d4; }");

    QLabel* title = new QLabel("新建任务", card);
    title->setAlignment(Qt::AlignCenter);
    title->setStyleSheet("QLabel { font-size: 10pt; font-weight: bold; color: #0078d4; }");

    QToolButton* btn = new QToolButton(card);
    btn->setText("创建");
    btn->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    btn->setStyleSheet("QToolButton { background: #e5f3ff; border: 1px solid #99ccff; border-radius: 4px; padding: 6px 12px; } QToolButton:hover { background: #cce8ff; } ");
    connect(btn, &QToolButton::clicked, this, [this]() { emit newTaskRequested(); });

    layout->addWidget(icon);
    layout->addWidget(title);
    layout->addStretch();
    layout->addWidget(btn, 0, Qt::AlignCenter);

    return card;
}

bool TasksView::eventFilter(QObject* obj, QEvent* event)
{
    if (event->type() == QEvent::MouseButtonRelease) {
        QWidget* w = qobject_cast<QWidget*>(obj);
        if (w) {
            QVariant v = w->property("taskId");
            if (v.isValid()) {
                emit analyzeRequested(v.toString());
                return true;
            }
        }
    }
    return QWidget::eventFilter(obj, event);
}

QWidget* TasksView::createTaskCard(const TaskInfo& info)
{
    QWidget* card = new QWidget(m_container);
    card->setMinimumSize(160, 110);
    card->setMaximumSize(160, 110);
    card->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    card->setStyleSheet("QWidget { border: 1px solid #d0d0d0; background: white; border-radius: 6px; }");
    card->setProperty("taskId", info.id);
    card->installEventFilter(this);

    QVBoxLayout* layout = new QVBoxLayout(card);
    layout->setContentsMargins(12, 12, 12, 12);
    layout->setSpacing(6);

    QLabel* title = new QLabel(info.name, card);
    title->setStyleSheet("QLabel { font-size: 10pt; font-weight: bold; color: #333; }");

    QLabel* created = new QLabel(QString("建立时间: %1").arg(info.createdAt.toString("yyyy-MM-dd hh:mm")), card);
    created->setStyleSheet("QLabel { color: #666; }");

    // 操作区
    QHBoxLayout* actions = new QHBoxLayout();
    actions->setSpacing(8);

    auto makeBtn = [card](const QString& text) {
        QToolButton* b = new QToolButton(card);
        b->setText(text);
        b->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
        b->setStyleSheet("QToolButton { background: #f5f5f5; border: 1px solid #d0d0d0; border-radius: 4px; padding: 4px 8px; } QToolButton:hover { background: #eeeeee; }");
        return b;
    };

    QToolButton* btnCollect = makeBtn("采集");
    QToolButton* btnAnalyze = makeBtn("分析");
    QToolButton* btnDelete  = makeBtn("删除");

    connect(btnCollect, &QToolButton::clicked, this, [this, info]() { emit collectRequested(info.id); });
    connect(btnAnalyze, &QToolButton::clicked, this, [this, info]() { emit analyzeRequested(info.id); });
    connect(btnDelete,  &QToolButton::clicked, this, [this, info]() { emit deleteRequested(info.id); });

    actions->addWidget(btnCollect);
    actions->addWidget(btnAnalyze);
    actions->addWidget(btnDelete);

    layout->addWidget(title);
    layout->addWidget(created);
    layout->addStretch();
    layout->addLayout(actions);

    return card;
}
