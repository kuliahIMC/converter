
#pragma once

#include <QWidget>
#include <QStringList>

class QLabel;
class QPushButton;
class QComboBox;
class QSlider;
class QProgressBar;
class DropListWidget;
class ThemeManager;

class ImageCompressionService;
class VideoCompressionService;

class CompressPage : public QWidget
{
    Q_OBJECT

public:
    explicit CompressPage(ThemeManager *themeManager, QWidget *parent = nullptr);

    QStringList files() const;
    int quality() const;
    QString category() const;

public slots:
    void addFiles(const QStringList &filePaths);
    void selectFilesFromDialog();
    void removeSelected();
    void clearFiles();

private slots:
    void startCompression();
    void cancelCompression();

    void processNextFile();

    void handleCompressionFinished(
        const QString &outputPath,
        qint64 originalSize,
        qint64 compressedSize
    );

    void handleCompressionFailed(const QString &errorMessage);
    void handleCompressionCancelled();

private:
    void setupUI();
    void updateStatus();

    QString createOutputPath(const QString &inputPath) const;
    QString formatFileSize(qint64 bytes) const;

    bool isImageFile(const QString &filePath) const;
    bool isVideoFile(const QString &filePath) const;

    void finishCompression();

    ThemeManager *themeManager_ = nullptr;

    QStringList errors_;

    bool batchFinished_ = false;

    QStringList files_;
    QStringList failedFiles_;

    QString outputDirectory_;

    int currentIndex_ = 0;
    int successCount_ = 0;
    int failedCount_ = 0;

    bool isProcessing_ = false;
    bool cancelRequested_ = false;

    ImageCompressionService *imageService_ = nullptr;
    VideoCompressionService *videoService_ = nullptr;

    DropListWidget *fileList_ = nullptr;
    QComboBox *categoryCombo_ = nullptr;
    QSlider *qualitySlider_ = nullptr;

    QLabel *qualityLabel_ = nullptr;
    QLabel *statusLabel_ = nullptr;

    QPushButton *compressButton_ = nullptr;
    QPushButton *cancelButton_ = nullptr;
    QPushButton *outputFolderButton_ = nullptr;

    QProgressBar *progressBar_ = nullptr;
};
