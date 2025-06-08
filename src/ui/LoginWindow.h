#ifndef LOGINWINDOW_H
#define LOGINWINDOW_H

#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QMouseEvent>
#include <QPixmap>
#include <QFile>
#include <QPropertyAnimation>
#include <QTimer>
#include <QPainter>
#include <QRandomGenerator>

class LoginWindow : public QDialog
{
    Q_OBJECT
    
public:
    explicit LoginWindow(QWidget *parent = nullptr);

private slots:
    void onLoginClicked();
    void onLoginSuccess();
    void onLoginFailed(const QString& error);
    void onRegisterClicked();
    void onForgotPasswordClicked();
    void onMinimizeClicked();   // 新增
    void onCloseClicked();      // 新增
    void onSendCodeClicked();
    void onUsernameChanged(const QString& username);

private:
    void setupUI();
    void playShowAnimation();
    void playToMainAnimation();
    
    QLineEdit* m_usernameEdit;
    QLineEdit* m_passwordEdit;
    QLineEdit* m_codeEdit;
    QPushButton* m_loginButton;
    QPushButton* m_sendCodeButton;
    QLabel* m_statusLabel;
    QPushButton* m_minButton;   // 新增
    QPushButton* m_closeButton; // 新增
    QString m_sentCode;
    QTimer* m_codeTimer;
    int m_codeCountdown = 0;
    QPoint m_dragPos;
    QLabel* m_avatarLabel;
    QLabel* m_captchaLabel;
    QString m_captchaText;
    void generateCaptcha();
protected:
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    bool eventFilter(QObject* obj, QEvent* event) override;
};

#endif // LOGINWINDOW_H