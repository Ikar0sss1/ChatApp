#include "MainWindow.h"
#include "core/UserManager.h"
#include "ContactListWidget.h"
#include "ChatWindow.h"
#include "LoginWindow.h"
#include <QMenuBar>
#include <QAction>
#include <QInputDialog>
#include <QMessageBox>
#include <QGraphicsDropShadowEffect>
#include <QMouseEvent>
#include <QSplitter>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QWidget>
#include <QLineEdit>
#include <QLabel>
#include <QFrame>
#include <QFileDialog>
#include <QPropertyAnimation>
#include <QFile>
#include "InputWindow.h"
#include "DeleteWindow.h"
#include "SettingsDialog.h"
#include <QApplication>
#include <QSettings>
#include "core/NetworkManager.h"
#include <QPainter>
#include <QBrush>

void applyAppStyle(bool darkMode, int fontSize) {
    QString qss;
    QFile f(darkMode ? ":/resources/qss/dark.qss" : ":/resources/qss/style.qss");
    if (f.open(QFile::ReadOnly)) {
        qss = f.readAll();
        f.close();
    }
    qss += QString("\nQWidget { font-size: %1px; }").arg(fontSize);
    qApp->setStyleSheet(qss);
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowFlags(Qt::FramelessWindowHint | Qt::Window);
    setStyleSheet("QMainWindow { background: #f5f5f5; }");
    
    QSettings settings("YourCompany", "YourApp");
    int fontSize = settings.value("fontSize", 15).toInt();
    bool darkMode = settings.value("darkMode", false).toBool();
    applyAppStyle(darkMode, fontSize);

    setupUI();

    QPropertyAnimation* anim = new QPropertyAnimation(this, "windowOpacity");
    setWindowOpacity(0.0);
    anim->setDuration(400);
    anim->setStartValue(0.0);
    anim->setEndValue(1.0);
    anim->start(QAbstractAnimation::DeleteWhenStopped);

    connect(&NetworkManager::getInstance(), &NetworkManager::messageReceived,
            this, [this](const Message& message){
        QString myId = UserManager::getInstance().getCurrentUserId();
        QString senderId = message.getSenderId();
        QString receiverId = message.getReceiverId();
        
        QString contactId;
        
        // 判断消息方向和对话窗口应该属于哪个联系人
        if (senderId == myId) {
            // 我发送的消息，对话窗口应该是与接收者的窗口
            contactId = receiverId;
        } else {
            // 别人发给我的消息，对话窗口应该是与发送者的窗口
            contactId = senderId;
        }
        
        // 分发到对应的聊天窗口
        if (m_chatWindows.contains(contactId)) {
            // ChatWindow已经连接了NetworkManager::messageReceived信号
            // 它会处理属于自己的消息，这里不需要再处理
        } else {
            // 如果没有打开对话窗口但收到了消息，创建一个新窗口
            // 但只有当我不是发送者或者是接收者时才需要创建窗口
            if (senderId != myId || receiverId == myId) {
                ChatWindow* chatWindow = new ChatWindow(contactId);
                m_chatWindows[contactId] = chatWindow;
                m_chatStack->addWidget(chatWindow);
                
                // 如果是我收到的消息，需要高亮联系人
                if (receiverId == myId) {
                    // 高亮联系人列表中的该联系人
                    m_contactList->highlightContact(contactId, true);
                }
            }
        }
    });
}

void MainWindow::setupUI()
{
    setMinimumSize(1000, 700);

    // 主容器
    QWidget* centralWidget = new QWidget();
    centralWidget->setStyleSheet("background: #f5f5f5;");
    setCentralWidget(centralWidget);

    QVBoxLayout* mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    // 标题栏
    setupTitleBar(mainLayout);

    // 主要内容区域
    setupMainContent(mainLayout);
}

