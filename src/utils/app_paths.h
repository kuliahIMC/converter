#pragma once

#include <QString>

namespace AppPaths {

// Folder tempat executable binary MarhabanConverter berjalan
QString getBundleDir();

// Folder data pengguna (writable): %LOCALAPPDATA%/Marhaban Studio/Marhaban Converter/data
// atau fallback ke <bundle_dir>/data
QString getDataDir();

// Folder data bawaan aplikasi di direktori bundle: <bundle_dir>/data
QString getBundleDataDir();

// Folder bin di direktori bundle: <bundle_dir>/bin
QString getBinDir();

// Mencari executable (ffmpeg.exe atau ffprobe.exe) di lokasi lokal atau di PATH sistem
QString findExecutable(const QString &commandName);

// Helper spesifik untuk ffmpeg dan ffprobe
QString getFfmpegPath();
QString getFfprobePath();

} // namespace AppPaths

