#ifndef MESSAGEBUBBLE_H
#define MESSAGEBUBBLE_H

#include <QWidget>
#include <QLabel>
#include <QHBoxLayout>
#include "models/Message.h"

class MessageBubble : public QWidget
{
    Q_OBJECT
    
public:
    explicit MessageBubble(const Message& message, QWidget *parent = nullptr);
    
private:
    void setupUI();
    void setupBubble(bool isMe);  // 添加新函数声明
    void updateContent();
    bool isMyMessage() const;
    
    Message m_message;
    QLabel* m_contentLabel;
    QLabel* m_timeLabel;
};

#endif // MESSAGEBUBBLE_H