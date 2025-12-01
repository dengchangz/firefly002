#include "auth/LoginDialog.h"
#include "core/Application.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QSettings>
#include <QCryptographicHash>
#include <QMessageBox>
#include <QGraphicsDropShadowEffect>
#include <QFrame>

LoginDialog::LoginDialog(QWidget *parent)
    : QDialog(parent)
    , m_usernameEdit(nullptr)
    , m_passwordEdit(nullptr)
    , m_loginButton(nullptr)
    , m_cancelButton(nullptr)
    , m_rememberCheckBox(nullptr)
    , m_errorLabel(nullptr)
{
    setupUi();
    loadSettings();
}

void LoginDialog::setupUi()
{
    // 去除标题栏和边框
    setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setFixedSize(450, 520);
    setModal(true);
    
    // 设置对话框背景渐变（科技蓝）
    setStyleSheet(R"(
        QDialog {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
                                        stop:0 #1e3c72, stop:1 #2a5298);
        }
    )");
    
    // 主布局
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(30, 30, 30, 30);
    
    // 创建白色卡片容器
    QWidget* cardWidget = new QWidget(this);
    cardWidget->setStyleSheet(R"(
        QWidget {
            background: white;
            border-radius: 15px;
        }
    )");
    cardWidget->setGraphicsEffect(createShadowEffect());
    
    QVBoxLayout* cardLayout = new QVBoxLayout(cardWidget);
    cardLayout->setSpacing(20);
    cardLayout->setContentsMargins(40, 40, 40, 40);
    
    // Logo/图标区域
    QLabel* logoLabel = new QLabel("💰", this);
    logoLabel->setAlignment(Qt::AlignCenter);
    logoLabel->setStyleSheet(R"(
        QLabel {
            font-size: 48pt;
            padding: 10px;
        }
    )");
    cardLayout->addWidget(logoLabel);
    
    // 标题
    QLabel* titleLabel = new QLabel("资金分析系统", this);
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet(R"(
        QLabel {
            font-size: 22pt;
            font-weight: bold;
            color: #2d3748;
            padding: 5px;
        }
    )");
    cardLayout->addWidget(titleLabel);
    
    // 副标题
    QLabel* subtitleLabel = new QLabel("欢迎使用，请登录您的账户", this);
    subtitleLabel->setAlignment(Qt::AlignCenter);
    subtitleLabel->setStyleSheet(R"(
        QLabel {
            font-size: 10pt;
            color: #718096;
            padding-bottom: 10px;
        }
    )");
    cardLayout->addWidget(subtitleLabel);
    
    // 表单布局
    QVBoxLayout* formLayout = new QVBoxLayout();
    formLayout->setSpacing(80);  // 设置为15px间距
    formLayout->setContentsMargins(0, 0, 0, 0);
    
    // 用户名输入框（独立圆角）
    m_usernameEdit = new QLineEdit(this);
    m_usernameEdit->setPlaceholderText("请输入用户名");
    m_usernameEdit->setMinimumHeight(48);
    m_usernameEdit->setStyleSheet(R"(
        QLineEdit {
            border: 2px solid #e2e8f0;
            border-radius: 8px;
            padding: 12px 15px;
            font-size: 11pt;
            background: #f7fafc;
            color: #2d3748;
            margin-bottom: 0px;
        }
        QLineEdit:focus {
            border: 2px solid #2a5298;
            background: white;
        }
        QLineEdit:hover {
            border: 2px solid #cbd5e0;
        }
    )");
    connect(m_usernameEdit, &QLineEdit::textChanged, this, &LoginDialog::onUsernameChanged);
    formLayout->addWidget(m_usernameEdit);
    
    // 密码输入框（独立圆角）
    m_passwordEdit = new QLineEdit(this);
    m_passwordEdit->setPlaceholderText("请输入密码");
    m_passwordEdit->setEchoMode(QLineEdit::Password);
    m_passwordEdit->setMinimumHeight(48);
    m_passwordEdit->setStyleSheet(R"(
        QLineEdit {
            border: 2px solid #e2e8f0;
            border-radius: 8px;
            padding: 12px 15px;
            font-size: 11pt;
            background: #f7fafc;
            color: #2d3748;
            margin-top: 0px;
        }
        QLineEdit:focus {
            border: 2px solid #2a5298;
            background: white;
        }
        QLineEdit:hover {
            border: 2px solid #cbd5e0;
        }
    )");
    connect(m_passwordEdit, &QLineEdit::textChanged, this, &LoginDialog::onPasswordChanged);
    connect(m_passwordEdit, &QLineEdit::returnPressed, this, &LoginDialog::onLoginClicked);
    formLayout->addWidget(m_passwordEdit);
    
    // 记住密码选项
    m_rememberCheckBox = new QCheckBox("记住密码", this);
    m_rememberCheckBox->setStyleSheet(R"(
        QCheckBox {
            font-size: 10pt;
            color: #4a5568;
            spacing: 8px;
        }
        QCheckBox::indicator {
            width: 18px;
            height: 18px;
            border-radius: 4px;
            border: 2px solid #cbd5e0;
        }
        QCheckBox::indicator:checked {
            background: #2a5298;
            border: 2px solid #2a5298;
            image: url(data:image/svg+xml;base64,PHN2ZyB3aWR0aD0iMTIiIGhlaWdodD0iOSIgdmlld0JveD0iMCAwIDEyIDkiIGZpbGw9Im5vbmUiIHhtbG5zPSJodHRwOi8vd3d3LnczLm9yZy8yMDAwL3N2ZyI+PHBhdGggZD0iTTEgNEw0LjUgNy41TDExIDEiIHN0cm9rZT0id2hpdGUiIHN0cm9rZS13aWR0aD0iMiIgc3Ryb2tlLWxpbmVjYXA9InJvdW5kIi8+PC9zdmc+);
        }
        QCheckBox::indicator:hover {
            border: 2px solid #2a5298;
        }
    )");
    formLayout->addWidget(m_rememberCheckBox);
    
    cardLayout->addLayout(formLayout);
    
    // 错误提示标签
    m_errorLabel = new QLabel(this);
    m_errorLabel->setAlignment(Qt::AlignCenter);
    m_errorLabel->setStyleSheet(R"(
        QLabel {
            color: #e53e3e;
            font-size: 10pt;
            background: #fff5f5;
            border: 1px solid #feb2b2;
            border-radius: 6px;
            padding: 8px 12px;
        }
    )");
    m_errorLabel->setVisible(false);
    cardLayout->addWidget(m_errorLabel);
    
    // 登录按钮
    m_loginButton = new QPushButton("登 录", this);
    m_loginButton->setMinimumHeight(48);
    m_loginButton->setEnabled(false);
    m_loginButton->setCursor(Qt::PointingHandCursor);
    m_loginButton->setStyleSheet(R"(
        QPushButton {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                                        stop:0 #1e3c72, stop:1 #2a5298);
            color: white;
            border: none;
            border-radius: 8px;
            font-size: 12pt;
            font-weight: bold;
            letter-spacing: 2px;
        }
        QPushButton:hover {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                                        stop:0 #163056, stop:1 #1f4278);
        }
        QPushButton:pressed {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                                        stop:0 #0f2440, stop:1 #163056);
        }
        QPushButton:disabled {
            background: #e2e8f0;
            color: #a0aec0;
        }
    )");
    connect(m_loginButton, &QPushButton::clicked, this, &LoginDialog::onLoginClicked);
    cardLayout->addWidget(m_loginButton);
    
    // 取消按钮
    m_cancelButton = new QPushButton("取 消", this);
    m_cancelButton->setMinimumHeight(48);
    m_cancelButton->setCursor(Qt::PointingHandCursor);
    m_cancelButton->setStyleSheet(R"(
        QPushButton {
            background: transparent;
            color: #4a5568;
            border: 2px solid #e2e8f0;
            border-radius: 8px;
            font-size: 11pt;
            font-weight: 500;
            letter-spacing: 2px;
        }
        QPushButton:hover {
            background: #f7fafc;
            border: 2px solid #cbd5e0;
            color: #2d3748;
        }
        QPushButton:pressed {
            background: #edf2f7;
        }
    )");
    connect(m_cancelButton, &QPushButton::clicked, this, &LoginDialog::onCancelClicked);
    cardLayout->addWidget(m_cancelButton);
    
    mainLayout->addWidget(cardWidget);
    setLayout(mainLayout);
}

