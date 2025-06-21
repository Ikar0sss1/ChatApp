#include "ForgotPasswordWindow.h"
#include "core/UserManager.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QPropertyAnimation>
#include <QRandomGenerator>
#include <QPainter>
#include <QEvent>

ForgotPasswordWindow::ForgotPasswordWindow(QWidget *parent)
    : QDialog(parent)
{
    setWindowFlags(Qt::FramelessWindowHint | Qt::Window);
    setAttribute(Qt::WA_TranslucentBackground);
    setupUI();
    
    connect(&UserManager::getInstance(), &UserManager::resetPasswordSuccess,
            this, &ForgotPasswordWindow::onResetSuccess);
    connect(&UserManager::getInstance(), &UserManager::resetPasswordFailed,
            this, &ForgotPasswordWindow::onResetFailed);
}

void ForgotPasswordWindow::setupUI()
{
    setFixedSize(360, 500);

    QVBoxLayout* outerLayout = new QVBoxLayout(this);
    outerLayout->setContentsMargins(0, 0, 0, 0);

    QWidget* bgWidget = new QWidget(this);
    bgWidget->setObjectName("bgWidget");
    bgWidget->setStyleSheet(
        "#bgWidget {"
        "background: white;"
        "border-radius: 8px;"
        "}"
    );
    
    QVBoxLayout* layout = new QVBoxLayout(bgWidget);
    layout->setSpacing(15);
    layout->setContentsMargins(30, 20, 30, 30);

    // 返回按钮
    m_backButton = new QPushButton("返回");
    m_backButton->setFixedSize(60, 32);
    m_backButton->setStyleSheet(
        "QPushButton { "
        "   background: #f0f0f0; "
        "   border: 1px solid #e0e0e0; "
        "   border-radius: 4px; "
        "   color: #2196F3; "
        "   font-size: 14px; "
        "} "
        "QPushButton:hover { background: #e0e0e0; }"
    );
    connect(m_backButton, &QPushButton::clicked, this, &ForgotPasswordWindow::onBackToLoginClicked);
    layout->addWidget(m_backButton, 0, Qt::AlignLeft);

    // 标题
    QLabel* titleLabel = new QLabel("找回密码");
    titleLabel->setObjectName("titleLabel");
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet("QLabel#titleLabel { font-size: 22px; color: #333333; margin-top: 10px; }");
    layout->addWidget(titleLabel);
    layout->addSpacing(20);

    // 用户名输入框
    m_usernameEdit = new QLineEdit();
    m_usernameEdit->setPlaceholderText("请输入用户名");
    m_usernameEdit->setMinimumHeight(40);
    m_usernameEdit->setStyleSheet(
        "QLineEdit { "
        "   border: none; "
        "   background-color: #f5f5f5; "
        "   border-radius: 4px; "
        "   padding: 0 10px; "
        "   font-size: 14px; "
        "}"
    );
    layout->addWidget(m_usernameEdit);

    // 验证码图片和输入框
    QHBoxLayout* captchaLayout = new QHBoxLayout();
    m_verificationCodeEdit = new QLineEdit();
    m_verificationCodeEdit->setPlaceholderText("输入验证码");
    m_verificationCodeEdit->setMinimumHeight(40);
    m_verificationCodeEdit->setMaximumWidth(100);
    m_verificationCodeEdit->setStyleSheet(m_usernameEdit->styleSheet());
    captchaLayout->addWidget(m_verificationCodeEdit);

    m_captchaLabel = new QLabel();
    m_captchaLabel->setFixedSize(90, 40);
    m_captchaLabel->setStyleSheet("border-radius: 4px; background: #f0f0f0; border: 1px solid #e0e0e0;");
    m_captchaLabel->setCursor(Qt::PointingHandCursor);
    m_captchaLabel->setToolTip("点击刷新验证码");
    m_captchaLabel->installEventFilter(this);
    captchaLayout->addWidget(m_captchaLabel);

    m_verifyButton = new QPushButton("验证");
    m_verifyButton->setFixedSize(70, 40);
    m_verifyButton->setStyleSheet(
        "QPushButton { "
        "   background-color: #2196F3; "
        "   border: none; "
        "   border-radius: 4px; "
        "   color: white; "
        "   font-size: 14px; "
        "} "
        "QPushButton:hover { background-color: #1E88E5; } "
        "QPushButton:pressed { background-color: #1976D2; } "
    );
    connect(m_verifyButton, &QPushButton::clicked, this, &ForgotPasswordWindow::onVerifyClicked);
    captchaLayout->addWidget(m_verifyButton);

    layout->addLayout(captchaLayout);

    // 生成初始验证码
    generateCaptcha();

    // 新密码输入框
    m_newPasswordEdit = new QLineEdit();
    m_newPasswordEdit->setPlaceholderText("请输入新密码");
    m_newPasswordEdit->setEchoMode(QLineEdit::Password);
    m_newPasswordEdit->setMinimumHeight(40);
    m_newPasswordEdit->setStyleSheet(m_usernameEdit->styleSheet());
    m_newPasswordEdit->setEnabled(false);  // 初始禁用，验证通过后启用
    layout->addWidget(m_newPasswordEdit);

    // 确认密码输入框
    m_confirmPasswordEdit = new QLineEdit();
    m_confirmPasswordEdit->setPlaceholderText("请确认新密码");
    m_confirmPasswordEdit->setEchoMode(QLineEdit::Password);
    m_confirmPasswordEdit->setMinimumHeight(40);
    m_confirmPasswordEdit->setStyleSheet(m_usernameEdit->styleSheet());
    m_confirmPasswordEdit->setEnabled(false);  // 初始禁用，验证通过后启用
    layout->addWidget(m_confirmPasswordEdit);

    // 重置密码按钮
    m_resetButton = new QPushButton("重置密码");
    m_resetButton->setMinimumHeight(40);
    m_resetButton->setCursor(Qt::PointingHandCursor);
    m_resetButton->setStyleSheet(
        "QPushButton { "
        "   background-color: #2196F3; "
        "   border: none; "
        "   border-radius: 4px; "
        "   color: white; "
        "   font-size: 15px; "
        "} "
        "QPushButton:hover { background-color: #1E88E5; } "
        "QPushButton:pressed { background-color: #1976D2; } "
        "QPushButton:disabled { background-color: #BBDEFB; }"
    );
    m_resetButton->setEnabled(false);  // 初始禁用，验证通过后启用
    connect(m_resetButton, &QPushButton::clicked, this, &ForgotPasswordWindow::onResetClicked);
    layout->addWidget(m_resetButton);

    // 状态标签
    m_statusLabel = new QLabel();
    m_statusLabel->setAlignment(Qt::AlignCenter);
    m_statusLabel->setStyleSheet("color: #f44336; font-size: 13px;");
    layout->addWidget(m_statusLabel);

    layout->addStretch();

    outerLayout->addWidget(bgWidget);
}

