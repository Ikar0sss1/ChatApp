#include "ChatWindow.h"
#include "MessageBubble.h"
#include "core/MessageHandler.h"
#include "core/UserManager.h"
#include <QFileDialog>
#include <QMenu>
#include <QAction>
#include <QTimer>
#include <QScrollBar>
#include <QGraphicsDropShadowEffect>
#include <QMouseEvent>
#include <QMessageBox>
#include <QDateTime>
#include "utils/Logger.h"
#include <QPainter>
#include <QUuid>
#include "DragImageTextEdit.h"
#include <QDebug>

ChatWindow::ChatWindow(const QString& contactId, QWidget *parent)
    : QWidget(parent)
    , m_contactId(contactId)
{
    // 移除透明属性
    setWindowFlags(Qt::FramelessWindowHint | Qt::Window);
    // setAttribute(Qt::WA_TranslucentBackground); // 删除这行
    
    // 设置背景色为白色
    setStyleSheet("ChatWindow { background: white; }");
    
    setupUI();
    loadChatHistory();
    
    connectToServer();
    
    // 连接网络信号
    connect(&NetworkManager::getInstance(), &NetworkManager::connectionStatusChanged,
            this, &ChatWindow::onConnectionStatusChanged);
    connect(&NetworkManager::getInstance(), &NetworkManager::errorOccurred,
            this, &ChatWindow::onErrorOccurred);

    connect(static_cast<DragImageTextEdit*>(m_inputEdit), &DragImageTextEdit::imageDropped, this, [this](const QString& filePath){
        // 直接调用你的图片发送逻辑
        QFile file(filePath);
        if (!file.open(QIODevice::ReadOnly)) return;
        QByteArray imageData = file.readAll();
        file.close();

        Message msg;
        msg.setMessageId(QUuid::createUuid().toString(QUuid::WithoutBraces));
        msg.setType(MessageType::Image);
        msg.setSenderId(UserManager::getInstance().getCurrentUserId());
        msg.setReceiverId(m_contactId);
        msg.setFileName(QFileInfo(filePath).fileName());
        msg.setContent("[图片] " + QFileInfo(filePath).fileName());
        msg.setTimestamp(QDateTime::currentDateTime());
        msg.setFileData(imageData);

        NetworkManager::getInstance().sendMessage(msg);
    });
}

ChatWindow::~ChatWindow()
{
    NetworkManager::getInstance().disconnect();
}

