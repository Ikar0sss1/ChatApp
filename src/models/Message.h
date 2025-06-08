#ifndef MESSAGE_H
#define MESSAGE_H

#include <QString>
#include <QDateTime>
#include <QByteArray>

enum class MessageType {
    Text,
    File,
    Image,
    Emoji
};

class Message
{
public:
    Message();
    Message(const QString& senderId, const QString& receiverId, const QString& content);
    
    // Getter methods
    QString getMessageId() const { return m_messageId; }
    QString getSenderId() const { return m_senderId; }
    QString getReceiverId() const { return m_receiverId; }
    QString getContent() const { return m_content; }
    MessageType getType() const { return m_type; }
    QDateTime getTimestamp() const { return m_timestamp; }
    QString getFilePath() const { return m_filePath; }
    QString getFileName() const { return m_fileName; }
    QString getEmojiCode() const { return m_emojiCode; }
    QByteArray getFileData() const { return m_fileData; }
    
    // Setter methods
    void setMessageId(const QString& messageId) { m_messageId = messageId; }
    void setSenderId(const QString& senderId) { m_senderId = senderId; }
    void setReceiverId(const QString& receiverId) { m_receiverId = receiverId; }
    void setContent(const QString& content) { m_content = content; }
    void setType(MessageType type) { m_type = type; }
    void setTimestamp(const QDateTime& timestamp) { m_timestamp = timestamp; }
    void setFilePath(const QString& filePath) { m_filePath = filePath; }
    void setFileName(const QString& fileName) { m_fileName = fileName; }
    void setEmojiCode(const QString& emojiCode) { m_emojiCode = emojiCode; }
    void setFileData(const QByteArray& data) { m_fileData = data; }
    
    // Utility methods
    QString getFormattedTime() const;
    bool isFileMessage() const { return m_type == MessageType::File; }
    bool isEmojiMessage() const { return m_type == MessageType::Emoji; }
    
    // Operators
    bool operator==(const Message& other) const;

private:
    QString m_messageId;
    QString m_senderId;
    QString m_receiverId;
    QString m_content;
    MessageType m_type;
    QDateTime m_timestamp;
    QString m_filePath;
    QString m_fileName;
    QString m_emojiCode;
    QByteArray m_fileData;
    
    void generateMessageId();
};

#endif // MESSAGE_H