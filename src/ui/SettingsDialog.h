#pragma once
#include <QDialog>
#include <QPushButton>
#include <QSlider>
#include <QLabel>
#include <QLineEdit>

class SettingsDialog : public QDialog
{
    Q_OBJECT
public:
    explicit SettingsDialog(QWidget* parent = nullptr);

    int fontSize() const;
    QString downloadPath() const;
    bool isDarkMode() const;

signals:
    void fontSizeChanged(int);
    void darkModeChanged(bool);
    void downloadPathChanged(const QString&);

private slots:
    void onDarkModeToggled();
    void onFontSizeChanged(int value);
    void onChoosePath();
    void onApply();

private:
    QPushButton* m_darkModeBtn;
    QSlider* m_fontSlider;
    QLabel* m_fontPreview;
    QLineEdit* m_pathEdit;
    QPushButton* m_choosePathBtn;
    QPushButton* m_okBtn;
    QPushButton* m_cancelBtn;

    bool m_darkMode;
};
