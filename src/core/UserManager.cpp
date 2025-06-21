#include "UserManager.h"
#include "DatabaseManager.h"
#include "utils/Logger.h"
#include <QUuid>
#include <QSqlQuery>
#include <QDateTime>
#include <QRandomGenerator>
#include <QCryptographicHash>
#include <QSqlError>

UserManager::UserManager()
{
}

UserManager& UserManager::getInstance()
{
    static UserManager instance;
    return instance;
}

bool UserManager::login(const QString& username, const QString& password)
{
    User user = DatabaseManager::getInstance().getUserById(username);

    if (user.getUserId().isEmpty()) {
        // 新用户，创建账户
        user.setUserId(QUuid::createUuid().toString(QUuid::WithoutBraces));
        user.setUsername(username);
        user.setNickname(username);
        user.setPassword(password); // 新增

        if (DatabaseManager::getInstance().insertUser(user)) {
            m_currentUser = user;
            emit loginSuccess();
            return true;
        } else {
            emit loginFailed("创建用户失败");
            return false;
        }
    } else {
        // 校验密码
        if (user.getPassword() != password) {
            emit loginFailed("密码错误");
            return false;
        }
        m_currentUser = user;
        emit loginSuccess();
        return true;
    }
}

void UserManager::logout()
{
    m_currentUser = User();
}

bool UserManager::addContact(const QString& contactId)
{
    if (!isLoggedIn()) {
        return false;
    }
    
    // 首先通过用户名查找用户
    User contact = DatabaseManager::getInstance().getUserById(contactId);
    if (contact.getUserId().isEmpty()) {
        Logger::getInstance().log("找不到用户: " + contactId, Logger::ERROR);
        return false;
    }
    
    // 检查是否已经是联系人
    if (DatabaseManager::getInstance().isContact(getCurrentUserId(), contact.getUserId())) {
        Logger::getInstance().log("该用户已经是联系人", Logger::WARNING);
        return false;
    }
    
    // 添加联系人
    if (DatabaseManager::getInstance().addContact(getCurrentUserId(), contact.getUserId())) {
        emit contactAdded(contact);
        return true;
    }
    
    return false;
}

bool UserManager::removeContact(const QString& contactId)
{
    if (!isLoggedIn()) {
        return false;
    }
    
    if (DatabaseManager::getInstance().removeContact(getCurrentUserId(), contactId)) {
        emit contactRemoved(contactId);
        return true;
    }
    
    return false;
}

QList<User> UserManager::getContacts()
{
    if (!isLoggedIn()) {
        return QList<User>();
    }
    
    return DatabaseManager::getInstance().getContacts(getCurrentUserId());
}

bool UserManager::registerUser(const QString& nickname, const QString& password, const QString& avatarPath)
{
    // 1. 验证输入
    if (nickname.isEmpty() || password.isEmpty()) {
        emit registerFailed("输入不能为空");
        return false;
    }

    // 2. 检查用户名是否已存在
    QSqlQuery query(DatabaseManager::getInstance().getDatabase());
    query.prepare("SELECT user_id FROM users WHERE nickname = ?");
    query.addBindValue(nickname);
    
    if (!query.exec()) {
        emit registerFailed("数据库查询失败: " + query.lastError().text());
        return false;
    }
    
    if (query.next()) {
        emit registerFailed("用户名已被注册");
        return false;
    }

    // 3. 插入新用户
    QString userId = QUuid::createUuid().toString(QUuid::WithoutBraces);

    query.prepare("INSERT INTO users (user_id, username, nickname, password, avatar) VALUES (?, ?, ?, ?, ?)");
    query.addBindValue(userId);
    query.addBindValue(nickname);  // 用昵称作为用户名
    query.addBindValue(nickname);
    query.addBindValue(password);
    query.addBindValue(avatarPath);

    if (!query.exec()) {
        emit registerFailed("注册失败: " + query.lastError().text());
        return false;
    }

    emit registerSuccess();
    return true;
}

// 生成随机盐值
QString UserManager::generateSalt()
{
    const int saltLength = 16;
    QString salt;
    const QString chars = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    
    for (int i = 0; i < saltLength; ++i) {
        int index = QRandomGenerator::global()->bounded(chars.length());
        salt.append(chars.at(index));
    }
    
    return salt;
}

// 密码哈希函数
QString UserManager::hashPassword(const QString& password, const QString& salt)
{
    QByteArray combined = (password + salt).toUtf8();
    QByteArray hash = QCryptographicHash::hash(combined, QCryptographicHash::Sha256);
    return QString(hash.toHex());
}

void UserManager::updateCurrentUserAvatar(const QString& avatarPath)
{
    if (!isLoggedIn()) return;
    QSqlQuery query(DatabaseManager::getInstance().getDatabase());
    query.prepare("UPDATE users SET avatar=? WHERE user_id=?");
    query.addBindValue(avatarPath);
    query.addBindValue(m_currentUser.getUserId());
    query.exec();
    m_currentUser.setAvatar(avatarPath);
}

User UserManager::getContactById(const QString& userId)
{
    return DatabaseManager::getInstance().getUserById(userId);
}

User UserManager::getUserById(const QString& userId)
{
    return DatabaseManager::getInstance().getUserById(userId);
}

bool UserManager::resetPassword(const QString& username, const QString& newPassword)
{
    if (username.isEmpty() || newPassword.isEmpty()) {
        emit resetPasswordFailed("用户名或密码不能为空");
        return false;
    }

    // 检查用户是否存在
    User user = DatabaseManager::getInstance().getUserById(username);
    if (user.getUserId().isEmpty()) {
        emit resetPasswordFailed("用户不存在");
        return false;
    }

    // 更新密码
    QSqlQuery query(DatabaseManager::getInstance().getDatabase());
    query.prepare("UPDATE users SET password=? WHERE username=?");
    query.addBindValue(newPassword);
    query.addBindValue(username);

    if (!query.exec()) {
        emit resetPasswordFailed("数据库错误: " + query.lastError().text());
        return false;
    }

    if (query.numRowsAffected() <= 0) {
        emit resetPasswordFailed("密码更新失败");
        return false;
    }

    emit resetPasswordSuccess();
    return true;
}