void MainWindow::setupTitleBar(QVBoxLayout* mainLayout)
{
    QWidget* titleBar = new QWidget();
    titleBar->setFixedHeight(50);
    titleBar->setStyleSheet("background: #FFFFFF; border-bottom: 1px solid #E6E6E6;");
    
    QHBoxLayout* titleLayout = new QHBoxLayout(titleBar);
    titleLayout->setContentsMargins(15, 0, 15, 0);
    titleLayout->setSpacing(0);

    // 左侧Logo和标题
    QHBoxLayout* leftLayout = new QHBoxLayout();
    leftLayout->setContentsMargins(12, 0, 0, 0);
    leftLayout->setSpacing(12);
    
    m_myAvatarLabel = new QLabel();
    m_myAvatarLabel->setFixedSize(36, 36);
    m_myAvatarLabel->setStyleSheet("border-radius: 18px; background: #eee;");
    QString avatarPath = UserManager::getInstance().getCurrentUser().getAvatar();
    if (!avatarPath.isEmpty() && QFile::exists(avatarPath)) {
        QPixmap avatarPixmap(avatarPath);
        if(!avatarPixmap.isNull()) {
            // 创建圆形头像
            QPixmap rounded(36, 36);
            rounded.fill(Qt::transparent);
            QPainter painter(&rounded);
            painter.setRenderHint(QPainter::Antialiasing);
            painter.setPen(Qt::NoPen);
            painter.setBrush(QBrush(avatarPixmap.scaled(36, 36, Qt::KeepAspectRatio, Qt::SmoothTransformation)));
            painter.drawEllipse(0, 0, 36, 36);
            m_myAvatarLabel->setPixmap(rounded);
        }
    } else {
        m_myAvatarLabel->setPixmap(QPixmap(":/icons/default-avatar.png").scaled(36, 36, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }
    leftLayout->addWidget(m_myAvatarLabel);

    QLabel* titleLabel = new QLabel("聊天应用");
    titleLabel->setStyleSheet("font-size: 16px; font-weight: bold; color: #333333;");
    leftLayout->addWidget(titleLabel);

    m_editAvatarBtn = new QPushButton("更换头像");
    m_editAvatarBtn->setCursor(Qt::PointingHandCursor);
    m_editAvatarBtn->setStyleSheet(
        "QPushButton { background: #f5f5f5; border: none; border-radius: 4px; color: #222; font-size: 13px; padding: 6px 12px; }"
        "QPushButton:hover { background: #e1e1e1; color: #2196F3; }"
    );
    connect(m_editAvatarBtn, &QPushButton::clicked, this, [=](){
        QString file = QFileDialog::getOpenFileName(this, "选择头像", "", "图片文件 (*.png *.jpg *.jpeg)");
        if (!file.isEmpty()) {
            QPixmap avatarPixmap(file);
            if(!avatarPixmap.isNull()) {
                // 创建圆形头像
                QPixmap rounded(36, 36);
                rounded.fill(Qt::transparent);
                QPainter painter(&rounded);
                painter.setRenderHint(QPainter::Antialiasing);
                painter.setPen(Qt::NoPen);
                painter.setBrush(QBrush(avatarPixmap.scaled(36, 36, Qt::KeepAspectRatio, Qt::SmoothTransformation)));
                painter.drawEllipse(0, 0, 36, 36);
                m_myAvatarLabel->setPixmap(rounded);
            }
            UserManager::getInstance().updateCurrentUserAvatar(file);
            refreshMyAvatar();
        }
    });
    leftLayout->addWidget(m_editAvatarBtn);

    leftLayout->addStretch();
    titleLayout->addLayout(leftLayout);

    // 添加退出登录按钮
    m_logoutButton = new QPushButton("退出登录");
    m_logoutButton->setCursor(Qt::PointingHandCursor);
    m_logoutButton->setStyleSheet(
        "QPushButton { background: #f44336; border: none; border-radius: 4px; color: white; font-size: 13px; padding: 6px 12px; }"
        "QPushButton:hover { background: #e53935; }"
    );
    connect(m_logoutButton, &QPushButton::clicked, this, &MainWindow::onLogoutClicked);
    titleLayout->addWidget(m_logoutButton);
    
    // 右侧控制按钮
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->setSpacing(8);

    // 设置按钮 - 使用齿轮符号 ⚙
    m_settingsButton = new QPushButton("⚙");
    m_settingsButton->setFixedSize(32, 32);
    m_settingsButton->setStyleSheet(
        "QPushButton { "
        "   background: transparent; "
        "   border: 1px solid #e0e0e0; "
        "   border-radius: 4px; "
        "   padding: 0px; "
        "   font-size: 16px; "
        "   color: #666666; "
        "} "
        "QPushButton:hover { "
        "   background: #e0e0e0; "
        "   color: #333333; "
        "}"
    );
    connect(m_settingsButton, &QPushButton::clicked, this, &MainWindow::onSettingsClicked);
    buttonLayout->addWidget(m_settingsButton);

    // 最小化按钮 - 使用下划线符号 −
    m_minButton = new QPushButton("−");
    m_minButton->setFixedSize(32, 32);
    m_minButton->setStyleSheet(
        "QPushButton { "
        "   background: transparent; "
        "   border: 1px solid #e0e0e0; "
        "   border-radius: 4px; "
        "   padding: 0px; "
        "   font-size: 16px; "
        "   font-weight: bold; "
        "   color: #666666; "
        "} "
        "QPushButton:hover { "
        "   background: #e0e0e0; "
        "   color: #333333; "
        "}"
    );
    connect(m_minButton, &QPushButton::clicked, this, &MainWindow::onMinimizeClicked);
    buttonLayout->addWidget(m_minButton);

    // 最大化按钮 - 使用方框符号 ☐
    m_maxButton = new QPushButton("☐");
    m_maxButton->setFixedSize(32, 32);
    m_maxButton->setStyleSheet(
        "QPushButton { "
        "   background: transparent; "
        "   border: 1px solid #e0e0e0; "
        "   border-radius: 4px; "
        "   padding: 0px; "
        "   font-size: 14px; "
        "   color: #666666; "
        "} "
        "QPushButton:hover { "
        "   background: #e0e0e0; "
        "   color: #333333; "
        "}"
    );
    connect(m_maxButton, &QPushButton::clicked, this, &MainWindow::onMaximizeClicked);
    buttonLayout->addWidget(m_maxButton);

    // 关闭按钮 - 使用 × 符号
    m_closeButton = new QPushButton("×");
    m_closeButton->setFixedSize(32, 32);
    m_closeButton->setStyleSheet(
        "QPushButton { "
        "   background: transparent; "
        "   border: 1px solid #e0e0e0; "
        "   border-radius: 4px; "
        "   padding: 0px; "
        "   font-size: 18px; "
        "   font-weight: bold; "
        "   color: #666666; "
        "} "
        "QPushButton:hover { "
        "   background: #FF5F56; "
        "   color: white; "
        "   border-color: #FF5F56; "
        "}"
    );
    connect(m_closeButton, &QPushButton::clicked, this, &MainWindow::onCloseClicked);
    buttonLayout->addWidget(m_closeButton);

    titleLayout->addLayout(buttonLayout);

    mainLayout->addWidget(titleBar);
}

void MainWindow::setupMainContent(QVBoxLayout* mainLayout)
{
    // 创建主分割器
    QSplitter* splitter = new QSplitter();
    splitter->setHandleWidth(1);
    splitter->setChildrenCollapsible(false);
    splitter->setStyleSheet("QSplitter::handle { background-color: #e0e0e0; }");

    // 左侧面板
    setupLeftPanel(splitter);
    
    // 右侧聊天区域
    setupChatArea(splitter);

    // 设置分割器比例
    splitter->setSizes({260, 740});
    
    mainLayout->addWidget(splitter);
}

void MainWindow::setupLeftPanel(QSplitter* splitter)
{
    QWidget* leftPanel = new QWidget();
    leftPanel->setFixedWidth(260);
    leftPanel->setStyleSheet("background: #FFFFFF;");
    
    QVBoxLayout* leftLayout = new QVBoxLayout(leftPanel);
    leftLayout->setContentsMargins(0, 0, 0, 0);
    leftLayout->setSpacing(0);

    // 导航栏
    setupNavigationBar(leftLayout);
    
    // 搜索区域
    setupSearchArea(leftLayout);
    
    // 联系人列表
    m_contactList = new ContactListWidget();
    m_contactList->setStyleSheet(
        "ContactListWidget { "
        "   background: #FFFFFF; "
        "   border: none; "
        "}"
    );
    connect(m_contactList, &ContactListWidget::contactSelected, this, &MainWindow::onContactSelected);
    leftLayout->addWidget(m_contactList);

    splitter->addWidget(leftPanel);
}

void MainWindow::setupNavigationBar(QVBoxLayout* leftLayout)
{
    QWidget* navBar = new QWidget();
    navBar->setFixedHeight(50);
    navBar->setStyleSheet("background: #FAFAFA; border-bottom: 1px solid #F0F0F0;");
    
    QHBoxLayout* navLayout = new QHBoxLayout(navBar);
    navLayout->setContentsMargins(12, 0, 12, 0);
    navLayout->setSpacing(8);

    // 聊天按钮 - 可以保留图标或改为符号 💬
    m_chatNavButton = new QPushButton("💬 聊天");
    m_chatNavButton->setFixedHeight(36);
    m_chatNavButton->setStyleSheet(
        "QPushButton { "
        "   background: #2196F3; "
        "   border: none; "
        "   border-radius: 4px; "
        "   color: white; "
        "   font-size: 14px; "
        "   padding-left: 12px; "
        "   text-align: left; "
        "} "
        "QPushButton:hover { background: #1E88E5; }"
    );
    navLayout->addWidget(m_chatNavButton);

    // 联系人按钮 - 可以保留图标或改为符号 👥
    m_contactNavButton = new QPushButton("👥 联系人");
    m_contactNavButton->setFixedHeight(36);
    m_contactNavButton->setStyleSheet(
        "QPushButton { "
        "   background: transparent; "
        "   border: none; "
        "   border-radius: 4px; "
        "   color: #666666; "
        "   font-size: 14px; "
        "   padding-left: 12px; "
        "   text-align: left; "
        "} "
        "QPushButton:hover { background: #F0F0F0; }"
    );
    navLayout->addWidget(m_contactNavButton);

    navLayout->addStretch();

    // 添加联系人按钮
    m_addContactButton = new QPushButton("+");
    m_addContactButton->setFixedSize(30, 30);
    m_addContactButton->setStyleSheet(
        "QPushButton { "
        "   background: #2196F3; "
        "   border: none; "
        "   border-radius: 4px; "
        "   color: white; "
        "   font-size: 18px; "
        "   font-weight: bold; "
        "} "
        "QPushButton:hover { background: #1E88E5; }"
    );
    connect(m_addContactButton, &QPushButton::clicked, this, &MainWindow::onAddContactClicked);
    navLayout->addWidget(m_addContactButton);

    leftLayout->addWidget(navBar);
}

void MainWindow::setupSearchArea(QVBoxLayout* leftLayout)
{
    QWidget* searchContainer = new QWidget();
    searchContainer->setFixedHeight(50);
    searchContainer->setStyleSheet("background: #FFFFFF;");
    
    QHBoxLayout* searchLayout = new QHBoxLayout(searchContainer);
    searchLayout->setContentsMargins(12, 8, 12, 8);
    
    QWidget* searchBox = new QWidget();
    searchBox->setStyleSheet(
        "background: #F5F5F5; "
        "border-radius: 4px; "
    );
    
    QHBoxLayout* searchBoxLayout = new QHBoxLayout(searchBox);
    searchBoxLayout->setContentsMargins(8, 0, 8, 0);
    searchBoxLayout->setSpacing(6);
    
    // 搜索图标 - 使用符号 🔍
    QLabel* searchIcon = new QLabel("🔍");
    searchIcon->setStyleSheet("font-size: 14px;");
    searchBoxLayout->addWidget(searchIcon);
    
    m_searchEdit = new QLineEdit();
    m_searchEdit->setPlaceholderText("搜索联系人");
    m_searchEdit->setFixedHeight(34);
    m_searchEdit->setFrame(false);
    m_searchEdit->setStyleSheet(
        "QLineEdit { "
        "   background: transparent; "
        "   border: none; "
        "   font-size: 14px; "
        "   color: #333333; "
        "} "
    );
    connect(m_searchEdit, &QLineEdit::textChanged, this, [=](const QString& text){
        m_contactList->filterContacts(text);
    });
    searchBoxLayout->addWidget(m_searchEdit);
    
    searchLayout->addWidget(searchBox);
    leftLayout->addWidget(searchContainer);
}

void MainWindow::setupChatArea(QSplitter* splitter)
{
    QWidget* chatContainer = new QWidget();
    chatContainer->setStyleSheet("background: #F7F7F7;");
    
    QVBoxLayout* chatLayout = new QVBoxLayout(chatContainer);
    chatLayout->setContentsMargins(0, 0, 0, 0);
    chatLayout->setSpacing(0);

    // 欢迎界面
    QWidget* welcomeWidget = new QWidget();
    welcomeWidget->setStyleSheet("background: #FFFFFF;");
    
    QVBoxLayout* welcomeLayout = new QVBoxLayout(welcomeWidget);
    welcomeLayout->setAlignment(Qt::AlignCenter);
    
    // 聊天图标 - 使用符号
    QLabel* welcomeIcon = new QLabel("💬");
    welcomeIcon->setAlignment(Qt::AlignCenter);
    welcomeIcon->setStyleSheet("font-size: 80px;");
    welcomeLayout->addWidget(welcomeIcon);
    
    QLabel* welcomeText = new QLabel("选择一个联系人开始聊天");
    welcomeText->setAlignment(Qt::AlignCenter);
    welcomeText->setStyleSheet("font-size: 16px; color: #666666; margin-top: 20px;");
    welcomeLayout->addWidget(welcomeText);
    
    QLabel* welcomeSubText = new QLabel("在左侧列表中点击联系人即可开始对话");
    welcomeSubText->setAlignment(Qt::AlignCenter);
    welcomeSubText->setStyleSheet("font-size: 14px; color: #999999; margin-top: 10px;");
    welcomeLayout->addWidget(welcomeSubText);

    m_chatStack = new QStackedWidget();
    m_chatStack->addWidget(welcomeWidget);
    
    chatLayout->addWidget(m_chatStack);
    splitter->addWidget(chatContainer);
}

void MainWindow::onContactSelected(const QString& contactId)
{
    if (!m_chatWindows.contains(contactId)) {
        ChatWindow* chatWindow = new ChatWindow(contactId);
        m_chatWindows[contactId] = chatWindow;
        m_chatStack->addWidget(chatWindow);
    }
    m_chatStack->setCurrentWidget(m_chatWindows[contactId]);

    // 取消高亮
    m_contactList->highlightContact(contactId, false);
}

void MainWindow::onAddContactClicked()
{
    InputWindow dlg(this);
    dlg.setWindowTitle("添加联系人");
    if (dlg.exec() == QDialog::Accepted) {
        QString username = dlg.getInputText();
        if (UserManager::getInstance().addContact(username)) {
            m_contactList->refreshContacts();
        } else {
            QMessageBox::warning(this, "错误", "添加联系人失败");
        }
    }
}

void MainWindow::onMinimizeClicked()
{
    showMinimized();
}

void MainWindow::onMaximizeClicked()
{
    if (isMaximized()) {
        showNormal();
        m_maxButton->setText("☐");  // 最大化符号
    } else {
        showMaximized();
        m_maxButton->setText("❐");  // 还原符号
    }
}

void MainWindow::onCloseClicked()
{
    close();
}

void MainWindow::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        m_dragging = true;
        m_dragPosition = event->globalPos() - frameGeometry().topLeft();
        event->accept();
    }
}

