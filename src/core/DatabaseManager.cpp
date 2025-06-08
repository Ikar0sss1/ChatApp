#include "DatabaseManager.h"
#include <QStandardPaths>
#include <QDir>
#include <QVariant>
#include "utils/Logger.h"

DatabaseManager::DatabaseManager()
    : m_initialized(false)
{
}

DatabaseManager::~DatabaseManager()
{
    if (m_database.isOpen()) {
        m_database.close();
    }
}

DatabaseManager& DatabaseManager::getInstance()
{
    static DatabaseManager instance;
    return instance;
}

bool DatabaseManager::initialize()
{
    if (m_initialized) {
        return true;
    }
    
    Logger::getInstance().log("正在初始化数据库连接...");
    
    // 使用 MySQL 数据库
    m_database = QSqlDatabase::addDatabase("QMYSQL");
    m_database.setHostName("localhost");
    m_database.setDatabaseName("chat");
    m_database.setUserName("root");
    m_database.setPassword("123456");
    
    if (!m_database.open()) {
        Logger::getInstance().log("数据库连接失败: " + m_database.lastError().text(), Logger::ERROR);
        return false;
    }
    
    // 创建数据库（如果不存在）
    QSqlQuery query(m_database);
    if (!query.exec("CREATE DATABASE IF NOT EXISTS chat CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci")) {
        Logger::getInstance().log("创建数据库失败: " + query.lastError().text(), Logger::ERROR);
        return false;
    }
    
    // 使用数据库
    if (!query.exec("USE chat")) {
        Logger::getInstance().log("切换数据库失败: " + query.lastError().text(), Logger::ERROR);
        return false;
    }
    
    if (!createTables()) {
        Logger::getInstance().log("创建表失败", Logger::ERROR);
        m_database.close();
        return false;
    }
    
    m_initialized = true;
    Logger::getInstance().log("数据库初始化成功");
    return true;
}

QString DatabaseManager::getDatabasePath()
{
    QString dataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    return dataPath + "/imchat.db";
}

bool DatabaseManager::createTables()
{
    QSqlQuery query(m_database);
    
    // 创建用户表，简化结构
    QString createUsersTable = R"(
        CREATE TABLE IF NOT EXISTS users (
            user_id VARCHAR(36) PRIMARY KEY,
            username VARCHAR(50) NOT NULL,
            nickname VARCHAR(50) NOT NULL,
            password VARCHAR(100) NOT NULL,
            created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
            UNIQUE (username),
            UNIQUE (nickname)
        ) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
    )";
    
    if (!query.exec(createUsersTable)) {
        Logger::getInstance().log("Failed to create users table: " + query.lastError().text(), Logger::ERROR);
        return false;
    }

    // 创建联系人表
    QString createContactsTable = R"(
        CREATE TABLE IF NOT EXISTS contacts (
            user_id VARCHAR(36) NOT NULL,
            contact_id VARCHAR(36) NOT NULL,
            created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
            PRIMARY KEY (user_id, contact_id),
            FOREIGN KEY (user_id) REFERENCES users(user_id) ON DELETE CASCADE,
            FOREIGN KEY (contact_id) REFERENCES users(user_id) ON DELETE CASCADE
        ) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
    )";
    
    if (!query.exec(createContactsTable)) {
        Logger::getInstance().log("Failed to create contacts table: " + query.lastError().text(), Logger::ERROR);
        return false;
    }

    // 创建消息表
    QString createMessagesTable = R"(
        CREATE TABLE IF NOT EXISTS messages (
            id INT AUTO_INCREMENT PRIMARY KEY,
            message_id VARCHAR(64),
            sender_id VARCHAR(36),
            receiver_id VARCHAR(36),
            content TEXT,
            message_type INT,
            timestamp VARCHAR(32),
            file_path TEXT,
            file_name TEXT,
            emoji_code TEXT,
            file_data LONGBLOB
        ) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
    )";
    if (!query.exec(createMessagesTable)) {
        Logger::getInstance().log("Failed to create messages table: " + query.lastError().text(), Logger::ERROR);
        return false;
    }

    return true;
}

bool DatabaseManager::insertUser(const User& user)
{
    QSqlQuery query(m_database);
    query.prepare("INSERT INTO users (user_id, username, nickname, password) VALUES (?, ?, ?, ?)");
    query.addBindValue(user.getUserId());
    query.addBindValue(user.getUsername());
    query.addBindValue(user.getNickname());
    query.addBindValue(user.getPassword()); // 新增

    return query.exec();
}

