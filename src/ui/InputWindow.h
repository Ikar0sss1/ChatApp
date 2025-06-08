#pragma once

#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QPropertyAnimation>

class InputWindow : public QDialog
{
    Q_OBJECT
public:
    explicit InputWindow(QWidget* parent = nullptr);

    QString getInputText() const;

private:
    QLineEdit* m_inputEdit;
    QPushButton* m_okButton;
    QPushButton* m_cancelButton;
    QLabel* m_titleLabel;
    QLineEdit* m_lineEdit;
};
