#include "compress_page.h"

#include "drop_list.h"
#include "../services/format_registry.h"
#include "../services/image_compression_service.h"
#include "../services/video_compression_service.h"

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
#include <QSlider>
#include <QVBoxLayout>

#include <QIcon>
#include <QImageReader>
#include <QPixmap>
#include <QSize>
#include <QStyle>

// Ukuran thumbnail persegi (1:1) dan tinggi baris di daftar
static constexpr int kThumbSize = 96;
static constexpr int kRowHeight = 116;

CompressPage::CompressPage(QWidget *parent)
    : QWidget(parent),
      imageService_(new ImageCompressionService(this)),
      videoService_(new VideoCompressionService(this))
{
    setupUI();

    connect(
        compressButton_,
        &QPushButton::clicked,
        this,
        &CompressPage::startCompression
    );

    connect(
        cancelButton_,
        &QPushButton::clicked,
        this,
        &CompressPage::cancelCompression
    );

    connect(
        outputFolderButton_,
        &QPushButton::clicked,
        this,
        [this]() {
            const QString directory =
                QFileDialog::getExistingDirectory(
                    this,
                    "Pilih Folder Hasil Kompresi",
                    outputDirectory_.isEmpty()
                        ? QDir::homePath()
                        : outputDirectory_
                );

            if (!directory.isEmpty()) {
                outputDirectory_ = directory;

                outputFolderButton_->setText(
                    "Folder: " + QDir(directory).dirName()
                );
            }
        }
    );

    connect(
        imageService_,
        &ImageCompressionService::finished,
        this,
        &CompressPage::handleCompressionFinished
    );

    connect(
        imageService_,
        &ImageCompressionService::failed,
        this,
        &CompressPage::handleCompressionFailed
    );

    connect(
        imageService_,
        &ImageCompressionService::cancelled,
        this,
        &CompressPage::handleCompressionCancelled
    );

    connect(
        videoService_,
        &VideoCompressionService::finished,
        this,
        &CompressPage::handleCompressionFinished
    );

    connect(
        videoService_,
        &VideoCompressionService::failed,
        this,
        &CompressPage::handleCompressionFailed
    );

    connect(
        videoService_,
        &VideoCompressionService::cancelled,
        this,
        &CompressPage::handleCompressionCancelled
    );

    connect(
        videoService_,
        &VideoCompressionService::progressChanged,
        this,
        [this](int percentage) {
            if (!isProcessing_ || files_.isEmpty())
                return;

            const int total = files_.size();

            const int overallProgress =
                ((currentIndex_ * 100) + percentage) / total;

            progressBar_->setValue(
                qBound(0, overallProgress, 100)
            );
        }
    );
}

QStringList CompressPage::files() const
{
    return files_;
}

int CompressPage::quality() const
{
    return qualitySlider_ ? qualitySlider_->value() : 80;
}

QString CompressPage::category() const
{
    return categoryCombo_
        ? categoryCombo_->currentText()
        : QString();
}

