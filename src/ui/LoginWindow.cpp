#include "LoginWindow.h"
#include "MainWindow.h"
#include "RegisterWindow.h"
#include "core/UserManager.h"
#include "utils/Config.h"
#include <QMessageBox>
#include <QGraphicsDropShadowEffect>
#include <QMouseEvent>
#include <QFile>
#include <QPropertyAnimation>
#include <QTimer>
#include <QRandomGenerator>
#include <QPainter>
#include <QMouseEvent>
#include "InputWindow.h"

LoginWindow::LoginWindow(QWidget *parent)
    : QDialog(parent)
{
    setWindowFlags(Qt::FramelessWindowHint | Qt::Window);
    setAttribute(Qt::WA_TranslucentBackground);
    m_codeTimer = new QTimer(this);
    setupUI();
    playShowAnimation();

    connect(&UserManager::getInstance(), &UserManager::loginSuccess,
            this, &LoginWindow::onLoginSuccess);
    connect(&UserManager::getInstance(), &UserManager::loginFailed,
            this, &LoginWindow::onLoginFailed);

    Config::getInstance().initialize();
    m_usernameEdit->setText(Config::getInstance().getLastUsername());

    m_captchaLabel->installEventFilter(this);

    connect(m_usernameEdit, &QLineEdit::textChanged, this, &LoginWindow::onUsernameChanged);

    onUsernameChanged(m_usernameEdit->text());
}

