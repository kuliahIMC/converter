#include "format_registry.h"

#include <QFileInfo>
#include <QImageWriter>
#include <QPair>
#include <QList>
#include <QImageReader>

#include "format_registry.h"
#include "../utils/app_paths.h"

#include <QProcess>
#include <QSet>
#include <QStringList>
#include <QRegularExpression>


namespace {

QSet<QString> getFfmpegCapabilities(const QString &argument)
{
    QSet<QString> capabilities;

    const QString ffmpeg = AppPaths::getFfmpegPath();

    if (ffmpeg.isEmpty())
        return capabilities;

    QProcess process;

    process.start(
        ffmpeg,
        {
            "-hide_banner",
            argument
        }
    );

    if (!process.waitForStarted(3000))
        return capabilities;

    if (!process.waitForFinished(5000)) {
        process.kill();
        process.waitForFinished();
        return capabilities;
    }

    if (process.exitStatus() != QProcess::NormalExit ||
        process.exitCode() != 0) {
        return capabilities;
    }

    const QString output =
        QString::fromLocal8Bit(process.readAllStandardOutput());

    const QStringList lines = output.split('\n');

    for (const QString &line : lines) {
        const QStringList parts =
            line.trimmed().split(
                QRegularExpression("\\s+"),
                Qt::SkipEmptyParts
            );

        if (parts.size() >= 2)
            capabilities.insert(parts.at(1).toLower());
    }

    return capabilities;
}

bool hasAny(const QSet<QString> &available,
            const QStringList &candidates)
{
    for (const QString &candidate : candidates) {
        if (available.contains(candidate.toLower()))
            return true;
    }

    return false;
}

}


QStringList FormatRegistry::imageOutputFormats()
{
    const auto qtFormats = QImageWriter::supportedImageFormats();

    const QSet<QString> qtSupported(
        [&]() {
            QSet<QString> result;

            for (const QByteArray &format : qtFormats)
                result.insert(QString::fromLatin1(format).toLower());

            return result;
        }()
    );

    const QSet<QString> encoders =
        getFfmpegCapabilities("-encoders");

    const QSet<QString> muxers =
        getFfmpegCapabilities("-muxers");

    QStringList formats;

    // JPEG
    if (hasAny(encoders, {"mjpeg"}) &&
        muxers.contains("image2")) {
        formats << "JPG";
    }

    // PNG
    if (encoders.contains("png") &&
        muxers.contains("image2")) {
        formats << "PNG";
    }

    // WEBP
    if (encoders.contains("libwebp") &&
        muxers.contains("image2")) {
        formats << "WEBP";
    }

    // BMP
    if (encoders.contains("bmp") &&
        muxers.contains("image2")) {
        formats << "BMP";
    }

    // TIFF
    if (encoders.contains("tiff") &&
        muxers.contains("image2")) {
        formats << "TIFF";
    }

    // AVIF
    if (hasAny(encoders, {
            "libaom-av1",
            "librav1e",
            "libsvtav1"
        }) &&
        muxers.contains("avif")) {
        formats << "AVIF";
    }

    // JPEG 2000
    if (hasAny(encoders, {
            "libopenjpeg",
            "jpeg2000"
        }) &&
        muxers.contains("image2")) {
        formats << "JP2";
    }

    // TGA
    if (encoders.contains("targa") &&
        muxers.contains("image2")) {
        formats << "TGA";
    }

    // ICO: Qt image writer
    if (qtSupported.contains("ico"))
        formats << "ICO";

    // SVG: QtSvg
    formats << "SVG";

    return formats;
    }

QStringList FormatRegistry::videoOutputFormats()
{
    return {
        "MP4", "MKV", "AVI", "MOV", "WEBM",
        "FLV", "WMV", "MPEG", "MPG", "M4V",
        "3GP", "TS", "MTS", "M2TS", "OGV"
    };
}

QStringList FormatRegistry::audioOutputFormats()
{
    return {
        "MP3", "WAV", "FLAC", "AAC",
        "M4A", "OGG", "OPUS", "AIFF"
    };
}

QStringList FormatRegistry::supportedImageInputExtensions()
{
    return {
        ".jpg", ".jpeg", ".png", ".webp", ".dds",
        ".bmp", ".tif", ".tiff", ".tga", ".heic",
        ".heif", ".avif", ".jp2", ".j2k", ".j2c",
        ".jpf", ".jpx", ".ico", ".svg"
    };
}

QStringList FormatRegistry::supportedVideoInputExtensions()
{
    return {
        ".mp4", ".mkv", ".avi", ".mov", ".webm",
        ".flv", ".wmv", ".mpeg", ".mpg", ".m4v",
        ".3gp", ".ts", ".mts", ".m2ts", ".ogv"
    };
}

QStringList FormatRegistry::supportedAudioInputExtensions()
{
    return {
        ".mp3", ".wav", ".flac", ".aac", ".m4a",
        ".ogg", ".oga", ".opus", ".aiff", ".aif",
        ".wma", ".alac",
        // Kontainer video juga didukung sebagai sumber audio
        ".mp4", ".mkv", ".avi", ".mov", ".webm",
        ".flv", ".wmv", ".mpeg", ".mpg", ".m4v",
        ".3gp", ".ts", ".mts", ".m2ts", ".ogv"
    };
}