void CompressPage::setupUI()
{
    // ===== STYLE HALAMAN (gaya Notion) =====
    // Dipasang di halaman ini saja supaya QSS tema tidak terganggu.
    setStyleSheet(R"QSS(
        /* Tombol utama: ungu */
        QPushButton#primaryButton {
            background: #5645d4;
            color: #ffffff;
            border: none;
            border-radius: 8px;
            padding: 9px 18px;
            font-family: "Inter", "Segoe UI", sans-serif;
            font-size: 14px;
            font-weight: 500;
        }
        QPushButton#primaryButton:hover   { background: #4534b3; }
        QPushButton#primaryButton:pressed { background: #3a2a99; }
        QPushButton#primaryButton:disabled {
            background: rgba(255, 255, 255, 0.55);
            color: #a4a097;
            border: 1px solid #e5e3df;
        }

        /* Tombol sekunder: Hapus, Bersihkan, Pilih Folder */
        QPushButton#secondaryButton {
            background: rgba(255, 255, 255, 0.88);
            color: #1a1a1a;
            border: 1px solid #c8c4be;
            border-radius: 8px;
            padding: 9px 16px;
            font-family: "Inter", "Segoe UI", sans-serif;
            font-size: 14px;
            font-weight: 500;
        }
        QPushButton#secondaryButton:hover {
            background: #ffffff;
            border-color: #787671;
        }
        QPushButton#secondaryButton:pressed {
            background: #f0eeec;
        }
        QPushButton#secondaryButton:disabled {
            background: rgba(255, 255, 255, 0.45);
            color: #a4a097;
            border-color: #e5e3df;
        }

        /* Pilihan kategori */
        QComboBox {
            background: rgba(255, 255, 255, 0.90);
            color: #1a1a1a;
            border: 1px solid #c8c4be;
            border-radius: 8px;
            padding: 7px 12px;
            font-family: "Inter", "Segoe UI", sans-serif;
            font-size: 14px;
        }
        QComboBox:hover { border-color: #5645d4; }
        QComboBox QAbstractItemView {
            background: #ffffff;
            color: #1a1a1a;
            border: 1px solid #e5e3df;
            selection-background-color: #e6e0f5;
            selection-color: #391c57;
            outline: none;
        }

        /* Slider kualitas */
        QSlider::groove:horizontal {
            height: 6px;
            background: rgba(255, 255, 255, 0.60);
            border-radius: 3px;
        }
        QSlider::sub-page:horizontal {
            background: #5645d4;
            border-radius: 3px;
        }
        QSlider::handle:horizontal {
            background: #ffffff;
            border: 2px solid #5645d4;
            width: 16px;
            height: 16px;
            margin: -7px 0;
            border-radius: 10px;
        }
        QSlider::handle:horizontal:hover {
            background: #e6e0f5;
        }
        QSlider::sub-page:horizontal:disabled {
            background: #bbb8b1;
        }
        QSlider::handle:horizontal:disabled {
            border-color: #bbb8b1;
        }

        /* Progress bar tipis berwarna ungu */
        QProgressBar {
            background: rgba(255, 255, 255, 0.60);
            color: #37352f;
            border: none;
            border-radius: 9px;
            min-height: 18px;
            max-height: 18px;
            text-align: center;
            font-size: 11px;
            font-weight: 600;
        }
        QProgressBar::chunk {
            background: #5645d4;
            border-radius: 9px;
        }
    )QSS");

    auto *layout = new QVBoxLayout(this);

    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(12);

    auto *title = new QLabel("Media Compressor");

    title->setStyleSheet(
        "font-size:26px;font-weight:700;"
    );

    auto *description = new QLabel(
        "Kurangi ukuran file gambar dan video."
    );

    description->setStyleSheet(
        "font-size:13px;color:#737373;"
    );

    layout->addWidget(title);
    layout->addWidget(description);
    layout->addSpacing(12);

    auto *toolbar = new QHBoxLayout;
    toolbar->setSpacing(8);

    auto *addButton = new QPushButton("+ Tambahkan Media");
    addButton->setObjectName("primaryButton");
    addButton->setCursor(Qt::PointingHandCursor);
    addButton->setMinimumHeight(38);

    connect(
        addButton,
        &QPushButton::clicked,
        this,
        &CompressPage::selectFilesFromDialog
    );

    auto *removeButton = new QPushButton("Hapus");
    removeButton->setObjectName("secondaryButton");
    removeButton->setCursor(Qt::PointingHandCursor);
    removeButton->setMinimumHeight(38);

    connect(
        removeButton,
        &QPushButton::clicked,
        this,
        &CompressPage::removeSelected
    );

    auto *clearButton = new QPushButton("Bersihkan");
    clearButton->setObjectName("secondaryButton");
    clearButton->setCursor(Qt::PointingHandCursor);
    clearButton->setMinimumHeight(38);

    connect(
        clearButton,
        &QPushButton::clicked,
        this,
        &CompressPage::clearFiles
    );

    toolbar->addWidget(addButton);
    toolbar->addWidget(removeButton);
    toolbar->addWidget(clearButton);
    toolbar->addStretch();

    categoryCombo_ = new QComboBox;
    categoryCombo_->setCursor(Qt::PointingHandCursor);
    categoryCombo_->setMinimumHeight(38);

    categoryCombo_->addItems({
        "Otomatis",
        "Gambar",
        "Video"
    });

    auto *categoryLabel = new QLabel("Kategori:");
    categoryLabel->setStyleSheet("font-size:13px;font-weight:600;");

    toolbar->addWidget(categoryLabel);
    toolbar->addWidget(categoryCombo_);

    layout->addLayout(toolbar);

    fileList_ = new DropListWidget(this);

    fileList_->setFilterMode(
        DropListWidget::FilterMode::Compress
    );

    fileList_->setSelectionMode(
        QAbstractItemView::ExtendedSelection
    );

    fileList_->setMinimumHeight(260);
    fileList_->setIconSize(QSize(kThumbSize, kThumbSize));
    fileList_->setSpacing(4);
    fileList_->setVerticalScrollMode(
        QAbstractItemView::ScrollPerPixel
    );

    // Awal kosong = transparan (hanya bingkai putus-putus ala drop zone).
    // Saat file masuk, tiap file tampil sebagai kartu.
    fileList_->viewport()->setAutoFillBackground(false);
    fileList_->setStyleSheet(R"QSS(
            QListWidget {
                background: transparent;
                border: 2px dashed rgba(120, 118, 113, 0.55);
                border-radius: 16px;
                padding: 8px;
                outline: none;
                font-family: "Inter", "Segoe UI", sans-serif;
                font-size: 14px;
                font-weight: 500;
            }
            QListWidget::item {
                background: rgba(255, 255, 255, 0.90);
                color: #1a1a1a;
                border: 1px solid #e5e3df;
                border-radius: 12px;
                padding: 8px 12px;
                margin: 2px 2px;
            }
            QListWidget::item:hover {
                background: #ffffff;
                border: 1px solid #c8c4be;
            }
            QListWidget::item:selected {
                background: #e6e0f5;
                color: #391c57;
                border: 2px solid #5645d4;
            }

            QScrollBar:vertical {
                background: transparent;
                width: 10px;
                margin: 4px 2px;
            }
            QScrollBar::handle:vertical {
                background: rgba(120, 118, 113, 0.55);
                border-radius: 4px;
                min-height: 30px;
            }
            QScrollBar::handle:vertical:hover {
                background: rgba(86, 69, 212, 0.85);
            }
            QScrollBar::add-line:vertical,
            QScrollBar::sub-line:vertical {
                height: 0px;
            }
            QScrollBar::add-page:vertical,
            QScrollBar::sub-page:vertical {
                background: transparent;
            }
        )QSS");

    connect(
        fileList_,
        &DropListWidget::filesDropped,
        this,
        &CompressPage::addFiles
    );

    layout->addWidget(fileList_, 1);

    auto *qualityTitle = new QLabel("Kualitas kompresi");

    qualityTitle->setStyleSheet(
        "font-size:13px;font-weight:600;"
    );

    auto *qualityLayout = new QHBoxLayout;
    qualityLayout->setSpacing(12);

    qualitySlider_ = new QSlider(Qt::Horizontal);
    qualitySlider_->setRange(10, 100);
    qualitySlider_->setValue(80);
    qualitySlider_->setCursor(Qt::PointingHandCursor);

    qualityLabel_ = new QLabel("80%");
    qualityLabel_->setMinimumWidth(45);
    qualityLabel_->setStyleSheet(
        "font-size:13px;font-weight:600;"
    );

    connect(
        qualitySlider_,
        &QSlider::valueChanged,
        this,
        [this](int value) {
            qualityLabel_->setText(
                QString("%1%").arg(value)
            );
        }
    );

    qualityLayout->addWidget(qualitySlider_, 1);
    qualityLayout->addWidget(qualityLabel_);

    layout->addWidget(qualityTitle);
    layout->addLayout(qualityLayout);

    outputFolderButton_ = new QPushButton(
        "Pilih Folder Hasil"
    );
    outputFolderButton_->setObjectName("secondaryButton");
    outputFolderButton_->setCursor(Qt::PointingHandCursor);
    outputFolderButton_->setMinimumHeight(38);

    layout->addWidget(outputFolderButton_);

    progressBar_ = new QProgressBar;
    progressBar_->setRange(0, 100);
    progressBar_->setValue(0);
    progressBar_->setTextVisible(true);
    progressBar_->setVisible(false);

    layout->addWidget(progressBar_);

    auto *bottom = new QHBoxLayout;
    bottom->setSpacing(8);

    statusLabel_ = new QLabel(
        "Belum ada media dipilih."
    );

    statusLabel_->setStyleSheet(
        "font-size:13px;color:#737373;"
    );

    compressButton_ = new QPushButton("Kompres");
    compressButton_->setObjectName("primaryButton");
    compressButton_->setCursor(Qt::PointingHandCursor);
    compressButton_->setMinimumHeight(38);
    compressButton_->setMinimumWidth(105);
    compressButton_->setEnabled(false);

    cancelButton_ = new QPushButton("Batalkan");
    cancelButton_->setObjectName("cancelButton");
    cancelButton_->setCursor(Qt::PointingHandCursor);
    cancelButton_->setVisible(false);

    cancelButton_->setMinimumHeight(38);
    cancelButton_->setMinimumWidth(105);

    cancelButton_->setStyleSheet(R"(
        QPushButton#cancelButton {
            background-color: #ffffff;
            color: #dc2626;
            border: 1px solid #fecaca;
            border-radius: 8px;
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
    
    cancelButton_->setEnabled(true);

    bottom->addWidget(statusLabel_);
    bottom->addStretch();
    bottom->addWidget(cancelButton_);
    bottom->addWidget(compressButton_);

    layout->addLayout(bottom);

    updateStatus();
}