void LoginWindow::setupUI()
{
    setWindowTitle("登录 - Simple IMChat");
    setFixedSize(360, 520);

    QVBoxLayout* outerLayout = new QVBoxLayout(this);
    outerLayout->setContentsMargins(0, 0, 0, 0);

    QWidget* bgWidget = new QWidget(this);
    bgWidget->setObjectName("bgWidget");
    bgWidget->setStyleSheet(
        "#bgWidget {"
        "background: white;"
        "border-radius: 18px;"
        "}"
    );
    
    QVBoxLayout* layout = new QVBoxLayout(bgWidget);
    layout->setSpacing(15);
    layout->setContentsMargins(30, 30, 30, 30);

    // 阴影
    QGraphicsDropShadowEffect* shadow = new QGraphicsDropShadowEffect(bgWidget);
    shadow->setBlurRadius(24);
    shadow->setOffset(0, 4);
    shadow->setColor(QColor(0,0,0,60));
    bgWidget->setGraphicsEffect(shadow);

    // 右上角按钮布局
    QHBoxLayout* titleLayout = new QHBoxLayout();
    titleLayout->setContentsMargins(0, 0, 0, 0);
    titleLayout->addStretch();

    m_minButton = new QPushButton("-");
    m_minButton->setFixedSize(28, 28);
    m_minButton->setStyleSheet("QPushButton { background: #F0F0F0; border: none; border-radius: 4px; font-size: 18px; } QPushButton:hover { background: #E1F6EF; }");
    connect(m_minButton, &QPushButton::clicked, this, &LoginWindow::onMinimizeClicked);
    titleLayout->addWidget(m_minButton);

    m_closeButton = new QPushButton("×");
    m_closeButton->setFixedSize(28, 28);
    m_closeButton->setStyleSheet("QPushButton { background: #F0F0F0; border: none; border-radius: 4px; font-size: 18px; } QPushButton:hover { background: #FF4D4F; color: white; }");
    connect(m_closeButton, &QPushButton::clicked, this, &LoginWindow::onCloseClicked);
    titleLayout->addWidget(m_closeButton);

    layout->addLayout(titleLayout);

    // 头像
    m_avatarLabel = new QLabel();
    m_avatarLabel->setFixedSize(80, 80);
    m_avatarLabel->setAlignment(Qt::AlignCenter);
    m_avatarLabel->setStyleSheet("border-radius: 40px; background: #eee;");
    layout->addWidget(m_avatarLabel, 0, Qt::AlignHCenter);
    m_avatarLabel->setPixmap(QPixmap(":/icons/logo").scaled(80, 80, Qt::KeepAspectRatio, Qt::SmoothTransformation));

    // 标题
    QLabel* titleLabel = new QLabel("微信登录");
    titleLabel->setObjectName("titleLabel");
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet("QLabel#titleLabel { font-size: 22px; color: #353535; }");
    layout->addWidget(titleLabel);
    layout->addSpacing(20);

    // 用户名输入框
    m_usernameEdit = new QLineEdit();
    m_usernameEdit->setPlaceholderText("请输入用户名");
    m_usernameEdit->setMinimumHeight(40);
    m_usernameEdit->setStyleSheet(
        "QLineEdit { "
        "   border: 1px solid #DCDCDC; "
        "   border-radius: 4px; "
        "   padding: 0 10px; "
        "   font-size: 14px; "
        "}"
    );
    layout->addWidget(m_usernameEdit);

    // 密码输入框
    m_passwordEdit = new QLineEdit();
    m_passwordEdit->setPlaceholderText("请输入密码");
    m_passwordEdit->setEchoMode(QLineEdit::Password);
    m_passwordEdit->setMinimumHeight(40);
    m_passwordEdit->setStyleSheet(m_usernameEdit->styleSheet());
    layout->addWidget(m_passwordEdit);

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
    layout->addSpacing(20);

    // 生成初始验证码
    generateCaptcha();

    // 登录按钮
    m_loginButton = new QPushButton("登录");
    m_loginButton->setMinimumHeight(40);
    m_loginButton->setCursor(Qt::PointingHandCursor);
    m_loginButton->setStyleSheet(
        "QPushButton { "
        "   background-color: #07C160; "
        "   border: none; "
        "   border-radius: 4px; "
        "   color: white; "
        "   font-size: 15px; "
        "} "
        "QPushButton:hover { background-color: #06AD56; } "
        "QPushButton:pressed { background-color: #059B4C; } "
        "QPushButton:disabled { background-color: #91E6B3; }"
    );
    connect(m_loginButton, &QPushButton::clicked, this, &LoginWindow::onLoginClicked);
    layout->addWidget(m_loginButton);

    // 状态标签
    m_statusLabel = new QLabel();
    m_statusLabel->setAlignment(Qt::AlignCenter);
    m_statusLabel->setStyleSheet("color: #FF0000;");
    layout->addWidget(m_statusLabel);

    // 底部链接
    QHBoxLayout* bottomLayout = new QHBoxLayout();
    QLabel* registerLink = new QLabel("<a href=\"#\" style=\"color:#07C160;text-decoration:none;\">注册账号</a>");
    QLabel* forgotLink = new QLabel("<a href=\"#\" style=\"color:#07C160;text-decoration:none;\">忘记密码</a>");
    registerLink->setCursor(Qt::PointingHandCursor);
    forgotLink->setCursor(Qt::PointingHandCursor);
    connect(registerLink, &QLabel::linkActivated, this, &LoginWindow::onRegisterClicked);
    connect(forgotLink, &QLabel::linkActivated, this, &LoginWindow::onForgotPasswordClicked);

    bottomLayout->addWidget(registerLink);
    bottomLayout->addStretch();
    bottomLayout->addWidget(forgotLink);
    layout->addLayout(bottomLayout);

    layout->addStretch();

    outerLayout->addWidget(bgWidget);
}

void LoginWindow::onSendCodeClicked()
{
    m_sentCode = QString::number(QRandomGenerator::global()->bounded(1000, 9999));
    QMessageBox::information(this, "验证码", "本次验证码为: " + m_sentCode);
    m_codeCountdown = 60;
    m_sendCodeButton->setEnabled(false);
    m_sendCodeButton->setText("重新发送(60)");
    m_codeTimer->start(1000);
}

