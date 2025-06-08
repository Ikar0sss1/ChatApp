#include "User.h"

User::User()
{
}

User::User(const QString& userId, const QString& username)
    : m_userId(userId)
    , m_username(username)
    , m_nickname(username)
{
}

bool User::operator==(const User& other) const
{
    return m_userId == other.m_userId;
}