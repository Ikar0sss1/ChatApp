#include "MessageBubble.h"
#include "core/UserManager.h"
#include <QFileInfo>

MessageBubble::MessageBubble(const Message& message, QWidget *parent)
    : QWidget(parent)
    , m_message(message)
{
    setupUI();
    updateContent();
}

void MessageBubble::setupUI()
{
    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->setContentsMargins(10, 5, 10, 5);
    
    bool isMe = isMyMessage();
    setupBubble(isMe);  // 调用新函数
    
    if (isMe) {
        layout->addStretch();
    }
    
    // 消息内容容器
    QWidget* bubbleWidget = new QWidget();
    QVBoxLayout* bubbleLayout = new QVBoxLayout(bubbleWidget);
    bubbleLayout->setContentsMargins(10, 8, 10, 8);
    
    // 消息内容
    m_contentLabel = new QLabel();
    m_contentLabel->setWordWrap(true);
    m_contentLabel->setMaximumWidth(300);
    bubbleLayout->addWidget(m_contentLabel);
    
    // 时间标签
    m_timeLabel = new QLabel(m_message.getFormattedTime());
    m_timeLabel->setStyleSheet("color: gray; font-size: 10px;");
    m_timeLabel->setAlignment(isMyMessage() ? Qt::AlignRight : Qt::AlignLeft);
    bubbleLayout->addWidget(m_timeLabel);
    
    // 设置样式
    if (isMyMessage()) {
        bubbleWidget->setStyleSheet(R"(
            QWidget {
                background-color: #007AFF;
                color: white;
                border-radius: 10px;
            }
        )");
    } else {
        bubbleWidget->setStyleSheet(R"(
            QWidget {
                background-color: #E5E5EA;
                color: black;
                border-radius: 10px;
            }
        )");
    }
    
    layout->addWidget(bubbleWidget);
    
    if (!isMyMessage()) {
        layout->addStretch();
    }
}

void MessageBubble::setupBubble(bool isMe)
{
    setObjectName("message-bubble");
    setProperty("class", isMe ? "me" : "you");
    
    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setContentsMargins(8, 8, 8, 8);
    
    if (!isMe) {
        QLabel *avatar = new QLabel();
        avatar->setPixmap(QPixmap(":/icons/default-avatar").scaled(32, 32, Qt::KeepAspectRatio));
        layout->addWidget(avatar);
    }
    
    QLabel *content = new QLabel(m_message.getContent());
    content->setWordWrap(true);
    content->setTextInteractionFlags(Qt::TextSelectableByMouse);
    layout->addWidget(content);
    
    if (isMe) {
        layout->addStretch();
    }
}

void MessageBubble::updateContent()
{
    switch (m_message.getType()) {
        case MessageType::Text:
            m_contentLabel->setText(m_message.getContent());
            break;
            
        case MessageType::File: {
            QFileInfo fileInfo(m_message.getFilePath());
            QString content = QString("📎 %1").arg(m_message.getFileName());
            m_contentLabel->setText(content);
            break;
        }
        
        case MessageType::Emoji:
            m_contentLabel->setText(m_message.getEmojiCode());
            m_contentLabel->setStyleSheet("font-size: 24px;");
            break;
    }
}

bool MessageBubble::isMyMessage() const
{
    return m_message.getSenderId() == UserManager::getInstance().getCurrentUserId();
}