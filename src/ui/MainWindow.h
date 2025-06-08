#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSplitter>
#include <QStackedWidget>
#include <QLineEdit>
#include "ContactListWidget.h"
#include "ChatWindow.h"
#include <QPushButton>
#include <QVBoxLayout>
#include <QLabel>
#include <QFileDialog>
#include <QMap>

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = nullptr);
    void refreshMyAvatar();
    void onDeleteContactClicked(const QString& contactName);

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private slots:
    void onContactSelected(const QString& contactId);
    void onAddContactClicked();
    void onMinimizeClicked();
    void onCloseClicked();
    void onSettingsClicked();

private:
    void setupUI();
    void setupTitleBar(QVBoxLayout* mainLayout);
    void setupMainContent(QVBoxLayout* mainLayout);
    void setupLeftPanel(QSplitter* splitter);
    void setupNavigationBar(QVBoxLayout* leftLayout);
    void setupSearchArea(QVBoxLayout* leftLayout);
    void setupChatArea(QSplitter* splitter);

    // UI组件
    QSplitter* m_splitter;
    ContactListWidget* m_contactList;
    QStackedWidget* m_chatStack;
    QMap<QString, ChatWindow*> m_chatWindows;

    // 标题栏按钮
    QPushButton* m_minButton;
    QPushButton* m_closeButton;

    // 导航栏按钮
    QPushButton* m_chatNavButton;
    QPushButton* m_contactNavButton;
    QPushButton* m_addContactButton;

    // 搜索框
    QLineEdit* m_searchEdit;

    // 拖拽相关
    bool m_dragging = false;
    QPoint m_dragPosition;

    QLabel* m_myAvatarLabel;
    QPushButton* m_editAvatarBtn;

    QPushButton* m_settingsButton;

    QPushButton* m_refreshButton;
};

#endif // MAINWINDOW_H