User DatabaseManager::getUserById(const QString& userId)
{
    QSqlQuery query(m_database);
    query.prepare("SELECT * FROM users WHERE user_id = ? OR username = ?");
    query.addBindValue(userId);
    query.addBindValue(userId);

    if (query.exec() && query.next()) {
        User user;
        user.setUserId(query.value("user_id").toString());
        user.setUsername(query.value("username").toString());
        user.setNickname(query.value("nickname").toString());
        user.setPassword(query.value("password").toString());
        user.setAvatar(query.value("avatar").toString());
        return user;
    }

    return User();
}

bool DatabaseManager::insertMessage(const Message& message)
{
    QSqlQuery query(m_database);
    query.prepare(R"(
        INSERT INTO messages (message_id, sender_id, receiver_id, content, message_type, 
                             timestamp, file_path, file_name, emoji_code, file_data)
        VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?)
    )");
    
    query.addBindValue(message.getMessageId());
    query.addBindValue(message.getSenderId());
    query.addBindValue(message.getReceiverId());
    query.addBindValue(message.getContent());
    query.addBindValue(static_cast<int>(message.getType()));
    query.addBindValue(message.getTimestamp().toString(Qt::ISODate));
    query.addBindValue(message.getFilePath());
    query.addBindValue(message.getFileName());
    query.addBindValue(message.getEmojiCode());
    query.addBindValue(message.getFileData());

    return query.exec();
}

QList<Message> DatabaseManager::getMessagesBetweenUsers(const QString& user1, const QString& user2)
{
    QList<Message> messages;
    QSqlQuery query(m_database);
    query.prepare(R"(
        SELECT * FROM messages 
        WHERE (sender_id = ? AND receiver_id = ?) OR (sender_id = ? AND receiver_id = ?)
        ORDER BY timestamp ASC
    )");
    query.addBindValue(user1);
    query.addBindValue(user2);
    query.addBindValue(user2);
    query.addBindValue(user1);
    
    if (query.exec()) {
        while (query.next()) {
            Message message;
            message.setMessageId(query.value("message_id").toString());
            message.setSenderId(query.value("sender_id").toString());
            message.setReceiverId(query.value("receiver_id").toString());
            message.setContent(query.value("content").toString());
            message.setType(static_cast<MessageType>(query.value("message_type").toInt()));
            message.setTimestamp(QDateTime::fromString(query.value("timestamp").toString(), Qt::ISODate));
            message.setFilePath(query.value("file_path").toString());
            message.setFileName(query.value("file_name").toString());
            message.setEmojiCode(query.value("emoji_code").toString());
            message.setFileData(query.value("file_data").toByteArray());
            
            messages.append(message);
        }
    }
    
    return messages;
}

bool DatabaseManager::addContact(const QString& userId, const QString& contactId)
{
    QSqlQuery query(m_database);
    query.prepare("INSERT INTO contacts (user_id, contact_id) VALUES (?, ?) ON DUPLICATE KEY UPDATE user_id = user_id");
    query.addBindValue(userId);
    query.addBindValue(contactId);
    
    return query.exec();
}

bool DatabaseManager::removeContact(const QString& userId, const QString& contactId)
{
    QSqlQuery query(m_database);
    query.prepare("DELETE FROM contacts WHERE user_id = ? AND contact_id = ?");
    query.addBindValue(userId);
    query.addBindValue(contactId);
    
    return query.exec();
}

QList<User> DatabaseManager::getContacts(const QString& userId)
{
    QList<User> contacts;
    QSqlQuery query(m_database);
    query.prepare(R"(
        SELECT u.* FROM users u
        INNER JOIN contacts c ON u.user_id = c.contact_id
        WHERE c.user_id = ?
    )");
    query.addBindValue(userId);
    
    if (query.exec()) {
        while (query.next()) {
            User user;
            user.setUserId(query.value("user_id").toString());
            user.setUsername(query.value("username").toString());
            user.setNickname(query.value("nickname").toString());
            user.setAvatar(query.value("avatar").toString());
            contacts.append(user);
        }
    }
    
    return contacts;
}

bool DatabaseManager::isContact(const QString& userId, const QString& contactId)
{
    QSqlQuery query(m_database);
    query.prepare("SELECT COUNT(*) FROM contacts WHERE user_id = ? AND contact_id = ?");
    query.addBindValue(userId);
    query.addBindValue(contactId);
    
    if (query.exec() && query.next()) {
        return query.value(0).toInt() > 0;
    }
    
    return false;
}