QStringList FormatRegistry::supportedImageCompressExtensions()
{
    return {
        ".jpg", ".jpeg", ".png", ".webp", ".bmp",
        ".tif", ".tiff", ".heic", ".heif"
    };
}

QStringList FormatRegistry::supportedVideoCompressExtensions()
{
    return supportedVideoInputExtensions();
}

QString FormatRegistry::extensionForImageOutput(
    const QString &formatKey)
{
    const QString key = formatKey.trimmed().toUpper();

    if (key == "JPG") return "jpg";
    if (key == "JPEG") return "jpg";
    if (key == "PNG") return "png";
    if (key == "WEBP") return "webp";
    if (key == "BMP") return "bmp";
    if (key == "TIFF") return "tiff";
    if (key == "AVIF") return "avif";
    if (key == "JP2") return "jp2";
    if (key == "TGA") return "tga";
    if (key == "ICO") return "ico";
    if (key == "ICO") return "ico";
    if (key == "SVG") return "svg";



    return {};
}

QString FormatRegistry::extensionForVideoOutput(const QString &formatKey)
{
    const QString key = formatKey.trimmed().toUpper();
    if (key == "MP4") return "mp4";
    if (key == "MKV") return "mkv";
    if (key == "AVI") return "avi";
    if (key == "MOV") return "mov";
    if (key == "WEBM") return "webm";
    if (key == "FLV") return "flv";
    if (key == "WMV") return "wmv";
    if (key == "MPEG") return "mpeg";
    if (key == "MPG") return "mpg";
    if (key == "M4V") return "m4v";
    if (key == "3GP") return "3gp";
    if (key == "TS") return "ts";
    if (key == "MTS") return "mts";
    if (key == "M2TS") return "m2ts";
    if (key == "OGV") return "ogv";
    return key.toLower();
}

QString FormatRegistry::extensionForAudioOutput(const QString &formatKey)
{
    const QString key = formatKey.trimmed().toUpper();
    if (key == "MP3") return "mp3";
    if (key == "WAV") return "wav";
    if (key == "FLAC") return "flac";
    if (key == "AAC") return "aac";
    if (key == "M4A") return "m4a";
    if (key == "OGG") return "ogg";
    if (key == "OPUS") return "opus";
    if (key == "AIFF") return "aiff";
    return key.toLower();
}

QString FormatRegistry::audioFileDialogFilter()
{
    return "Media Files (*.mp3 *.wav *.flac *.aac *.m4a *.ogg *.oga *.opus *.aiff *.aif *.wma *.alac "
           "*.mp4 *.mkv *.avi *.mov *.webm *.flv *.wmv *.mpeg *.mpg *.m4v *.3gp *.ts *.mts *.m2ts *.ogv);;"
           "Semua File (*.*)";
}

QString FormatRegistry::videoFileDialogFilter()
{
    return "File Video (*.mp4 *.mkv *.avi *.mov *.webm *.flv *.wmv "
           "*.mpeg *.mpg *.m4v *.3gp *.ts *.mts *.m2ts *.ogv);;"
           "Semua File (*.*)";
}

QString FormatRegistry::imageFileDialogFilter()
{
    return "File Gambar (*.jpg *.jpeg *.png *.webp *.bmp *.tif *.tiff *.heic *.heif *.avif *.jp2 *.j2k *.j2c *.jpf *.jpx *.tga *.dds *.ico *.svg);;"
           "Semua File (*.*)";
}

QString FormatRegistry::compressFileDialogFilter()
{
    return "Media Files (*.jpg *.jpeg *.png *.webp *.bmp *.tif *.tiff *.heic *.heif "
           "*.mp4 *.mkv *.avi *.mov *.webm *.flv *.wmv *.mpeg *.mpg *.m4v *.3gp *.ts *.mts *.m2ts *.ogv);;"
           "Semua File (*.*)";
}

bool FormatRegistry::isAudioInputSupported(const QString &filePath)
{
    const QString ext = "." + QFileInfo(filePath).suffix().toLower();
    return supportedAudioInputExtensions().contains(ext);
}

bool FormatRegistry::isVideoInputSupported(const QString &filePath)
{
    const QString ext = "." + QFileInfo(filePath).suffix().toLower();
    return supportedVideoInputExtensions().contains(ext);
}

bool FormatRegistry::isImageInputSupported(const QString &filePath)
{
    const QString ext = "." + QFileInfo(filePath).suffix().toLower();
    return supportedImageInputExtensions().contains(ext);
}

bool FormatRegistry::isImageCompressSupported(const QString &filePath)
{
    const QString ext = "." + QFileInfo(filePath).suffix().toLower();
    return supportedImageCompressExtensions().contains(ext);
}

bool FormatRegistry::isVideoCompressSupported(const QString &filePath)
{
    const QString ext = "." + QFileInfo(filePath).suffix().toLower();
    return supportedVideoCompressExtensions().contains(ext);
}

