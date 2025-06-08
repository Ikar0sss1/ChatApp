#include "NetworkManager.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QFileInfo>
#include <QFile>
#include "utils/Logger.h"
#include "models/Message.h"
#include "UserManager.h"
#include "MessageHandler.h"

NetworkManager& NetworkManager::getInstance()
{
    static NetworkManager instance;
    return instance;
}

NetworkManager::NetworkManager(QObject *parent)
    : QObject(parent)
    , m_socket(new QTcpSocket(this))
    , m_server(new QTcpServer(this))
    , m_isServer(false)
{
    connect(m_socket, &QTcpSocket::connected, this, [this]() {
        emit connectionStatusChanged(true);
    });

    connect(m_socket, &QTcpSocket::disconnected, this, [this]() {
        emit connectionStatusChanged(false);
    });

    connect(m_socket, &QTcpSocket::readyRead, this, [this]() {
        QByteArray data = m_socket->readAll();
        processReceivedData(data);
    });

    connect(m_socket, &QTcpSocket::errorOccurred, this, [this](QAbstractSocket::SocketError error) {
        emit errorOccurred(m_socket->errorString());
    });

    connect(m_server, &QTcpServer::newConnection, this, [this]() {
        m_socket = m_server->nextPendingConnection();
        connect(m_socket, &QTcpSocket::readyRead, this, [this]() {
            QByteArray data = m_socket->readAll();
            processReceivedData(data);
        });
        emit connectionStatusChanged(true);
    });
}

NetworkManager::~NetworkManager()
{
    disconnect();
}

bool NetworkManager::connectToServer(const QString& host, quint16 port)
{
    if (m_isServer) {
        m_server->close();
        m_isServer = false;
    }

    m_socket->connectToHost(host, port);
    return m_socket->waitForConnected(5000);
}

bool NetworkManager::startServer(quint16 port)
{
    if (!m_isServer) {
        m_socket->disconnectFromHost();
        m_isServer = true;
    }

    return m_server->listen(QHostAddress::Any, port);
}

void NetworkManager::sendMessage(const QString& message)
{
    if (m_socket->state() == QAbstractSocket::ConnectedState) {
        Message msg;
        msg.setMessageId(QUuid::createUuid().toString(QUuid::WithoutBraces));
        msg.setContent(message);
        msg.setType(MessageType::Text);
        msg.setSenderId(UserManager::getInstance().getCurrentUserId());
        msg.setTimestamp(QDateTime::currentDateTime());
        
        QByteArray data = messageToJson(msg);
        m_socket->write(data);
        m_socket->flush();
        
        // 保存消息到本地数据库
        MessageHandler::getInstance().saveMessage(msg);
    }
}

void NetworkManager::sendFile(const QString& filePath, const QString& receiverId)
{
    if (m_socket->state() == QAbstractSocket::ConnectedState) {
        QFile file(filePath);
        if (file.open(QIODevice::ReadOnly)) {
            QFileInfo fileInfo(filePath);
            QByteArray fileData = file.readAll(); // 读取文件内容

            Message fileMsg;
            fileMsg.setMessageId(QUuid::createUuid().toString(QUuid::WithoutBraces));
            fileMsg.setType(MessageType::File);
            fileMsg.setSenderId(UserManager::getInstance().getCurrentUserId());
            fileMsg.setReceiverId(receiverId);
            fileMsg.setFilePath(filePath);
            fileMsg.setFileName(fileInfo.fileName());
            fileMsg.setContent(QString("发送了文件: %1").arg(fileInfo.fileName()));
            fileMsg.setTimestamp(QDateTime::currentDateTime());
            fileMsg.setFileData(fileData); // 关键：设置二进制内容

            m_socket->write(messageToJson(fileMsg));
            m_socket->flush();

            // 保存消息到本地数据库
            MessageHandler::getInstance().saveMessage(fileMsg);

            file.close();
        }
    }
}

void NetworkManager::sendImageMessage(const Message& imageMsg)
{
    if (m_socket->state() == QAbstractSocket::ConnectedState) {
        m_socket->write(messageToJson(imageMsg));
        m_socket->flush();
        MessageHandler::getInstance().saveMessage(imageMsg);
    }
}

void NetworkManager::sendMessage(const Message& msg)
{
    if (m_socket->state() == QAbstractSocket::ConnectedState) {
        QByteArray data = messageToJson(msg);
        m_socket->write(data);
        m_socket->flush();
        // 保存消息到本地数据库
        MessageHandler::getInstance().saveMessage(msg);
    }
}

void NetworkManager::disconnect()
{
    if (m_isServer) {
        m_server->close();
    }
    m_socket->disconnectFromHost();
}

void NetworkManager::processReceivedData(const QByteArray& data)
{
    Message message = messageFromJson(data);
    if (!message.getMessageId().isEmpty()) {
        // 不要修改接收者ID，保持原始的消息结构
        // message.setReceiverId(UserManager::getInstance().getCurrentUserId()); // 删除这行
        
        // 保存消息到本地数据库
        MessageHandler::getInstance().saveMessage(message);
        emit messageReceived(message);
    }
}

QByteArray NetworkManager::messageToJson(const Message& message)
{
    QJsonObject json;
    json["messageId"] = message.getMessageId();
    json["senderId"] = message.getSenderId();
    json["receiverId"] = message.getReceiverId();
    json["content"] = message.getContent();
    json["type"] = static_cast<int>(message.getType());
    json["timestamp"] = message.getTimestamp().toString(Qt::ISODate);
    json["filePath"] = message.getFilePath();
    json["fileName"] = message.getFileName();
    json["emojiCode"] = message.getEmojiCode();
    if (!message.getFileData().isEmpty())
        json["fileData"] = QString::fromLatin1(message.getFileData().toBase64());
    else
        json["fileData"] = QString();

    QJsonDocument doc(json);
    return doc.toJson(QJsonDocument::Compact) + "\n";
}

Message NetworkManager::messageFromJson(const QByteArray& json)
{
    QJsonDocument doc = QJsonDocument::fromJson(json);
    QJsonObject obj = doc.object();
    
    Message message;
    message.setMessageId(obj["messageId"].toString());
    message.setSenderId(obj["senderId"].toString());
    message.setReceiverId(obj["receiverId"].toString());
    message.setContent(obj["content"].toString());
    message.setType(static_cast<MessageType>(obj["type"].toInt()));
    message.setTimestamp(QDateTime::fromString(obj["timestamp"].toString(), Qt::ISODate));
    message.setFilePath(obj["filePath"].toString());
    message.setFileName(obj["fileName"].toString());
    message.setEmojiCode(obj["emojiCode"].toString());
    if (obj.contains("fileData") && obj["fileData"].isString()) {
        QByteArray fileData = QByteArray::fromBase64(obj["fileData"].toString().toLatin1());
        message.setFileData(fileData);
    }
    return message;
}

