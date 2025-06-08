#include "Message.h"
#include <QUuid>

Message::Message()
    : m_type(MessageType::Text)
    , m_timestamp(QDateTime::currentDateTime())
{
    generateMessageId();
}

Message::Message(const QString& senderId, const QString& receiverId, const QString& content)
    : m_senderId(senderId)
    , m_receiverId(receiverId)
    , m_content(content)
    , m_type(MessageType::Text)
    , m_timestamp(QDateTime::currentDateTime())
{
    generateMessageId();
}

void Message::generateMessageId()
{
    m_messageId = QUuid::createUuid().toString(QUuid::WithoutBraces);
}

QString Message::getFormattedTime() const
{
    QDateTime now = QDateTime::currentDateTime();
    if (m_timestamp.date() == now.date()) {
        return m_timestamp.toString("hh:mm");
    } else {
        return m_timestamp.toString("MM-dd hh:mm");
    }
}

bool Message::operator==(const Message& other) const
{
    return m_messageId == other.m_messageId;
}