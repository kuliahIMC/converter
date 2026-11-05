#pragma once

#include <QWidget>
#include <QStringList>

class QLabel;
class QPushButton;
class QComboBox;
class QProgressBar;
class DropListWidget;
class AudioConverterService;
class ThemeManager;

class AudioPage : public QWidget {
    Q_OBJECT

public:
    explicit AudioPage(ThemeManager *themeManager, QWidget *parent = nullptr);

    QStringList files() const;
    QString selectedOutputFormat() const;

public slots:
    void addFiles(const QStringList &filePaths);
    void selectFilesFromDialog();
    void removeSelected();
    void clearFiles();
    void startConversion();
    void cancelConversion();

private slots:
    void processNextFile();
    void handleConversionFinished(const QString &outputPath);
    void handleConversionFailed(const QString &error);
    void updateProgress(int percentage);

private:
    void setupUI();
    void updateStatus();
    QString createOutputPath(const QString &inputPath) const;
    void finishBatch();

    QStringList files_;
    QStringList pendingFiles_;
    QStringList errors_;
    QString outputDirectory_;

    ThemeManager *themeManager_ = nullptr;
    bool batchFinished_ = false;

    DropListWidget *fileList_ = nullptr;
    QComboBox *formatCombo_ = nullptr;
    QLabel *statusLabel_ = nullptr;
    QLabel *progressLabel_ = nullptr;
    QPushButton *convertButton_ = nullptr;
    QPushButton *cancelButton_ = nullptr;
    QProgressBar *progressBar_ = nullptr;

    AudioConverterService *converter_ = nullptr;

    int completedFiles_ = 0;
    int totalFiles_ = 0;
    bool batchCancelled_ = false;
};