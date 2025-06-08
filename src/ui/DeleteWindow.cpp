#include "DeleteWindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QGraphicsDropShadowEffect>

DeleteWindow::DeleteWindow(QWidget* parent, const QString& contactName)
    : QDialog(parent)
{
    setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);
    setAttribute(Qt::WA_TranslucentBackground);
    setFixedSize(340, 160);

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    QWidget* bgWidget = new QWidget(this);
    bgWidget->setObjectName("bgWidget");
    bgWidget->setStyleSheet("#bgWidget { background: white; border-radius: 16px; }");
    QVBoxLayout* layout = new QVBoxLayout(bgWidget);
    layout->setContentsMargins(24, 18, 24, 18);
    layout->setSpacing(18);

    // 图标和文本横向布局
    QHBoxLayout* iconTextLayout = new QHBoxLayout();
    QLabel* icon = new QLabel(bgWidget);
    icon->setText("🗑️");
    icon->setStyleSheet("font-size: 28px; margin-right: 8px;");
    iconTextLayout->addWidget(icon, 0, Qt::AlignVCenter);
    QLabel* label = new QLabel(QString("确定要删除联系人 %1 吗?").arg(contactName), bgWidget);
    label->setStyleSheet("font-size: 17px; color: #222; font-weight: bold;");
    iconTextLayout->addWidget(label, 1, Qt::AlignVCenter);
    layout->addLayout(iconTextLayout);

    QHBoxLayout* btnLayout = new QHBoxLayout();
    btnLayout->addStretch();
    QPushButton* yesBtn = new QPushButton("确定", bgWidget);
    yesBtn->setMinimumHeight(32);
    yesBtn->setStyleSheet(
        "QPushButton { background: #07C160; color: white; border: none; border-radius: 8px; font-size: 16px; min-width: 80px; }"
        "QPushButton:hover { background: #06AD56; }"
        "QPushButton:pressed { background: #059B4C; }"
    );
    connect(yesBtn, &QPushButton::clicked, this, [=](){ m_confirmed = true; accept(); });
    btnLayout->addWidget(yesBtn);

    QPushButton* noBtn = new QPushButton("取消", bgWidget);
    noBtn->setMinimumHeight(32);
    noBtn->setStyleSheet(
        "QPushButton { background: #f7f7f7; color: #222; border: 1.5px solid #e3e3e3; border-radius: 8px; font-size: 16px; min-width: 80px; }"
        "QPushButton:hover { background: #e1f6ef; border-color: #07C160; color: #07C160; }"
    );
    connect(noBtn, &QPushButton::clicked, this, &DeleteWindow::reject);
    btnLayout->addWidget(noBtn);
    layout->addLayout(btnLayout);

    mainLayout->addWidget(bgWidget);

    // 阴影
    QGraphicsDropShadowEffect* shadow = new QGraphicsDropShadowEffect(bgWidget);
    shadow->setBlurRadius(24);
    shadow->setOffset(0, 4);
    shadow->setColor(QColor(0,0,0,60));
    bgWidget->setGraphicsEffect(shadow);
}