void CompressPage::selectFilesFromDialog()
{
    const QStringList selected =
        QFileDialog::getOpenFileNames(
            this,
            "Pilih Media untuk Dikompres",
            QString(),
            "Semua media (*.jpg *.jpeg *.png *.webp *.bmp "
            "*.tif *.tiff *.mp4 *.mkv *.mov *.avi *.webm "
            "*.flv *.wmv *.mpeg *.mpg *.m4v *.3gp "
            "*.ts *.mts *.m2ts *.ogv);;Semua file (*.*)"
        );

    if (!selected.isEmpty())
        addFiles(selected);
}

void CompressPage::addFiles(const QStringList &filePaths)
{
    if (isProcessing_)
        return;

    for (const QString &path : filePaths) {
        const QFileInfo info(path);

        if (!info.exists() || !info.isFile())
            continue;

        if (files_.contains(info.absoluteFilePath()))
            continue;

        const bool image =
            FormatRegistry::isImageCompressSupported(path);

        const bool video =
            FormatRegistry::isVideoCompressSupported(path);

        if (!image && !video)
            continue;

        const QString absolutePath = info.absoluteFilePath();

        files_.append(absolutePath);

        auto *item = new QListWidgetItem(
            info.fileName(),
            fileList_
        );

        item->setData(Qt::UserRole, absolutePath);
        item->setToolTip(absolutePath);

        // Tampilan saja: gambar = thumbnail persegi (crop tengah),
        // video = ikon play persegi
        bool iconSet = false;

        if (image) {
            QImageReader reader(absolutePath);
            reader.setAutoTransform(true);

            const QPixmap thumbnail =
                QPixmap::fromImage(reader.read());

            if (!thumbnail.isNull()) {
                const QPixmap scaled = thumbnail.scaled(
                    kThumbSize,
                    kThumbSize,
                    Qt::KeepAspectRatioByExpanding,
                    Qt::SmoothTransformation
                );

                const int x = (scaled.width() - kThumbSize) / 2;
                const int y = (scaled.height() - kThumbSize) / 2;

                item->setIcon(QIcon(
                    scaled.copy(x, y, kThumbSize, kThumbSize)
                ));

                iconSet = true;
            }
        }

        if (!iconSet) {
            item->setIcon(style()->standardIcon(
                video ? QStyle::SP_MediaPlay
                      : QStyle::SP_FileIcon
            ));
        }

        item->setSizeHint(QSize(0, kRowHeight));
    }

    updateStatus();
}

