#include "video_page.h"
#include "drop_list.h"
#include "../app_context.h"

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

#include "dialogs/feedback_dialog.h"
#include "../themes/theme_manager.h"

#include <QIcon>
#include <QSize>
#include <QStyle>

#include <QLineEdit>
#include <QStandardPaths>

// Ukuran ikon video (persegi 1:1) dan tinggi baris di daftar
static constexpr int kIconSize = 72;
static constexpr int kRowHeight = 96;

VideoPage::VideoPage(ThemeManager *themeManager, QWidget *parent)
    : QWidget(parent),
      themeManager_(themeManager)
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

void VideoPage::setupUI()
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
            border-radius: 5px;
            min-height: 10px;
            max-height: 10px;
        }
        QProgressBar::chunk {
            background: #5645d4;
            border-radius: 5px;
        }
    )QSS");

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(12);

    auto *title = new QLabel("Video Converter");
    title->setStyleSheet(
        "font-size:26px;font-weight:700;"
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
    toolbar->setSpacing(8);

    auto *addButton = new QPushButton("+ Tambahkan Video");
    addButton->setObjectName("primaryButton");
    addButton->setCursor(Qt::PointingHandCursor);
    addButton->setMinimumHeight(38);

    connect(
        addButton,
        &QPushButton::clicked,
        this,
        &VideoPage::selectFilesFromDialog
    );

    auto *removeButton = new QPushButton("Hapus");
    removeButton->setObjectName("secondaryButton");
    removeButton->setCursor(Qt::PointingHandCursor);
    removeButton->setMinimumHeight(38);

    connect(
        removeButton,
        &QPushButton::clicked,
        this,
        &VideoPage::removeSelected
    );

    auto *clearButton = new QPushButton("Bersihkan");
    clearButton->setObjectName("secondaryButton");
    clearButton->setCursor(Qt::PointingHandCursor);
    clearButton->setMinimumHeight(38);

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
    formatCombo_->setMinimumHeight(38);
    formatCombo_->addItems(FormatRegistry::videoOutputFormats());
    formatCombo_->setCursor(Qt::PointingHandCursor);

    toolbar->addWidget(formatLabel);
    toolbar->addWidget(formatCombo_);

    layout->addLayout(toolbar);
    // Output folder selector
auto *outputLayout = new QHBoxLayout;
outputLayout->setSpacing(8);

auto *outputLabel = new QLabel("Simpan ke:");
outputLabel->setStyleSheet("font-size:13px;font-weight:600;");

outputPathEdit_ = new QLineEdit(this);
outputPathEdit_->setPlaceholderText(
    "Pilih folder penyimpanan video..."
);
outputPathEdit_->setMinimumHeight(38);
outputPathEdit_->setClearButtonEnabled(true);
outputPathEdit_->setStyleSheet(R"(
    QLineEdit {
        background: rgba(255,255,255,0.90);
        color: #1a1a1a;
        border: 1px solid #c8c4be;
        border-radius: 8px;
        padding: 7px 12px;
        font-size: 13px;
    }
    QLineEdit:focus {
        border: 1px solid #5645d4;
    }
)");

browseOutputButton_ = new QPushButton("Browse");
browseOutputButton_->setObjectName("secondaryButton");
browseOutputButton_->setMinimumHeight(38);
browseOutputButton_->setCursor(Qt::PointingHandCursor);

outputLayout->addWidget(outputLabel);
outputLayout->addWidget(outputPathEdit_, 1);
outputLayout->addWidget(browseOutputButton_);

layout->addLayout(outputLayout);

// Load saved video folder
QString savedFolder =
    AppContext::instance().settings().getVideoOutputFolder();

if (savedFolder.isEmpty() || !QDir(savedFolder).exists()) {
    savedFolder = QStandardPaths::writableLocation(
        QStandardPaths::MoviesLocation
    );
}

if (savedFolder.isEmpty() || !QDir(savedFolder).exists()) {
    savedFolder = QDir::homePath();
}

outputPathEdit_->setText(
    QDir::toNativeSeparators(savedFolder)
);

connect(
    browseOutputButton_,
    &QPushButton::clicked,
    this,
    &VideoPage::browseOutputFolder
);

connect(
    outputPathEdit_,
    &QLineEdit::editingFinished,
    this,
    &VideoPage::saveOutputFolder
);

    // File list
    fileList_ = new DropListWidget(this);
    fileList_->setFilterMode(DropListWidget::FilterMode::Video);
    fileList_->setSelectionMode(QAbstractItemView::ExtendedSelection);
    fileList_->setMinimumHeight(260);
    fileList_->setIconSize(QSize(kIconSize, kIconSize));
    fileList_->setSpacing(4);
    fileList_->setVerticalScrollMode(
        QAbstractItemView::ScrollPerPixel
    );

    // Awal kosong = transparan (hanya bingkai putus-putus ala drop zone).
    // Saat video masuk, tiap file tampil sebagai kartu.
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
    bottomLayout->setSpacing(8);

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
    convertButton_->setMinimumHeight(38);
    convertButton_->setMinimumWidth(105);
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

            // Ikon video persegi (tampilan saja, tanpa membaca isi video)
            item->setIcon(style()->standardIcon(
                QStyle::SP_MediaPlay
            ));
            item->setSizeHint(QSize(0, kRowHeight));

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

    const QString folder = QDir::cleanPath(
        QDir::fromNativeSeparators(
            outputPathEdit_->text().trimmed()
        )
    );

    QFileInfo folderInfo(folder);

    if (folder.isEmpty() ||
        !folderInfo.exists() ||
        !folderInfo.isDir() ||
        !folderInfo.isWritable()) {

        QMessageBox::warning(
            this,
            "Folder Output Tidak Valid",
            "Folder penyimpanan tidak ditemukan atau tidak bisa ditulis.\n"
            "Silakan pilih folder yang valid."
        );

        return;
    }

    outputDirectory_ = folder;

    outputPathEdit_->setText(
        QDir::toNativeSeparators(folder)
    );

    AppContext::instance().settings().setVideoOutputFolder(folder);

    pendingFiles_ = files_;
    errors_.clear();

    totalFiles_ = pendingFiles_.size();
    completedFiles_ = 0;
    batchCancelled_ = false;
    batchFinished_ = false;

    fileList_->setEnabled(false);
    formatCombo_->setEnabled(false);
    outputPathEdit_->setEnabled(false);
    browseOutputButton_->setEnabled(false);

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
    if (batchFinished_) {
        return;
    }

    batchFinished_ = true;

    fileList_->setEnabled(true);
    formatCombo_->setEnabled(true);
    outputPathEdit_->setEnabled(true);
    browseOutputButton_->setEnabled(true);

    cancelButton_->hide();
    progressBar_->hide();
    progressLabel_->hide();

    updateStatus();

    const int failedCount = errors_.size();

    const int successCount =
        completedFiles_ - failedCount;

    const int skippedCount = batchCancelled_
        ? qMax(0, totalFiles_ - completedFiles_)
        : 0;

    FeedbackDialog::showResult(
        this,
        themeManager_,
        successCount,
        failedCount,
        skippedCount,
        errors_,
        batchCancelled_
    );

    pendingFiles_.clear();
    errors_.clear();
}

