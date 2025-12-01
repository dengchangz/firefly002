#include "ui/MainWindow.h"
#include "ui/ribbon/RibbonBar.h"
#include "network/ZmqClient.h"
#include "auth/AuthManager.h"
#include "auth/LoginDialog.h"
#include "core/Application.h"
#include "core/Logger.h"
#include "ui/tasks/TasksView.h"
#include <QMessageBox>
#include <QToolButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTreeWidgetItem>
#include <QDateTime>
#include <QTimer>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QMdiSubWindow>
#include <QUuid>
#include <QJsonObject>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_ribbonBar(nullptr)
    , m_mdiArea(nullptr)
    , m_statusBar(nullptr)
    , m_statusLabel(nullptr)
    , m_connectionLabel(nullptr)
    , m_userLabel(nullptr)
    , m_leftDock(nullptr)
    , m_rightDock(nullptr)
    , m_newTaskDock(nullptr)
    , m_taskTree(nullptr)
    , m_logList(nullptr)
    , m_leftToggleBtn(nullptr)
    , m_rightToggleBtn(nullptr)
    , m_zmqClient(nullptr)
    , m_authManager(nullptr)
    , m_tasksView(nullptr)
{
    // 初始化认证管理器
    m_authManager = AuthManager::instance();
    
    setupUi();
    setupConnections();
    
    // 显示登录对话框
    QTimer::singleShot(100, this, &MainWindow::showLoginDialog);
}

MainWindow::~MainWindow()
{
}





void MainWindow::setupUi()
{
    setWindowTitle("资金分析系统 v1.0.0");
    resize(1400, 900);
    
    setupRibbonMinimal();
    setupLeftPanel();
    setupRightPanel();
    if (m_leftDock) m_leftDock->hide();
    if (m_rightDock) m_rightDock->hide();
    setupCentralWidget();
    setupStatusBar();
}

void MainWindow::setupRibbon()
{
    m_ribbonBar = new RibbonBar(this);
    
    // 设置更美观的样式
    m_ribbonBar->setStyleSheet(R"(
        QTabWidget::pane {
            border: 1px solid #d0d0d0;
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                        stop:0 #f8f8f8, stop:1 #e8e8e8);
        }
        QTabBar::tab {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                        stop:0 #f0f0f0, stop:1 #e0e0e0);
            border: 1px solid #c0c0c0;
            border-bottom: none;
            padding: 10px 24px;
            margin-right: 2px;
            border-top-left-radius: 4px;
            border-top-right-radius: 4px;
            font-size: 11pt;
            color: #404040;
        }
        QTabBar::tab:selected {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                        stop:0 #ffffff, stop:1 #f8f8f8);
            border-bottom: 2px solid #0078d4;
            color: #0078d4;
            font-weight: bold;
        }
        QTabBar::tab:hover {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                        stop:0 #f8f8f8, stop:1 #f0f0f0);
        }
    )");
    // ==================== 1. 分析任务 ====================
    RibbonTab* taskTab = m_ribbonBar->addTab("📋 分析任务");
    if (!taskTab) return;
    
    // 任务管理组
    RibbonGroup* taskMgmtGroup = taskTab->addGroup("任务管理");
    if (taskMgmtGroup) {
        QToolButton* btnNewTask = taskMgmtGroup->addLargeButton("新建任务", QIcon());
        QToolButton* btnOpenTask = taskMgmtGroup->addLargeButton("打开任务", QIcon());
        QToolButton* btnSaveTask = taskMgmtGroup->addLargeButton("保存任务", QIcon());
        if (btnNewTask) connect(btnNewTask, &QToolButton::clicked, this, [this]() {
            openTaskManagerView();
            if (m_tasksView) emit m_tasksView->newTaskRequested();
        });
        if (btnOpenTask) connect(btnOpenTask, &QToolButton::clicked, this, [this]() { openTaskManagerView(); });
    }
    
    // 分析执行组
    RibbonGroup* analysisExecGroup = taskTab->addGroup("分析执行");
    if (analysisExecGroup) {
        QToolButton* btnStartAnalysis = analysisExecGroup->addLargeButton("开始分析", QIcon());
        QToolButton* btnStopAnalysis = analysisExecGroup->addLargeButton("停止分析", QIcon());
        QToolButton* btnAnalysisHistory = analysisExecGroup->addLargeButton("历史记录", QIcon());
        if (btnStartAnalysis) connect(btnStartAnalysis, &QToolButton::clicked, this, [this]() {
            showAdvancedTabsIfNeeded();
        });
    }
    
    // 数据管理标签稍后显示（进入具体任务后）
    
    // 可视分析标签稍后显示（进入具体任务后）
    
    // 报告生成标签稍后显示（进入具体任务后）
    
    // ==================== 5. 工具集 ====================
    RibbonTab* toolsTab = m_ribbonBar->addTab("🔧 工具集");
    if (!toolsTab) return;
    
    // 系统工具组
    RibbonGroup* systemToolsGroup = toolsTab->addGroup("系统工具");
    if (systemToolsGroup) {
        QToolButton* btnSettings = systemToolsGroup->addLargeButton("系统设置", QIcon());
        QToolButton* btnUserMgmt = systemToolsGroup->addLargeButton("用户管理", QIcon());
        QToolButton* btnLogViewer = systemToolsGroup->addLargeButton("日志查看", QIcon());
        if (btnSettings) connect(btnSettings, &QToolButton::clicked, this, &MainWindow::onSettings);
    }
    
    // 视图管理组
    RibbonGroup* viewGroup = toolsTab->addGroup("视图管理");
    if (viewGroup) {
        QToolButton* btnShowLeftPanel = viewGroup->addLargeButton("任务面板", QIcon());
        QToolButton* btnShowRightPanel = viewGroup->addLargeButton("日志面板", QIcon());
        
        if (btnShowLeftPanel) {
            connect(btnShowLeftPanel, &QToolButton::clicked, this, [this]() {
                if (m_leftDock) {
                    m_leftDock->setVisible(!m_leftDock->isVisible());
                    if (m_leftDock->isVisible() && m_logList) {
                        m_logList->addItem("▶ " + QDateTime::currentDateTime().toString("hh:mm:ss") + 
                                          " - 左侧面板已显示");
                        m_logList->scrollToBottom();
                    }
                }
            });
        }
        
        if (btnShowRightPanel) {
            connect(btnShowRightPanel, &QToolButton::clicked, this, [this]() {
                if (m_rightDock) {
                    m_rightDock->setVisible(!m_rightDock->isVisible());
                    if (m_rightDock->isVisible() && m_logList) {
                        m_logList->addItem("▶ " + QDateTime::currentDateTime().toString("hh:mm:ss") + 
                                          " - 右侧面板已显示");
                        m_logList->scrollToBottom();
                    }
                }
            });
        }
    }
    // 数据工具组
    RibbonGroup* dataToolsGroup = toolsTab->addGroup("数据工具");
    if (dataToolsGroup) {
        QToolButton* btnBackup = dataToolsGroup->addLargeButton("数据备份", QIcon());
        QToolButton* btnRestore = dataToolsGroup->addLargeButton("数据恢复", QIcon());
        QToolButton* btnCleanup = dataToolsGroup->addLargeButton("数据清理", QIcon());
    }
    
    // 账户管理组
    RibbonGroup* accountGroup = toolsTab->addGroup("账户");
    if (accountGroup) {
        QToolButton* btnProfile = accountGroup->addLargeButton("个人资料", QIcon());
        QToolButton* btnChangePassword = accountGroup->addLargeButton("修改密码", QIcon());
        QToolButton* btnLogout = accountGroup->addLargeButton("退出登录", QIcon());
        if (btnLogout) connect(btnLogout, &QToolButton::clicked, this, &MainWindow::onLogout);
    }
    
    // ==================== 6. 关于 ====================
    RibbonTab* aboutTab = m_ribbonBar->addTab("ℹ️ 关于");
    if (!aboutTab) return;
    
    // 帮助组
    RibbonGroup* helpGroup = aboutTab->addGroup("帮助");
    if (helpGroup) {
        QToolButton* btnUserGuide = helpGroup->addLargeButton("用户手册", QIcon());
        QToolButton* btnOnlineHelp = helpGroup->addLargeButton("在线帮助", QIcon());
        QToolButton* btnFeedback = helpGroup->addLargeButton("问题反馈", QIcon());
    }
    
    // 关于系统组
    RibbonGroup* aboutSystemGroup = aboutTab->addGroup("系统信息");
    if (aboutSystemGroup) {
        QToolButton* btnAbout = aboutSystemGroup->addLargeButton("关于系统", QIcon());
        QToolButton* btnVersion = aboutSystemGroup->addLargeButton("版本信息", QIcon());
        QToolButton* btnLicense = aboutSystemGroup->addLargeButton("许可证", QIcon());
        if (btnAbout) connect(btnAbout, &QToolButton::clicked, this, &MainWindow::onAbout);
    }
    
    // 将Ribbon添加到主窗口
    QWidget* ribbonContainer = new QWidget(this);
    QVBoxLayout* layout = new QVBoxLayout(ribbonContainer);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addWidget(m_ribbonBar);
    
    setMenuWidget(ribbonContainer);
}

