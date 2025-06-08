#include "SettingsDialog.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFileDialog>
#include <QSettings>
#include <QFile>

SettingsDialog::SettingsDialog(QWidget* parent)
    : QDialog(parent), m_darkMode(false)
{
    setWindowTitle("设置");
    setFixedSize(400, 320);
    setStyleSheet(
        "QDialog { background: #F7F7F7; border-radius: 14px; }"
        "QLabel { color: #222; font-size: 15px; }"
        "QLineEdit { background: #fff; border: 1.5px solid #E3E3E3; border-radius: 8px; padding: 6px 10px; font-size: 15px; }"
        "QPushButton { background: #F7F7F7; border: 1.5px solid #E3E3E3; border-radius: 8px; color: #222; font-size: 15px; padding: 6px 16px; }"
        "QPushButton:hover { background: #E1F6EF; border-color: #07C160; color: #07C160; }"
        "QPushButton:checked { background: #353535; color: #fff; border-color: #07C160; }"
        "QSlider::groove:horizontal { border: 1px solid #E3E3E3; height: 8px; background: #EDEDED; border-radius: 4px; }"
        "QSlider::handle:horizontal { background: #07C160; border: 1px solid #07C160; width: 18px; margin: -5px 0; border-radius: 9px; }"
        "QSlider { min-height: 24px; }"
    );

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(18);
    mainLayout->setContentsMargins(28, 22, 28, 22);

    // 深色模式
    m_darkModeBtn = new QPushButton();
    m_darkModeBtn->setCheckable(true);
    connect(m_darkModeBtn, &QPushButton::clicked, this, &SettingsDialog::onDarkModeToggled);
    mainLayout->addWidget(m_darkModeBtn);

    // 字体大小
    QHBoxLayout* fontLayout = new QHBoxLayout();
    QLabel* fontLabel = new QLabel("字体大小：");
    m_fontSlider = new QSlider(Qt::Horizontal);
    m_fontSlider->setRange(10, 24);
    fontLayout->addWidget(fontLabel);
    fontLayout->addWidget(m_fontSlider);
    mainLayout->addLayout(fontLayout);

    m_fontPreview = new QLabel("示例文本 Sample Text");
    m_fontPreview->setAlignment(Qt::AlignCenter);
    m_fontPreview->setMinimumHeight(36);
    m_fontPreview->setMaximumHeight(48);
    m_fontPreview->setMaximumWidth(300);
    m_fontPreview->setStyleSheet("background: #fff; border-radius: 8px; border: 1.5px solid #E3E3E3; margin-top: 4px; font-size: 15px;");
    mainLayout->addWidget(m_fontPreview);

    // 文件下载路径
    QHBoxLayout* pathLayout = new QHBoxLayout();
    QLabel* pathLabel = new QLabel("文件下载路径：");
    m_pathEdit = new QLineEdit();
    m_pathEdit->setReadOnly(true);
    m_choosePathBtn = new QPushButton("选择...");
    connect(m_choosePathBtn, &QPushButton::clicked, this, &SettingsDialog::onChoosePath);

    pathLayout->addWidget(pathLabel);
    pathLayout->addWidget(m_pathEdit);
    pathLayout->addWidget(m_choosePathBtn);
    mainLayout->addLayout(pathLayout);

    // 读取设置
    QSettings settings("YourCompany", "YourApp");
    m_fontSlider->setValue(settings.value("fontSize", 15).toInt());
    m_pathEdit->setText(settings.value("downloadPath", "").toString());
    m_darkMode = settings.value("darkMode", false).toBool();
    m_darkModeBtn->setChecked(m_darkMode);

    // 初始化字体和主题
    onFontSizeChanged(m_fontSlider->value());
    onDarkModeToggled();

    // 拖拽条实时响应
    connect(m_fontSlider, &QSlider::valueChanged, this, &SettingsDialog::onFontSizeChanged);

    // 确认/取消按钮
    QHBoxLayout* btnLayout = new QHBoxLayout();
    btnLayout->addStretch();
    m_okBtn = new QPushButton("应用");
    m_cancelBtn = new QPushButton("取消");
    connect(m_okBtn, &QPushButton::clicked, this, &SettingsDialog::onApply);
    connect(m_cancelBtn, &QPushButton::clicked, this, &SettingsDialog::reject);
    btnLayout->addWidget(m_okBtn);
    btnLayout->addWidget(m_cancelBtn);
    mainLayout->addLayout(btnLayout);
}

void SettingsDialog::onDarkModeToggled()
{
    m_darkMode = m_darkModeBtn->isChecked();
    // 设置自身QSS
    QFile f(m_darkMode ? ":/resources/qss/dark.qss" : ":/resources/qss/style.qss");
    if (f.open(QFile::ReadOnly)) {
        this->setStyleSheet(f.readAll());
        f.close();
    }
    // 按钮文本切换
    if (m_darkMode) {
        m_darkModeBtn->setText("切换浅色模式");
    } else {
        m_darkModeBtn->setText("切换深色模式");
    }
}

void SettingsDialog::onFontSizeChanged(int value)
{
    QFont f = m_fontPreview->font();
    f.setPointSize(value);
    m_fontPreview->setFont(f);
}

void SettingsDialog::onChoosePath()
{
    QString dir = QFileDialog::getExistingDirectory(this, "选择下载路径");
    if (!dir.isEmpty()) {
        m_pathEdit->setText(dir);
        emit downloadPathChanged(dir);
    }
}

void SettingsDialog::onApply()
{
    // 保存设置
    QSettings settings("YourCompany", "YourApp");
    settings.setValue("fontSize", m_fontSlider->value());
    settings.setValue("downloadPath", m_pathEdit->text());
    settings.setValue("darkMode", m_darkMode);

    accept();
}

int SettingsDialog::fontSize() const { return m_fontSlider->value(); }
QString SettingsDialog::downloadPath() const { return m_pathEdit->text(); }
bool SettingsDialog::isDarkMode() const { return m_darkMode; }
