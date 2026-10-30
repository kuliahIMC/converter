#include "audio_page.h"
#include "drop_list.h"

#include "../services/format_registry.h"
#include "../services/audio_converter_service.h"

#include <QComboBox>
#include <QFileDialog>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QLabel>
#include <QListWidgetItem>
#include <QPushButton>
#include <QVBoxLayout>
#include <QProgressBar>
#include <QMessageBox>
#include <QDir>

AudioPage::AudioPage(QWidget *parent)
    : QWidget(parent)
{
    converter_ = new AudioConverterService(this);

    connect(
        converter_,
        &AudioConverterService::progressChanged,
        this,
        &AudioPage::updateProgress
    );

    connect(
        converter_,
        &AudioConverterService::finished,
        this,
        &AudioPage::handleConversionFinished
    );

    connect(
        converter_,
        &AudioConverterService::failed,
        this,
        &AudioPage::handleConversionFailed
    );

    connect(
        converter_,
        &AudioConverterService::cancelled,
        this,
        [this]() {
            finishBatch();
        }
    );

    setupUI();
}

QStringList AudioPage::files() const
{
    return files_;
}

QString AudioPage::selectedOutputFormat() const
{
    return formatCombo_ ? formatCombo_->currentText() : QString();
}

void AudioPage::setupUI()
{
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    auto *title = new QLabel("Audio Converter");
    title->setStyleSheet("font-size: 24px; font-weight: 700;");

    auto *description = new QLabel(
        "Convert audio atau ekstrak audio dari video."
    );
    description->setObjectName("descriptionLabel");
    description->setStyleSheet("font-size: 13px; margin-top: 4px;");

    layout->addWidget(title);
    layout->addWidget(description);
    layout->addSpacing(20);

    // Toolbar
    auto *toolbar = new QHBoxLayout;
    toolbar->setSpacing(10);

    auto *addButton = new QPushButton("+ Tambahkan Media");
    addButton->setObjectName("primaryButton");
    addButton->setCursor(Qt::PointingHandCursor);

    connect(
        addButton,
        &QPushButton::clicked,
        this,
        &AudioPage::selectFilesFromDialog
    );

    auto *removeButton = new QPushButton("Hapus");
    removeButton->setCursor(Qt::PointingHandCursor);

    connect(
        removeButton,
        &QPushButton::clicked,
        this,
        &AudioPage::removeSelected
    );

    auto *clearButton = new QPushButton("Bersihkan");
    clearButton->setCursor(Qt::PointingHandCursor);

    connect(
        clearButton,
        &QPushButton::clicked,
        this,
        &AudioPage::clearFiles
    );

    toolbar->addWidget(addButton);
    toolbar->addWidget(removeButton);
    toolbar->addWidget(clearButton);
    toolbar->addStretch();

    auto *formatLabel = new QLabel("Output:");
    formatLabel->setStyleSheet("font-size: 13px; font-weight: 600;");

    formatCombo_ = new QComboBox;
    formatCombo_->setMinimumWidth(130);
    formatCombo_->addItems(FormatRegistry::audioOutputFormats());
    formatCombo_->setCursor(Qt::PointingHandCursor);

    toolbar->addWidget(formatLabel);
    toolbar->addWidget(formatCombo_);

    layout->addLayout(toolbar);
    layout->addSpacing(14);

    // File list (gaya diatur oleh ThemeManager)
    fileList_ = new DropListWidget(this);
    fileList_->setFilterMode(DropListWidget::FilterMode::Audio);
    fileList_->setSelectionMode(QAbstractItemView::ExtendedSelection);
    fileList_->setMinimumHeight(240);

    connect(
        fileList_,
        &DropListWidget::filesDropped,
        this,
        &AudioPage::addFiles
    );

    layout->addWidget(fileList_, 1);
    layout->addSpacing(14);

    // Progress (gaya diatur oleh ThemeManager)
    progressLabel_ = new QLabel("Menunggu konversi.");
    progressLabel_->setObjectName("descriptionLabel");
    progressLabel_->setStyleSheet("font-size: 12px;");

    progressBar_ = new QProgressBar;
    progressBar_->setRange(0, 1000);
    progressBar_->setValue(0);
    progressBar_->setTextVisible(false);
    progressBar_->setFixedHeight(8);

    layout->addWidget(progressLabel_);
    layout->addWidget(progressBar_);
    layout->addSpacing(12);

    // Bottom action
    auto *bottomLayout = new QHBoxLayout;

    statusLabel_ = new QLabel("Belum ada media dipilih.");
    statusLabel_->setObjectName("descriptionLabel");
    statusLabel_->setStyleSheet("font-size: 13px;");

    cancelButton_ = new QPushButton("Batalkan");
    cancelButton_->setEnabled(false);

    connect(
        cancelButton_,
        &QPushButton::clicked,
        this,
        &AudioPage::cancelConversion
    );

    convertButton_ = new QPushButton("Convert");
    convertButton_->setObjectName("primaryButton");
    convertButton_->setCursor(Qt::PointingHandCursor);
    convertButton_->setEnabled(false);

    connect(
        convertButton_,
        &QPushButton::clicked,
        this,
        &AudioPage::startConversion
    );

    bottomLayout->addWidget(statusLabel_);
    bottomLayout->addStretch();
    bottomLayout->addWidget(cancelButton_);
    bottomLayout->addWidget(convertButton_);

    layout->addLayout(bottomLayout);

    updateStatus();
}

void AudioPage::selectFilesFromDialog()
{
    const QStringList selected = QFileDialog::getOpenFileNames(
        this,
        "Pilih Audio atau Video",
        QString(),
        FormatRegistry::audioFileDialogFilter()
    );

    if (!selected.isEmpty()) {
        addFiles(selected);
    }
}