void MainWindow::setupRibbonMinimal()
{
    if (menuWidget()) {
        QWidget* old = menuWidget();
        setMenuWidget(nullptr);
        if (old) old->deleteLater();
    }
    if (m_ribbonBar) {
        m_ribbonBar->deleteLater();
        m_ribbonBar = nullptr;
    }
    setupRibbon();
}

void MainWindow::setupCentralWidget()
{
    // 使用MDI多文档界面
    m_mdiArea = new QMdiArea(this);
    m_mdiArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_mdiArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_mdiArea->setViewMode(QMdiArea::TabbedView);
    m_mdiArea->setTabsClosable(true);
    m_mdiArea->setTabsMovable(true);
    m_mdiArea->setStyleSheet(R"(
        QMdiArea {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                        stop:0 #f5f5f5, stop:1 #e8e8e8);
        }
        QMdiArea QTabBar::tab {
            min-width: 140px;
            max-width: 140px;
        }
        QMdiArea QTabBar::tab:selected {
            font-weight: bold;
            color: #0078d4;
        }
    )" );
    setCentralWidget(m_mdiArea);
}

void MainWindow::setupLeftPanel()
{
    // 创建左侧停靠窗口
    m_leftDock = new QDockWidget("导航", this);
    m_leftDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    m_leftDock->setFeatures(QDockWidget::DockWidgetClosable | 
                            QDockWidget::DockWidgetMovable | 
                            QDockWidget::DockWidgetFloatable);
    m_leftDock->setMinimumWidth(200);
    m_leftDock->setMaximumWidth(320);
    
    // 导航树
    m_taskTree = new QTreeWidget(m_leftDock);
    m_taskTree->setHeaderHidden(true);
    m_taskTree->setStyleSheet(R"(
        QTreeWidget { border: 1px solid #d0d0d0; background: white; font-size: 10pt; }
        QTreeWidget::item { padding: 5px; }
        QTreeWidget::item:selected { background: #0078d4; color: white; }
        QTreeWidget::item:hover { background: #e5f3ff; }
    )");

    QTreeWidgetItem* root = new QTreeWidgetItem(QStringList("任务中心"));
    m_taskTree->addTopLevelItem(root);
    QTreeWidgetItem* myTasks = new QTreeWidgetItem(QStringList("我的任务"));
    QTreeWidgetItem* stats   = new QTreeWidgetItem(QStringList("任务统计"));
    QTreeWidgetItem* share   = new QTreeWidgetItem(QStringList("分享任务"));
    root->addChild(myTasks);
    root->addChild(stats);
    root->addChild(share);
    root->setExpanded(true);

    m_leftDock->setWidget(m_taskTree);
    addDockWidget(Qt::LeftDockWidgetArea, m_leftDock);

    // 导航切换响应（当前仅实现我的任务）
    connect(m_taskTree, &QTreeWidget::itemClicked, this, [this](QTreeWidgetItem* item, int){
        if (!item) return;
        if (item->text(0) == "我的任务") {
            if (m_tasksView) m_tasksView->setTasks(m_tasks);
        }
        // TODO: 任务统计、分享任务后续实现
    });
}

void MainWindow::setupRightPanel()
{
    // 创建右侧停靠窗口
    m_rightDock = new QDockWidget("日志与通知", this);
    m_rightDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    m_rightDock->setFeatures(QDockWidget::DockWidgetClosable | 
                             QDockWidget::DockWidgetMovable | 
                             QDockWidget::DockWidgetFloatable);
    m_rightDock->setMinimumWidth(250);
    m_rightDock->setMaximumWidth(500);
    
    // 创建面板容器
    QWidget* rightWidget = new QWidget(m_rightDock);
    QVBoxLayout* rightLayout = new QVBoxLayout(rightWidget);
    rightLayout->setContentsMargins(5, 5, 5, 5);
    rightLayout->setSpacing(5);
    
    // 创建顶部工具栏
    QWidget* rightToolBar = new QWidget(rightWidget);
    QHBoxLayout* rightToolLayout = new QHBoxLayout(rightToolBar);
    rightToolLayout->setContentsMargins(0, 0, 0, 0);
    rightToolLayout->setSpacing(5);
    
    QLabel* rightTitleLabel = new QLabel("📝 日志通知", rightToolBar);
    rightTitleLabel->setStyleSheet("QLabel { font-weight: bold; font-size: 10pt; color: #0078d4; }");
    
    // 清空按钮
    QToolButton* clearBtn = new QToolButton(rightToolBar);
    clearBtn->setText("🗑");
    clearBtn->setToolTip("清空日志");
    clearBtn->setFixedSize(24, 24);
    clearBtn->setStyleSheet(R"(
        QToolButton {
            background: transparent;
            border: 1px solid #d0d0d0;
            border-radius: 3px;
        }
        QToolButton:hover {
            background: #ffe5e5;
            border: 1px solid #ff0000;
        }
        QToolButton:pressed {
            background: #ffcccc;
        }
    )");
    
    // 收起按钮
    m_rightToggleBtn = new QToolButton(rightToolBar);
    m_rightToggleBtn->setText("▶");
    m_rightToggleBtn->setToolTip("收起面板");
    m_rightToggleBtn->setFixedSize(24, 24);
    m_rightToggleBtn->setStyleSheet(R"(
        QToolButton {
            background: transparent;
            border: 1px solid #d0d0d0;
            border-radius: 3px;
            font-weight: bold;
        }
        QToolButton:hover {
            background: #e5f3ff;
            border: 1px solid #0078d4;
        }
        QToolButton:pressed {
            background: #cce8ff;
        }
    )");
    connect(m_rightToggleBtn, &QToolButton::clicked, this, [this]() {
        if (m_rightDock->isVisible()) {
            m_rightDock->hide();
        }
    });
    
    rightToolLayout->addWidget(rightTitleLabel);
    rightToolLayout->addStretch();
    rightToolLayout->addWidget(clearBtn);
    rightToolLayout->addWidget(m_rightToggleBtn);
    
    rightLayout->addWidget(rightToolBar);
    
    // 创建日志列表
    m_logList = new QListWidget(rightWidget);
    m_logList->setStyleSheet(R"(
        QListWidget {
            border: 1px solid #d0d0d0;
            background: white;
            font-size: 9pt;
            font-family: 'Consolas', 'Courier New', monospace;
            border-radius: 3px;
        }
        QListWidget::item {
            padding: 5px;
            border-bottom: 1px solid #f0f0f0;
        }
        QListWidget::item:selected {
            background: #0078d4;
            color: white;
        }
    )");
    
    // 添加示例日志
    m_logList->addItem("✅ 系统启动成功");
    m_logList->addItem("🔌 等待连接后端服务...");
    m_logList->addItem("ℹ️ 就绪，等待操作...");
    
    // 连接清空按钮
    connect(clearBtn, &QToolButton::clicked, this, [this]() {
        m_logList->clear();
        m_logList->addItem("🗑 " + QDateTime::currentDateTime().toString("hh:mm:ss") + " - 日志已清空");
    });
    
    rightLayout->addWidget(m_logList);
    
    rightWidget->setLayout(rightLayout);
    m_rightDock->setWidget(rightWidget);
    addDockWidget(Qt::RightDockWidgetArea, m_rightDock);
    
    // 添加显示/隐藏的连接
    connect(m_rightDock, &QDockWidget::visibilityChanged, this, [this](bool visible) {
        if (visible && m_rightToggleBtn) {
            m_rightToggleBtn->setText("▶");
            m_rightToggleBtn->setToolTip("收起面板");
        }
    });
}

void MainWindow::setupStatusBar()
{
    m_statusBar = statusBar();
    
    m_statusLabel = new QLabel("就绪", this);
    m_statusBar->addWidget(m_statusLabel, 1);
    
    // 用户信息标签
    m_userLabel = new QLabel("未登录", this);
    m_userLabel->setStyleSheet("QLabel { color: #0078d4; padding: 0 10px; }");
    m_statusBar->addPermanentWidget(m_userLabel);
    
    m_connectionLabel = new QLabel("未连接", this);
    m_connectionLabel->setStyleSheet("QLabel { color: red; }");
    m_statusBar->addPermanentWidget(m_connectionLabel);
}

void MainWindow::setupConnections()
{
    // 创建ZeroMQ客户端
    m_zmqClient = new ZmqClient(this);
    
    connect(m_zmqClient, &ZmqClient::connected,
            this, &MainWindow::onZmqConnected);
    connect(m_zmqClient, &ZmqClient::disconnected,
            this, &MainWindow::onZmqDisconnected);
    connect(m_zmqClient, &ZmqClient::errorOccurred,
            this, &MainWindow::onZmqError);
    connect(m_zmqClient, &ZmqClient::notificationReceived,
            this, &MainWindow::onNotificationReceived);
    
    // 连接认证信号
    connect(m_authManager, &AuthManager::loginSuccess,
            this, &MainWindow::onLoginSuccess);
    connect(m_authManager, &AuthManager::loginFailed,
            this, &MainWindow::onLoginFailed);
}

bool MainWindow::connectToBackend()
{
    Application* app = Application::instance();
    
    QString host = app->getConfigValue("backend/host", "localhost");
    int reqPort = app->getConfigValue("backend/req_port", "5555").toInt();
    int pubPort = app->getConfigValue("backend/pub_port", "5556").toInt();
    
    QString reqEndpoint = QString("tcp://%1:%2").arg(host).arg(reqPort);
    QString subEndpoint = QString("tcp://%1:%2").arg(host).arg(pubPort);
    
    Logger::instance()->info("Connecting to backend: " + reqEndpoint);
    
    bool success = m_zmqClient->connectToServer(reqEndpoint, subEndpoint);
    
    if (success) {
        m_zmqClient->subscribe("");  // 订阅所有消息
    }
    return success;
}

void MainWindow::updateStatusBar(const QString& message)
{
    m_statusLabel->setText(message);
}

// ==================== 认证槽函数 ====================

// ==================== Ribbon按钮槽函数 ====================
void MainWindow::showLoginDialog()
{
    LoginDialog loginDlg(this);
    
    if (loginDlg.exec() == QDialog::Accepted) {
        QString username = loginDlg.getUsername();
        QString password = loginDlg.getPassword();
        
        // 执行登录
        m_authManager->login(username, password);
    } else {
        // 用户取消登录，退出程序
        Logger::instance()->info("User cancelled login, exiting...");
        QTimer::singleShot(0, qApp, &QApplication::quit);
    }
}

void MainWindow::onLoginSuccess(const QString& username)
{
    Logger::instance()->info("Login success: " + username);
    
    updateUserInfo();
    
    // 登录后仅显示：分析任务、工具集、关于
    setupRibbonMinimal();
    
    // 打开任务管理视图
    openTaskManagerView();
    
    // 连接到后端
    connectToBackend();
    
    // 添加日志
    if (m_logList) {
        m_logList->addItem("✅ " + QDateTime::currentDateTime().toString("hh:mm:ss") + 
                          " - 用户 " + username + " 登录成功");
        m_logList->scrollToBottom();
    }
    
    updateStatusBar("欢迎，" + username);
}

void MainWindow::onLoginFailed(const QString& reason)
{
    Logger::instance()->warning("Login failed: " + reason);
    
    QMessageBox::critical(this, "登录失败", reason);
    
    // 重新显示登录对话框
    QTimer::singleShot(100, this, &MainWindow::showLoginDialog);
}

void MainWindow::onLogout()
{
    if (!m_authManager->isAuthenticated()) {
        return;
    }
    
    QMessageBox::StandardButton reply = QMessageBox::question(
        this,
        "确认退出",
        "确定要退出登录吗？",
        QMessageBox::Yes | QMessageBox::No
    );
    
    if (reply == QMessageBox::Yes) {
        QString username = m_authManager->getCurrentUser();
        m_authManager->logout();
        
        // 添加日志
        if (m_logList) {
            m_logList->addItem("🚪 " + QDateTime::currentDateTime().toString("hh:mm:ss") + 
                              " - 用户 " + username + " 退出登录");
            m_logList->scrollToBottom();
        }
        
        // 重新显示登录对话框
        QTimer::singleShot(100, this, &MainWindow::showLoginDialog);
    }
}

void MainWindow::updateUserInfo()
{
    if (m_authManager->isAuthenticated()) {
        QString username = m_authManager->getCurrentUser();
        QString role = m_authManager->getCurrentRole();
        
        QString roleText;
        if (role == "administrator") {
            roleText = "管理员";
        } else if (role == "user") {
            roleText = "普通用户";
        } else if (role == "guest") {
            roleText = "访客";
        } else {
            roleText = role;
        }
        
        m_userLabel->setText("👤 " + username + " (" + roleText + ")");
        m_userLabel->setStyleSheet("QLabel { color: green; padding: 0 10px; }");
    } else {
        m_userLabel->setText("未登录");
        m_userLabel->setStyleSheet("QLabel { color: red; padding: 0 10px; }");
    }
}

void MainWindow::openTaskManagerView()
{
    Logger::instance()->info("Opening TasksView...");
    if (m_logList) { m_logList->addItem("📂 " + QDateTime::currentDateTime().toString("hh:mm:ss") + " - 打开任务管理视图"); m_logList->scrollToBottom(); }
    if (m_leftDock) m_leftDock->hide();
    if (m_rightDock) m_rightDock->hide();
    // 如果任务视图未创建，则创建
    if (!m_tasksView) {
        m_tasksView = new TasksView(nullptr);
        // 连接新建任务信号，弹出右侧新建面板
        connect(m_tasksView, &TasksView::newTaskRequested, this, [this]() {
            if (!m_newTaskDock) {
                m_newTaskDock = new QDockWidget("新建任务", this);
                m_newTaskDock->setAllowedAreas(Qt::RightDockWidgetArea);
                QWidget* form = new QWidget(m_newTaskDock);
                QVBoxLayout* v = new QVBoxLayout(form);
                v->setContentsMargins(10,10,10,10);
                v->setSpacing(8);
                QLabel* title = new QLabel("创建新的分析任务", form);
                title->setStyleSheet("QLabel { font-weight: bold; font-size: 11pt; color: #0078d4; }");
                QLineEdit* nameEdit = new QLineEdit(form);
                nameEdit->setPlaceholderText("任务名称");
                QLineEdit* commissionerEdit = new QLineEdit(form);
                commissionerEdit->setPlaceholderText("任务委托人");
                QLineEdit* summaryEdit = new QLineEdit(form);
                summaryEdit->setPlaceholderText("任务简要");
                QPushButton* okBtn = new QPushButton("确定", form);
                QPushButton* cancelBtn = new QPushButton("取消", form);
                QHBoxLayout* btns = new QHBoxLayout();
                btns->addStretch();
                btns->addWidget(okBtn);
                btns->addWidget(cancelBtn);
                v->addWidget(title);
                // 任务编号与创建时间
                QLineEdit* idEdit = new QLineEdit(form);
                idEdit->setReadOnly(true);
                idEdit->setText(QUuid::createUuid().toString(QUuid::WithoutBraces));
                QLineEdit* timeEdit = new QLineEdit(form);
                timeEdit->setReadOnly(true);
                timeEdit->setText(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm"));
                v->addWidget(new QLabel("任务编号", form));
                v->addWidget(idEdit);
                v->addWidget(new QLabel("创建时间", form));
                v->addWidget(timeEdit);
                v->addWidget(nameEdit);
                v->addWidget(commissionerEdit);
                v->addWidget(summaryEdit);
                v->addLayout(btns);
                form->setLayout(v);
                m_newTaskDock->setWidget(form);
                addDockWidget(Qt::RightDockWidgetArea, m_newTaskDock);
                connect(cancelBtn, &QPushButton::clicked, this, [this]() { if (m_newTaskDock) m_newTaskDock->hide(); });
                connect(okBtn, &QPushButton::clicked, this, [this, nameEdit, commissionerEdit, summaryEdit, idEdit, timeEdit]() {
                    QString name = nameEdit->text().trimmed();
                    if (name.isEmpty()) { QMessageBox::warning(this, "提示", "请输入任务名称"); return; }
                    // 请求后端创建任务
                    QJsonObject params; params["task_name"] = name;
                    QJsonObject resp = m_zmqClient ? m_zmqClient->request("task.create", params) : QJsonObject();
                    QString taskId = resp.value("data").toObject().value("task_id").toString();
                    if (taskId.isEmpty()) taskId = idEdit->text();
                    TaskInfo info; info.id = taskId; info.name = name; info.createdAt = QDateTime::currentDateTime();
                    info.summary = summaryEdit->text().trimmed(); info.commissioner = commissionerEdit->text().trimmed();
                    m_tasks.prepend(info);
                    if (m_tasksView) m_tasksView->addTaskFront(info);
                    // 已创建任务，仍停留在任务视图，待点击进入
                    if (m_logList) { m_logList->addItem("🆕 " + QDateTime::currentDateTime().toString("hh:mm:ss") + " - 新建任务: " + name + " (" + taskId + ")"); m_logList->scrollToBottom(); }
                    m_newTaskDock->hide();
                });
            }
            m_newTaskDock->show(); m_newTaskDock->raise();
        });
        // 进入任务后才显示左右面板，并进入任务工作区
        connect(m_tasksView, &TasksView::analyzeRequested, this, [this](const QString& taskId) {
            if (m_leftDock) m_leftDock->show();
            if (m_rightDock) m_rightDock->show();
            showAdvancedTabsIfNeeded();
            if (m_ribbonBar) m_ribbonBar->setCurrentTab(1);
            // 关闭任务视图子窗口
            QMdiSubWindow* tvWin = nullptr;
            for (QMdiSubWindow* w : m_mdiArea->subWindowList()) {
                if (w && w->widget() == m_tasksView) { tvWin = w; break; }
            }
            if (tvWin) tvWin->close();
            // 打开任务工作区
            // 创建三个子窗口：数据管理、可视分析、报告生成
            auto ensureSubWindow = [this](const QString& title, QWidget* content) -> QMdiSubWindow* {
                QMdiSubWindow* existing = nullptr;
                for (QMdiSubWindow* w : m_mdiArea->subWindowList()) {
                    if (w && w->windowTitle() == title) { existing = w; break; }
                }
                if (!existing) {
                    existing = m_mdiArea->addSubWindow(content);
                    existing->setWindowTitle(title);
                    existing->showMaximized();
                }
                return existing;
            };
            // 数据管理
            QWidget* data = new QWidget;
            QVBoxLayout* dv = new QVBoxLayout(data);
            QLabel* dtitle = new QLabel(QString("数据管理 - 任务 %1").arg(taskId), data);
            dtitle->setStyleSheet("QLabel { font-weight: bold; font-size: 11pt; color: #0078d4; }");
            QHBoxLayout* dactions = new QHBoxLayout();
            QToolButton* btnImportFile = new QToolButton(data); btnImportFile->setText("导入文件");
            QToolButton* btnCleanData = new QToolButton(data); btnCleanData->setText("数据清洗");
            QToolButton* btnQueryData = new QToolButton(data); btnQueryData->setText("查询数据");
            dactions->addWidget(btnImportFile);
            dactions->addWidget(btnCleanData);
            dactions->addWidget(btnQueryData);
            connect(btnImportFile, &QToolButton::clicked, this, &MainWindow::onImportData);
            connect(btnCleanData, &QToolButton::clicked, this, &MainWindow::onCleanData);
            connect(btnQueryData, &QToolButton::clicked, this, &MainWindow::onQueryData);
            dv->addWidget(dtitle);
            dv->addLayout(dactions);
            data->setLayout(dv);
            QMdiSubWindow* dataWin = ensureSubWindow(QString("数据管理 - 任务 %1").arg(taskId), data);
            // 可视分析
            QWidget* visual = new QWidget;
            QVBoxLayout* vv = new QVBoxLayout(visual);
            QLabel* vtitle = new QLabel(QString("可视分析 - 任务 %1").arg(taskId), visual);
            vv->addWidget(vtitle);
            visual->setLayout(vv);
            QMdiSubWindow* visualWin = ensureSubWindow(QString("可视分析 - 任务 %1").arg(taskId), visual);
            // 报告生成
            QWidget* report = new QWidget;
            QVBoxLayout* rv = new QVBoxLayout(report);
            QLabel* rtitle = new QLabel(QString("报告生成 - 任务 %1").arg(taskId), report);
            rv->addWidget(rtitle);
            report->setLayout(rv);
            QMdiSubWindow* reportWin = ensureSubWindow(QString("报告生成 - 任务 %1").arg(taskId), report);
            // 默认激活数据管理
            m_mdiArea->setActiveSubWindow(dataWin);
            if (m_logList) { m_logList->addItem("🔍 " + QDateTime::currentDateTime().toString("hh:mm:ss") + " - 进入任务 " + taskId); m_logList->scrollToBottom(); }
        });
        // 采集也进入任务工作区
        connect(m_tasksView, &TasksView::collectRequested, this, [this](const QString& taskId) {
            if (m_leftDock) m_leftDock->show();
            if (m_rightDock) m_rightDock->show();
            showAdvancedTabsIfNeeded();
            if (m_ribbonBar) m_ribbonBar->setCurrentTab(1);
            QMdiSubWindow* tvWin = nullptr;
            for (QMdiSubWindow* w : m_mdiArea->subWindowList()) {
                if (w && w->widget() == m_tasksView) { tvWin = w; break; }
            }
            if (tvWin) tvWin->close();
            // 创建三个子窗口：数据管理、可视分析、报告生成
            auto ensureSubWindow = [this](const QString& title, QWidget* content) -> QMdiSubWindow* {
                QMdiSubWindow* existing = nullptr;
                for (QMdiSubWindow* w : m_mdiArea->subWindowList()) {
                    if (w && w->windowTitle() == title) { existing = w; break; }
                }
                if (!existing) {
                    existing = m_mdiArea->addSubWindow(content);
                    existing->setWindowTitle(title);
                    existing->showMaximized();
                }
                return existing;
            };
            // 数据管理
            QWidget* data = new QWidget;
            QVBoxLayout* dv = new QVBoxLayout(data);
            QLabel* dtitle = new QLabel(QString("数据管理 - 任务 %1").arg(taskId), data);
            dtitle->setStyleSheet("QLabel { font-weight: bold; font-size: 11pt; color: #0078d4; }");
            QHBoxLayout* dactions = new QHBoxLayout();
            QToolButton* btnImportFile = new QToolButton(data); btnImportFile->setText("导入文件");
            QToolButton* btnCleanData = new QToolButton(data); btnCleanData->setText("数据清洗");
            QToolButton* btnQueryData = new QToolButton(data); btnQueryData->setText("查询数据");
            dactions->addWidget(btnImportFile);
            dactions->addWidget(btnCleanData);
            dactions->addWidget(btnQueryData);
            connect(btnImportFile, &QToolButton::clicked, this, &MainWindow::onImportData);
            connect(btnCleanData, &QToolButton::clicked, this, &MainWindow::onCleanData);
            connect(btnQueryData, &QToolButton::clicked, this, &MainWindow::onQueryData);
            dv->addWidget(dtitle);
            dv->addLayout(dactions);
            data->setLayout(dv);
            QMdiSubWindow* dataWin = ensureSubWindow(QString("数据管理 - 任务 %1").arg(taskId), data);
            // 可视分析
            QWidget* visual = new QWidget;
            QVBoxLayout* vv = new QVBoxLayout(visual);
            QLabel* vtitle = new QLabel(QString("可视分析 - 任务 %1").arg(taskId), visual);
            vv->addWidget(vtitle);
            visual->setLayout(vv);
            QMdiSubWindow* visualWin = ensureSubWindow(QString("可视分析 - 任务 %1").arg(taskId), visual);
            // 报告生成
            QWidget* report = new QWidget;
            QVBoxLayout* rv = new QVBoxLayout(report);
            QLabel* rtitle = new QLabel(QString("报告生成 - 任务 %1").arg(taskId), report);
            rv->addWidget(rtitle);
            report->setLayout(rv);
            QMdiSubWindow* reportWin = ensureSubWindow(QString("报告生成 - 任务 %1").arg(taskId), report);
            // 默认激活数据管理
            m_mdiArea->setActiveSubWindow(dataWin);
            if (m_logList) { m_logList->addItem("📥 " + QDateTime::currentDateTime().toString("hh:mm:ss") + " - 采集任务 " + taskId); m_logList->scrollToBottom(); }
        });
    }
    // 刷新任务列表（可能为空，仅显示新建卡片）
    m_tasksView->setTasks(m_tasks);
    // 如果已存在子窗口，激活它；否则添加
    QMdiSubWindow* existing = nullptr;
    for (QMdiSubWindow* w : m_mdiArea->subWindowList()) {
        if (w && w->widget() == m_tasksView) { existing = w; break; }
    }
    if (!existing) {
        existing = m_mdiArea->addSubWindow(m_tasksView);
    }
    existing->setWindowTitle("任务管理");
    m_mdiArea->setActiveSubWindow(existing);
    existing->showMaximized();
}


void MainWindow::showAdvancedTabsIfNeeded()
{
    if (!m_ribbonBar) return;
    // 已有完整菜单则仅切换到“数据管理”
    if (m_ribbonBar->tabCount() >= 6) { m_ribbonBar->setCurrentTab(1); return; }

    // 重建完整 Ribbon，确保顺序：分析任务、数据管理、可视分析、报告生成、工具集、关于
    if (menuWidget()) {
        QWidget* old = menuWidget();
        setMenuWidget(nullptr);
        if (old) old->deleteLater();
    }
    if (m_ribbonBar) { m_ribbonBar->deleteLater(); }
    m_ribbonBar = new RibbonBar(this);

    // 1. 分析任务
    RibbonTab* taskTab = m_ribbonBar->addTab("📋 分析任务");
    if (taskTab) {
        RibbonGroup* taskMgmtGroup = taskTab->addGroup("任务管理");
        if (taskMgmtGroup) {
            QToolButton* btnNewTask = taskMgmtGroup->addLargeButton("新建任务", QIcon());
            QToolButton* btnOpenTask = taskMgmtGroup->addLargeButton("打开任务", QIcon());
            QToolButton* btnSaveTask = taskMgmtGroup->addLargeButton("保存任务", QIcon());
            if (btnNewTask) connect(btnNewTask, &QToolButton::clicked, this, [this]() {
                openTaskManagerView();
                if (m_tasksView) emit m_tasksView->newTaskRequested();
            });
            if (btnOpenTask) connect(btnOpenTask, &QToolButton::clicked, this, [this]() { openTaskManagerView(); });
        }
        RibbonGroup* analysisExecGroup = taskTab->addGroup("分析执行");
        if (analysisExecGroup) {
            QToolButton* btnStartAnalysis = analysisExecGroup->addLargeButton("开始分析", QIcon());
            QToolButton* btnStopAnalysis = analysisExecGroup->addLargeButton("停止分析", QIcon());
            QToolButton* btnAnalysisHistory = analysisExecGroup->addLargeButton("历史记录", QIcon());
            if (btnStartAnalysis) connect(btnStartAnalysis, &QToolButton::clicked, this, [this]() {
                showAdvancedTabsIfNeeded();
            });
        }
    }

    // 2. 数据管理
    RibbonTab* dataTab = m_ribbonBar->addTab("💾 数据管理");
    if (dataTab) {
        RibbonGroup* importGroup = dataTab->addGroup("数据导入");
        if (importGroup) {
            QToolButton* btnImportFile = importGroup->addLargeButton("导入文件", QIcon());
            QToolButton* btnImportDB   = importGroup->addLargeButton("导入数据库", QIcon());
            QToolButton* btnImportAPI  = importGroup->addLargeButton("API接口", QIcon());
            if (btnImportFile) connect(btnImportFile, &QToolButton::clicked, this, &MainWindow::onImportData);
        }
        RibbonGroup* processGroup = dataTab->addGroup("数据处理");
        if (processGroup) {
            QToolButton* btnCleanData = processGroup->addLargeButton("数据清洗", QIcon());
            QToolButton* btnTransform = processGroup->addLargeButton("数据转换", QIcon());
            QToolButton* btnValidate  = processGroup->addLargeButton("数据校验", QIcon());
            if (btnCleanData) connect(btnCleanData, &QToolButton::clicked, this, &MainWindow::onCleanData);
        }
        RibbonGroup* queryGroup = dataTab->addGroup("数据查询");
        if (queryGroup) {
            QToolButton* btnQuery  = queryGroup->addLargeButton("查询数据", QIcon());
            QToolButton* btnFilter = queryGroup->addLargeButton("数据筛选", QIcon());
            QToolButton* btnExport = queryGroup->addLargeButton("导出数据", QIcon());
            if (btnQuery) connect(btnQuery, &QToolButton::clicked, this, &MainWindow::onQueryData);
        }
    }

    // 3. 可视分析
    RibbonTab* visualTab = m_ribbonBar->addTab("📊 可视分析");
    if (visualTab) {
        RibbonGroup* penetrationGroup = visualTab->addGroup("资金穿透");
        if (penetrationGroup) {
            penetrationGroup->addLargeButton("穿透分析", QIcon());
            penetrationGroup->addLargeButton("流转路径", QIcon());
            penetrationGroup->addLargeButton("关系图谱", QIcon());
        }
        RibbonGroup* statsGroup = visualTab->addGroup("统计分析");
        if (statsGroup) {
            statsGroup->addLargeButton("统计汇总", QIcon());
            statsGroup->addLargeButton("趋势分析", QIcon());
            statsGroup->addLargeButton("对比分析", QIcon());
        }
        RibbonGroup* chartGroup = visualTab->addGroup("可视化图表");
        if (chartGroup) {
            chartGroup->addLargeButton("网络图", QIcon());
            chartGroup->addLargeButton("柱状图", QIcon());
            chartGroup->addLargeButton("饼图", QIcon());
        }
    }

    // 4. 报告生成
    RibbonTab* reportTab = m_ribbonBar->addTab("📄 报告生成");
    if (reportTab) {
        RibbonGroup* reportCreateGroup = reportTab->addGroup("报告创建");
        if (reportCreateGroup) {
            QToolButton* btnNewReport  = reportCreateGroup->addLargeButton("新建报告", QIcon());
            reportCreateGroup->addLargeButton("选择模板", QIcon());
            reportCreateGroup->addLargeButton("自定义报告", QIcon());
            if (btnNewReport) connect(btnNewReport, &QToolButton::clicked, this, &MainWindow::onGenerateReport);
        }
        RibbonGroup* reportEditGroup = reportTab->addGroup("报告编辑");
        if (reportEditGroup) {
            reportEditGroup->addLargeButton("编辑内容", QIcon());
            reportEditGroup->addLargeButton("插入图表", QIcon());
            reportEditGroup->addLargeButton("预览报告", QIcon());
        }
        RibbonGroup* reportExportGroup = reportTab->addGroup("报告导出");
        if (reportExportGroup) {
            reportExportGroup->addLargeButton("导出PDF", QIcon());
            reportExportGroup->addLargeButton("导出Word", QIcon());
            reportExportGroup->addLargeButton("导出Excel", QIcon());
        }
    }

    // 5. 工具集
    RibbonTab* toolsTab = m_ribbonBar->addTab("🔧 工具集");
    if (toolsTab) {
        RibbonGroup* systemToolsGroup = toolsTab->addGroup("系统工具");
        if (systemToolsGroup) {
            QToolButton* btnSettings = systemToolsGroup->addLargeButton("系统设置", QIcon());
            QToolButton* btnUserMgmt = systemToolsGroup->addLargeButton("用户管理", QIcon());
            QToolButton* btnLogViewer = systemToolsGroup->addLargeButton("日志查看", QIcon());
            if (btnSettings) connect(btnSettings, &QToolButton::clicked, this, &MainWindow::onSettings);
        }
        RibbonGroup* viewGroup = toolsTab->addGroup("视图管理");
        if (viewGroup) {
            QToolButton* btnShowLeftPanel = viewGroup->addLargeButton("任务面板", QIcon());
            QToolButton* btnShowRightPanel = viewGroup->addLargeButton("日志面板", QIcon());
            if (btnShowLeftPanel) {
                connect(btnShowLeftPanel, &QToolButton::clicked, this, [this]() {
                    if (m_leftDock) {
                        m_leftDock->setVisible(!m_leftDock->isVisible());
                        if (m_leftDock->isVisible() && m_logList) {
                            m_logList->addItem("▶ " + QDateTime::currentDateTime().toString("hh:mm:ss") + " - 左侧面板已显示");
                            m_logList->scrollToBottom();
                        }
                    }
                });
            }
            if (btnShowRightPanel) {
                connect(btnShowRightPanel, &QToolButton::clicked, this, [this]() {
                    if (m_rightDock) {
                        m_rightDock->setVisible(!m_rightDock->isVisible());
                        if (m_rightDock->isVisible() && m_logList) {
                            m_logList->addItem("▶ " + QDateTime::currentDateTime().toString("hh:mm:ss") + " - 右侧面板已显示");
                            m_logList->scrollToBottom();
                        }
                    }
                });
            }
        }
        RibbonGroup* dataToolsGroup = toolsTab->addGroup("数据工具");
        if (dataToolsGroup) {
            dataToolsGroup->addLargeButton("数据备份", QIcon());
            dataToolsGroup->addLargeButton("数据恢复", QIcon());
            dataToolsGroup->addLargeButton("数据清理", QIcon());
        }
        RibbonGroup* accountGroup = toolsTab->addGroup("账户");
        if (accountGroup) {
            QToolButton* btnProfile = accountGroup->addLargeButton("个人资料", QIcon());
            QToolButton* btnChangePassword = accountGroup->addLargeButton("修改密码", QIcon());
            QToolButton* btnLogout = accountGroup->addLargeButton("退出登录", QIcon());
            if (btnLogout) connect(btnLogout, &QToolButton::clicked, this, &MainWindow::onLogout);
        }
    }

    // 6. 关于
    RibbonTab* aboutTab = m_ribbonBar->addTab("ℹ️ 关于");
    if (aboutTab) {
        RibbonGroup* helpGroup = aboutTab->addGroup("帮助");
        if (helpGroup) {
            helpGroup->addLargeButton("用户手册", QIcon());
            helpGroup->addLargeButton("在线帮助", QIcon());
            helpGroup->addLargeButton("问题反馈", QIcon());
        }
        RibbonGroup* aboutSystemGroup = aboutTab->addGroup("系统信息");
        if (aboutSystemGroup) {
            QToolButton* btnAbout = aboutSystemGroup->addLargeButton("关于系统", QIcon());
            aboutSystemGroup->addLargeButton("版本信息", QIcon());
            aboutSystemGroup->addLargeButton("许可证", QIcon());
            if (btnAbout) connect(btnAbout, &QToolButton::clicked, this, &MainWindow::onAbout);
        }
    }

    // 将Ribbon添加到主窗口
    QWidget* ribbonContainer = new QWidget(this);
    QVBoxLayout* layout = new QVBoxLayout(ribbonContainer);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addWidget(m_ribbonBar);
    setMenuWidget(ribbonContainer);

    // 默认切换到“数据管理”标签
    m_ribbonBar->setCurrentTab(1);
}


// ==================== Ribbon按钮槽函数 ====================

void MainWindow::onNewTask()
{
    Logger::instance()->info("Creating new task...");
    updateStatusBar("创建新任务...");
    
    // TODO: 显示新建任务对话框
    QMessageBox::information(this, "提示", "新建任务功能开发中...");
}


void MainWindow::onOpenTask()
{
    Logger::instance()->info("Opening task...");
    updateStatusBar("打开任务...");
    
    // TODO: 显示任务列表对话框
    QMessageBox::information(this, "提示", "打开任务功能开发中...");
}


void MainWindow::onImportData()
{
    Logger::instance()->info("Importing data...");
    updateStatusBar("导入数据...");
    
    // TODO: 显示数据导入对话框
    QMessageBox::information(this, "提示", "数据导入功能开发中...");
}


void MainWindow::onCleanData()
{
    Logger::instance()->info("Cleaning data...");
    updateStatusBar("清洗数据...");
    
    // TODO: 显示数据清洗对话框
    QMessageBox::information(this, "提示", "数据清洗功能开发中...");
}


void MainWindow::onQueryData()
{
    Logger::instance()->info("Querying data...");
    updateStatusBar("查询数据...");
    
    // TODO: 显示数据查询窗口
    QMessageBox::information(this, "提示", "数据查询功能开发中...");
}


void MainWindow::onAnalyzeData()
{
    Logger::instance()->info("Analyzing data...");
    updateStatusBar("分析数据...");
    
    // 测试ZeroMQ通信
    QJsonObject params;
    params["test"] = "hello";
    
    QJsonObject response = m_zmqClient->request("test.ping", params);
    
    QString message = QString("服务器响应: %1").arg(response["status"].toString());
    QMessageBox::information(this, "测试", message);
}


void MainWindow::onGenerateReport()
{
    Logger::instance()->info("Generating report...");
    updateStatusBar("生成报告...");
    
    // TODO: 显示报告生成对话框
    QMessageBox::information(this, "提示", "报告生成功能开发中...");
}


void MainWindow::onSettings()
{
    Logger::instance()->info("Opening settings...");
    
    // TODO: 显示设置对话框
    QMessageBox::information(this, "提示", "系统设置功能开发中...");
}


void MainWindow::onAbout()
{
    QString aboutText = R"(
<h2>资金分析系统 v1.0.0</h2>
<p>基于Qt 6 + ZeroMQ + Python + DuckDB开发</p>
<p>Copyright © 2024 FundAnalysis Team</p>
<p><b>技术栈:</b></p>
<ul>
<li>前端: Qt 6.0+ C++</li>
<li>后端: Python 3.9+</li>
<li>通信: ZeroMQ</li>
<li>数据库: DuckDB</li>
</ul>
    )";
    
    QMessageBox::about(this, "关于", aboutText);
}

// ==================== ZeroMQ槽函数 ====================

void MainWindow::onZmqConnected()
{
    Logger::instance()->info("Connected to backend");
    m_connectionLabel->setText("已连接");
    m_connectionLabel->setStyleSheet("QLabel { color: green; }");
    updateStatusBar("已连接到后端服务");
    
    // 添加日志
    if (m_logList) {
        m_logList->addItem("✅ " + QDateTime::currentDateTime().toString("hh:mm:ss") + " - 后端连接成功");
        m_logList->scrollToBottom();
    }
}

void MainWindow::onZmqDisconnected()
{
    Logger::instance()->warning("Disconnected from backend");
    m_connectionLabel->setText("未连接");
    m_connectionLabel->setStyleSheet("QLabel { color: red; }");
    updateStatusBar("与后端服务断开连接");
    
    // 添加日志
    if (m_logList) {
        m_logList->addItem("❌ " + QDateTime::currentDateTime().toString("hh:mm:ss") + " - 后端连接断开");
        m_logList->scrollToBottom();
    }
}

void MainWindow::onZmqError(const QString& error)
{
    Logger::instance()->error("ZeroMQ error: " + error);
    QMessageBox::critical(this, "连接错误", "无法连接到后端服务:\n" + error);
}

void MainWindow::onNotificationReceived(const QString& type, const QJsonObject& data)
{
    Logger::instance()->debug("Received notification: " + type);
    
    if (type == "progress") {
        int current = data["current"].toInt();
        int total = data["total"].toInt();
        QString message = data["message"].toString();
        
        updateStatusBar(QString("%1 (%2/%3)").arg(message).arg(current).arg(total));
        
        // 添加日志
        if (m_logList) {
            m_logList->addItem(QString("📈 %1 - %2 [%3/%4]")
                .arg(QDateTime::currentDateTime().toString("hh:mm:ss"))
                .arg(message)
                .arg(current)
                .arg(total));
            m_logList->scrollToBottom();
        }
    }
}