void LoginDialog::loadSettings()
{
    Application* app = Application::instance();
    
    QString savedUsername = app->getConfigValue("auth/username", "");
    bool rememberPwd = app->getConfigValue("auth/remember_password", "false") == "true";
    
    if (!savedUsername.isEmpty()) {
        m_usernameEdit->setText(savedUsername);
        m_rememberCheckBox->setChecked(rememberPwd);
    }
    
    // 默认测试账号提示
    if (savedUsername.isEmpty()) {
        m_usernameEdit->setPlaceholderText("默认用户名: admin");
        m_passwordEdit->setPlaceholderText("默认密码: admin123");
    }
}

void LoginDialog::saveSettings()
{
    Application* app = Application::instance();
    
    if (m_rememberCheckBox->isChecked()) {
        app->setConfigValue("auth/username", m_username);
        app->setConfigValue("auth/remember_password", "true");
    } else {
        app->setConfigValue("auth/username", "");
        app->setConfigValue("auth/remember_password", "false");
    }
}

bool LoginDialog::validateInput()
{
    m_errorLabel->setVisible(false);
    
    QString username = m_usernameEdit->text().trimmed();
    QString password = m_passwordEdit->text();
    
    if (username.isEmpty()) {
        m_errorLabel->setText("❌ 用户名不能为空");
        m_errorLabel->setVisible(true);
        m_usernameEdit->setFocus();
        return false;
    }
    
    if (password.isEmpty()) {
        m_errorLabel->setText("❌ 密码不能为空");
        m_errorLabel->setVisible(true);
        m_passwordEdit->setFocus();
        return false;
    }
    
    if (username.length() < 3) {
        m_errorLabel->setText("❌ 用户名长度至少3个字符");
        m_errorLabel->setVisible(true);
        m_usernameEdit->setFocus();
        return false;
    }
    
    if (password.length() < 6) {
        m_errorLabel->setText("❌ 密码长度至少6个字符");
        m_errorLabel->setVisible(true);
        m_passwordEdit->setFocus();
        return false;
    }
    
    return true;
}

