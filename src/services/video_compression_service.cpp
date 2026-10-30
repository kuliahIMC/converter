
#include "video_compression_service.h"

#include <QFile>
#include <QFileInfo>
#include <QRegularExpression>

VideoCompressionService::VideoCompressionService(QObject *parent)
    : QObject(parent),
      process_(new QProcess(this))
{
    process_->setProcessChannelMode(QProcess::SeparateChannels);

    connect(
        process_,
        &QProcess::readyReadStandardError,
        this,
        &VideoCompressionService::readOutput
    );

    connect(
        process_,
        QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
        this,
        &VideoCompressionService::handleFinished
    );
}

VideoCompressionService::~VideoCompressionService()
{
    if (isRunning()) {
        process_->kill();
        process_->waitForFinished(1000);
    }
}

bool VideoCompressionService::isRunning() const
{
    return process_ &&
           process_->state() != QProcess::NotRunning;
}

QString VideoCompressionService::ffmpegPath() const
{
    return "ffmpeg";
}

void VideoCompressionService::compress(
    const QString &inputPath,
    const QString &outputPath,
    int quality)
{
    if (isRunning())
        return;

    if (!QFileInfo::exists(inputPath)) {
        emit failed("File video tidak ditemukan.");
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
    const int crf = 40 - (clampedQuality * 20 / 100);

    QString videoCodec = "libx264";
    QString audioCodec = "aac";
    QString audioBitrate = "128k";

    if (extension == "webm" || extension == "ogv") {
        videoCodec = extension == "webm" ? "libvpx-vp9" : "libtheora";
        audioCodec = extension == "webm" ? "libopus" : "libvorbis";
        audioBitrate = "128k";
    } else if (extension == "avi") {
        videoCodec = "mpeg4";
        audioCodec = "libmp3lame";
        audioBitrate = "128k";
    }

    QStringList arguments;

    arguments
        << "-y"
        << "-hide_banner"
        << "-nostats"
        << "-progress" << "pipe:2"
        << "-i" << inputPath
        << "-map" << "0:v:0"
        << "-map" << "0:a?"
        << "-c:v" << videoCodec;

    if (videoCodec == "libvpx-vp9") {
        arguments << "-crf" << QString::number(crf)
                  << "-b:v" << "0";
    } else if (videoCodec == "libtheora") {
        arguments << "-q:v"
                  << QString::number(qBound(1, clampedQuality / 10, 10));
    } else if (videoCodec == "mpeg4") {
        arguments << "-q:v"
                  << QString::number(qBound(2, 31 - clampedQuality * 29 / 100, 31));
    } else {
        arguments << "-preset" << "medium"
                  << "-crf" << QString::number(crf);
    }

    arguments
        << "-c:a" << audioCodec
        << "-b:a" << audioBitrate;

    if (extension == "mp4" ||
        extension == "m4v" ||
        extension == "mov") {
        arguments << "-movflags" << "+faststart";
    }

    arguments << outputPath;

    outputPath_ = outputPath;
    originalSize_ = QFileInfo(inputPath).size();
    durationMs_ = 0;
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

void VideoCompressionService::readOutput()
{
    static const QRegularExpression durationRegex(
        R"(Duration:\s*(\d+):(\d+):(\d+(?:\.\d+)?))"
    );

    static const QRegularExpression timeRegex(
        R"(out_time=(\d+):(\d+):(\d+(?:\.\d+)?))"
    );

    const QString output = QString::fromLocal8Bit(
        process_->readAllStandardError()
    );

    auto durationMatch = durationRegex.match(output);

    if (durationMatch.hasMatch()) {
        const double hours = durationMatch.captured(1).toDouble();
        const double minutes = durationMatch.captured(2).toDouble();
        const double seconds = durationMatch.captured(3).toDouble();

        durationMs_ = static_cast<qint64>(
            (hours * 3600 + minutes * 60 + seconds) * 1000
        );
    }

    auto timeMatch = timeRegex.match(output);

    if (timeMatch.hasMatch() && durationMs_ > 0) {
        const double hours = timeMatch.captured(1).toDouble();
        const double minutes = timeMatch.captured(2).toDouble();
        const double seconds = timeMatch.captured(3).toDouble();

        const qint64 currentMs = static_cast<qint64>(
            (hours * 3600 + minutes * 60 + seconds) * 1000
        );

        const int percentage = qBound(
            0,
            static_cast<int>(currentMs * 100 / durationMs_),
            99
        );

        emit progressChanged(percentage);
    }
}

void VideoCompressionService::cancel()
{
    if (!isRunning())
        return;

    cancelled_ = true;
    process_->kill();
}

void VideoCompressionService::handleFinished(
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
                ? "Kompresi video gagal."
                : error
        );

        return;
    }

    const qint64 compressedSize =
        QFileInfo(outputPath_).size();

    emit progressChanged(100);

    emit finished(
        outputPath_,
        originalSize_,
        compressedSize
    );
}
