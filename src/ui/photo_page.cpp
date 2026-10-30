#include "photo_page.h"
#include "drop_list.h"
#include "../services/format_registry.h"
#include "../services/image_converter_service.h"

#include <QAbstractItemView>
#include <QComboBox>
#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QImageWriter>
#include <QLabel>
#include <QListWidgetItem>
#include <QMessageBox>
#include <QProgressBar>
#include <QPushButton>
#include <QTimer>
#include <QVBoxLayout>

PhotoPage::PhotoPage(QWidget *parent)
    : QWidget(parent)
{
    converter_ = new ImageConverterService(this);

    connect(
        converter_, &ImageConverterService::finished,
        this, [this](const QString &) {
            ++completedFiles_;
            processNextFile();
        }
    );

    connect(
        converter_, &ImageConverterService::failed,
        this, [this](const QString &error) {
            errors_.append(error);
            ++completedFiles_;
            processNextFile();
        }
    );

    connect(
        converter_, &ImageConverterService::cancelled,
        this, [this]() {
            batchCancelled_ = true;
            finishBatchConversion();
        }
    );

    setupUI();
}

QStringList PhotoPage::files() const
{
    return files_;
}

QString PhotoPage::selectedOutputFormat() const
{
    return formatCombo_
        ? formatCombo_->currentText()
        : QString();
}

void PhotoPage::setupUI()
{
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(12);

    auto *title = new QLabel("Photo Converter");
    title->setStyleSheet(
        "font-size:24px;font-weight:700;"
    );

    auto *description = new QLabel(
        "Konversi gambar ke berbagai format."
    );
    description->setStyleSheet(
        "font-size:13px;color:#737373;"
    );

    layout->addWidget(title);
    layout->addWidget(description);
    layout->addSpacing(12);

    auto *toolbar = new QHBoxLayout;

    auto *addButton = new QPushButton("+ Tambahkan Gambar");
    addButton->setObjectName("primaryButton");
    addButton->setCursor(Qt::PointingHandCursor);

    connect(
        addButton, &QPushButton::clicked,
        this, &PhotoPage::selectFilesFromDialog
    );

    auto *removeButton = new QPushButton("Hapus");
    removeButton->setCursor(Qt::PointingHandCursor);

    connect(
        removeButton, &QPushButton::clicked,
        this, &PhotoPage::removeSelected
    );

    auto *clearButton = new QPushButton("Bersihkan");
    clearButton->setCursor(Qt::PointingHandCursor);

    connect(
        clearButton, &QPushButton::clicked,
        this, &PhotoPage::clearFiles
    );

    toolbar->addWidget(addButton);
    toolbar->addWidget(removeButton);
    toolbar->addWidget(clearButton);
    toolbar->addStretch();

    formatCombo_ = new QComboBox;

    const QList<QByteArray> supportedFormats =
        QImageWriter::supportedImageFormats();

    for (const QString &format :
         FormatRegistry::imageOutputFormats()) {

        const QByteArray key =
            format.toLower().toLatin1();

        if (supportedFormats.contains(key)) {
            formatCombo_->addItem(format);
        }
    }

    formatCombo_->setMinimumWidth(130);
    formatCombo_->setCursor(Qt::PointingHandCursor);

    auto *formatLabel = new QLabel("Output:");
    formatLabel->setStyleSheet("font-size:13px;font-weight:600;");

    toolbar->addWidget(formatLabel);
    toolbar->addWidget(formatCombo_);

    layout->addLayout(toolbar);

    fileList_ = new DropListWidget(this);
    fileList_->setFilterMode(
        DropListWidget::FilterMode::Image
    );
    fileList_->setSelectionMode(
        QAbstractItemView::ExtendedSelection
    );
    fileList_->setMinimumHeight(240);

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
        fileList_, &DropListWidget::filesDropped,
        this, &PhotoPage::addFiles
    );

    layout->addWidget(fileList_, 1);

    progressBar_ = new QProgressBar;
    progressBar_->setRange(0, 100);
    progressBar_->setValue(0);
    progressBar_->setTextVisible(true);
    progressBar_->setVisible(false);

    layout->addWidget(progressBar_);

    auto *bottom = new QHBoxLayout;

    statusLabel_ = new QLabel("Belum ada gambar dipilih.");
    statusLabel_->setStyleSheet(
        "font-size:13px;color:#737373;"
    );

    convertButton_ = new QPushButton("Convert");
    convertButton_->setObjectName("primaryButton");
    convertButton_->setCursor(Qt::PointingHandCursor);
    convertButton_->setEnabled(false);

    cancelButton_ = new QPushButton("Batalkan");
    cancelButton_->setObjectName("cancelButton");
    cancelButton_->setMinimumHeight(38);
    cancelButton_->setMinimumWidth(105);
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
    cancelButton_->setEnabled(false);

    connect(
        convertButton_, &QPushButton::clicked,
        this, &PhotoPage::startBatchConversion
    );

    connect(
        cancelButton_, &QPushButton::clicked,
        this, &PhotoPage::cancelConversion
    );

    bottom->addWidget(statusLabel_);
    bottom->addStretch();
    bottom->addWidget(cancelButton_);
    bottom->addWidget(convertButton_);

    layout->addLayout(bottom);

    updateStatus();
}

void PhotoPage::selectFilesFromDialog()
{
    const QStringList selected = QFileDialog::getOpenFileNames(
        this,
        "Pilih Gambar",
        QString(),
        FormatRegistry::imageFileDialogFilter()
    );

    if (!selected.isEmpty()) {
        addFiles(selected);
    }
}

