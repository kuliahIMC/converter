#include "format_registry.h"

#include <QFileInfo>

QStringList FormatRegistry::imageOutputFormats()
{
    return {
        "JPG", "JPEG", "PNG", "WEBP", "BMP",
        "TIFF", "HEIC", "AVIF", "JP2", "TGA",
        "DDS", "ICO", "SVG"
    };
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

QString FormatRegistry::extensionForImageOutput(const QString &formatKey)
{
    const QString key = formatKey.trimmed().toUpper();
    if (key == "JPG") return "jpg";
    if (key == "JPEG") return "jpeg";
    if (key == "PNG") return "png";
    if (key == "WEBP") return "webp";
    if (key == "BMP") return "bmp";
    if (key == "TIFF") return "tiff";
    if (key == "HEIC") return "heic";
    if (key == "AVIF") return "avif";
    if (key == "JP2") return "jp2";
    if (key == "TGA") return "tga";
    if (key == "DDS") return "dds";
    if (key == "ICO") return "ico";
    if (key == "SVG") return "svg";
    return key.toLower();
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

