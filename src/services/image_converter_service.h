#pragma once

#include <QObject>
#include <QString>
#include <QImage>
#include <QImageWriter>

class ImageConverterService : public QObject
{
    Q_OBJECT

public:
    explicit ImageConverterService(QObject *parent = nullptr);

    bool isRunning() const;

public slots:
    void convert(
        const QString &inputPath,
        const QString &outputPath,
        const QString &format
    );

    void cancel();

signals:
    void started();
    void finished(const QString &outputPath);
    void failed(const QString &errorMessage);
    void cancelled();

private:
    bool running_ = false;
    bool cancelled_ = false;
};