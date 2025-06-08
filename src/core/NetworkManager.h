#ifndef NETWORKMANAGER_H
#define NETWORKMANAGER_H

#include <QObject>
#include <QTcpSocket>
#include <QTcpServer>
#include <QHostAddress>
#include <QString>
#include <QByteArray>
#include "models/Message.h"

class NetworkManager : public QObject
{
    Q_OBJECT

public:
    static NetworkManager& getInstance();
    bool connectToServer(const QString& host, quint16 port);
    bool startServer(quint16 port);
    void sendMessage(const QString& message);
    void sendMessage(const Message& msg);
    void sendFile(const QString& filePath, const QString& receiverId);
    void sendImageMessage(const Message& imageMsg);
    void disconnect();

signals:
    void messageReceived(const Message& message);
    void connectionStatusChanged(bool connected);
    void errorOccurred(const QString& error);

private:
    explicit NetworkManager(QObject *parent = nullptr);
    ~NetworkManager();
    NetworkManager(const NetworkManager&) = delete;
    NetworkManager& operator=(const NetworkManager&) = delete;

    void processReceivedData(const QByteArray& data);
    QByteArray messageToJson(const Message& message);
    Message messageFromJson(const QByteArray& json);

    QTcpSocket* m_socket;
    QTcpServer* m_server;
    bool m_isServer;
};

#endif // NETWORKMANAGER_H