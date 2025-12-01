#include "ui/ribbon/RibbonBar.h"
#include <QLabel>
#include <QFrame>

// ==================== RibbonBar Implementation ====================

RibbonBar::RibbonBar(QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    
    m_tabWidget = new QTabWidget(this);
    m_tabWidget->setDocumentMode(true);
    m_tabWidget->setStyleSheet(R"(
        QTabWidget::pane {
            border: 1px solid #c0c0c0;
            background: #f0f0f0;
        }
        QTabBar::tab {
            background: #e0e0e0;
            border: 1px solid #c0c0c0;
            padding: 8px 20px;
            margin-right: 2px;
        }
        QTabBar::tab:selected {
            background: #f0f0f0;
            border-bottom: none;
        }
        QTabBar::tab:hover {
            background: #f5f5f5;
        }
    )");
    
    layout->addWidget(m_tabWidget);
    setLayout(layout);
    
    setMinimumHeight(120);
    setMaximumHeight(150);
}

RibbonBar::~RibbonBar()
{
    // Qt 的父子关系会自动清理子控件
}

RibbonTab* RibbonBar::addTab(const QString& title)
{
    RibbonTab* tab = new RibbonTab(this);
    m_tabWidget->addTab(tab, title);
    return tab;
}

void RibbonBar::setCurrentTab(int index)
{
    m_tabWidget->setCurrentIndex(index);
}

RibbonTab* RibbonBar::getTab(int index)
{
    return qobject_cast<RibbonTab*>(m_tabWidget->widget(index));
}

int RibbonBar::tabCount() const
{
    return m_tabWidget->count();
}

// ==================== RibbonTab Implementation ====================

RibbonTab::RibbonTab(QWidget *parent)
    : QWidget(parent)
{
    m_layout = new QHBoxLayout(this);
    m_layout->setContentsMargins(5, 5, 5, 5);
    m_layout->setSpacing(2);
    m_layout->addStretch();
    
    setLayout(m_layout);
}

RibbonTab::~RibbonTab()
{
    // Qt 的父子关系会自动清理子控件
    m_groups.clear();
}

RibbonGroup* RibbonTab::addGroup(const QString& title)
{
    // 添加分隔线（如果不是第一个组）
    if (m_groups.count() > 0) {
        QFrame* separator = new QFrame(this);
        separator->setFrameShape(QFrame::VLine);
        separator->setFrameShadow(QFrame::Sunken);
        separator->setMinimumHeight(50);
        separator->setMaximumWidth(2);
        // 在stretch之前插入分隔线
        int insertPos = m_layout->count() - 1;
        m_layout->insertWidget(insertPos, separator);
    }
    
    // 创建并添加组
    RibbonGroup* group = new RibbonGroup(title, this);
    m_groups.append(group);
    
    // 在stretch之前插入组
    int insertPos = m_layout->count() - 1;
    m_layout->insertWidget(insertPos, group);
    
    return group;
}

RibbonGroup* RibbonTab::getGroup(int index)
{
    if (index >= 0 && index < m_groups.count()) {
        return m_groups[index];
    }
    return nullptr;
}

int RibbonTab::groupCount() const
{
    return m_groups.count();
}

// ==================== RibbonGroup Implementation ====================

RibbonGroup::RibbonGroup(const QString& title, QWidget *parent)
    : QWidget(parent)
    , m_title(title)
{
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(3, 3, 3, 3);
    m_mainLayout->setSpacing(2);
    
    // 按钮容器
    QWidget* buttonWidget = new QWidget(this);
    m_buttonLayout = new QHBoxLayout(buttonWidget);
    m_buttonLayout->setContentsMargins(0, 0, 0, 0);
    m_buttonLayout->setSpacing(2);
    buttonWidget->setLayout(m_buttonLayout);
    
    // 标题标签
    QLabel* titleLabel = new QLabel(title, this);
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet("QLabel { color: #606060; font-size: 9pt; }");
    
    m_mainLayout->addWidget(buttonWidget);
    m_mainLayout->addWidget(titleLabel);
    
    setLayout(m_mainLayout);
}

RibbonGroup::~RibbonGroup()
{
    // Qt 的父子关系会自动清理子控件
    m_buttons.clear();
}

QToolButton* RibbonGroup::addLargeButton(const QString& text, const QIcon& icon)
{
    QToolButton* button = new QToolButton(this);
    button->setText(text);
    button->setIcon(icon);
    button->setIconSize(QSize(32, 32));
    button->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    button->setAutoRaise(true);
    button->setMinimumSize(60, 65);
    
    button->setStyleSheet(R"(
        QToolButton {
            background: transparent;
            border: 1px solid transparent;
            border-radius: 3px;
            padding: 5px;
        }
        QToolButton:hover {
            background: #e5f3ff;
            border: 1px solid #99ccff;
        }
        QToolButton:pressed {
            background: #cce8ff;
        }
    )");
    
    m_buttonLayout->addWidget(button);
    m_buttons.append(button);
    
    return button;
}

QToolButton* RibbonGroup::addSmallButton(const QString& text, const QIcon& icon)
{
    QToolButton* button = new QToolButton(this);
    button->setText(text);
    button->setIcon(icon);
    button->setIconSize(QSize(16, 16));
    button->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    button->setAutoRaise(true);
    button->setMinimumSize(80, 24);
    
    button->setStyleSheet(R"(
        QToolButton {
            background: transparent;
            border: 1px solid transparent;
            border-radius: 3px;
            padding: 3px 8px;
            text-align: left;
        }
        QToolButton:hover {
            background: #e5f3ff;
            border: 1px solid #99ccff;
        }
        QToolButton:pressed {
            background: #cce8ff;
        }
    )");
    
    m_buttonLayout->addWidget(button);
    m_buttons.append(button);
    
    return button;
}

void RibbonGroup::addSeparator()
{
    QFrame* separator = new QFrame(this);
    separator->setFrameShape(QFrame::VLine);
    separator->setFrameShadow(QFrame::Sunken);
    m_buttonLayout->addWidget(separator);
}