void ForgotPasswordWindow::generateCaptcha()
{
    // 生成4位随机字母数字
    const QString chars = "ABCDEFGHJKLMNPQRSTUVWXYZ23456789";
    m_captchaText.clear();
    for (int i = 0; i < 4; ++i)
        m_captchaText += chars.at(QRandomGenerator::global()->bounded(chars.size()));

    QPixmap pix(90, 40);
    pix.fill(Qt::white);
    QPainter painter(&pix);
    QFont font = painter.font();
    font.setPointSize(18);
    font.setBold(true);
    painter.setFont(font);

    // 随机颜色和位置
    for (int i = 0; i < 4; ++i) {
        QColor color(QRandomGenerator::global()->bounded(50, 200),
                     QRandomGenerator::global()->bounded(50, 200),
                     QRandomGenerator::global()->bounded(50, 200));
        painter.setPen(color);
        painter.drawText(10 + i*20, QRandomGenerator::global()->bounded(22, 36), m_captchaText.mid(i,1));
    }
    // 干扰线
    for (int i = 0; i < 6; ++i) {
        painter.setPen(QColor(QRandomGenerator::global()->bounded(100, 220),
                              QRandomGenerator::global()->bounded(100, 220),
                              QRandomGenerator::global()->bounded(100, 220)));
        painter.drawLine(QRandomGenerator::global()->bounded(0, 90), QRandomGenerator::global()->bounded(0, 40),
                         QRandomGenerator::global()->bounded(0, 90), QRandomGenerator::global()->bounded(0, 40));
    }
    m_captchaLabel->setPixmap(pix);
}

