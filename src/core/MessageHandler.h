#ifndef MESSAGEHANDLER_H
#define MESSAGEHANDLER_H

#include <QObject>
#include <QString>
#include "models/Message.h"

class MessageHandler : public QObject
{
    Q_OBJECT

public:
    static MessageHandler& getInstance();
    
    void sendMessage(const QString& content, const QString& receiverId);
    void sendFile(const QString& filePath, const QString& receiverId);
    void sendEmoji(const QString& emoji, const QString& receiverId);
    QList<Message> getChatHistory(const QString& contactId);
    void saveMessage(const Message& message);

signals:
    void messageReceived(const Message& message);
    void messageSent(const QString& content);

private:
    explicit MessageHandler(QObject *parent = nullptr);
    ~MessageHandler();
    MessageHandler(const MessageHandler&) = delete;
    MessageHandler& operator=(const MessageHandler&) = delete;

    void onNetworkMessageReceived(const Message& message);
};

#endif // MESSAGEHANDLER_H