void VideoPage::browseOutputFolder()
{
    const QString currentFolder =
        outputPathEdit_->text().trimmed();

    const QString folder = QFileDialog::getExistingDirectory(
        this,
        "Pilih Folder Penyimpanan Video",
        QDir(currentFolder).exists()
            ? currentFolder
            : QDir::homePath()
    );

    if (folder.isEmpty()) {
        return;
    }

    outputPathEdit_->setText(
        QDir::toNativeSeparators(folder)
    );

    saveOutputFolder();
}


void VideoPage::saveOutputFolder()
{
    const QString folder = QDir::cleanPath(
        QDir::fromNativeSeparators(
            outputPathEdit_->text().trimmed()
        )
    );

    QFileInfo info(folder);

    if (folder.isEmpty() ||
        !info.exists() ||
        !info.isDir() ||
        !info.isWritable()) {

        QMessageBox::warning(
            this,
            "Folder Tidak Valid",
            "Folder tidak ditemukan atau tidak memiliki izin tulis."
        );

        const QString saved =
            AppContext::instance().settings().getVideoOutputFolder();

        if (!saved.isEmpty() && QDir(saved).exists()) {
            outputPathEdit_->setText(
                QDir::toNativeSeparators(saved)
            );
        }

        return;
    }

    outputPathEdit_->setText(
        QDir::toNativeSeparators(folder)
    );

    AppContext::instance().settings().setVideoOutputFolder(folder);
}