#include "video_page.h"
#include "drop_list.h"

#include "../services/format_registry.h"
#include "../services/video_converter_service.h"

#include <QAbstractItemView>
#include <QComboBox>
#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QLabel>
#include <QListWidgetItem>
#include <QMessageBox>
#include <QProgressBar>
#include <QPushButton>
#include <QVBoxLayout>

VideoPage::VideoPage(QWidget *parent)
    : QWidget(parent)
{
    converter_ = new VideoConverterService(this);

    connect(
        converter_,
        &VideoConverterService::progressChanged,
        this,
        &VideoPage::handleConversionProgress
    );

    connect(
        converter_,
        &VideoConverterService::finished,
        this,
        &VideoPage::handleConversionFinished
    );

    connect(
        converter_,
        &VideoConverterService::failed,
        this,
        &VideoPage::handleConversionFailed
    );

    connect(
        converter_,
        &VideoConverterService::cancelled,
        this,
        &VideoPage::handleConversionCancelled
    );

    setupUI();
}

QStringList VideoPage::files() const
{
    return files_;
}

QString VideoPage::selectedOutputFormat() const
{
    return formatCombo_ ? formatCombo_->currentText() : QString();
}

// ... existing code ...
void VideoPage::setupUI()
{
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(12);

    auto *title = new QLabel("Video Converter");
    title->setStyleSheet(
        "font-size:24px;font-weight:700;"
    );

    auto *description = new QLabel(
        "Convert video ke berbagai format dengan FFmpeg."
    );

    description->setStyleSheet(
        "font-size:13px;color:#737373;"
    );

    layout->addWidget(title);
    layout->addWidget(description);
    layout->addSpacing(12);

    // Toolbar
    auto *toolbar = new QHBoxLayout;

    auto *addButton = new QPushButton("+ Tambahkan Video");
    addButton->setObjectName("primaryButton");
    addButton->setCursor(Qt::PointingHandCursor);

    connect(
        addButton,
        &QPushButton::clicked,
        this,
        &VideoPage::selectFilesFromDialog
    );

    auto *removeButton = new QPushButton("Hapus");
    removeButton->setCursor(Qt::PointingHandCursor);

    connect(
        removeButton,
        &QPushButton::clicked,
        this,
        &VideoPage::removeSelected
    );

    auto *clearButton = new QPushButton("Bersihkan");
    clearButton->setCursor(Qt::PointingHandCursor);

    connect(
        clearButton,
        &QPushButton::clicked,
        this,
        &VideoPage::clearFiles
    );

    toolbar->addWidget(addButton);
    toolbar->addWidget(removeButton);
    toolbar->addWidget(clearButton);
    toolbar->addStretch();

    auto *formatLabel = new QLabel("Output:");
    formatLabel->setStyleSheet("font-size:13px;font-weight:600;");

    formatCombo_ = new QComboBox;
    formatCombo_->setMinimumWidth(130);
    formatCombo_->addItems(FormatRegistry::videoOutputFormats());
    formatCombo_->setCursor(Qt::PointingHandCursor);

    toolbar->addWidget(formatLabel);
    toolbar->addWidget(formatCombo_);

    layout->addLayout(toolbar);

    // File list
    fileList_ = new DropListWidget(this);
    fileList_->setFilterMode(DropListWidget::FilterMode::Video);
    fileList_->setSelectionMode(QAbstractItemView::ExtendedSelection);
    fileList_->setMinimumHeight(220);

    fileList_->setStyleSheet(R"(
            QListWidget {
                border: 1px solid rgba(255, 255, 255, 0.15);
                border-radius: 8px;
                padding: 6px;
                outline: none;
            }
            QListWidget::item {
                padding: 8px 10px;
                border-bottom: 1px solid rgba(255, 255, 255, 0.05);
            }
            QListWidget::item:selected {
                background: rgba(255, 255, 255, 0.15);
                color: #ffffff;
            }
        )");

    connect(
        fileList_,
        &DropListWidget::filesDropped,
        this,
        &VideoPage::addFiles
    );

    layout->addWidget(fileList_, 1);

    // Progress section
    progressLabel_ = new QLabel("Menunggu konversi...");
    progressLabel_->setStyleSheet(
        "font-size:13px;color:#737373;"
    );

    progressBar_ = new QProgressBar;
    progressBar_->setRange(0, 1000);
    progressBar_->setValue(0);
    progressBar_->setTextVisible(false);

    progressBar_->hide();
    progressLabel_->hide();

    layout->addWidget(progressLabel_);
    layout->addWidget(progressBar_);

    // Bottom section
    auto *bottomLayout = new QHBoxLayout;

    statusLabel_ = new QLabel("Belum ada video dipilih.");
    statusLabel_->setStyleSheet(
        "font-size:13px;color:#737373;"
    );

    cancelButton_ = new QPushButton("Batalkan");
    cancelButton_->setObjectName("cancelButton");
    cancelButton_->setMinimumHeight(38);
    cancelButton_->setMinimumWidth(105);
    cancelButton_->setCursor(Qt::PointingHandCursor);
    cancelButton_->setStyleSheet(R"(
        QPushButton#cancelButton {
            background-color: #ffffff;
            color: #dc2626;
            border: 1px solid #fecaca;
            border-radius: 7px;
            padding: 8px 16px;
            font-size: 13px;
            font-weight: 600;
        }

        QPushButton#cancelButton:hover {
            background-color: #fef2f2;
            border-color: #ef4444;
        }

        QPushButton#cancelButton:pressed {
            background-color: #fee2e2;
        }

        QPushButton#cancelButton:disabled {
            background-color: #f5f5f5;
            color: #a3a3a3;
            border-color: #e5e5e5;
        }
    )");
    cancelButton_->hide();

    connect(
        cancelButton_,
        &QPushButton::clicked,
        this,
        &VideoPage::cancelConversion
    );

    convertButton_ = new QPushButton("Convert");
    convertButton_->setObjectName("primaryButton");
    convertButton_->setCursor(Qt::PointingHandCursor);
    convertButton_->setEnabled(false);

    connect(
        convertButton_,
        &QPushButton::clicked,
        this,
        &VideoPage::startBatchConversion
    );

    bottomLayout->addWidget(statusLabel_);
    bottomLayout->addStretch();
    bottomLayout->addWidget(cancelButton_);
    bottomLayout->addWidget(convertButton_);

    layout->addLayout(bottomLayout);

    updateStatus();
}

