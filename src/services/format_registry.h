#pragma once

#include <QString>
#include <QStringList>

class FormatRegistry {
public:
    // Output Format Keys
    static QStringList imageOutputFormats();
    static QStringList videoOutputFormats();
    static QStringList audioOutputFormats();

    // Input Extension Lists (with leading dot, lowercase)
    static QStringList supportedImageInputExtensions();
    static QStringList supportedVideoInputExtensions();
    static QStringList supportedAudioInputExtensions();
    static QStringList supportedImageCompressExtensions();
    static QStringList supportedVideoCompressExtensions();

    // Extension helpers for output keys
    static QString extensionForImageOutput(const QString &formatKey);
    static QString extensionForVideoOutput(const QString &formatKey);
    static QString extensionForAudioOutput(const QString &formatKey);

    // Filter strings for QFileDialog
    static QString audioFileDialogFilter();
    static QString videoFileDialogFilter();
    static QString imageFileDialogFilter();
    static QString compressFileDialogFilter();

    // Validation
    static bool isAudioInputSupported(const QString &filePath);
    static bool isVideoInputSupported(const QString &filePath);
    static bool isImageInputSupported(const QString &filePath);
    static bool isImageCompressSupported(const QString &filePath);
    static bool isVideoCompressSupported(const QString &filePath);
};

