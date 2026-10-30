#pragma once

#include <QListWidget>
#include <QStringList>

class DropListWidget : public QListWidget {
    Q_OBJECT

public:
    enum class FilterMode {
        All,
        Audio,
        Video,
        Image,
        Compress
    };

    explicit DropListWidget(QWidget *parent = nullptr);

    void setFilterMode(FilterMode mode);
    FilterMode filterMode() const;

    bool isFileAccepted(const QString &filePath) const;

signals:
    void filesDropped(const QStringList &files);

protected:
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dragMoveEvent(QDragMoveEvent *event) override;
    void dropEvent(QDropEvent *event) override;

private:
    FilterMode filterMode_ = FilterMode::All;
};