void ChatWindow::setupUI()
{
    // 创建主布局
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    
    // 创建状态标签
    m_statusLabel = new QLabel(this);
    m_statusLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(m_statusLabel);
    
    // 创建消息显示区域
    m_scrollArea = new QScrollArea(this);
    m_scrollArea->setWidgetResizable(true);
    m_messageContainer = new QWidget(m_scrollArea);
    m_messageLayout = new QVBoxLayout(m_messageContainer);
    m_messageLayout->setAlignment(Qt::AlignTop);
    m_scrollArea->setWidget(m_messageContainer);
    mainLayout->addWidget(m_scrollArea);
    
    // 创建输入区域
    QHBoxLayout* inputLayout = new QHBoxLayout();
    inputLayout->setContentsMargins(8, 8, 8, 8);
    inputLayout->setSpacing(8);
    m_inputEdit = new DragImageTextEdit();
    m_inputEdit->setPlaceholderText("输入消息...");
    m_inputEdit->setMaximumHeight(100);
    m_inputEdit->setStyleSheet(
        "QTextEdit { "
        "   border: 1px solid #E0E0E0; "
        "   border-radius: 4px; "
        "   padding: 8px; "
        "   font-size: 14px; "
        "}"
    );
    inputLayout->addWidget(m_inputEdit);

    // 表情按钮
    m_emojiButton = new QPushButton(this);
    m_emojiButton->setFixedSize(40, 40);
    m_emojiButton->setText("表情"); // 或 setIcon(QIcon(":/icons/emoji.png"));
    m_emojiButton->setStyleSheet(
        "QPushButton { "
        "   background: #07C160; "
        "   border: none; "
        "   border-radius: 8px; "
        "   color: white; "
        "   font-size: 16px; "
        "} "
        "QPushButton:hover { background: #06AD56; }"
    );
    connect(m_emojiButton, &QPushButton::clicked, this, &ChatWindow::onEmojiClicked);
    inputLayout->addWidget(m_emojiButton);

    // 图片按钮
    m_imageButton = new QPushButton(this);
    m_imageButton->setFixedSize(40, 40);
    m_imageButton->setText("图片");
    m_imageButton->setStyleSheet(
        "QPushButton { "
        "   background: #07C160; "
        "   border: none; "
        "   border-radius: 8px; "
        "   color: white; "
        "   font-size: 16px; "
        "} "
        "QPushButton:hover { background: #06AD56; }"
    );
    connect(m_imageButton, &QPushButton::clicked, this, &ChatWindow::onImageClicked);
    inputLayout->addWidget(m_imageButton);

    // 文件按钮
    m_fileButton = new QPushButton(this);
    m_fileButton->setFixedSize(40, 40);
    m_fileButton->setText("文件"); // 或 setIcon(QIcon(":/icons/file.png"));
    m_fileButton->setStyleSheet(
        "QPushButton { "
        "   background: #07C160; "
        "   border: none; "
        "   border-radius: 8px; "
        "   color: white; "
        "   font-size: 16px; "
        "} "
        "QPushButton:hover { background: #06AD56; }"
    );
    connect(m_fileButton, &QPushButton::clicked, this, &ChatWindow::onFileClicked);
    inputLayout->addWidget(m_fileButton);

    // 发送按钮
    m_sendButton = new QPushButton("发送");
    m_sendButton->setFixedSize(80, 40);
    m_sendButton->setStyleSheet(
        "QPushButton { "
        "   background: #07C160; "
        "   border: none; "
        "   border-radius: 4px; "
        "   color: white; "
        "   font-size: 14px; "
        "} "
        "QPushButton:hover { background: #06AD56; }"
    );
    connect(m_sendButton, &QPushButton::clicked, this, &ChatWindow::onSendClicked);
    inputLayout->addWidget(m_sendButton);

    mainLayout->addLayout(inputLayout);

    // 创建表情选择对话框
    m_emojiDialog = new EmojiDialog(this);
    connect(m_emojiDialog, &EmojiDialog::emojiSelected, this, &ChatWindow::onEmojiSelected);

    QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect(this);
    shadow->setBlurRadius(18); // 建议18以内
    shadow->setOffset(0, 4);
    shadow->setColor(QColor(0, 0, 0, 60));
    setGraphicsEffect(shadow);

    setMinimumSize(400, 500); // 保证窗口有足够空间
}

void ChatWindow::loadChatHistory()
{
    QList<Message> messages = MessageHandler::getInstance().getChatHistory(m_contactId);
    for (const Message& message : messages) {
        addMessage(message);
    }
}

