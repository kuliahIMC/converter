#include "video_converter_service.h"

#include <QFileInfo>
#include <QRegularExpression>
#include <QDir>

VideoConverterService::VideoConverterService(QObject *parent)
    : QObject(parent)
{
    process_ = new QProcess(this);

    connect(
        process_,
        &QProcess::readyReadStandardError,
        this,
        &VideoConverterService::readOutput
    );

    connect(
        process_,
        qOverload<int, QProcess::ExitStatus>(&QProcess::finished),
        this,
        &VideoConverterService::handleFinished
    );
}

VideoConverterService::~VideoConverterService()
{
    if (process_->state() != QProcess::NotRunning) {
        process_->kill();
        process_->waitForFinished(2000);
    }
}

bool VideoConverterService::isRunning() const
{
    return process_->state() != QProcess::NotRunning;
}

QString VideoConverterService::ffmpegPath() const
{
    return QStringLiteral("ffmpeg");
}

void VideoConverterService::convert(
    const QString &inputPath,
    const QString &outputPath,
    const QString &format
)
{
    if (isRunning()) {
        emit failed("Proses konversi video masih berjalan.");
        return;
    }

    if (!QFileInfo::exists(inputPath)) {
        emit failed("File video tidak ditemukan.");
        return;
    }

    if (outputPath.isEmpty()) {
        emit failed("Lokasi output tidak valid.");
        return;
    }

    if (QFileInfo(inputPath).absoluteFilePath()
        == QFileInfo(outputPath).absoluteFilePath()) {
        emit failed("File output tidak boleh sama dengan file input.");
        return;
    }

    outputPath_ = outputPath;
    durationMs_ = 0;
    cancelled_ = false;

    const QString extension =
        QFileInfo(outputPath).suffix().toLower();

    QStringList arguments;

    arguments << "-y"
              << "-hide_banner"
              << "-nostats"
              << "-progress" << "pipe:2"
              << "-i" << inputPath;

    // Pengaturan codec berdasarkan format output.
    if (extension == "webm") {
        arguments << "-c:v" << "libvpx-vp9"
                  << "-crf" << "32"
                  << "-b:v" << "0"
                  << "-c:a" << "libopus";
    }
    else if (extension == "avi") {
        arguments << "-c:v" << "mpeg4"
                  << "-q:v" << "5"
                  << "-c:a" << "libmp3lame"
                  << "-q:a" << "4";
    }
    else if (extension == "gif") {
        arguments << "-an"
                  << "-vf"
                  << "fps=15,scale=640:-1:flags=lanczos";
    }
    else {
        arguments << "-c:v" << "libx264"
                  << "-preset" << "medium"
                  << "-crf" << "23"
                  << "-c:a" << "aac"
                  << "-b:a" << "192k";
    }

    if (extension == "mp4" || extension == "m4v" || extension == "mov") {
        arguments << "-movflags" << "+faststart";
    }
    
    arguments << outputPath;

    process_->start(ffmpegPath(), arguments);

    if (!process_->waitForStarted(3000)) {
        emit failed(
            "FFmpeg gagal dijalankan. Pastikan FFmpeg sudah terpasang "
            "dan tersedia di PATH."
        );
        return;
    }

    emit started();
}

void VideoConverterService::readOutput()
{
    const QString output =
        QString::fromLocal8Bit(process_->readAllStandardError());

    // Membaca durasi video.
    static const QRegularExpression durationRegex(
        R"(Duration:\s*(\d+):(\d+):(\d+(?:\.\d+)?))"
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

    // Membaca waktu proses FFmpeg.
    static const QRegularExpression timeRegex(
        R"(out_time=(\d+):(\d+):(\d+(?:\.\d+)?))"
    );

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
            static_cast<int>(
                (static_cast<double>(currentMs) / durationMs_) * 100
            ),
            99
        );

        emit progressChanged(percentage);
    }
}

void VideoConverterService::handleFinished(
    int exitCode,
    QProcess::ExitStatus exitStatus
)
{
    if (cancelled_) {
        emit cancelled();
        return;
    }

    if (exitStatus == QProcess::NormalExit && exitCode == 0) {
        emit progressChanged(100);
        emit finished(outputPath_);
        return;
    }

    QString error =
        QString::fromLocal8Bit(process_->readAllStandardError()).trimmed();

    if (error.isEmpty()) {
        error = "Konversi video gagal. Periksa format dan file input.";
    }

    emit failed(error);
}

void VideoConverterService::cancel()
{
    if (!isRunning()) {
        return;
    }

    cancelled_ = true;
    process_->kill();
}