void CompressPage::removeSelected()
{
    if (isProcessing_)
        return;

    const auto selected = fileList_->selectedItems();

    for (auto *item : selected) {
        files_.removeAll(
            item->data(Qt::UserRole).toString()
        );

        delete item;
    }

    updateStatus();
}

void CompressPage::clearFiles()
{
    if (isProcessing_)
        return;

    files_.clear();
    fileList_->clear();

    updateStatus();
}

bool CompressPage::isImageFile(const QString &filePath) const
{
    return FormatRegistry::isImageCompressSupported(filePath);
}

bool CompressPage::isVideoFile(const QString &filePath) const
{
    return FormatRegistry::isVideoCompressSupported(filePath);
}

QString CompressPage::createOutputPath(
    const QString &inputPath) const
{
    const QFileInfo info(inputPath);

    const QString directory = outputDirectory_.isEmpty()
        ? info.absolutePath()
        : outputDirectory_;

    const QString baseName = info.completeBaseName();
    const QString suffix = info.suffix();

    QString outputPath = QDir(directory).filePath(
        baseName + "_compressed." + suffix
    );

    int counter = 2;

    while (QFileInfo::exists(outputPath)) {
        outputPath = QDir(directory).filePath(
            QString("%1_compressed_%2.%3")
                .arg(baseName)
                .arg(counter++)
                .arg(suffix)
        );
    }

    return outputPath;
}

