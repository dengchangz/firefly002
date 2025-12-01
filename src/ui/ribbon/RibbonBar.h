#ifndef RIBBONBAR_H
#define RIBBONBAR_H

#include <QWidget>
#include <QTabWidget>
#include <QToolButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QString>
#include <QList>

class RibbonTab;
class RibbonGroup;

class RibbonBar : public QWidget
{
    Q_OBJECT

public:
    explicit RibbonBar(QWidget *parent = nullptr);
    ~RibbonBar();
    
    // 添加标签页
    RibbonTab* addTab(const QString& title);
    
    // 设置当前标签页
    void setCurrentTab(int index);
    
    // 获取标签页
    RibbonTab* getTab(int index);
    int tabCount() const;

private:
    QTabWidget* m_tabWidget;
};

class RibbonTab : public QWidget
{
    Q_OBJECT

public:
    explicit RibbonTab(QWidget *parent = nullptr);
    ~RibbonTab();
    
    // 添加组
    RibbonGroup* addGroup(const QString& title);
    
    // 获取组
    RibbonGroup* getGroup(int index);
    int groupCount() const;

private:
    QHBoxLayout* m_layout;
    QList<RibbonGroup*> m_groups;
};

class RibbonGroup : public QWidget
{
    Q_OBJECT

public:
    explicit RibbonGroup(const QString& title, QWidget *parent = nullptr);
    ~RibbonGroup();
    
    // 添加大按钮（带图标和文字）
    QToolButton* addLargeButton(const QString& text, const QIcon& icon);
    
    // 添加小按钮（只有图标）
    QToolButton* addSmallButton(const QString& text, const QIcon& icon);
    
    // 添加分隔符
    void addSeparator();

private:
    QString m_title;
    QVBoxLayout* m_mainLayout;
    QHBoxLayout* m_buttonLayout;
    QList<QToolButton*> m_buttons;
};

#endif // RIBBONBAR_H
