#ifndef CONTACTLISTWIDGET_H
#define CONTACTLISTWIDGET_H

#include <QWidget>
#include <QListWidget>
#include <QVBoxLayout>
#include <QPushButton>
#include "models/User.h"

class ContactListWidget : public QWidget
{
    Q_OBJECT
    
public:
    explicit ContactListWidget(QWidget *parent = nullptr);
    
    void refreshContacts();
    void filterContacts(const QString& text);
    void highlightContact(const QString& contactId, bool highlight = true);

signals:
    void contactSelected(const QString& contactId);

private slots:
    void onContactClicked(QListWidgetItem* item);
    void onRemoveContactClicked();
    void onRefreshClicked();
    void onAddContactClicked(); // 新增

private:
    void setupUI();
    void loadContacts();
    
    QListWidget* m_contactList;
    QPushButton* m_removeButton;
    QPushButton* m_refreshButton;
};

#endif // CONTACTLISTWIDGET_H