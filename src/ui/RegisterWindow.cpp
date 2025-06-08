#include "RegisterWindow.h"
#include "core/UserManager.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGraphicsDropShadowEffect>
#include <QMouseEvent>
#include <QMessageBox>
#include <QFileDialog>
#include <QPropertyAnimation>
#include <QTimer>
#include <QRandomGenerator>
#include <QPainter>
#include <QEvent>

RegisterWindow::RegisterWindow(QWidget *parent) : QDialog(parent)
{
    setWindowFlags(Qt::FramelessWindowHint | Qt::Window);
    setAttribute(Qt::WA_TranslucentBackground);
    m_codeTimer = new QTimer(this);
    setupUI();

    connect(&UserManager::getInstance(), &UserManager::registerSuccess,
            this, &RegisterWindow::onRegisterSuccess);
    connect(&UserManager::getInstance(), &UserManager::registerFailed,
            this, &RegisterWindow::onRegisterFailed);
}

void RegisterWindow::setupUI()
{
    setFixedSize(420, 560);

    QVBoxLayout* outerLayout = new QVBoxLayout(this);
    outerLayout->setContentsMargins(0, 0, 0, 0);

    QWidget* bgWidget = new QWidget(this);
    bgWidget->setObjectName("bgWidget");
    bgWidget->setStyleSheet(R"(
        #bgWidget {
            background: white;
            border-radius: 20px;
        }
    )");

    QGraphicsDropShadowEffect* shadow = new QGraphicsDropShadowEffect(bgWidget);
    shadow->setBlurRadius(20);
    shadow->setColor(QColor(0, 0, 0, 80));
    shadow->setOffset(0, 4);
    bgWidget->setGraphicsEffect(shadow);

    QVBoxLayout* layout = new QVBoxLayout(bgWidget);
    layout->setContentsMargins(40, 30, 40, 30);
    layout->setSpacing(15);

    // 返回按钮
    m_backButton = new QPushButton("返回");
    m_backButton->setFixedSize(60, 32);
    m_backButton->setStyleSheet("QPushButton { background: #F7F7F7; border: none; border-radius: 8px; color: #07C160; font-size: 15px; } QPushButton:hover { background: #E1F6EF; }");
    connect(m_backButton, &QPushButton::clicked, this, &RegisterWindow::onBackToLoginClicked);
    layout->addWidget(m_backButton, 0, Qt::AlignLeft);

    // 标题
    m_titleLabel = new QLabel("欢迎注册");
    m_titleLabel->setAlignment(Qt::AlignCenter);
    m_titleLabel->setStyleSheet("font-size: 24px; color: #333333; font-weight: bold;");
    layout->addWidget(m_titleLabel);
    layout->addSpacing(10);

    // 昵称输入框
    m_nicknameEdit = new QLineEdit();
    m_nicknameEdit->setPlaceholderText("输入昵称");
    m_nicknameEdit->setMinimumHeight(45);
    layout->addWidget(m_nicknameEdit);

    // 验证码图片和输入框
    QHBoxLayout* captchaLayout = new QHBoxLayout();
    m_codeEdit = new QLineEdit();
    m_codeEdit->setPlaceholderText("输入验证码");
    m_codeEdit->setMinimumHeight(36);
    m_codeEdit->setMaximumWidth(100);
    captchaLayout->addWidget(m_codeEdit);

    m_captchaLabel = new QLabel();
    m_captchaLabel->setFixedSize(90, 36);
    m_captchaLabel->setStyleSheet("border-radius: 6px; background: #F7F7F7; border: 1px solid #E3E3E3;");
    captchaLayout->addWidget(m_captchaLabel);

    layout->addLayout(captchaLayout);

    // 生成初始验证码
    generateCaptcha();

    // 点击验证码图片可刷新
    m_captchaLabel->installEventFilter(this);

    // 密码输入框
    m_passwordEdit = new QLineEdit();
    m_passwordEdit->setPlaceholderText("输入密码");
    m_passwordEdit->setEchoMode(QLineEdit::Password);
    m_passwordEdit->setMinimumHeight(45);
    layout->addWidget(m_passwordEdit);

    // 确认密码输入框
    m_confirmPasswordEdit = new QLineEdit();
    m_confirmPasswordEdit->setPlaceholderText("确认密码");
    m_confirmPasswordEdit->setEchoMode(QLineEdit::Password);
    m_confirmPasswordEdit->setMinimumHeight(45);
    layout->addWidget(m_confirmPasswordEdit);

    // 同意协议复选框
    m_agreementCheck = new QCheckBox("已阅读并同意服务协议和隐私保护指引");
    m_agreementCheck->setStyleSheet("QCheckBox { color: #666666; }");
    layout->addWidget(m_agreementCheck);

    // 注册按钮
    m_registerButton = new QPushButton("立即注册");
    m_registerButton->setMinimumHeight(45);
    m_registerButton->setEnabled(false);
    layout->addWidget(m_registerButton);

    // 状态标签
    m_statusLabel = new QLabel();
    m_statusLabel->setAlignment(Qt::AlignCenter);
    m_statusLabel->setStyleSheet("color: #FF4D4F;");
    layout->addWidget(m_statusLabel);

    connect(m_agreementCheck, &QCheckBox::toggled, m_registerButton, &QPushButton::setEnabled);
    connect(m_registerButton, &QPushButton::clicked, this, &RegisterWindow::onRegisterClicked);

    outerLayout->addWidget(bgWidget);
}

