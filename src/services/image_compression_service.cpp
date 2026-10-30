#include "image_compression_service.h"

#include <QFile>
#include <QFileInfo>
#include <QRegularExpression>

ImageCompressionService::ImageCompressionService(QObject *parent)
    : QObject(parent),
      process_(new QProcess(this))
{
    process_->setProcessChannelMode(QProcess::SeparateChannels);

    connect(
        process_,
        QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
        this,
        &ImageCompressionService::handleFinished
    );
}

ImageCompressionService::~ImageCompressionService()
{
    if (isRunning()) {
        process_->kill();
        process_->waitForFinished(1000);
    }
}

bool ImageCompressionService::isRunning() const
{
    return process_ &&
           process_->state() != QProcess::NotRunning;
}

QString ImageCompressionService::ffmpegPath() const
{
    return "ffmpeg";
}

void ImageCompressionService::compress(
    const QString &inputPath,
    const QString &outputPath,
    int quality)
{
    if (isRunning())
        return;

    if (!QFileInfo::exists(inputPath)) {
        emit failed("File gambar tidak ditemukan.");
        return;
    }

    if (outputPath.isEmpty()) {
        emit failed("Lokasi output tidak valid.");
        return;
    }

    if (QFileInfo(inputPath).absoluteFilePath() ==
        QFileInfo(outputPath).absoluteFilePath()) {
        emit failed("File output tidak boleh sama dengan file input.");
        return;
    }

    const QString extension =
        QFileInfo(outputPath).suffix().toLower();

    const int clampedQuality = qBound(10, quality, 100);

    QStringList arguments;

    arguments << "-y"
              << "-hide_banner"
              << "-loglevel" << "error"
              << "-i" << inputPath
              << "-frames:v" << "1";

    if (extension == "jpg" || extension == "jpeg") {
        const int qScale = qBound(
            2,
            31 - (clampedQuality * 29 / 100),
            31
        );

        arguments << "-q:v" << QString::number(qScale);
    }
    else if (extension == "webp") {
        arguments << "-c:v" << "libwebp"
                  << "-quality" << QString::number(clampedQuality);
    }
    else if (extension == "png") {
        const int compression = qBound(
            0,
            9 - (clampedQuality * 9 / 100),
            9
        );

        arguments << "-compression_level"
                  << QString::number(compression);
    }

    arguments << outputPath;

    outputPath_ = outputPath;
    originalSize_ = QFileInfo(inputPath).size();
    cancelled_ = false;

    process_->start(ffmpegPath(), arguments);

    if (!process_->waitForStarted(3000)) {
        emit failed(
            "FFmpeg tidak dapat dijalankan: " +
            process_->errorString()
        );
        return;
    }

    emit started();
}

void ImageCompressionService::cancel()
{
    if (!isRunning())
        return;

    cancelled_ = true;
    process_->kill();
}

void ImageCompressionService::handleFinished(
    int exitCode,
    QProcess::ExitStatus exitStatus)
{
    if (cancelled_) {
        QFile::remove(outputPath_);
        emit cancelled();
        return;
    }

    if (exitStatus != QProcess::NormalExit || exitCode != 0) {
        const QString error = QString::fromLocal8Bit(
            process_->readAllStandardError()
        ).trimmed();

        QFile::remove(outputPath_);

        emit failed(
            error.isEmpty()
                ? "Kompresi gambar gagal."
                : error
        );

        return;
    }

    const qint64 compressedSize =
        QFileInfo(outputPath_).size();

    emit finished(
        outputPath_,
        originalSize_,
        compressedSize
    );
}