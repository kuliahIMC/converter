#include "drop_list.h"
#include "../services/format_registry.h"

#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QUrl>

DropListWidget::DropListWidget(QWidget *parent)
    : QListWidget(parent)
{
    setAcceptDrops(true);
}

void DropListWidget::setFilterMode(FilterMode mode)
{
    filterMode_ = mode;
}

DropListWidget::FilterMode DropListWidget::filterMode() const
{
    return filterMode_;
}

bool DropListWidget::isFileAccepted(const QString &filePath) const
{
    switch (filterMode_) {
    case FilterMode::Audio:
        return FormatRegistry::isAudioInputSupported(filePath);
    case FilterMode::Video:
        return FormatRegistry::isVideoInputSupported(filePath);
    case FilterMode::Image:
        return FormatRegistry::isImageInputSupported(filePath);
    case FilterMode::Compress:
        return FormatRegistry::isImageCompressSupported(filePath) ||
               FormatRegistry::isVideoCompressSupported(filePath);
    case FilterMode::All:
    default:
        return true;
    }
}

void DropListWidget::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasUrls()) {
        event->acceptProposedAction();
    } else {
        event->ignore();
    }
}

void DropListWidget::dragMoveEvent(QDragMoveEvent *event)
{
    if (event->mimeData()->hasUrls()) {
        event->acceptProposedAction();
    } else {
        event->ignore();
    }
}

void DropListWidget::dropEvent(QDropEvent *event)
{
    QStringList acceptedFiles;

    const QList<QUrl> urls = event->mimeData()->urls();
    for (const QUrl &url : urls) {
        if (url.isLocalFile()) {
            const QString localPath = url.toLocalFile();
            if (isFileAccepted(localPath)) {
                acceptedFiles.append(localPath);
            }
        }
    }

    if (!acceptedFiles.isEmpty()) {
        emit filesDropped(acceptedFiles);
    }

    event->acceptProposedAction();
}

