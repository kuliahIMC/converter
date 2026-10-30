#pragma once

#include <QWidget>
#include <QStringList>

class QLabel;
class QPushButton;
class QComboBox;
class QProgressBar;
class DropListWidget;
class VideoConverterService;

class VideoPage : public QWidget
{
    Q_OBJECT

public:
    explicit VideoPage(QWidget *parent = nullptr);

    QStringList files() const;
    QString selectedOutputFormat() const;

public slots:
    void addFiles(const QStringList &filePaths);
    void selectFilesFromDialog();
    void removeSelected();
    void clearFiles();

private slots:
    void startBatchConversion();
    void processNextFile();
    void handleConversionProgress(int percentage);
    void handleConversionFinished(const QString &outputPath);
    void handleConversionFailed(const QString &error);
    void handleConversionCancelled();
    void cancelConversion();

private:
    void setupUI();
    void updateStatus();
    void finishBatch();
    QString createOutputPath(const QString &inputPath) const;

    QStringList files_;
    QStringList pendingFiles_;
    QStringList errors_;
    QString outputDirectory_;

    int totalFiles_ = 0;
    int completedFiles_ = 0;
    bool batchCancelled_ = false;

    DropListWidget *fileList_ = nullptr;
    QComboBox *formatCombo_ = nullptr;
    QLabel *statusLabel_ = nullptr;
    QLabel *progressLabel_ = nullptr;
    QPushButton *convertButton_ = nullptr;
    QPushButton *cancelButton_ = nullptr;
    QProgressBar *progressBar_ = nullptr;

    VideoConverterService *converter_ = nullptr;
};