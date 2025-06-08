#include "MessageHandler.h"
#include "NetworkManager.h"
#include "UserManager.h"
#include "DatabaseManager.h"
#include <QFileInfo>
#include "utils/Logger.h"
#include <QUuid>

MessageHandler& MessageHandler::getInstance()
{
    static MessageHandler instance;
    return instance;
}

MessageHandler::MessageHandler(QObject *parent)
    : QObject(parent)
{
    connect(&NetworkManager::getInstance(), &NetworkManager::messageReceived,
            this, &MessageHandler::messageReceived);
}

MessageHandler::~MessageHandler()
{
}

void MessageHandler::sendMessage(const QString& content, const QString& receiverId)
{
    Message msg;
    msg.setMessageId(QUuid::createUuid().toString(QUuid::WithoutBraces));
    msg.setContent(content);
    msg.setType(MessageType::Text);
    msg.setSenderId(UserManager::getInstance().getCurrentUserId());
    msg.setReceiverId(receiverId);
    msg.setTimestamp(QDateTime::currentDateTime());
    NetworkManager::getInstance().sendMessage(msg);
    emit messageSent(content);
}

void MessageHandler::sendFile(const QString& filePath, const QString& receiverId)
{
    // TODO: 实现文件发送功能
    Logger::getInstance().log("File sending not implemented yet");
}

void MessageHandler::sendEmoji(const QString& emoji, const QString& receiverId)
{
    Message msg;
    msg.setMessageId(QUuid::createUuid().toString(QUuid::WithoutBraces));
    msg.setContent(emoji);
    msg.setType(MessageType::Emoji);
    msg.setSenderId(UserManager::getInstance().getCurrentUserId());
    msg.setReceiverId(receiverId);
    msg.setTimestamp(QDateTime::currentDateTime());
    NetworkManager::getInstance().sendMessage(msg);
    emit messageSent(emoji);
}

QList<Message> MessageHandler::getChatHistory(const QString& contactId)
{
    QString currentUserId = UserManager::getInstance().getCurrentUserId();
    return DatabaseManager::getInstance().getMessagesBetweenUsers(currentUserId, contactId);
}

void MessageHandler::onNetworkMessageReceived(const Message& message)
{
    saveMessage(message);
    emit messageReceived(message);
}

void MessageHandler::saveMessage(const Message& message)
{
    DatabaseManager::getInstance().insertMessage(message);
}