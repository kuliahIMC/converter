#pragma once

#include <QWidget>
#include <QStringList>

class QLabel;
class QPushButton;
class QComboBox;
class QProgressBar;
class DropListWidget;
class ImageConverterService;

class PhotoPage : public QWidget
{
    Q_OBJECT

public:
    explicit PhotoPage(QWidget *parent = nullptr);

    QStringList files() const;
    QString selectedOutputFormat() const;

public slots:
    void addFiles(const QStringList &filePaths);
    void selectFilesFromDialog();
    void removeSelected();
    void clearFiles();
    void startBatchConversion();
    void cancelConversion();

private:
    void setupUI();
    void updateStatus();
    void processNextFile();
    void finishBatchConversion();
    QString createOutputPath(const QString &inputPath) const;

    QStringList files_;
    QStringList pendingFiles_;
    QStringList errors_;
    QString outputDirectory_;

    DropListWidget *fileList_ = nullptr;
    QComboBox *formatCombo_ = nullptr;
    QLabel *statusLabel_ = nullptr;
    QPushButton *convertButton_ = nullptr;
    QPushButton *cancelButton_ = nullptr;
    QProgressBar *progressBar_ = nullptr;

    ImageConverterService *converter_ = nullptr;

    int completedFiles_ = 0;
    int totalFiles_ = 0;
    bool batchCancelled_ = false;
};