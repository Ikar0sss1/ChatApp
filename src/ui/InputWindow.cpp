#include "InputWindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFile>
#include <QPropertyAnimation>
#include <QGraphicsDropShadowEffect>

InputWindow::InputWindow(QWidget* parent)
    : QDialog(parent)
{
    setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);
    setAttribute(Qt::WA_TranslucentBackground);
    setFixedSize(400, 220);
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    QWidget* bgWidget = new QWidget(this);
    bgWidget->setObjectName("bgWidget");
    bgWidget->setStyleSheet(
        "#bgWidget { background: white; border-radius: 16px; }"
    );
    QVBoxLayout* layout = new QVBoxLayout(bgWidget);
    layout->setContentsMargins(32, 28, 32, 24);
    layout->setSpacing(18);

    QLabel* label = new QLabel("请输入联系人用户名", bgWidget);
    label->setStyleSheet("font-size: 18px; color: #222; font-weight: bold;");
    layout->addWidget(label);

    m_lineEdit = new QLineEdit(bgWidget);
    m_lineEdit->setPlaceholderText("用户名");
    m_lineEdit->setMinimumHeight(36);
    m_lineEdit->setStyleSheet(
        "QLineEdit { background: #f7f7f7; border: 1.5px solid #e3e3e3; border-radius: 8px; padding: 8px 12px; font-size: 16px; color: #333; }"
        "QLineEdit:focus { border: 1.5px solid #07C160; background: #fff; }"
    );
    layout->addWidget(m_lineEdit);

    QHBoxLayout* btnLayout = new QHBoxLayout();
    btnLayout->addStretch();
    QPushButton* okBtn = new QPushButton("确定", bgWidget);
    okBtn->setMinimumHeight(32);
    okBtn->setStyleSheet(
        "QPushButton { background: #07C160; color: white; border: none; border-radius: 8px; font-size: 16px; min-width: 80px; }"
        "QPushButton:hover { background: #06AD56; }"
        "QPushButton:pressed { background: #059B4C; }"
    );
    connect(okBtn, &QPushButton::clicked, this, &InputWindow::accept);
    btnLayout->addWidget(okBtn);
    QPushButton* cancelBtn = new QPushButton("取消", bgWidget);
    cancelBtn->setMinimumHeight(32);
    cancelBtn->setStyleSheet(
        "QPushButton { background: #f7f7f7; color: #222; border: 1.5px solid #e3e3e3; border-radius: 8px; font-size: 16px; min-width: 80px; }"
        "QPushButton:hover { background: #e1f6ef; border-color: #07C160; color: #07C160; }"
    );
    connect(cancelBtn, &QPushButton::clicked, this, &InputWindow::reject);
    btnLayout->addWidget(cancelBtn);
    layout->addLayout(btnLayout);

    mainLayout->addWidget(bgWidget);

    // 阴影效果
    QGraphicsDropShadowEffect* shadow = new QGraphicsDropShadowEffect(bgWidget);
    shadow->setBlurRadius(24);
    shadow->setOffset(0, 4);
    shadow->setColor(QColor(0,0,0,60));
    bgWidget->setGraphicsEffect(shadow);

    setWindowOpacity(0.0);
    QPropertyAnimation* anim = new QPropertyAnimation(this, "windowOpacity");
    anim->setDuration(300);
    anim->setStartValue(0.0);
    anim->setEndValue(1.0);
    anim->start(QAbstractAnimation::DeleteWhenStopped);
}

QString InputWindow::getInputText() const
{
    return m_lineEdit->text();
}
