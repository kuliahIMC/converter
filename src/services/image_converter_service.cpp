
#include "image_converter_service.h"

#include "../utils/app_paths.h"

#include <QFile>
#include <QFileInfo>
#include <QImage>
#include <QImageReader>
#include <QImageWriter>
#include <QTemporaryDir>
#include <QDir>
#include <QPainter>
#include <QSvgGenerator>
#include <QSvgRenderer>
#include <QRect>
#include <QPoint>
#include <QIODevice>

ImageConverterService::ImageConverterService(QObject *parent)
    : QObject(parent),
      process_(new QProcess(this))
{
    process_->setProcessChannelMode(QProcess::SeparateChannels);

    connect(
        process_,
        QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
        this,
        &ImageConverterService::handleFinished
    );

    connect(
        process_,
        &QProcess::errorOccurred,
        this,
        &ImageConverterService::handleProcessError
    );
}

ImageConverterService::~ImageConverterService()
{
    if (process_ &&
        process_->state() != QProcess::NotRunning) {
        process_->kill();
        process_->waitForFinished(1000);
    }

    cleanup();
}

bool ImageConverterService::isRunning() const
{
    return running_;
}

void ImageConverterService::cleanup()
{
    if (process_ &&
        process_->state() != QProcess::NotRunning) {
        process_->waitForFinished(1000);
    }

    delete temporaryDir_;
    temporaryDir_ = nullptr;

    running_ = false;
    cancelled_ = false;
}

bool ImageConverterService::prepareQtInput(
    const QString &inputPath)
{
    const QString extension =
        QFileInfo(inputPath).suffix().toLower();

    if (extension != "svg" && extension != "ico")
        return true;

    temporaryDir_ = new QTemporaryDir();

    if (!temporaryDir_->isValid())
        return false;

    QImageReader reader(inputPath);
    reader.setAutoTransform(true);

    const QImage image = reader.read();

    if (image.isNull())
        return false;

    const QString temporaryPng =
        temporaryDir_->filePath("input.png");

    if (!image.save(temporaryPng, "PNG"))
        return false;

    inputPath_ = temporaryPng;

    return true;
}

bool ImageConverterService::convertWithQt(
    const QString &inputPath,
    const QString &outputPath)
{
    QImageReader reader(inputPath);
    reader.setAutoTransform(true);

    const QImage image = reader.read();

    if (image.isNull()) {
        emit failed(
            "Gagal membaca gambar: " + reader.errorString()
        );
        return false;
    }

    QImageWriter writer(outputPath, "ico");

    if (!writer.write(image)) {
        emit failed(
            "Gagal menyimpan ICO: " + writer.errorString()
        );
        return false;
    }

    return true;
}

