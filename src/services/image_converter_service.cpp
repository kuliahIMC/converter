#include "image_converter_service.h"

#include <QFileInfo>
#include <QImageReader>
#include <QImageWriter>
#include <QByteArray>

ImageConverterService::ImageConverterService(QObject *parent)
    : QObject(parent)
{
}

bool ImageConverterService::isRunning() const
{
    return running_;
}

void ImageConverterService::convert(
    const QString &inputPath,
    const QString &outputPath,
    const QString &format)
{
    if (running_)
        return;

    running_ = true;
    cancelled_ = false;

    emit started();

    QImageReader reader(inputPath);
    reader.setAutoTransform(true);

    QImage image = reader.read();

    if (cancelled_) {
        running_ = false;
        emit cancelled();
        return;
    }

    if (image.isNull()) {
        running_ = false;
        emit failed(
            QString("Gagal membaca gambar: %1")
                .arg(reader.errorString())
        );
        return;
    }

    const QByteArray outputFormat =
        format.trimmed().toLower().toLatin1();

    QImageWriter writer(outputPath, outputFormat);

    if (!writer.write(image)) {
        running_ = false;
        emit failed(
            QString("Gagal menyimpan gambar: %1")
                .arg(writer.errorString())
        );
        return;
    }

    running_ = false;
    emit finished(outputPath);
}

void ImageConverterService::cancel()
{
    if (!running_)
        return;

    cancelled_ = true;
}