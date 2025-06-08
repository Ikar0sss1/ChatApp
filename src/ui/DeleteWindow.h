#ifndef DELETEWINDOW_H
#define DELETEWINDOW_H

#include <QDialog>
#include <QPushButton>
#include <QLabel>
#include <QPropertyAnimation>

class DeleteWindow : public QDialog
{
    Q_OBJECT
public:
    explicit DeleteWindow(QWidget* parent = nullptr, const QString& contactName = "");

    bool isConfirmed() const { return m_confirmed; }

private:
    QPushButton* m_okButton;
    QPushButton* m_cancelButton;
    QLabel* m_infoLabel;
    bool m_confirmed = false;
};

#endif // DELETEWINDOW_H