QString CompressPage::formatFileSize(qint64 bytes) const
{
    const double size = static_cast<double>(bytes);

    if (bytes >= 1024LL * 1024 * 1024) {
        return QString("%1 GB").arg(
            size / (1024.0 * 1024.0 * 1024.0), 0, 'f', 2
        );
    }

    if (bytes >= 1024LL * 1024) {
        return QString("%1 MB").arg(
            size / (1024.0 * 1024.0), 0, 'f', 2
        );
    }

    if (bytes >= 1024) {
        return QString("%1 KB").arg(
            size / 1024.0, 0, 'f', 2
        );
    }

    return QString("%1 B").arg(bytes);
}

void CompressPage::startCompression()
{
    if (isProcessing_ || files_.isEmpty())
        return;

    const QString directory =
        QFileDialog::getExistingDirectory(
            this,
            "Pilih Folder Hasil Kompresi",
            outputDirectory_.isEmpty()
                ? QDir::homePath()
                : outputDirectory_
        );

    if (directory.isEmpty())
        return;

    outputDirectory_ = directory;

    outputFolderButton_->setText(
        "Folder: " + QDir(directory).dirName()
    );

    currentIndex_ = 0;
    successCount_ = 0;
    failedCount_ = 0;

    failedFiles_.clear();

    cancelRequested_ = false;
    isProcessing_ = true;

    fileList_->setEnabled(false);
    categoryCombo_->setEnabled(false);
    qualitySlider_->setEnabled(false);
    outputFolderButton_->setEnabled(false);

    compressButton_->setEnabled(false);
    cancelButton_->setVisible(true);

    progressBar_->setVisible(true);
    progressBar_->setValue(0);

    processNextFile();
}

