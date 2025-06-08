#include <QApplication>
#include <QMessageBox>
#include <QFile>
#include <QTextStream>
#include "ui/LoginWindow.h"
#include "utils/Logger.h"
#include "core/DatabaseManager.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    app.setApplicationName("Simple IMChat");
    app.setApplicationVersion("1.0.0");
    
    // 初始化日志
    Logger::getInstance().initialize();
    
    // 初始化数据库
    if (!DatabaseManager::getInstance().initialize()) {
        QMessageBox::critical(nullptr, "错误", "数据库初始化失败，请检查数据库配置");
        return -1;
    }
    
    // 加载主窗口样式表
    QFile qss(":/qss/MainWindow.qss");
    if (qss.open(QFile::ReadOnly)) {
        app.setStyleSheet(qss.readAll());
        qss.close();
    }
    
    // 显示登录窗口
    LoginWindow loginWindow;
    loginWindow.show();
    
    return app.exec();
}