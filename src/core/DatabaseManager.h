#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include "models/User.h"
#include "models/Message.h"

class DatabaseManager : public QObject
{
    Q_OBJECT
    
public:
    static DatabaseManager& getInstance();
    
    bool initialize();
    
    // User operations
    bool insertUser(const User& user);
    User getUserById(const QString& userId);
    QList<User> getAllUsers();
    
    // Message operations
    bool insertMessage(const Message& message);
    QList<Message> getMessagesBetweenUsers(const QString& user1, const QString& user2);
    
    // Contact operations
    bool addContact(const QString& userId, const QString& contactId);
    bool removeContact(const QString& userId, const QString& contactId);
    QList<User> getContacts(const QString& userId);
    bool isContact(const QString& userId, const QString& contactId);
    QSqlDatabase& getDatabase() { return m_database; }

private:
    DatabaseManager();
    ~DatabaseManager();
    
    bool createTables();
    QString getDatabasePath();
    
    QSqlDatabase m_database;
    bool m_initialized;
};

#endif // DATABASEMANAGER_H