void VideoPage::selectFilesFromDialog()

{
    if (converter_->isRunning()) {
        return;
    }

    const QStringList selected = QFileDialog::getOpenFileNames(
        this,
        "Pilih File Video",
        QString(),
        FormatRegistry::videoFileDialogFilter()
    );

    if (!selected.isEmpty()) {
        addFiles(selected);
    }
}

void VideoPage::addFiles(const QStringList &filePaths)
{
    if (converter_->isRunning()) {
        return;
    }

    bool added = false;

    for (const QString &path : filePaths) {
        if (!files_.contains(path)
            && FormatRegistry::isVideoInputSupported(path)) {

            files_.append(path);

            auto *item = new QListWidgetItem(
                QFileInfo(path).fileName(),
                fileList_
            );

            item->setData(Qt::UserRole, path);
            item->setToolTip(path);

            added = true;
        }
    }

    if (added) {
        updateStatus();
    }
}

void VideoPage::removeSelected()
{
    if (converter_->isRunning()) {
        return;
    }

    const QList<QListWidgetItem *> selectedItems =
        fileList_->selectedItems();

    if (selectedItems.isEmpty()) {
        return;
    }

    for (auto *item : selectedItems) {
        const QString path = item->data(Qt::UserRole).toString();

        files_.removeAll(path);
        delete item;
    }

    updateStatus();
}

void VideoPage::clearFiles()
{
    if (converter_->isRunning() || files_.isEmpty()) {
        return;
    }

    files_.clear();
    fileList_->clear();

    updateStatus();
}

void VideoPage::updateStatus()
{
    const int count = files_.size();

    if (count == 0) {
        statusLabel_->setText("Belum ada video dipilih.");
        convertButton_->setEnabled(false);
    } else {
        statusLabel_->setText(
            QString("%1 video siap diproses.").arg(count)
        );

        convertButton_->setEnabled(true);
    }
}

