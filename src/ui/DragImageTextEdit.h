#pragma once
#include <QTextEdit>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QUrl>

class DragImageTextEdit : public QTextEdit
{
    Q_OBJECT
public:
    explicit DragImageTextEdit(QWidget* parent = nullptr) : QTextEdit(parent) {
        setAcceptDrops(true);
    }

signals:
    void imageDropped(const QString& filePath);

protected:
    void dragEnterEvent(QDragEnterEvent* event) override {
        if (event->mimeData()->hasUrls()) {
            QList<QUrl> urls = event->mimeData()->urls();
            if (!urls.isEmpty() && urls.first().toLocalFile().toLower().endsWith(
                ".png") || urls.first().toLocalFile().toLower().endsWith(".jpg") ||
                urls.first().toLocalFile().toLower().endsWith(".jpeg") ||
                urls.first().toLocalFile().toLower().endsWith(".bmp") ||
                urls.first().toLocalFile().toLower().endsWith(".gif")) {
                event->acceptProposedAction();
                return;
            }
        }
        QTextEdit::dragEnterEvent(event);
    }

    void dropEvent(QDropEvent* event) override {
        QList<QUrl> urls = event->mimeData()->urls();
        if (!urls.isEmpty()) {
            QString filePath = urls.first().toLocalFile();
            if (filePath.toLower().endsWith(".png") || filePath.toLower().endsWith(".jpg") ||
                filePath.toLower().endsWith(".jpeg") || filePath.toLower().endsWith(".bmp") ||
                filePath.toLower().endsWith(".gif")) {
                emit imageDropped(filePath);
                return;
            }
        }
        QTextEdit::dropEvent(event);
    }
};
