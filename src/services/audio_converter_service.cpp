#include "audio_converter_service.h"

#include <QProcess>
#include <QFileInfo>
#include <QDir>
#include <QRegularExpression>

#include "format_registry.h"

AudioConverterService::AudioConverterService(QObject *parent)
    : QObject(parent)
{
    process_ = new QProcess(this);

    connect(
        process_,
        &QProcess::readyReadStandardError,
        this,
        &AudioConverterService::readOutput
    );

    connect(
        process_,
        qOverload<int, QProcess::ExitStatus>(&QProcess::finished),
        this,
        &AudioConverterService::handleFinished
    );
}

AudioConverterService::~AudioConverterService()
{
    if (process_ && process_->state() != QProcess::NotRunning) {
        process_->kill();
        process_->waitForFinished(1000);
    }
}

bool AudioConverterService::isRunning() const
{
    return process_->state() != QProcess::NotRunning;
}

QString AudioConverterService::ffmpegPath() const
{
    return "ffmpeg";
}

void AudioConverterService::convert(
    const QString &inputPath,
    const QString &outputPath,
    const QString &format
)
{
    if (isRunning()) {
        emit failed("Konversi audio sedang berjalan.");
        return;
    }

    if (!QFileInfo::exists(inputPath)) {
        emit failed("File input tidak ditemukan.");
        return;
    }

    if (!FormatRegistry::isAudioInputSupported(inputPath)) {
        emit failed("Format file input tidak didukung.");
        return;
    }

    const QString normalizedFormat = format.trimmed().toUpper();

    if (!FormatRegistry::audioOutputFormats().contains(normalizedFormat)) {
        emit failed("Format output tidak didukung.");
        return;
    }

    if (outputPath.trimmed().isEmpty()) {
        emit failed("Lokasi output belum ditentukan.");
        return;
    }

    const QFileInfo outputInfo(outputPath);

    if (!QDir().mkpath(outputInfo.absolutePath())) {
        emit failed("Tidak dapat membuat folder output.");
        return;
    }

    outputPath_ = outputPath;
    cancelled_ = false;
    durationMs_ = 0;

    QStringList arguments;

    arguments << "-y"
              << "-hide_banner"
              << "-nostats"
              << "-progress" << "pipe:2"
              << "-i" << inputPath
              << "-vn"
              << "-c:a" << "aac";

    if (normalizedFormat == "MP3") {
        arguments << "-c:a" << "libmp3lame"
                  << "-q:a" << "2";
    }
    else if (normalizedFormat == "WAV") {
        arguments << "-c:a" << "pcm_s16le";
    }
    else if (normalizedFormat == "FLAC") {
        arguments << "-c:a" << "flac";
    }
    else if (normalizedFormat == "AAC") {
        arguments << "-c:a" << "aac";
    }
    else if (normalizedFormat == "M4A") {
        arguments << "-c:a" << "aac";
    }
    else if (normalizedFormat == "OGG") {
        arguments << "-c:a" << "libvorbis"
                  << "-q:a" << "5";
    }
    else if (normalizedFormat == "OPUS") {
        arguments << "-c:a" << "libopus"
                  << "-b:a" << "128k";
    }
    else if (normalizedFormat == "AIFF") {
        arguments << "-c:a" << "pcm_s16be";
    }

    arguments << outputPath;

    process_->start(ffmpegPath(), arguments);

    if (!process_->waitForStarted(3000)) {
        emit failed(
            "FFmpeg tidak dapat dijalankan. Pastikan FFmpeg sudah terpasang "
            "dan tersedia di PATH."
        );
        return;
    }

    emit started();
}

void AudioConverterService::readOutput()
{
    const QString output = QString::fromUtf8(
        process_->readAllStandardError()
    );

    static const QRegularExpression durationRegex(
        R"(Duration:\s*(\d+):(\d+):(\d+(?:\.\d+)?))"
    );

    static const QRegularExpression timeRegex(
        R"(out_time_ms=(\d+))"
    );

    QRegularExpressionMatch durationMatch =
        durationRegex.match(output);

    if (durationMatch.hasMatch()) {
        const double hours = durationMatch.captured(1).toDouble();
        const double minutes = durationMatch.captured(2).toDouble();
        const double seconds = durationMatch.captured(3).toDouble();

        durationMs_ = static_cast<qint64>(
            (hours * 3600 + minutes * 60 + seconds) * 1000
        );
    }

    if (durationMs_ <= 0) {
        return;
    }

    QRegularExpressionMatchIterator matches =
        timeRegex.globalMatch(output);

    while (matches.hasNext()) {
        const QRegularExpressionMatch match = matches.next();

        const qint64 currentMs =
            match.captured(1).toLongLong() / 1000;

        const int percentage = qBound(
            0,
            static_cast<int>(
                (currentMs * 100) / durationMs_
            ),
            99
        );

        emit progressChanged(percentage);
    }
}

void AudioConverterService::handleFinished(
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

    const QString error = QString::fromUtf8(
        process_->readAllStandardError()
    ).trimmed();

    emit failed(
        error.isEmpty()
            ? "Konversi audio gagal."
            : error
    );
}

void AudioConverterService::cancel()
{
    if (!isRunning()) {
        return;
    }

    cancelled_ = true;
    process_->kill();
}