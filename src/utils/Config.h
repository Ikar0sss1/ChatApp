#ifndef CONFIG_H
#define CONFIG_H

#include <QString>
#include <QSettings>

class Config
{
public:
    static Config& getInstance();
    
    void initialize();
    
    QString getServerHost() const { return m_serverHost; }
    int getServerPort() const { return m_serverPort; }
    QString getLastUsername() const { return m_lastUsername; }
    QString getDownloadPath() const { return m_downloadPath; }
    
    void setLastUsername(const QString& username);
    void setDownloadPath(const QString& path);

private:
    Config();
    ~Config();
    
    QSettings* m_settings;
    QString m_serverHost;
    int m_serverPort;
    QString m_lastUsername;
    QString m_downloadPath;
};

#endif // CONFIG_H