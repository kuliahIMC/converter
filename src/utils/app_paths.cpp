#include "app_paths.h"

#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QStandardPaths>

namespace AppPaths {

QString getBundleDir()
{
    return QCoreApplication::applicationDirPath();
}

QString getBundleDataDir()
{
    return QDir(getBundleDir()).filePath("data");
}

QString getDataDir()
{
    const QString localAppData = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation);
    if (!localAppData.isEmpty()) {
        QDir dir(localAppData);
        return dir.filePath("Marhaban Studio/Marhaban Converter/data");
    }

    return getBundleDataDir();
}

QString getBinDir()
{
    return QDir(getBundleDir()).filePath("bin");
}

QString findExecutable(const QString &commandName)
{
    QString exeName = commandName;
    if (!exeName.endsWith(".exe", Qt::CaseInsensitive)) {
        exeName += ".exe";
    }

    // 1. Cek di <bundle_dir>/bin/
    const QString inBin = QDir(getBinDir()).filePath(exeName);
    if (QFileInfo::exists(inBin)) {
        return QDir::toNativeSeparators(inBin);
    }

    // 2. Cek di <bundle_dir>/external/ffmpeg/
    const QString inExternal = QDir(getBundleDir()).filePath("external/ffmpeg/" + exeName);
    if (QFileInfo::exists(inExternal)) {
        return QDir::toNativeSeparators(inExternal);
    }

    // 3. Cek di <bundle_dir>/ langsung
    const QString inRoot = QDir(getBundleDir()).filePath(exeName);
    if (QFileInfo::exists(inRoot)) {
        return QDir::toNativeSeparators(inRoot);
    }

    // 4. Fallback ke PATH sistem
    const QString inPath = QStandardPaths::findExecutable(commandName);
    if (!inPath.isEmpty()) {
        return QDir::toNativeSeparators(inPath);
    }

    return QString();
}

QString getFfmpegPath()
{
    return findExecutable("ffmpeg");
}

QString getFfprobePath()
{
    return findExecutable("ffprobe");
}

} // namespace AppPaths

