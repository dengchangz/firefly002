#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStatusBar>
#include <QLabel>
#include <QMdiArea>
#include <QDockWidget>
#include <QTreeWidget>
#include <QListWidget>
#include <QToolButton>
#include <QVector>
#include "ui/tasks/TasksView.h"

class RibbonBar;
class ZmqClient;
class AuthManager;
class TasksView;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    // 认证槽函数
    void onLoginSuccess(const QString& username);
    void onLoginFailed(const QString& reason);
    void onLogout();
    
    // Ribbon按钮槽函数
    void onNewTask();
    void onOpenTask();
    void onImportData();
    void onCleanData();
    void onQueryData();
    void onAnalyzeData();
    void onGenerateReport();
    void onSettings();
    void onAbout();
    
    // ZeroMQ信号槽
    void onZmqConnected();
    void onZmqDisconnected();
    void onZmqError(const QString& error);
    void onNotificationReceived(const QString& type, const QJsonObject& data);

private:
    void setupUi();
    void showLoginDialog();
    void updateUserInfo();
    void setupRibbon();
    void setupRibbonMinimal();
    void setupCentralWidget();
    void setupStatusBar();
    void setupConnections();
    void setupLeftPanel();     // 左侧面板
    void setupRightPanel();    // 右侧面板

    void openTaskManagerView();
    void showAdvancedTabsIfNeeded();
    
    bool connectToBackend();
    void updateStatusBar(const QString& message);

private:
    // UI组件
    RibbonBar* m_ribbonBar;
    QMdiArea* m_mdiArea;
    QStatusBar* m_statusBar;
    QLabel* m_statusLabel;
    QLabel* m_connectionLabel;
    QLabel* m_userLabel;  // 用户信息标签
    
    // 侧边面板
    QDockWidget* m_leftDock;      // 左侧停靠窗口
    QDockWidget* m_rightDock;     // 右侧停靠窗口
    QDockWidget* m_newTaskDock;   // 新建任务滑出面板
    QTreeWidget* m_taskTree;      // 导航树
    QListWidget* m_logList;       // 日志列表
    QToolButton* m_leftToggleBtn; // 左侧面板切换按钮
    QToolButton* m_rightToggleBtn;// 右侧面板切换按钮
    
    // 任务视图
    TasksView* m_tasksView;
    QVector<TaskInfo> m_tasks;
    
    // 网络和认证
    ZmqClient* m_zmqClient;
    AuthManager* m_authManager;
};

#endif // MAINWINDOW_H