void LoginWindow::onLoginClicked()
{
    QString username = m_usernameEdit->text().trimmed();
    QString password = m_passwordEdit->text();
    QString code = m_codeEdit->text().trimmed();

    if (username.isEmpty()) {
        m_statusLabel->setText("请输入用户名");
        return;
    }
    if (code.isEmpty()) {
        m_statusLabel->setText("请输入验证码");
        return;
    }
    if (code.compare(m_captchaText, Qt::CaseInsensitive) != 0) {
        m_statusLabel->setText("验证码错误");
        generateCaptcha();
        return;
    }

    m_loginButton->setEnabled(false);
    m_statusLabel->setText("登录中...");

    UserManager::getInstance().login(username, password);
}

void LoginWindow::onLoginSuccess()
{
    Config::getInstance().setLastUsername(m_usernameEdit->text());
    playToMainAnimation();
}

void LoginWindow::playShowAnimation()
{
    setWindowOpacity(0.0);
    QPropertyAnimation* anim = new QPropertyAnimation(this, "windowOpacity");
    anim->setDuration(400);
    anim->setStartValue(0.0);
    anim->setEndValue(1.0);
    anim->start(QAbstractAnimation::DeleteWhenStopped);
}

void LoginWindow::playToMainAnimation()
{
    QPropertyAnimation* anim = new QPropertyAnimation(this, "windowOpacity");
    anim->setDuration(400);
    anim->setStartValue(1.0);
    anim->setEndValue(0.0);
    connect(anim, &QPropertyAnimation::finished, this, [=](){
        MainWindow* mainWindow = new MainWindow();
        mainWindow->show();
        accept();
    });
    anim->start(QAbstractAnimation::DeleteWhenStopped);
}

void LoginWindow::onLoginFailed(const QString& error)
{
    m_loginButton->setEnabled(true);
    m_statusLabel->setText(error);
}

void LoginWindow::onRegisterClicked()
{
    RegisterWindow* registerWindow = new RegisterWindow(this);
    if (registerWindow->exec() == QDialog::Accepted) {
        // 注册成功后可以自动填充用户名
        m_usernameEdit->setText(registerWindow->getRegisteredUsername());
    }
    delete registerWindow;
}

void LoginWindow::onForgotPasswordClicked()
{
    // TODO: 打开找回密码窗口
    QMessageBox::information(this, "提示", "找回密码功能开发中...");
}

void LoginWindow::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        m_dragPos = event->globalPosition().toPoint() - frameGeometry().topLeft();
        event->accept();
    }
}

void LoginWindow::mouseMoveEvent(QMouseEvent* event)
{
    if (event->buttons() & Qt::LeftButton) {
        move(event->globalPosition().toPoint() - m_dragPos);
        event->accept();
    }
}

void LoginWindow::onMinimizeClicked()
{
    showMinimized();
}

void LoginWindow::onCloseClicked()
{
    close();
}

void LoginWindow::generateCaptcha()
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

bool LoginWindow::eventFilter(QObject* obj, QEvent* event)
{
    if (obj == m_captchaLabel && event->type() == QEvent::MouseButtonPress) {
        generateCaptcha();
        return true;
    }
    return QDialog::eventFilter(obj, event);
}

void LoginWindow::onUsernameChanged(const QString& username)
{
    QString avatarPath;
    if (!username.trimmed().isEmpty()) {
        User user = UserManager::getInstance().getUserById(username.trimmed());
        avatarPath = user.getAvatar();
    }
    if (!avatarPath.isEmpty() && QFile::exists(avatarPath)) {
        m_avatarLabel->setPixmap(QPixmap(avatarPath).scaled(80, 80, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    } else {
        // 使用你icons文件夹下的默认头像（如":/icons/default_avatar.png"）
        m_avatarLabel->setPixmap(QPixmap(":/icons/default_avatar.png").scaled(80, 80, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }
}