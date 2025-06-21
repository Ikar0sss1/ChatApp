#ifndef FORGOTPASSWORDWINDOW_H
#define FORGOTPASSWORDWINDOW_H

#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QCheckBox>
#include <QTimer>
#include <QPoint>
#include <QMouseEvent>

class ForgotPasswordWindow : public QDialog
{
    Q_OBJECT

public:
    explicit ForgotPasswordWindow(QWidget *parent = nullptr);

private slots:
    void onResetClicked();
    void onBackToLoginClicked();
    void onResetSuccess();
    void onResetFailed(const QString& error);
    void generateCaptcha();
    void onVerifyClicked();

private:
    void setupUI();
    bool validateInput();
    void playBackAnimation();

    // UI组件
    QLineEdit* m_usernameEdit;
    QLineEdit* m_verificationCodeEdit;
    QLineEdit* m_newPasswordEdit;
    QLineEdit* m_confirmPasswordEdit;
    QPushButton* m_resetButton;
    QPushButton* m_verifyButton;
    QPushButton* m_backButton;
    QLabel* m_statusLabel;
    QLabel* m_captchaLabel;
    QString m_captchaText;
    
    // 拖动相关
    bool m_dragging = false;
    QPoint m_dragPosition;

protected:
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    bool eventFilter(QObject* obj, QEvent* event) override;
};

#endif // FORGOTPASSWORDWINDOW_H 