void ChatWindow::addMessage(const Message& message)
{
    static QDateTime lastTime;
    bool isOutgoing = message.getSenderId() == UserManager::getInstance().getCurrentUserId();

    // 时间戳
    if (!lastTime.isValid() || lastTime.secsTo(message.getTimestamp()) > 300) {
        QLabel* timeLabel = new QLabel(message.getTimestamp().toString("yyyy/MM/dd HH:mm"));
        timeLabel->setAlignment(Qt::AlignCenter);
        timeLabel->setStyleSheet("color: #999; font-size: 12px; margin: 16px 0 8px 0;");
        m_messageLayout->addWidget(timeLabel);
        lastTime = message.getTimestamp();
    }

    QHBoxLayout* rowLayout = new QHBoxLayout();
    rowLayout->setAlignment(isOutgoing ? Qt::AlignRight : Qt::AlignLeft);

    QLabel* avatarLabel = new QLabel();
    avatarLabel->setFixedSize(40, 40);
    QString avatarPath = isOutgoing ? UserManager::getInstance().getCurrentUser().getAvatar()
                                    : UserManager::getInstance().getContactById(m_contactId).getAvatar();
    QPixmap avatarPixmap = avatarPath.isEmpty() ? QPixmap(40,40) : QPixmap(avatarPath).scaled(40,40,Qt::KeepAspectRatio,Qt::SmoothTransformation);
    if (avatarPixmap.isNull()) { avatarPixmap = QPixmap(40,40); avatarPixmap.fill(Qt::gray); }
    avatarLabel->setPixmap(avatarPixmap);

    QWidget* contentWidget = nullptr;

    if (message.getType() == MessageType::Image) {
        qDebug() << "收到图片消息:" << message.getFileName() << "数据大小:" << message.getFileData().size();
        QLabel* imageLabel = new QLabel();
        QPixmap pixmap;
        pixmap.loadFromData(message.getFileData());
        imageLabel->setPixmap(pixmap.scaled(200, 200, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        imageLabel->setStyleSheet("border-radius: 8px;");
        imageLabel->setCursor(Qt::PointingHandCursor);
        imageLabel->installEventFilter(this);
        contentWidget = imageLabel;
    } else if (message.getType() == MessageType::File) {
        qDebug() << "接收到文件消息:" << message.getFileName() << "数据大小:" << message.getFileData().size();
        QLabel* fileLabel = new QLabel(QString("<a href=\"#\">%1</a>").arg(message.getFileName()));
        fileLabel->setTextFormat(Qt::RichText);
        fileLabel->setTextInteractionFlags(Qt::TextBrowserInteraction);
        fileLabel->setOpenExternalLinks(false);
        fileLabel->setStyleSheet("background: #fff; color: #222; border-radius: 8px; padding: 10px 16px; border: 1px solid #F0F0F0; max-width: 380px;");
        connect(fileLabel, &QLabel::linkActivated, this, [=](){
            QString savePath = QFileDialog::getSaveFileName(this, "保存文件", message.getFileName());
            if (!savePath.isEmpty()) {
                QFile outFile(savePath);
                if (outFile.open(QIODevice::WriteOnly)) {
                    outFile.write(message.getFileData());
                    outFile.close();
                    QMessageBox::information(this, "成功", "文件已保存");
                } else {
                    QMessageBox::warning(this, "错误", "无法保存文件");
                }
            }
        });
        contentWidget = fileLabel;
    } else {
        QLabel* messageLabel = new QLabel(message.getContent());
        messageLabel->setWordWrap(true);
        messageLabel->setTextFormat(Qt::PlainText);
        messageLabel->setStyleSheet(isOutgoing ?
            "background: #95EC69; color: #222; border-radius: 8px; padding: 10px 16px; max-width: 380px;" :
            "background: #fff; color: #222; border-radius: 8px; padding: 10px 16px; border: 1px solid #F0F0F0; max-width: 380px;");
        contentWidget = messageLabel;
    }

    if (isOutgoing) {
        rowLayout->addStretch();
        rowLayout->addWidget(contentWidget);
        rowLayout->addWidget(avatarLabel);
    } else {
        rowLayout->addWidget(avatarLabel);
        rowLayout->addWidget(contentWidget);
        rowLayout->addStretch();
    }

    m_messageLayout->addLayout(rowLayout);

    // 滚动到底部
    QTimer::singleShot(0, [this]() {
        m_scrollArea->verticalScrollBar()->setValue(
            m_scrollArea->verticalScrollBar()->maximum()
        );
    });
}

void ChatWindow::onSendClicked()
{
    QString content = m_inputEdit->toPlainText().trimmed();
    if (!content.isEmpty()) {
        // 创建完整的消息对象
        Message msg;
        msg.setMessageId(QUuid::createUuid().toString(QUuid::WithoutBraces));
        msg.setType(MessageType::Text);  // 假设有这个枚举值
        msg.setSenderId(UserManager::getInstance().getCurrentUserId());
        msg.setReceiverId(m_contactId);
        msg.setContent(content);
        msg.setTimestamp(QDateTime::currentDateTime());
        
        // 发送消息到网络
        NetworkManager::getInstance().sendMessage(msg);
        
        // 立即在当前聊天窗口显示消息
        addMessage(msg);
        
        // 清空输入框
        m_inputEdit->clear();
    }
}

void ChatWindow::onEmojiClicked()
{
    QPoint globalPos = m_inputEdit->mapToGlobal(QPoint(m_inputEdit->width()/2 - m_emojiDialog->width()/2, m_inputEdit->height()));
    m_emojiDialog->move(globalPos);
    m_emojiDialog->show();
}

void ChatWindow::onEmojiSelected(const QString& emoji)
{
    // 在光标位置插入表情
    m_inputEdit->insertPlainText(emoji);
    m_inputEdit->setFocus();
}

void ChatWindow::onConnectionStatusChanged(bool connected)
{
    m_statusLabel->setText(connected ? "已连接" : "未连接");
    m_sendButton->setEnabled(connected);
}

void ChatWindow::onErrorOccurred(const QString& error)
{
    QMessageBox::warning(this, "网络错误", error);
}

void ChatWindow::connectToServer()
{
    QString currentUserId = UserManager::getInstance().getCurrentUserId();
    
    // 使用用户ID的哈希值来决定谁作为服务器
    // 这样可以确保两个用户之间的连接总是由同一个用户作为服务器
    bool isServer = currentUserId < m_contactId;
    
    if (isServer) {
        if (NetworkManager::getInstance().startServer(8080)) {
            m_statusLabel->setText("服务器已启动，等待连接...");
        } else {
            m_statusLabel->setText("服务器启动失败");
        }
    } else {
        if (NetworkManager::getInstance().connectToServer("localhost", 8080)) {
            m_statusLabel->setText("已连接到服务器");
        } else {
            m_statusLabel->setText("连接服务器失败");
        }
    }
}

void ChatWindow::onImageClicked()
{
    QString filePath = QFileDialog::getOpenFileName(this, "选择图片", "", "图片文件 (*.png *.jpg *.jpeg *.bmp *.gif)");
    if (filePath.isEmpty()) return;

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::warning(this, "错误", "无法打开图片");
        return;
    }
    QByteArray imageData = file.readAll();
    file.close();

    Message msg;
    msg.setMessageId(QUuid::createUuid().toString(QUuid::WithoutBraces));
    msg.setType(MessageType::Image);
    msg.setSenderId(UserManager::getInstance().getCurrentUserId());
    msg.setReceiverId(m_contactId);
    msg.setFileName(QFileInfo(filePath).fileName());
    msg.setContent("[图片] " + QFileInfo(filePath).fileName());
    msg.setTimestamp(QDateTime::currentDateTime());
    msg.setFileData(imageData);

    // 发送到网络
    NetworkManager::getInstance().sendMessage(msg);
    
    // 立即在当前窗口显示
    addMessage(msg);
}

void ChatWindow::onFileClicked()
{
    QString filePath = QFileDialog::getOpenFileName(this, "选择要发送的文件");
    if (filePath.isEmpty()) return;

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::warning(this, "错误", "无法打开文件");
        return;
    }
    QByteArray fileData = file.readAll();
    file.close();

    Message msg;
    msg.setMessageId(QUuid::createUuid().toString(QUuid::WithoutBraces));
    msg.setType(MessageType::File);
    msg.setSenderId(UserManager::getInstance().getCurrentUserId());
    msg.setReceiverId(m_contactId);
    msg.setFileName(QFileInfo(filePath).fileName());
    msg.setContent("[文件] " + QFileInfo(filePath).fileName());
    msg.setTimestamp(QDateTime::currentDateTime());
    msg.setFileData(fileData);

    NetworkManager::getInstance().sendMessage(msg);
}

bool ChatWindow::eventFilter(QObject* obj, QEvent* event)
{
    if (event->type() == QEvent::MouseButtonPress) {
        QLabel* imageLabel = qobject_cast<QLabel*>(obj);
        if (imageLabel) {
            QDialog* dialog = new QDialog(this);
            dialog->setWindowTitle("查看图片");
            QVBoxLayout* layout = new QVBoxLayout(dialog);
            QLabel* bigImage = new QLabel(dialog);
            bigImage->setPixmap(imageLabel->pixmap().scaled(800, 800, Qt::KeepAspectRatio, Qt::SmoothTransformation));
            layout->addWidget(bigImage);
            dialog->exec();
            delete dialog;
            return true;
        }
    }
    return QWidget::eventFilter(obj, event);
}

void ChatWindow::refreshMessages()
{
    // 清空消息区
    QLayoutItem* item;
    while ((item = m_messageLayout->takeAt(0)) != nullptr) {
        if (item->widget()) delete item->widget();
        delete item;
    }
    loadChatHistory();
}