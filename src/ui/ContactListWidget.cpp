#include "ContactListWidget.h"
#include "core/UserManager.h"
#include <QMessageBox>
#include <QLabel> 
#include <QLineEdit>
#include <QInputDialog>
#include <QFile>

ContactListWidget::ContactListWidget(QWidget *parent)
    : QWidget(parent)
{
    setupUI();
    loadContacts();
}

void ContactListWidget::setupUI()
{
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setSpacing(0);
    layout->setContentsMargins(0, 0, 0, 0);

    // // 搜索栏
    // QHBoxLayout* searchLayout = new QHBoxLayout();
    // searchLayout->setContentsMargins(8, 8, 8, 8);
    // QLineEdit* searchEdit = new QLineEdit();
    // searchEdit->setPlaceholderText("搜索联系人");
    // searchEdit->setMinimumHeight(32);
    // searchLayout->addWidget(searchEdit);

    // // "+"号按钮
    // QPushButton* addButton = new QPushButton("+");
    // addButton->setFixedSize(32, 32);
    // addButton->setStyleSheet("QPushButton { background: #07C160; color: white; border: none; border-radius: 6px; font-size: 22px; } QPushButton:hover { background: #05A150; }");
    // connect(addButton, &QPushButton::clicked, this, &ContactListWidget::onAddContactClicked);
    // searchLayout->addWidget(addButton);

    // layout->addLayout(searchLayout);

    // 联系人列表
    m_contactList = new QListWidget();
    connect(m_contactList, &QListWidget::itemClicked, this, &ContactListWidget::onContactClicked);
    layout->addWidget(m_contactList);

    // 按钮布局
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    m_removeButton = new QPushButton("删除联系人");
    m_removeButton->setMinimumHeight(36);
    m_removeButton->setStyleSheet(
        "QPushButton { background-color: #07C160; color: white; border: none; border-radius: 4px; font-size: 15px; min-width: 100px; }"
        "QPushButton:hover { background-color: #05A150; }"
        "QPushButton:pressed { background-color: #038B43; }"
    );
    connect(m_removeButton, &QPushButton::clicked, this, &ContactListWidget::onRemoveContactClicked);
    buttonLayout->addWidget(m_removeButton);

    m_refreshButton = new QPushButton("刷新");
    m_refreshButton->setMinimumHeight(36);
    m_refreshButton->setStyleSheet(
        "QPushButton { background-color: #07C160; color: white; border: none; border-radius: 4px; font-size: 15px; min-width: 80px; }"
        "QPushButton:hover { background-color: #05A150; }"
        "QPushButton:pressed { background-color: #038B43; }"
    );
    connect(m_refreshButton, &QPushButton::clicked, this, &ContactListWidget::onRefreshClicked);
    buttonLayout->addWidget(m_refreshButton);

    layout->addLayout(buttonLayout);
}

void ContactListWidget::loadContacts()
{
    m_contactList->clear();
    QList<User> contacts = UserManager::getInstance().getContacts();
    for (const User& contact : contacts) {
        QWidget* itemWidget = new QWidget();
        QHBoxLayout* layout = new QHBoxLayout(itemWidget);
        layout->setContentsMargins(12, 6, 12, 6);
        layout->setSpacing(12);
        QLabel* avatar = new QLabel();
        avatar->setFixedSize(40, 40);
        avatar->setStyleSheet("border-radius: 20px; background: #eee;");
        QString avatarPath = contact.getAvatar();
        if (!avatarPath.isEmpty() && QFile::exists(avatarPath)) {
            avatar->setPixmap(QPixmap(avatarPath).scaled(40, 40, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        } else {
            avatar->setPixmap(QPixmap(":/icons/default_avatar.png").scaled(40, 40, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        }
        layout->addWidget(avatar);
        QVBoxLayout* infoLayout = new QVBoxLayout();
        QLabel* nameLabel = new QLabel(contact.getNickname());
        nameLabel->setObjectName("nicknameLabel");
        nameLabel->setStyleSheet("font-weight: bold; font-size: 15px; color: #222;");
        infoLayout->addWidget(nameLabel);
        layout->addLayout(infoLayout);
        itemWidget->setLayout(layout);
        QListWidgetItem* item = new QListWidgetItem();
        item->setSizeHint(itemWidget->sizeHint());
        item->setData(Qt::UserRole, contact.getUserId());
        m_contactList->addItem(item);
        m_contactList->setItemWidget(item, itemWidget);
    }
}

void ContactListWidget::refreshContacts()
{
    loadContacts();
}

void ContactListWidget::onContactClicked(QListWidgetItem* item)
{
    if (item) {
        QString contactId = item->data(Qt::UserRole).toString();
        emit contactSelected(contactId);
    }
}

void ContactListWidget::onRemoveContactClicked()
{
    QListWidgetItem* currentItem = m_contactList->currentItem();
    if (!currentItem) {
        QMessageBox::information(this, "提示", "请先选择要删除的联系人");
        return;
    }
    
    QString contactId = currentItem->data(Qt::UserRole).toString();
    QString contactName = currentItem->text();
    
    int ret = QMessageBox::question(this, "确认删除", 
                                   QString("确定要删除联系人 %1 吗?").arg(contactName));
    
    if (ret == QMessageBox::Yes) {
        if (UserManager::getInstance().removeContact(contactId)) {
            refreshContacts();
        } else {
            QMessageBox::warning(this, "错误", "删除联系人失败");
        }
    }
}

void ContactListWidget::onRefreshClicked()
{
    refreshContacts();
}

void ContactListWidget::onAddContactClicked()
{
    // 这里可以弹出添加联系人对话框，或发出信号让 MainWindow 处理
    // 示例：弹出输入框
    bool ok;
    QString contactId = QInputDialog::getText(this, "添加联系人", "请输入联系人用户名:", QLineEdit::Normal, "", &ok);
    if (ok && !contactId.isEmpty()) {
        if (UserManager::getInstance().addContact(contactId)) {
            refreshContacts();
        } else {
            QMessageBox::warning(this, "错误", "添加联系人失败");
        }
    }
}

void ContactListWidget::filterContacts(const QString& text) {
    for (int i = 0; i < m_contactList->count(); ++i) {
        QListWidgetItem* item = m_contactList->item(i);
        if (item) {
            QWidget* widget = m_contactList->itemWidget(item);
            if (widget) {
                QLabel* nameLabel = widget->findChild<QLabel*>("nicknameLabel");
                if (nameLabel) {
                    bool match = nameLabel->text().contains(text, Qt::CaseInsensitive);
                    item->setHidden(!match);
                }
            }
        }
    }
}

void ContactListWidget::highlightContact(const QString& contactId, bool highlight) {
    for (int i = 0; i < m_contactList->count(); ++i) {
        QListWidgetItem* item = m_contactList->item(i);
        if (item && item->data(Qt::UserRole).toString() == contactId) {
            if (highlight)
                item->setBackground(Qt::yellow); // 高亮
            else
                item->setBackground(Qt::white);  // 取消高亮（或用默认色）
        }
    }
}