void ForgotPasswordWindow::onVerifyClicked()
{
    QString username = m_usernameEdit->text().trimmed();
    QString verificationCode = m_verificationCodeEdit->text().trimmed();

    if (username.isEmpty()) {
        m_statusLabel->setText("请输入用户名");
        return;
    }
    if (verificationCode.isEmpty()) {
        m_statusLabel->setText("请输入验证码");
        return;
    }
    if (verificationCode.compare(m_captchaText, Qt::CaseInsensitive) != 0) {
        m_statusLabel->setText("验证码错误");
        generateCaptcha();
        return;
    }

    // 验证用户名是否存在
    User user = UserManager::getInstance().getUserById(username);
    if (user.getUserId().isEmpty()) {
        m_statusLabel->setText("用户名不存在");
        return;
    }

    // 验证通过，启用密码重置相关控件
    m_newPasswordEdit->setEnabled(true);
    m_confirmPasswordEdit->setEnabled(true);
    m_resetButton->setEnabled(true);
    m_usernameEdit->setEnabled(false);  // 锁定用户名
    m_verificationCodeEdit->setEnabled(false);  // 锁定验证码
    m_verifyButton->setEnabled(false);  // 禁用验证按钮

    m_statusLabel->setText("请输入新密码");
    m_statusLabel->setStyleSheet("color: #2196F3; font-size: 13px;");
}

void ForgotPasswordWindow::onResetClicked()
{
    if (!validateInput()) return;

    QString username = m_usernameEdit->text().trimmed();
    QString newPassword = m_newPasswordEdit->text();

    m_resetButton->setEnabled(false);
    m_statusLabel->setText("正在重置密码...");
    m_statusLabel->setStyleSheet("color: #2196F3; font-size: 13px;");

    UserManager::getInstance().resetPassword(username, newPassword);
}

bool ForgotPasswordWindow::validateInput()
{
    QString newPassword = m_newPasswordEdit->text();
    QString confirmPassword = m_confirmPasswordEdit->text();

    if (newPassword.isEmpty()) {
        m_statusLabel->setText("请输入新密码");
        m_statusLabel->setStyleSheet("color: #f44336; font-size: 13px;");
        return false;
    }

    if (newPassword != confirmPassword) {
        m_statusLabel->setText("两次密码不一致");
        m_statusLabel->setStyleSheet("color: #f44336; font-size: 13px;");
        return false;
    }

    return true;
}

void ForgotPasswordWindow::onResetSuccess()
{
    QMessageBox::information(this, "提示", "密码重置成功，请使用新密码登录。");
    accept();
}

void ForgotPasswordWindow::onResetFailed(const QString& error)
{
    m_resetButton->setEnabled(true);
    m_statusLabel->setText("密码重置失败: " + error);
    m_statusLabel->setStyleSheet("color: #f44336; font-size: 13px;");
}

void ForgotPasswordWindow::onBackToLoginClicked()
{
    playBackAnimation();
}

void ForgotPasswordWindow::playBackAnimation()
{
    QPropertyAnimation* anim = new QPropertyAnimation(this, "windowOpacity");
    anim->setDuration(400);
    anim->setStartValue(1.0);
    anim->setEndValue(0.0);
    connect(anim, &QPropertyAnimation::finished, this, [=](){ reject(); });
    anim->start(QAbstractAnimation::DeleteWhenStopped);
}

void ForgotPasswordWindow::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        m_dragging = true;
        m_dragPosition = event->globalPosition().toPoint() - frameGeometry().topLeft();
        event->accept();
    }
}

void ForgotPasswordWindow::mouseMoveEvent(QMouseEvent* event)
{
    if (m_dragging && (event->buttons() & Qt::LeftButton)) {
        move(event->globalPosition().toPoint() - m_dragPosition);
        event->accept();
    }
}

bool ForgotPasswordWindow::eventFilter(QObject* obj, QEvent* event)
{
    if (obj == m_captchaLabel && event->type() == QEvent::MouseButtonPress) {
        generateCaptcha();
        return true;
    }
    return QDialog::eventFilter(obj, event);
} 