#ifndef USERMANAGER_H
#define USERMANAGER_H

#include <QObject>
#include "models/User.h"

class UserManager : public QObject
{
    Q_OBJECT
    
public:
    static UserManager& getInstance();
    
    bool login(const QString& username, const QString& password);
    void logout();
    
    User getCurrentUser() const { return m_currentUser; }
    QString getCurrentUserId() const { return m_currentUser.getUserId(); }
    bool isLoggedIn() const { return !m_currentUser.getUserId().isEmpty(); }
    
    // Contact management
    bool addContact(const QString& contactId);
    bool removeContact(const QString& contactId);
    QList<User> getContacts();
    
    bool registerUser(const QString& nickname, const QString& password, const QString& avatarPath);
    void updateCurrentUserAvatar(const QString& avatarPath);
    
    // 密码重置
    bool resetPassword(const QString& username, const QString& newPassword);
    
    User getContactById(const QString& userId);
    User getUserById(const QString& userId);

signals:
    void loginSuccess();
    void loginFailed(const QString& error);
    void contactAdded(const User& contact);
    void contactRemoved(const QString& contactId);
    void registerSuccess();
    void registerFailed(const QString& error);
    void resetPasswordSuccess();
    void resetPasswordFailed(const QString& error);

private:
    UserManager();
    
    QString generateSalt();
    QString hashPassword(const QString& password, const QString& salt);
    
    User m_currentUser;
};

#endif // USERMANAGER_H