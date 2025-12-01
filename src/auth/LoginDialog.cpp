#include "auth/LoginDialog.h"
#include "core/Application.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QSettings>
#include <QCryptographicHash>
#include <QMessageBox>

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
    setWindowTitle("用户登录 - 资金分析系统");
    setFixedSize(400, 280);
    setModal(true);
    
    // 主布局
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(15);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    
    // 标题
    QLabel* titleLabel = new QLabel("资金分析系统", this);
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet(R"(
        QLabel {
            font-size: 18pt;
            font-weight: bold;
            color: #0078d4;
            padding: 10px;
        }
    )");
    mainLayout->addWidget(titleLabel);
    
    // 登录表单组
    QGroupBox* formGroup = new QGroupBox("请输入登录信息", this);
    formGroup->setStyleSheet(R"(
        QGroupBox {
            font-size: 10pt;
            font-weight: bold;
            border: 2px solid #d0d0d0;
            border-radius: 5px;
            margin-top: 10px;
            padding-top: 10px;
        }
        QGroupBox::title {
            subcontrol-origin: margin;
            left: 10px;
            padding: 0 5px;
        }
    )");
    
    QFormLayout* formLayout = new QFormLayout(formGroup);
    formLayout->setSpacing(10);
    formLayout->setContentsMargins(15, 20, 15, 15);
    
    // 用户名输入
    m_usernameEdit = new QLineEdit(this);
    m_usernameEdit->setPlaceholderText("请输入用户名");
    m_usernameEdit->setMinimumHeight(30);
    m_usernameEdit->setStyleSheet(R"(
        QLineEdit {
            border: 1px solid #d0d0d0;
            border-radius: 3px;
            padding: 5px 10px;
            font-size: 10pt;
        }
        QLineEdit:focus {
            border: 2px solid #0078d4;
        }
    )");
    connect(m_usernameEdit, &QLineEdit::textChanged, this, &LoginDialog::onUsernameChanged);
    
    // 密码输入
    m_passwordEdit = new QLineEdit(this);
    m_passwordEdit->setPlaceholderText("请输入密码");
    m_passwordEdit->setEchoMode(QLineEdit::Password);
    m_passwordEdit->setMinimumHeight(30);
    m_passwordEdit->setStyleSheet(m_usernameEdit->styleSheet());
    connect(m_passwordEdit, &QLineEdit::textChanged, this, &LoginDialog::onPasswordChanged);
    connect(m_passwordEdit, &QLineEdit::returnPressed, this, &LoginDialog::onLoginClicked);
    
    formLayout->addRow("用户名:", m_usernameEdit);
    formLayout->addRow("密  码:", m_passwordEdit);
    
    // 记住密码选项
    m_rememberCheckBox = new QCheckBox("记住密码", this);
    formLayout->addRow("", m_rememberCheckBox);
    
    mainLayout->addWidget(formGroup);
    
    // 错误提示标签
    m_errorLabel = new QLabel(this);
    m_errorLabel->setAlignment(Qt::AlignCenter);
    m_errorLabel->setStyleSheet("QLabel { color: red; font-size: 9pt; }");
    m_errorLabel->setVisible(false);
    mainLayout->addWidget(m_errorLabel);
    
    // 按钮布局
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->setSpacing(10);
    
    m_loginButton = new QPushButton("登录", this);
    m_loginButton->setMinimumHeight(35);
    m_loginButton->setEnabled(false);
    m_loginButton->setStyleSheet(R"(
        QPushButton {
            background: #0078d4;
            color: white;
            border: none;
            border-radius: 3px;
            font-size: 10pt;
            font-weight: bold;
            padding: 5px 30px;
        }
        QPushButton:hover {
            background: #0063b1;
        }
        QPushButton:pressed {
            background: #005a9e;
        }
        QPushButton:disabled {
            background: #cccccc;
            color: #888888;
        }
    )");
    connect(m_loginButton, &QPushButton::clicked, this, &LoginDialog::onLoginClicked);
    
    m_cancelButton = new QPushButton("取消", this);
    m_cancelButton->setMinimumHeight(35);
    m_cancelButton->setStyleSheet(R"(
        QPushButton {
            background: white;
            color: #333333;
            border: 1px solid #d0d0d0;
            border-radius: 3px;
            font-size: 10pt;
            padding: 5px 30px;
        }
        QPushButton:hover {
            background: #f0f0f0;
        }
        QPushButton:pressed {
            background: #e0e0e0;
        }
    )");
    connect(m_cancelButton, &QPushButton::clicked, this, &LoginDialog::onCancelClicked);
    
    buttonLayout->addStretch();
    buttonLayout->addWidget(m_loginButton);
    buttonLayout->addWidget(m_cancelButton);
    buttonLayout->addStretch();
    
    mainLayout->addLayout(buttonLayout);
    
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
