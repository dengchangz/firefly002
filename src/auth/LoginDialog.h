#ifndef LOGINDIALOG_H
#define LOGINDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QCheckBox>
#include <QLabel>
#include <QGraphicsDropShadowEffect>

class LoginDialog : public QDialog
{
    Q_OBJECT

public:
    explicit LoginDialog(QWidget *parent = nullptr);
    
    QString getUsername() const;
    QString getPassword() const;
    bool rememberPassword() const;

private slots:
    void onLoginClicked();
    void onCancelClicked();
    void onUsernameChanged(const QString& text);
    void onPasswordChanged(const QString& text);

private:
    void setupUi();
    void loadSettings();
    void saveSettings();
    bool validateInput();
    QGraphicsDropShadowEffect* createShadowEffect();

private:
    QLineEdit* m_usernameEdit;
    QLineEdit* m_passwordEdit;
    QPushButton* m_loginButton;
    QPushButton* m_cancelButton;
    QCheckBox* m_rememberCheckBox;
    QLabel* m_errorLabel;
    
    QString m_username;
    QString m_password;
};

#endif // LOGINDIALOG_H
