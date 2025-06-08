#include "MainWindow.h"
#include "core/UserManager.h"
#include "ContactListWidget.h"
#include "ChatWindow.h"
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
    setStyleSheet("QMainWindow { background: #F7F7F7; }");
    
    // 添加阴影效果
    QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect(this);
    shadow->setBlurRadius(20);
    shadow->setOffset(0, 6);
    shadow->setColor(QColor(0, 0, 0, 80));
    setGraphicsEffect(shadow);
    
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
        // 判断对话的另一方是谁
        if (senderId == myId) {
            // 我发送的消息，对话窗口应该是接收者
            contactId = receiverId;
        } else {
            // 别人发送给我的消息，对话窗口应该是发送者
            contactId = senderId;
        }
        
        // 分发到对应的聊天窗口
        if (m_chatWindows.contains(contactId)) {
            m_chatWindows[contactId]->addMessage(message);
        }
    });

    connect(m_refreshButton, &QPushButton::clicked, this, [this](){
        ChatWindow* currentChat = qobject_cast<ChatWindow*>(m_chatStack->currentWidget());
        if (currentChat) {
            currentChat->refreshMessages();
        }
    });
}

void MainWindow::setupUI()
{
    setMinimumSize(1000, 700);

    // 主容器
    QWidget* centralWidget = new QWidget();
    centralWidget->setStyleSheet("background: #F7F7F7;");
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
    titleBar->setFixedHeight(45);
    titleBar->setStyleSheet("background: #FFFFFF; border-bottom: 1px solid #E6E6E6;");
    
    QHBoxLayout* titleLayout = new QHBoxLayout(titleBar);
    titleLayout->setContentsMargins(15, 0, 15, 0);
    titleLayout->setSpacing(0);

    // 左侧Logo和标题
    QHBoxLayout* leftLayout = new QHBoxLayout();
    leftLayout->setContentsMargins(12, 0, 0, 0);
    leftLayout->setSpacing(8);
    
    m_myAvatarLabel = new QLabel();
    m_myAvatarLabel->setFixedSize(48, 48);
    m_myAvatarLabel->setStyleSheet("border-radius: 24px; background: #eee;");
    QString avatarPath = UserManager::getInstance().getCurrentUser().getAvatar();
    if (!avatarPath.isEmpty() && QFile::exists(avatarPath)) {
        m_myAvatarLabel->setPixmap(QPixmap(avatarPath).scaled(48, 48, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    } else {
        m_myAvatarLabel->setPixmap(QPixmap(":/icons/logo").scaled(48, 48, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }
    leftLayout->addWidget(m_myAvatarLabel);

    m_editAvatarBtn = new QPushButton("更换头像");
    m_editAvatarBtn->setCursor(Qt::PointingHandCursor);
    m_editAvatarBtn->setStyleSheet(
        "QPushButton { background: #F7F7F7; border: 1.5px solid #E3E3E3; border-radius: 8px; color: #222; font-size: 14px; padding: 6px 16px; }"
        "QPushButton:hover { background: #E1F6EF; border-color: #07C160; color: #07C160; }"
    );
    connect(m_editAvatarBtn, &QPushButton::clicked, this, [=](){
        QString file = QFileDialog::getOpenFileName(this, "选择头像", "", "图片文件 (*.png *.jpg *.jpeg)");
        if (!file.isEmpty()) {
            m_myAvatarLabel->setPixmap(QPixmap(file).scaled(48, 48, Qt::KeepAspectRatio, Qt::SmoothTransformation));
            UserManager::getInstance().updateCurrentUserAvatar(file);
            refreshMyAvatar();
        }
    });
    leftLayout->addWidget(m_editAvatarBtn);

    leftLayout->addStretch();
    titleLayout->addLayout(leftLayout);

    // 右侧控制按钮
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->setSpacing(8);

    // 设置按钮用⚙️字符
    m_settingsButton = new QPushButton("⚙️");
    m_settingsButton->setFixedSize(32, 32);
    m_settingsButton->setStyleSheet(
        "QPushButton { "
        "   background: transparent; "
        "   border: none; "
        "   border-radius: 6px; "
        "   font-size: 20px; "
        "   color: #666666; "
        "} "
        "QPushButton:hover { "
        "   background: #E1F6EF; "
        "   color: #07C160; "
        "}"
    );
    connect(m_settingsButton, &QPushButton::clicked, this, &MainWindow::onSettingsClicked);
    buttonLayout->addWidget(m_settingsButton);

    m_minButton = new QPushButton("–");
    m_minButton->setFixedSize(32, 32);
    m_minButton->setStyleSheet(
        "QPushButton { "
        "   background: transparent; "
        "   border: none; "
        "   border-radius: 6px; "
        "   font-size: 16px; "
        "   color: #666666; "
        "} "
        "QPushButton:hover { "
        "   background: #F0F0F0; "
        "}"
    );
    connect(m_minButton, &QPushButton::clicked, this, &MainWindow::onMinimizeClicked);
    buttonLayout->addWidget(m_minButton);

    m_closeButton = new QPushButton("×");
    m_closeButton->setFixedSize(32, 32);
    m_closeButton->setStyleSheet(
        "QPushButton { "
        "   background: transparent; "
        "   border: none; "
        "   border-radius: 6px; "
        "   font-size: 20px; "
        "   color: #666666; "
        "} "
        "QPushButton:hover { "
        "   background: #FF5F56; "
        "   color: white; "
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
    splitter->setHandleWidth(0);
    splitter->setChildrenCollapsible(false);

    // 左侧面板
    setupLeftPanel(splitter);
    
    // 右侧聊天区域
    setupChatArea(splitter);

    // 设置分割器比例
    splitter->setSizes({300, 700});
    
    mainLayout->addWidget(splitter);
}

void MainWindow::setupLeftPanel(QSplitter* splitter)
{
    QWidget* leftPanel = new QWidget();
    leftPanel->setFixedWidth(300);
    leftPanel->setStyleSheet("background: #FFFFFF; border-right: 1px solid #E6E6E6;");
    
    QVBoxLayout* leftLayout = new QVBoxLayout(leftPanel);
    leftLayout->setContentsMargins(0, 0, 0, 0);
    leftLayout->setSpacing(0);

    // 导航栏
    setupNavigationBar(leftLayout);
    
    // 搜索区域 - 重新添加胶囊形状搜索栏
    setupSearchArea(leftLayout);
    
    // 联系人列表
    m_contactList = new ContactListWidget();
    m_contactList->setStyleSheet(
        "ContactListWidget { "
        "   background: transparent; "
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
    navBar->setStyleSheet("background: #FFFFFF; border-bottom: 1px solid #F0F0F0;");
    
    QHBoxLayout* navLayout = new QHBoxLayout(navBar);
    navLayout->setContentsMargins(15, 0, 15, 0);
    navLayout->setSpacing(20);

    // 聊天按钮
    m_chatNavButton = new QPushButton("聊天");
    m_chatNavButton->setFixedSize(60, 35);
    m_chatNavButton->setStyleSheet(
        "QPushButton { "
        "   background: #07C160; "
        "   border: none; "
        "   border-radius: 6px; "
        "   color: white; "
        "   font-size: 14px; "
        "   font-weight: bold; "
        "} "
        "QPushButton:hover { background: #06AD56; }"
    );
    navLayout->addWidget(m_chatNavButton);

    // 联系人按钮
    m_contactNavButton = new QPushButton("联系人");
    m_contactNavButton->setFixedSize(60, 35);
    m_contactNavButton->setStyleSheet(
        "QPushButton { "
        "   background: transparent; "
        "   border: none; "
        "   border-radius: 6px; "
        "   color: #666666; "
        "   font-size: 14px; "
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
        "   background: #07C160; "
        "   border: none; "
        "   border-radius: 15px; "
        "   color: white; "
        "   font-size: 18px; "
        "   font-weight: bold; "
        "} "
        "QPushButton:hover { background: #06AD56; }"
    );
    connect(m_addContactButton, &QPushButton::clicked, this, &MainWindow::onAddContactClicked);
    navLayout->addWidget(m_addContactButton);

    leftLayout->addWidget(navBar);
}

void MainWindow::setupSearchArea(QVBoxLayout* leftLayout)
{
    QWidget* searchContainer = new QWidget();
    searchContainer->setFixedHeight(60);
    searchContainer->setStyleSheet("background: #FFFFFF;");
    
    QVBoxLayout* searchLayout = new QVBoxLayout(searchContainer);
    searchLayout->setContentsMargins(15, 10, 15, 10);
    
    m_searchEdit = new QLineEdit();
    m_searchEdit->setPlaceholderText("搜索联系人");
    m_searchEdit->setFixedHeight(36);
    m_searchEdit->setStyleSheet(
        "QLineEdit { "
        "   background: #F5F5F5; "
        "   border: 1px solid #E0E0E0; "
        "   border-radius: 18px; "  // 胶囊形状：高度的一半
        "   padding: 0 16px; "
        "   font-size: 14px; "
        "   color: #333333; "
        "} "
        "QLineEdit:focus { "
        "   border: 1px solid #07C160; "
        "   background: #FFFFFF; "
        "}"
    );
    connect(m_searchEdit, &QLineEdit::textChanged, this, [=](const QString& text){
        m_contactList->filterContacts(text);
    });
    searchLayout->addWidget(m_searchEdit);
    
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
    
    QLabel* welcomeIcon = new QLabel();
    welcomeIcon->setPixmap(QPixmap(":/icons/logo").scaled(120, 120, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    welcomeIcon->setAlignment(Qt::AlignCenter);
    welcomeLayout->addWidget(welcomeIcon);
    
    QLabel* welcomeText = new QLabel("请选择一个联系人开始聊天");
    welcomeText->setAlignment(Qt::AlignCenter);
    welcomeText->setStyleSheet("font-size: 16px; color: #999999; margin-top: 20px;");
    welcomeLayout->addWidget(welcomeText);

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
    m_contactList->highlightContact(contactId, false); // 你可以扩展 highlightContact 支持取消高亮
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
        m_myAvatarLabel->setPixmap(QPixmap(avatarPath).scaled(48, 48, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    } else {
        m_myAvatarLabel->setPixmap(QPixmap(":/icons/logo").scaled(48, 48, Qt::KeepAspectRatio, Qt::SmoothTransformation));
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