void CompressPage::processNextFile()
{
    if (cancelRequested_) {
        finishCompression();
        return;
    }

    if (currentIndex_ >= files_.size()) {
        finishCompression();
        return;
    }

    const QString inputPath = files_.at(currentIndex_);

    const QString selectedCategory = category();

    const bool image = isImageFile(inputPath);
    const bool video = isVideoFile(inputPath);

    if (!QFileInfo::exists(inputPath)) {
        failedCount_++;
        failedFiles_.append(inputPath);
        currentIndex_++;
        processNextFile();
        return;
    }

    if ((selectedCategory == "Gambar" && !image) ||
        (selectedCategory == "Video" && !video) ||
        (!image && !video)) {

        failedCount_++;
        failedFiles_.append(inputPath);
        currentIndex_++;

        processNextFile();
        return;
    }

    const QString outputPath = createOutputPath(inputPath);

    const int initialProgress =
        currentIndex_ * 100 / files_.size();

    progressBar_->setValue(initialProgress);

    statusLabel_->setText(
        QString("Mengompres %1 (%2/%3)")
            .arg(QFileInfo(inputPath).fileName())
            .arg(currentIndex_ + 1)
            .arg(files_.size())
    );

    if (image) {
        imageService_->compress(
            inputPath,
            outputPath,
            quality()
        );
    } else {
        videoService_->compress(
            inputPath,
            outputPath,
            quality()
        );
    }
}

void CompressPage::handleCompressionFinished(
    const QString &outputPath,
    qint64 originalSize,
    qint64 compressedSize)
{
    successCount_++;

    QString result = QFileInfo(outputPath).fileName();

    if (originalSize > 0) {
        const double reduction =
            (1.0 - static_cast<double>(compressedSize) /
             static_cast<double>(originalSize)) * 100.0;

        result += QString(
            " | %1 → %2 (%3%)"
        )
            .arg(formatFileSize(originalSize))
            .arg(formatFileSize(compressedSize))
            .arg(reduction, 0, 'f', 1);
    }

    statusLabel_->setText(result);

    currentIndex_++;

    processNextFile();
}

void CompressPage::handleCompressionFailed(
    const QString &errorMessage)
{
    failedCount_++;

    if (currentIndex_ < files_.size()) {
        failedFiles_.append(files_.at(currentIndex_));
    }

    statusLabel_->setText(
        "Gagal: " + errorMessage.left(120)
    );

    currentIndex_++;

    processNextFile();
}

void CompressPage::cancelCompression()
{
    if (!isProcessing_)
        return;

    cancelRequested_ = true;

    cancelButton_->setEnabled(false);
    statusLabel_->setText("Membatalkan kompresi...");

    if (imageService_->isRunning()) {
        imageService_->cancel();
    } else if (videoService_->isRunning()) {
        videoService_->cancel();
    } else {
        finishCompression();
    }
}

void CompressPage::handleCompressionCancelled()
{
    finishCompression();
}

void CompressPage::finishCompression()
{
    isProcessing_ = false;

    fileList_->setEnabled(true);
    categoryCombo_->setEnabled(true);
    qualitySlider_->setEnabled(true);
    outputFolderButton_->setEnabled(true);

    cancelButton_->setVisible(false);
    cancelButton_->setEnabled(true);

    compressButton_->setEnabled(!files_.isEmpty());

    progressBar_->setValue(
        cancelRequested_
            ? (currentIndex_ * 100 / qMax(1, files_.size()))
            : 100
    );

    QString summary = QString(
        "Selesai: %1 berhasil, %2 gagal."
    )
        .arg(successCount_)
        .arg(failedCount_);

    if (cancelRequested_)
        summary.prepend("Dibatalkan. ");

    statusLabel_->setText(summary);

    QString details = summary;

    if (!failedFiles_.isEmpty()) {
        details += "\n\nFile yang gagal:\n";

        for (const QString &path : failedFiles_) {
            details += "\n• " + QFileInfo(path).fileName();
        }
    }

    QMessageBox::information(
        this,
        "Kompresi Media",
        details
    );

    updateStatus();
}

void CompressPage::updateStatus()
{
    const int count = files_.size();

    if (!isProcessing_) {
        statusLabel_->setText(
            count == 0
                ? "Belum ada media dipilih."
                : QString("%1 file siap dikompres.").arg(count)
        );
    }

    compressButton_->setEnabled(
        count > 0 && !isProcessing_
    );
}