void PhotoPage::addFiles(const QStringList &filePaths)
{
    if (converter_->isRunning() || !pendingFiles_.isEmpty()) {
        return;
    }

    for (const QString &path : filePaths) {
        if (files_.contains(path) ||
            !FormatRegistry::isImageInputSupported(path)) {
            continue;
        }

        files_.append(path);

        auto *item = new QListWidgetItem(
            QFileInfo(path).fileName(),
            fileList_
        );

        item->setData(Qt::UserRole, path);
        item->setToolTip(path);
    }

    updateStatus();
}

void PhotoPage::removeSelected()
{
    if (!pendingFiles_.isEmpty()) {
        return;
    }

    const auto selected = fileList_->selectedItems();

    for (auto *item : selected) {
        files_.removeAll(
            item->data(Qt::UserRole).toString()
        );

        delete item;
    }

    updateStatus();
}

void PhotoPage::clearFiles()
{
    if (!pendingFiles_.isEmpty()) {
        return;
    }

    files_.clear();
    fileList_->clear();

    updateStatus();
}

void PhotoPage::updateStatus()
{
    const int count = files_.size();

    statusLabel_->setText(
        count == 0
            ? "Belum ada gambar dipilih."
            : QString("%1 gambar siap diproses.").arg(count)
    );

    convertButton_->setEnabled(
        count > 0 &&
        !pendingFiles_.size() &&
        formatCombo_->count() > 0
    );
}

QString PhotoPage::createOutputPath(
    const QString &inputPath) const
{
    QFileInfo info(inputPath);

    QString extension =
        FormatRegistry::extensionForImageOutput(
            selectedOutputFormat()
        );

    if (extension.isEmpty()) {
        extension = selectedOutputFormat().toLower();
    }

    QString baseName = info.completeBaseName();

    QString outputPath = QDir(outputDirectory_).filePath(
        baseName + "_converted." + extension
    );

    int suffix = 1;

    while (QFileInfo::exists(outputPath)) {
        outputPath = QDir(outputDirectory_).filePath(
            QString("%1_converted_%2.%3")
                .arg(baseName)
                .arg(suffix++)
                .arg(extension)
        );
    }

    return outputPath;
}

void PhotoPage::startBatchConversion()
{
    if (files_.isEmpty() || !pendingFiles_.isEmpty()) {
        return;
    }

    if (selectedOutputFormat().isEmpty()) {
        QMessageBox::warning(
            this,
            "Format Tidak Tersedia",
            "Tidak ada format output gambar yang didukung."
        );
        return;
    }

    const QString folder = QFileDialog::getExistingDirectory(
        this,
        "Pilih Folder Output"
    );

    if (folder.isEmpty()) {
        return;
    }

    outputDirectory_ = folder;
    pendingFiles_ = files_;
    errors_.clear();

    completedFiles_ = 0;
    totalFiles_ = pendingFiles_.size();
    batchCancelled_ = false;

    progressBar_->setValue(0);
    progressBar_->setVisible(true);

    convertButton_->setEnabled(false);
    cancelButton_->setEnabled(true);

    formatCombo_->setEnabled(false);
    fileList_->setEnabled(false);

    statusLabel_->setText("Menyiapkan konversi gambar...");

    processNextFile();
}

void PhotoPage::processNextFile()
{
    if (batchCancelled_ || pendingFiles_.isEmpty()) {
        finishBatchConversion();
        return;
    }

    const QString inputPath = pendingFiles_.takeFirst();
    const QString outputPath = createOutputPath(inputPath);

    statusLabel_->setText(
        QString("Memproses %1 dari %2: %3")
            .arg(completedFiles_ + 1)
            .arg(totalFiles_)
            .arg(QFileInfo(inputPath).fileName())
    );

    progressBar_->setValue(
        totalFiles_ > 0
            ? completedFiles_ * 100 / totalFiles_
            : 0
    );

    QTimer::singleShot(0, this, [this, inputPath, outputPath]() {
        if (batchCancelled_) {
            finishBatchConversion();
            return;
        }

        converter_->convert(
            inputPath,
            outputPath,
            selectedOutputFormat()
        );
    });
}

void PhotoPage::cancelConversion()
{
    if (pendingFiles_.isEmpty() && !converter_->isRunning()) {
        return;
    }

    batchCancelled_ = true;
    pendingFiles_.clear();

    statusLabel_->setText("Menghentikan antrean konversi...");
    cancelButton_->setEnabled(false);
}

void PhotoPage::finishBatchConversion()
{
    pendingFiles_.clear();

    progressBar_->setValue(
        totalFiles_ > 0
            ? completedFiles_ * 100 / totalFiles_
            : 0
    );

    fileList_->setEnabled(true);
    formatCombo_->setEnabled(true);
    cancelButton_->setEnabled(false);

    updateStatus();

    const int successCount =
        completedFiles_ - errors_.size();

    QString message;

    if (batchCancelled_) {
        message = QString(
            "Konversi dibatalkan.\n\nBerhasil: %1\nGagal: %2"
        ).arg(successCount).arg(errors_.size());
    } else {
        message = QString(
            "Konversi gambar selesai.\n\nBerhasil: %1\nGagal: %2"
        ).arg(successCount).arg(errors_.size());
    }

    if (!errors_.isEmpty()) {
        message += "\n\nDetail error:\n";

        for (const QString &error : errors_) {
            message += "\n• " + error;
        }
    }

    QMessageBox::information(
        this,
        "Photo Converter",
        message
    );
}