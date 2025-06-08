#ifndef USER_H
#define USER_H

#include <QString>
#include <QPixmap>
#include <QLabel>

class User
{
public:
    User();
    User(const QString& userId, const QString& username);
    
    // Getter methods
    QString getUserId() const { return m_userId; }
    QString getUsername() const { return m_username; }
    QString getNickname() const { return m_nickname; }
    QString getAvatar() const { return m_avatar; }
    
    // Setter methods
    void setUserId(const QString& userId) { m_userId = userId; }
    void setUsername(const QString& username) { m_username = username; }
    void setNickname(const QString& nickname) { m_nickname = nickname; }
    void setAvatar(const QString& avatar) { m_avatar = avatar; }
    
    // Operators
    bool operator==(const User& other) const;
    void setPassword(const QString& pwd) { m_password = pwd; }
    QString getPassword() const { return m_password; }

private:
    QString m_userId;
    QString m_username;
    QString m_nickname;
    QString m_avatar;
    QString m_password;
    QLabel* m_avatarLabel;
};

#endif // USER_H