void RegisterWindow::onSendCodeClicked()
{
    // 生成4位数字验证码
    m_sentCode = QString::number(QRandomGenerator::global()->bounded(1000, 9999));
    QMessageBox::information(this, "验证码", "本次验证码为: " + m_sentCode); // 实际项目应通过短信/邮箱发送
    m_codeCountdown = 60;
    m_sendCodeButton->setEnabled(false);
    m_sendCodeButton->setText("重新发送(60)");
    m_codeTimer->start(1000);
}

void RegisterWindow::onRegisterClicked()
{
    if (!validateInput()) return;
    if (m_codeEdit->text().trimmed() != m_sentCode) {
        m_statusLabel->setText("验证码错误");
        return;
    }
    QString nickname = m_nicknameEdit->text().trimmed();
    QString password = m_passwordEdit->text();
    m_registerButton->setEnabled(false);
    m_statusLabel->setText("注册中...");
    UserManager::getInstance().registerUser(nickname, password, "");
}

bool RegisterWindow::validateInput()
{
    QString nickname = m_nicknameEdit->text().trimmed();
    QString password = m_passwordEdit->text();
    QString confirmPassword = m_confirmPasswordEdit->text();
    if (nickname.isEmpty()) { m_statusLabel->setText("请输入昵称"); return false; }
    if (password.isEmpty()) { m_statusLabel->setText("请输入密码"); return false; }
    if (password != confirmPassword) { m_statusLabel->setText("两次输入的密码不一致"); return false; }
    if (m_codeEdit->text().isEmpty()) { m_statusLabel->setText("请输入验证码"); return false; }
    return true;
}

void RegisterWindow::onRegisterSuccess()
{
    QMessageBox::information(this, "提示", "注册成功");
    accept();
}

void RegisterWindow::onRegisterFailed(const QString& error)
{
    m_registerButton->setEnabled(true);
    m_statusLabel->setText(error);
}

void RegisterWindow::onBackToLoginClicked()
{
    playBackAnimation();
}

void RegisterWindow::playBackAnimation()
{
    QPropertyAnimation* anim = new QPropertyAnimation(this, "windowOpacity");
    anim->setDuration(400);
    anim->setStartValue(1.0);
    anim->setEndValue(0.0);
    connect(anim, &QPropertyAnimation::finished, this, [=](){ reject(); });
    anim->start(QAbstractAnimation::DeleteWhenStopped);
}

void RegisterWindow::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        m_dragging = true;
        m_dragPosition = event->globalPosition().toPoint() - frameGeometry().topLeft();
    }
}

void RegisterWindow::mouseMoveEvent(QMouseEvent* event)
{
    if (event->buttons() & Qt::LeftButton && m_dragging) {
        move(event->globalPosition().toPoint() - m_dragPosition);
    }
}

void RegisterWindow::mouseReleaseEvent(QMouseEvent* event)
{
    m_dragging = false;
}

void RegisterWindow::generateCaptcha()
{
    // 生成4位随机字母数字
    const QString chars = "ABCDEFGHJKLMNPQRSTUVWXYZ23456789";
    m_captchaText.clear();
    for (int i = 0; i < 4; ++i)
        m_captchaText += chars.at(QRandomGenerator::global()->bounded(chars.size()));

    QPixmap pix(90, 36);
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
        painter.drawText(10 + i*20, QRandomGenerator::global()->bounded(22, 32), m_captchaText.mid(i,1));
    }
    // 干扰线
    for (int i = 0; i < 6; ++i) {
        painter.setPen(QColor(QRandomGenerator::global()->bounded(100, 220),
                              QRandomGenerator::global()->bounded(100, 220),
                              QRandomGenerator::global()->bounded(100, 220)));
        painter.drawLine(QRandomGenerator::global()->bounded(0, 90), QRandomGenerator::global()->bounded(0, 36),
                         QRandomGenerator::global()->bounded(0, 90), QRandomGenerator::global()->bounded(0, 36));
    }
    m_captchaLabel->setPixmap(pix);
}

bool RegisterWindow::eventFilter(QObject* obj, QEvent* event)
{
    if (obj == m_captchaLabel && event->type() == QEvent::MouseButtonPress) {
        generateCaptcha();
        return true;
    }
    return QDialog::eventFilter(obj, event);
}