void MainWindow::mouseMoveEvent(QMouseEvent *event)
{
    if (m_dragging && (event->buttons() & Qt::LeftButton)) {
        move(event->globalPos() - m_dragPosition);
        event->accept();
    }
}

void MainWindow::mouseReleaseEvent(QMouseEvent *event)
{
    m_dragging = false;
    event->accept();
}

void MainWindow::refreshMyAvatar()
{
    QString avatarPath = UserManager::getInstance().getCurrentUser().getAvatar();
    if (!avatarPath.isEmpty() && QFile::exists(avatarPath)) {
        QPixmap avatarPixmap(avatarPath);
        if(!avatarPixmap.isNull()) {
            // 创建圆形头像
            QPixmap rounded(36, 36);
            rounded.fill(Qt::transparent);
            QPainter painter(&rounded);
            painter.setRenderHint(QPainter::Antialiasing);
            painter.setPen(Qt::NoPen);
            painter.setBrush(QBrush(avatarPixmap.scaled(36, 36, Qt::KeepAspectRatio, Qt::SmoothTransformation)));
            painter.drawEllipse(0, 0, 36, 36);
            m_myAvatarLabel->setPixmap(rounded);
        }
    } else {
        m_myAvatarLabel->setPixmap(QPixmap(":/icons/default-avatar.png").scaled(36, 36, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }
}

void MainWindow::onDeleteContactClicked(const QString& contactName)
{
    DeleteWindow dlg(this, contactName);
    dlg.setWindowTitle("删除联系人");
    if (dlg.exec() == QDialog::Accepted && dlg.isConfirmed()) {
        // ...删除逻辑...
    }
}

void MainWindow::onSettingsClicked()
{
    SettingsDialog dlg(this);

    if (dlg.exec() == QDialog::Accepted) {
        bool darkMode = dlg.isDarkMode();
        int fontSize = dlg.fontSize();
        // 保存设置
        QSettings settings("YourCompany", "YourApp");
        settings.setValue("fontSize", fontSize);
        settings.setValue("darkMode", darkMode);
        settings.setValue("downloadPath", dlg.downloadPath());
        // 应用全局样式
        applyAppStyle(darkMode, fontSize);
    }
}

// 添加退出登录方法
void MainWindow::onLogoutClicked()
{
    // 弹出确认对话框
    QMessageBox::StandardButton reply = QMessageBox::question(
        this, 
        "退出登录", 
        "确定要退出登录吗？",
        QMessageBox::Yes | QMessageBox::No
    );
    
    if (reply == QMessageBox::Yes) {
        // 执行退出登录逻辑
        UserManager::getInstance().logout();
        
        // 显示退出动画
        QPropertyAnimation* anim = new QPropertyAnimation(this, "windowOpacity");
        anim->setDuration(400);
        anim->setStartValue(1.0);
        anim->setEndValue(0.0);
        connect(anim, &QPropertyAnimation::finished, this, [=](){
            // 创建并显示登录窗口
            LoginWindow* loginWindow = new LoginWindow();
            loginWindow->show();
            // 关闭当前窗口
            close();
        });
        anim->start(QAbstractAnimation::DeleteWhenStopped);
    }
}