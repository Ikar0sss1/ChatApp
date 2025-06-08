#ifndef CHATAPP_CHATWINDOW_H
#define CHATAPP_CHATWINDOW_H

#include <QWidget>
#include <QTextEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QScrollArea>
#include "models/Message.h"
#include "core/NetworkManager.h"
#include "MessageBubble.h"
#include "EmojiDialog.h"

class ChatWindow : public QWidget
{
    Q_OBJECT
    
public:
    explicit ChatWindow(const QString& contactId, QWidget *parent = nullptr);
    ~ChatWindow();

    void addMessage(const Message& message);
    void refreshMessages();

private slots:
    void onSendClicked();
    void onFileClicked();
    void onEmojiClicked();
    void onEmojiSelected(const QString& emoji);
    void onConnectionStatusChanged(bool connected);
    void onErrorOccurred(const QString& error);
    void onImageClicked();

private:
    void setupUI();
    void loadChatHistory();
    void showEmojiPicker();
    void connectToServer();

    QString m_contactId;
    QTextEdit* m_inputEdit;
    QPushButton* m_sendButton;
    QPushButton* m_fileButton;
    QPushButton* m_emojiButton;
    QPushButton* m_imageButton;
    QScrollArea* m_scrollArea;
    QWidget* m_messageContainer;
    QVBoxLayout* m_messageLayout;
    QLabel* m_statusLabel;
    EmojiDialog* m_emojiDialog;

    // 建议添加拖动相关成员变量
    bool m_dragging = false;
    QPoint m_dragPosition;

protected:
    bool eventFilter(QObject* obj, QEvent* event) override;
};

#endif // CHATAPP_CHATWINDOW_H