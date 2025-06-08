#ifndef CHATAPP_EMOJIDIALOG_H
#define CHATAPP_EMOJIDIALOG_H

#include <QDialog>
#include <QGridLayout>
#include <QPushButton>
#include <QScrollArea>

class EmojiDialog : public QDialog {
    Q_OBJECT

public:
    explicit EmojiDialog(QWidget* parent = nullptr);

signals:
    void emojiSelected(const QString& emoji);

private:
    void setupUI();
    void addEmojiButton(const QString& emoji);
    QScrollArea* m_scrollArea;
    QWidget* m_container;
    QGridLayout* m_gridLayout;
};

#endif //CHATAPP_EMOJIDIALOG_H 