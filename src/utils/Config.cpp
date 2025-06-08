#include "Config.h"
#include <QStandardPaths>
#include <QDir>

Config::Config()
    : m_settings(nullptr)
    , m_serverHost("localhost")
    , m_serverPort(8080)
{
}

Config::~Config()
{
    delete m_settings;
}

Config& Config::getInstance()
{
    static Config instance;
    return instance;
}

void Config::initialize()
{
    QString configPath = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    QDir().mkpath(configPath);
    
    m_settings = new QSettings(configPath + "/config.ini", QSettings::IniFormat);
    
    m_serverHost = m_settings->value("Server/host", "localhost").toString();
    m_serverPort = m_settings->value("Server/port", 8080).toInt();
    m_lastUsername = m_settings->value("User/lastUsername", "").toString();
    
    QString documentsPath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    m_downloadPath = m_settings->value("Paths/download", documentsPath + "/IMChat").toString();
    
    QDir().mkpath(m_downloadPath);
}

void Config::setLastUsername(const QString& username)
{
    m_lastUsername = username;
    if (m_settings) {
        m_settings->setValue("User/lastUsername", username);
    }
}

void Config::setDownloadPath(const QString& path)
{
    m_downloadPath = path;
    if (m_settings) {
        m_settings->setValue("Paths/download", path);
    }
    QDir().mkpath(m_downloadPath);
}