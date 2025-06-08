#ifndef REGISTERWINDOW_H
#define REGISTERWINDOW_H

#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QCheckBox>
#include <QFileDialog>
#include <QPropertyAnimation>
#include <QTimer>

class RegisterWindow : public QDialog
{
    Q_OBJECT

public:
    explicit RegisterWindow(QWidget *parent = nullptr);
    QString getRegisteredUsername() const { return m_nicknameEdit->text(); }

private slots:
    void onRegisterClicked();
    void onRegisterSuccess();
    void onRegisterFailed(const QString& error);
    void onSendCodeClicked();
    void onBackToLoginClicked();

private:
    void setupUI();
    bool validateInput();
    void playBackAnimation();

    QLineEdit* m_nicknameEdit;
    QLineEdit* m_passwordEdit;
    QLineEdit* m_confirmPasswordEdit;
    QLineEdit* m_codeEdit;
    QCheckBox* m_agreementCheck;
    QPushButton* m_registerButton;
    QPushButton* m_sendCodeButton;
    QPushButton* m_backButton;
    QLabel* m_statusLabel;
    QLabel* m_titleLabel;
    QLabel* m_codeLabel;
    QString m_sentCode;
    QTimer* m_codeTimer;
    int m_codeCountdown = 0;
    
    // 移动窗口相关
    QPoint m_dragPosition;
    bool m_dragging = false;

    QLabel* m_avatarLabel;
    QString m_avatarPath;
    QPushButton* m_chooseAvatarBtn;

    QLabel* m_captchaLabel;
    QString m_captchaText;
    void generateCaptcha();
    bool eventFilter(QObject* obj, QEvent* event) override;

protected:
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
};

#endif // REGISTERWINDOW_H