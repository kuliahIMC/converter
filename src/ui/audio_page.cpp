#include "audio_page.h"
#include "drop_list.h"

#include "../services/format_registry.h"
#include "../services/audio_converter_service.h"

#include <QAbstractItemView>
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

#include <QIcon>
#include <QSize>
#include <QStyle>

// Ukuran ikon audio (persegi 1:1) dan tinggi baris di daftar
static constexpr int kIconSize = 72;
static constexpr int kRowHeight = 96;

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

        /* Tombol sekunder: Hapus, Bersihkan */
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

        /* Tombol batal: merah lembut */
        QPushButton#cancelButton {
            background: #ffffff;
            color: #dc2626;
            border: 1px solid #fecaca;
            border-radius: 8px;
            padding: 8px 16px;
            font-size: 13px;
            font-weight: 600;
        }
        QPushButton#cancelButton:hover {
            background: #fef2f2;
            border-color: #ef4444;
        }
        QPushButton#cancelButton:pressed {
            background: #fee2e2;
        }
        QPushButton#cancelButton:disabled {
            background: #f5f5f5;
            color: #a3a3a3;
            border-color: #e5e5e5;
        }

        /* Pilihan format output */
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

        /* Progress bar tipis berwarna ungu */
        QProgressBar {
            background: rgba(255, 255, 255, 0.60);
            border: none;
            border-radius: 4px;
        }
        QProgressBar::chunk {
            background: #5645d4;
            border-radius: 4px;
        }
    )QSS");

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    auto *title = new QLabel("Audio Converter");
    title->setStyleSheet("font-size: 26px; font-weight: 700;");

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
    toolbar->setSpacing(8);

    auto *addButton = new QPushButton("+ Tambahkan Media");
    addButton->setObjectName("primaryButton");
    addButton->setCursor(Qt::PointingHandCursor);
    addButton->setMinimumHeight(38);

    connect(
        addButton,
        &QPushButton::clicked,
        this,
        &AudioPage::selectFilesFromDialog
    );

    auto *removeButton = new QPushButton("Hapus");
    removeButton->setObjectName("secondaryButton");
    removeButton->setCursor(Qt::PointingHandCursor);
    removeButton->setMinimumHeight(38);

    connect(
        removeButton,
        &QPushButton::clicked,
        this,
        &AudioPage::removeSelected
    );

    auto *clearButton = new QPushButton("Bersihkan");
    clearButton->setObjectName("secondaryButton");
    clearButton->setCursor(Qt::PointingHandCursor);
    clearButton->setMinimumHeight(38);

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
    formatCombo_->setMinimumHeight(38);
    formatCombo_->addItems(FormatRegistry::audioOutputFormats());
    formatCombo_->setCursor(Qt::PointingHandCursor);

    toolbar->addWidget(formatLabel);
    toolbar->addWidget(formatCombo_);

    layout->addLayout(toolbar);
    layout->addSpacing(14);

    // File list
    fileList_ = new DropListWidget(this);
    fileList_->setFilterMode(DropListWidget::FilterMode::Audio);
    fileList_->setSelectionMode(QAbstractItemView::ExtendedSelection);
    fileList_->setMinimumHeight(260);
    fileList_->setViewMode(QListView::ListMode);
    fileList_->setFlow(QListView::TopToBottom);
    fileList_->setWrapping(false);
    fileList_->setIconSize(QSize(kIconSize, kIconSize));
    fileList_->setSpacing(4);
    fileList_->setUniformItemSizes(false);
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
        &AudioPage::addFiles
    );

    layout->addWidget(fileList_, 1);
    layout->addSpacing(14);

    // Progress
    progressLabel_ = new QLabel("Menunggu konversi.");
    progressLabel_->setObjectName("descriptionLabel");
    progressLabel_->setStyleSheet("font-size: 12px;");

    progressBar_ = new QProgressBar;
    progressBar_->setRange(0, 1000);
    progressBar_->setValue(0);
    progressBar_->setTextVisible(false);
    progressBar_->setFixedHeight(8);

    layout->addWidget(progressLabel_);
    layout->addSpacing(4);
    layout->addWidget(progressBar_);
    layout->addSpacing(12);

    // Bottom action
    auto *bottomLayout = new QHBoxLayout;
    bottomLayout->setSpacing(8);

    statusLabel_ = new QLabel("Belum ada media dipilih.");
    statusLabel_->setObjectName("descriptionLabel");
    statusLabel_->setStyleSheet("font-size: 13px;");

    cancelButton_ = new QPushButton("Batalkan");
    cancelButton_->setObjectName("cancelButton");
    cancelButton_->setCursor(Qt::PointingHandCursor);
    cancelButton_->setMinimumHeight(38);
    cancelButton_->setMinimumWidth(105);
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
    convertButton_->setMinimumHeight(38);
    convertButton_->setMinimumWidth(105);
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

            // Ikon persegi (tampilan saja, tanpa membaca isi file)
            item->setIcon(style()->standardIcon(
                QStyle::SP_MediaVolume
            ));
            item->setSizeHint(QSize(0, kRowHeight));

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