void AudioPage::addFiles(const QStringList &filePaths)
{
    if (converter_->isRunning()) {
        return;
    }

    bool added = false;

    for (const QString &path : filePaths) {
        if (!files_.contains(path)
            && FormatRegistry::isAudioInputSupported(path)) {

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

void AudioPage::removeSelected()
{
    if (converter_->isRunning()) {
        return;
    }

    const QList<QListWidgetItem *> selectedItems =
        fileList_->selectedItems();

    for (auto *item : selectedItems) {
        files_.removeAll(item->data(Qt::UserRole).toString());
        delete item;
    }

    updateStatus();
}

void AudioPage::clearFiles()
{
    if (converter_->isRunning() || files_.isEmpty()) {
        return;
    }

    files_.clear();
    fileList_->clear();

    updateStatus();
}

void AudioPage::updateStatus()
{
    const int count = files_.size();

    if (count == 0) {
        statusLabel_->setText("Belum ada media dipilih.");
        convertButton_->setEnabled(false);
    } else {
        statusLabel_->setText(
            QString("%1 file siap diproses.").arg(count)
        );

        convertButton_->setEnabled(
            !converter_->isRunning()
        );
    }
}

// Memulai batch conversion
void AudioPage::startConversion()
{
    if (files_.isEmpty() || converter_->isRunning()) {
        return;
    }

    outputDirectory_ = QFileDialog::getExistingDirectory(
        this,
        "Pilih Folder Penyimpanan",
        QDir::homePath()
    );

    if (outputDirectory_.isEmpty()) {
        return;
    }

    pendingFiles_ = files_;
    errors_.clear();

    completedFiles_ = 0;
    totalFiles_ = pendingFiles_.size();
    batchCancelled_ = false;

    progressBar_->setValue(0);
    progressLabel_->setText("Memulai konversi...");

    convertButton_->setEnabled(false);
    cancelButton_->setEnabled(true);
    formatCombo_->setEnabled(false);
    fileList_->setEnabled(false);

    processNextFile();
}

QString AudioPage::createOutputPath(const QString &inputPath) const
{
    const QFileInfo inputInfo(inputPath);

    const QString extension =
        FormatRegistry::extensionForAudioOutput(
            selectedOutputFormat()
        );

    QString outputName =
        inputInfo.completeBaseName() + "_converted";

    QString outputPath = QDir(outputDirectory_).filePath(
        outputName + "." + extension
    );

    int suffix = 1;

    while (QFileInfo::exists(outputPath)) {
        outputPath = QDir(outputDirectory_).filePath(
            QString("%1_%2.%3")
                .arg(outputName)
                .arg(suffix++)
                .arg(extension)
        );
    }

    return outputPath;
}

void AudioPage::processNextFile()
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

    progressLabel_->setText(
        QString("Memproses: %1")
            .arg(QFileInfo(inputPath).fileName())
    );

    const QString outputPath = createOutputPath(inputPath);

    converter_->convert(
        inputPath,
        outputPath,
        selectedOutputFormat()
    );
}

void AudioPage::handleConversionFinished(const QString &outputPath)
{
    ++completedFiles_;

    progressLabel_->setText(
        QString("Berhasil: %1")
            .arg(QFileInfo(outputPath).fileName())
    );

    processNextFile();
}

void AudioPage::handleConversionFailed(const QString &error)
{
    ++completedFiles_;

    errors_.append(error);

    progressLabel_->setText(
        QString("File gagal diproses (%1/%2)")
            .arg(completedFiles_)
            .arg(totalFiles_)
    );

    processNextFile();
}

void AudioPage::updateProgress(int percentage)
{
    if (totalFiles_ <= 0) {
        return;
    }

    const int overallProgress = (
        completedFiles_ * 1000 + percentage * 10
    ) / totalFiles_;

    progressBar_->setValue(overallProgress);
}

void AudioPage::cancelConversion()
{
    if (!converter_->isRunning()) {
        return;
    }

    batchCancelled_ = true;
    pendingFiles_.clear();

    progressLabel_->setText("Membatalkan konversi...");

    converter_->cancel();
}

void AudioPage::finishBatch()
{
    const bool wasCancelled = batchCancelled_;

    cancelButton_->setEnabled(false);
    formatCombo_->setEnabled(true);
    fileList_->setEnabled(true);
    convertButton_->setEnabled(!files_.isEmpty());

    pendingFiles_.clear();

    if (wasCancelled) {
        progressLabel_->setText("Konversi dibatalkan.");
    } else {
        progressBar_->setValue(1000);

        progressLabel_->setText(
            QString("Selesai: %1 dari %2 file berhasil.")
                .arg(totalFiles_ - errors_.size())
                .arg(totalFiles_)
        );
    }

    if (wasCancelled) {
        QMessageBox::information(
            this,
            "Konversi Dibatalkan",
            "Proses konversi telah dibatalkan."
        );
    } else if (errors_.isEmpty()) {
        QMessageBox::information(
            this,
            "Konversi Selesai",
            QString("%1 file berhasil dikonversi.")
                .arg(totalFiles_)
        );
    } else {
        QMessageBox::warning(
            this,
            "Konversi Selesai dengan Kesalahan",
            QString("%1 file berhasil.\n%2 file gagal.\n\nDetail kesalahan:\n%3")
                .arg(totalFiles_ - errors_.size())
                .arg(errors_.size())
                .arg(errors_.join("\n\n"))
        );
    }

    batchCancelled_ = false;
    updateStatus();
}