QString VideoPage::createOutputPath(const QString &inputPath) const
{
    const QFileInfo inputInfo(inputPath);

    const QString extension =
        FormatRegistry::extensionForVideoOutput(selectedOutputFormat());

    const QString baseName =
        inputInfo.completeBaseName() + "_converted";

    QDir directory(outputDirectory_);

    QString outputPath = directory.filePath(
        baseName + "." + extension
    );

    int counter = 2;

    while (QFileInfo::exists(outputPath)) {
        outputPath = directory.filePath(
            QString("%1_%2.%3")
                .arg(baseName)
                .arg(counter++)
                .arg(extension)
        );
    }

    return outputPath;
}

void VideoPage::startBatchConversion()
{
    if (converter_->isRunning() || files_.isEmpty()) {
        return;
    }

    outputDirectory_ = QFileDialog::getExistingDirectory(
        this,
        "Pilih Folder Penyimpanan Video",
        QDir::homePath()
    );

    if (outputDirectory_.isEmpty()) {
        return;
    }

    pendingFiles_ = files_;
    errors_.clear();

    totalFiles_ = pendingFiles_.size();
    completedFiles_ = 0;
    batchCancelled_ = false;

    fileList_->setEnabled(false);
    formatCombo_->setEnabled(false);

    convertButton_->setEnabled(false);
    cancelButton_->show();

    progressBar_->setValue(0);
    progressBar_->show();

    progressLabel_->show();
    progressLabel_->setText("Memulai konversi...");

    processNextFile();
}

void VideoPage::processNextFile()
{
    if (batchCancelled_) {
        finishBatch();
        return;
    }

    if (pendingFiles_.isEmpty()) {
        finishBatch();
        return;
    }

    const QString inputPath = pendingFiles_.takeFirst();
    const QString outputPath = createOutputPath(inputPath);

    progressLabel_->setText(
        QString("Memproses %1 dari %2: %3")
            .arg(completedFiles_ + 1)
            .arg(totalFiles_)
            .arg(QFileInfo(inputPath).fileName())
    );

    converter_->convert(
        inputPath,
        outputPath,
        selectedOutputFormat()
    );
}

void VideoPage::handleConversionProgress(int percentage)
{
    if (totalFiles_ <= 0) {
        return;
    }

    const int overallProgress =
        (completedFiles_ * 1000 + percentage * 10) / totalFiles_;

    progressBar_->setValue(qBound(0, overallProgress, 1000));
}

void VideoPage::handleConversionFinished(const QString &outputPath)
{
    Q_UNUSED(outputPath);

    completedFiles_++;

    processNextFile();
}

void VideoPage::handleConversionFailed(const QString &error)
{
    errors_.append(error);

    completedFiles_++;

    processNextFile();
}

void VideoPage::handleConversionCancelled()
{
    batchCancelled_ = true;
    finishBatch();
}

void VideoPage::cancelConversion()
{
    if (!converter_->isRunning()) {
        return;
    }

    batchCancelled_ = true;
    pendingFiles_.clear();

    progressLabel_->setText("Membatalkan konversi...");

    converter_->cancel();
}

void VideoPage::finishBatch()
{
    fileList_->setEnabled(true);
    formatCombo_->setEnabled(true);

    cancelButton_->hide();
    progressBar_->hide();
    progressLabel_->hide();

    updateStatus();

    if (batchCancelled_) {
        QMessageBox::information(
            this,
            "Konversi Dibatalkan",
            QString(
                "Proses konversi dibatalkan.\n"
                "%1 video selesai diproses."
            ).arg(completedFiles_)
        );

        return;
    }

    const int successCount =
        totalFiles_ - errors_.size();

    if (errors_.isEmpty()) {
        QMessageBox::information(
            this,
            "Konversi Selesai",
            QString(
                "Berhasil mengonversi %1 video."
            ).arg(successCount)
        );
    } else {
        QString message =
            QString(
                "Berhasil: %1 video\n"
                "Gagal: %2 video\n\n"
                "Detail error:\n"
            ).arg(successCount).arg(errors_.size());

        for (int i = 0; i < errors_.size(); ++i) {
            message += QString("\n%1. %2")
                .arg(i + 1)
                .arg(errors_.at(i).left(300));
        }

        QMessageBox::warning(
            this,
            "Konversi Selesai dengan Error",
            message
        );
    }

    pendingFiles_.clear();
    errors_.clear();
}