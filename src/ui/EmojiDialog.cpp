#include "EmojiDialog.h"
#include <QVBoxLayout>

EmojiDialog::EmojiDialog(QWidget* parent) : QDialog(parent) {
    setWindowTitle("选择表情");
    setFixedSize(400, 300);
    setupUI();
}

void EmojiDialog::setupUI() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(10, 10, 10, 10);

    m_scrollArea = new QScrollArea(this);
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_scrollArea->setStyleSheet(
        "QScrollArea { border: none; }"
        "QScrollBar:vertical { border: none; background: #F0F0F0; width: 8px; }"
        "QScrollBar::handle:vertical { background: #C0C0C0; border-radius: 4px; }"
        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0px; }"
    );

    m_container = new QWidget(m_scrollArea);
    m_gridLayout = new QGridLayout(m_container);
    m_gridLayout->setSpacing(5);
    m_gridLayout->setContentsMargins(5, 5, 5, 5);

    // 添加所有表情
    QStringList emojis = {
        "😀", "😃", "😄", "😁", "😆", "😅", "🤣", "😂", "🙂", "🙃",
        "😉", "😊", "😇", "😍", "🤩", "😘", "😗", "😑", "😐", "🤨",
        "🤐", "🤔", "🤫", "🤭", "🤗", "🤑", "😝", "🤪", "😜", "😛",
        "😋", "😙", "😚", "☺️", "😶", "😏", "😒", "😬", "🙄", "🤥",
        "😌", "😔", "😪", "🤤", "😴", "😷", "🤒", "🤕", "🤢", "🤮",
        "🤧", "😨", "😧", "😦", "😳", "😲", "😯", "😮", "☹️", "😟",
        "🙁", "😕", "🧐", "🤓", "😎", "🤠", "🤯", "😵", "😰", "😥",
        "😢", "😭", "😱", "😖", "😣", "😞", "😓", "😩", "😫", "😤",
        "😡", "😠", "🤬", "😈", "👿", "🤡", "☠️", "💀", "👹", "👺",
        "👽", "🙈", "👻", "🙉", "🙊", "💋", "💌", "💘", "💝", "💖",
        "💗", "💓", "👶", "🧒", "👄", "👅", "👃", "🤳", "💬", "🖤",
        "💜", "💙", "💚", "💛", "❤️", "🧡", "💔", "💞", "💕"
    };

    int row = 0, col = 0;
    const int COLS = 10;
    for (const QString& emoji : emojis) {
        addEmojiButton(emoji);
        col++;
        if (col >= COLS) {
            col = 0;
            row++;
        }
    }

    m_scrollArea->setWidget(m_container);
    mainLayout->addWidget(m_scrollArea);
}

void EmojiDialog::addEmojiButton(const QString& emoji) {
    QPushButton* button = new QPushButton(emoji);
    button->setFixedSize(35, 35);
    button->setStyleSheet(
        "QPushButton { "
        "   border: none; "
        "   border-radius: 4px; "
        "   font-size: 20px; "
        "   background: transparent; "
        "} "
        "QPushButton:hover { "
        "   background: #E1F6EF; "
        "}"
    );
    
    connect(button, &QPushButton::clicked, this, [this, emoji]() {
        emit emojiSelected(emoji);
        close();
    });

    m_gridLayout->addWidget(button, m_gridLayout->rowCount() / 10, m_gridLayout->rowCount() % 10);
} 