void ImageConverterService::convert(
    const QString &inputPath,
    const QString &outputPath,
    const QString &format)
{
    if (running_)
        return;

    if (!QFileInfo::exists(inputPath)) {
        emit failed("File gambar tidak ditemukan.");
        return;
    }

    if (outputPath.trimmed().isEmpty()) {
        emit failed("Lokasi output tidak valid.");
        return;
    }

    if (QFileInfo(inputPath).absoluteFilePath() ==
        QFileInfo(outputPath).absoluteFilePath()) {
        emit failed(
            "File output tidak boleh sama dengan file input."
        );
        return;
    }

    const QString key = format.trimmed().toUpper();

    const QString extension = key == "JPG" ? "jpg" :
                              key == "JPEG" ? "jpg" :
                              key == "PNG" ? "png" :
                              key == "WEBP" ? "webp" :
                              key == "BMP" ? "bmp" :
                              key == "TIFF" ? "tiff" :
                              key == "AVIF" ? "avif" :
                              key == "JP2" ? "jp2" :
                              key == "TGA" ? "tga" :
                              key == "ICO" ? "ico" :
                              key == "SVG" ? "svg" :
                              QString();

    if (extension.isEmpty()) {
        emit failed("Format output tidak didukung.");
        return;
    }

    if (QFileInfo(outputPath).suffix().compare(
            extension, Qt::CaseInsensitive) != 0) {
        emit failed("Ekstensi file output tidak sesuai format.");
        return;
    }

    running_ = true;
    cancelled_ = false;

    inputPath_ = inputPath;
    outputPath_ = outputPath;

    emit started();

    // ICO menggunakan Qt Image Writer.
    if (key == "ICO") {
        const bool success =
            convertWithQt(inputPath, outputPath);

        if (cancelled_) {
            QFile::remove(outputPath);
            cleanup();
            emit cancelled();
            return;
        }

        cleanup();

        if (success)
            emit finished(outputPath);

        return;
    }

    // SVG menggunakan QtSvg.
    if (key == "SVG") {
        const QString inputExtension =
            QFileInfo(inputPath).suffix().toLower();

        // SVG ke SVG: pertahankan dokumen aslinya.
        if (inputExtension == "svg") {
            QSvgRenderer renderer(inputPath);

            if (!renderer.isValid()) {
                cleanup();
                emit failed(
                    "File SVG sumber tidak valid atau tidak dapat dibaca."
                );
                return;
            }

            if (QFile::exists(outputPath))
                QFile::remove(outputPath);

            if (!QFile::copy(inputPath, outputPath)) {
                cleanup();
                emit failed("Gagal menyalin file SVG.");
                return;
            }

            cleanup();
            emit finished(outputPath);
            return;
        }

        // Raster ke SVG: gambar raster ditanam ke dalam SVG.
        QImageReader reader(inputPath);
        reader.setAutoTransform(true);

        const QImage image = reader.read();

        if (image.isNull()) {
            const QString error = reader.errorString();

            cleanup();
            emit failed(
                "Gagal membaca gambar: " + error
            );
            return;
        }

        QFile outputFile(outputPath);

        if (!outputFile.open(QIODevice::WriteOnly)) {
            const QString error = outputFile.errorString();

            cleanup();
            emit failed(
                "Gagal membuat file SVG: " + error
            );
            return;
        }

        QSvgGenerator generator;
        generator.setOutputDevice(&outputFile);
        generator.setSize(image.size());
        generator.setViewBox(
            QRect(QPoint(0, 0), image.size())
        );
        generator.setTitle(
            QFileInfo(inputPath).completeBaseName()
        );
        generator.setDescription(
            "Generated by Marhaban Converter"
        );

        QPainter painter;

        if (!painter.begin(&generator)) {
            outputFile.close();
            QFile::remove(outputPath);

            cleanup();
            emit failed(
                "Gagal memulai proses penulisan SVG."
            );
            return;
        }

        painter.drawImage(
            QRect(QPoint(0, 0), image.size()),
            image
        );

        const bool painterFinished = painter.end();

        outputFile.close();

        if (!painterFinished ||
            outputFile.error() != QFileDevice::NoError) {
            QFile::remove(outputPath);

            cleanup();
            emit failed(
                "Gagal menyelesaikan penulisan file SVG."
            );
            return;
        }

        QSvgRenderer validator(outputPath);

        if (!validator.isValid()) {
            QFile::remove(outputPath);

            cleanup();
            emit failed(
                "File SVG hasil konversi tidak valid."
            );
            return;
        }

        cleanup();
        emit finished(outputPath);
        return;
    }

    // SVG dan ICO perlu dirender Qt menjadi PNG.
    if (!prepareQtInput(inputPath)) {
        cleanup();
        emit failed(
            "Gagal membaca atau merender gambar menggunakan Qt."
        );
        return;
    }

    const QString ffmpegInput = inputPath_;

    const QString ffmpeg = AppPaths::getFfmpegPath();

    if (ffmpeg.isEmpty()) {
        cleanup();
        emit failed(
            "FFmpeg tidak ditemukan. Periksa folder bin atau PATH."
        );
        return;
    }

    QStringList arguments;

    arguments << "-y"
              << "-hide_banner"
              << "-loglevel" << "error"
              << "-i" << ffmpegInput
              << "-map" << "0:v:0"
              << "-frames:v" << "1";

    if (key == "JPG" || key == "JPEG") {
        arguments << "-c:v" << "mjpeg"
                  << "-q:v" << "2";
    }
    else if (key == "PNG") {
        arguments << "-c:v" << "png"
                  << "-compression_level" << "6";
    }
    else if (key == "WEBP") {
        arguments << "-c:v" << "libwebp"
                  << "-quality" << "90"
                  << "-compression_level" << "6";
    }
    else if (key == "BMP") {
        arguments << "-c:v" << "bmp";
    }
    else if (key == "TIFF") {
        arguments << "-c:v" << "tiff";
    }
    else if (key == "AVIF") {
        arguments << "-c:v" << "libaom-av1"
                  << "-crf" << "30"
                  << "-b:v" << "0"
                  << "-f" << "avif";
    }
    else if (key == "JP2") {
        arguments << "-c:v" << "libopenjpeg";
    }
    else if (key == "TGA") {
        arguments << "-c:v" << "targa";
    }

    arguments << outputPath;

    process_->start(ffmpeg, arguments);
}

void ImageConverterService::cancel()
{
    if (!running_)
        return;

    cancelled_ = true;

    if (process_->state() != QProcess::NotRunning) {
        process_->kill();
        return;
    }

    QFile::remove(outputPath_);
    cleanup();

    emit cancelled();
}

void ImageConverterService::handleFinished(
    int exitCode,
    QProcess::ExitStatus exitStatus)
{
    const QString error =
        QString::fromLocal8Bit(
            process_->readAllStandardError()
        ).trimmed();

    if (cancelled_) {
        QFile::remove(outputPath_);
        cleanup();
        emit cancelled();
        return;
    }

    if (exitStatus != QProcess::NormalExit ||
        exitCode != 0) {
        QFile::remove(outputPath_);
        cleanup();

        emit failed(
            error.isEmpty()
                ? "Konversi gambar gagal."
                : error
        );

        return;
    }

    const bool outputExists =
        QFileInfo::exists(outputPath_) &&
        QFileInfo(outputPath_).size() > 0;

    if (!outputExists) {
        cleanup();
        emit failed(
            "FFmpeg selesai tetapi file output tidak ditemukan."
        );
        return;
    }

    QImageReader reader(outputPath_);
    QImage image = reader.read();

    if (image.isNull()) {
        const QString readerError = reader.errorString();

        QFile::remove(outputPath_);
        cleanup();

        emit failed(
            "File output gagal dibaca sebagai gambar: " +
            readerError
        );
        return;
    }

    const QString result = outputPath_;

    cleanup();

    emit finished(result);
}

void ImageConverterService::handleProcessError(
    QProcess::ProcessError error)
{
    if (error != QProcess::FailedToStart || !running_)
        return;

    const QString message =
        "FFmpeg gagal dijalankan: " + process_->errorString();

    QFile::remove(outputPath_);
    cleanup();

    emit failed(message);
}