QGraphicsDropShadowEffect* LoginDialog::createShadowEffect()
{
    QGraphicsDropShadowEffect* shadow = new QGraphicsDropShadowEffect(this);
    shadow->setBlurRadius(30);
    shadow->setColor(QColor(0, 0, 0, 60));
    shadow->setOffset(0, 10);
    return shadow;
}

void LoginDialog::onLoginClicked()
{
    if (!validateInput()) {
        return;
    }
    
    m_username = m_usernameEdit->text().trimmed();
    m_password = m_passwordEdit->text();
    
    // 保存设置
    saveSettings();
    
    // 接受对话框
    accept();
}

void LoginDialog::onCancelClicked()
{
    reject();
}

void LoginDialog::onUsernameChanged(const QString& text)
{
    // 当用户名和密码都不为空时启用登录按钮
    m_loginButton->setEnabled(!text.trimmed().isEmpty() && !m_passwordEdit->text().isEmpty());
    m_errorLabel->setVisible(false);
}

void LoginDialog::onPasswordChanged(const QString& text)
{
    // 当用户名和密码都不为空时启用登录按钮
    m_loginButton->setEnabled(!m_usernameEdit->text().trimmed().isEmpty() && !text.isEmpty());
    m_errorLabel->setVisible(false);
}

QString LoginDialog::getUsername() const
{
    return m_username;
}

QString LoginDialog::getPassword() const
{
    return m_password;
}

bool LoginDialog::rememberPassword() const
{
    return m_rememberCheckBox->isChecked();
}
