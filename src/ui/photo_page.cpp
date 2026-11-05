#include "photo_page.h"
#include "drop_list.h"

#include "../app_context.h"
#include "../services/format_registry.h"
#include "../services/image_converter_service.h"

#include <QAbstractItemView>
#include <QComboBox>
#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QIcon>
#include <QImageReader>
#include <QLabel>
#include <QLineEdit>
#include <QListWidgetItem>
#include <QMessageBox>
#include <QPixmap>
#include <QProgressBar>
#include <QPushButton>
#include <QSize>
#include <QStandardPaths>
#include <QStyle>
#include <QTimer>
#include <QVBoxLayout>
#include "dialogs/feedback_dialog.h"
#include "../themes/theme_manager.h"

// Ukuran thumbnail gambar
static constexpr int kThumbSize = 112;
static constexpr int kRowHeight = 132;

PhotoPage::PhotoPage(ThemeManager *themeManager, QWidget *parent)
    : QWidget(parent),
      themeManager_(themeManager)
{
    converter_ = new ImageConverterService(this);

    connect(
        converter_,
        &ImageConverterService::finished,
        this,
        [this](const QString &) {
            ++completedFiles_;
            processNextFile();
        }
    );

    connect(
        converter_,
        &ImageConverterService::failed,
        this,
        [this](const QString &error) {
            errors_.append(error);
            ++completedFiles_;
            processNextFile();
        }
    );

    connect(
        converter_,
        &ImageConverterService::cancelled,
        this,
        [this]() {
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
    setStyleSheet(R"QSS(
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

        QPushButton#primaryButton:hover {
            background: #4534b3;
        }

        QPushButton#primaryButton:pressed {
            background: #3a2a99;
        }

        QPushButton#primaryButton:disabled {
            background: rgba(255,255,255,0.55);
            color: #a4a097;
            border: 1px solid #e5e3df;
        }

        QPushButton#secondaryButton {
            background: rgba(255,255,255,0.88);
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

        QComboBox {
            background: rgba(255,255,255,0.90);
            color: #1a1a1a;
            border: 1px solid #c8c4be;
            border-radius: 8px;
            padding: 7px 12px;
            font-family: "Inter", "Segoe UI", sans-serif;
            font-size: 14px;
        }

        QComboBox:hover {
            border-color: #5645d4;
        }

        QComboBox QAbstractItemView {
            background: #ffffff;
            color: #1a1a1a;
            border: 1px solid #e5e3df;
            selection-background-color: #e6e0f5;
            selection-color: #391c57;
            outline: none;
        }

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

        QProgressBar {
            background: rgba(255,255,255,0.60);
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

    // Header
    auto *title = new QLabel("Photo Converter");
    title->setStyleSheet("font-size:26px;font-weight:700;");

    auto *description = new QLabel(
        "Konversi gambar ke berbagai format."
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

    auto *addButton = new QPushButton("+ Tambahkan Gambar");
    addButton->setObjectName("primaryButton");
    addButton->setCursor(Qt::PointingHandCursor);
    addButton->setMinimumHeight(38);

    connect(
        addButton,
        &QPushButton::clicked,
        this,
        &PhotoPage::selectFilesFromDialog
    );

    auto *removeButton = new QPushButton("Hapus");
    removeButton->setObjectName("secondaryButton");
    removeButton->setCursor(Qt::PointingHandCursor);
    removeButton->setMinimumHeight(38);

    connect(
        removeButton,
        &QPushButton::clicked,
        this,
        &PhotoPage::removeSelected
    );

    auto *clearButton = new QPushButton("Bersihkan");
    clearButton->setObjectName("secondaryButton");
    clearButton->setCursor(Qt::PointingHandCursor);
    clearButton->setMinimumHeight(38);

    connect(
        clearButton,
        &QPushButton::clicked,
        this,
        &PhotoPage::clearFiles
    );

    toolbar->addWidget(addButton);
    toolbar->addWidget(removeButton);
    toolbar->addWidget(clearButton);
    toolbar->addStretch();

    auto *formatLabel = new QLabel("Output:");
    formatLabel->setStyleSheet(
        "font-size:13px;font-weight:600;"
    );

    formatCombo_ = new QComboBox;

    for (const QString &format :
         FormatRegistry::imageOutputFormats()) {
        formatCombo_->addItem(format);
    }

    formatCombo_->setMinimumWidth(130);
    formatCombo_->setMinimumHeight(38);
    formatCombo_->setCursor(Qt::PointingHandCursor);

    if (formatCombo_->count() == 0) {
        formatCombo_->setEnabled(false);
    }

    toolbar->addWidget(formatLabel);
    toolbar->addWidget(formatCombo_);

    layout->addLayout(toolbar);

    // Output folder selector
    auto *outputLayout = new QHBoxLayout;
    outputLayout->setSpacing(8);

    auto *outputLabel = new QLabel("Simpan ke:");
    outputLabel->setStyleSheet(
        "font-size:13px;font-weight:600;"
    );

    outputPathEdit_ = new QLineEdit(this);
    outputPathEdit_->setPlaceholderText(
        "Pilih folder penyimpanan gambar..."
    );
    outputPathEdit_->setMinimumHeight(38);
    outputPathEdit_->setClearButtonEnabled(true);

    browseOutputButton_ = new QPushButton("Browse");
    browseOutputButton_->setObjectName("secondaryButton");
    browseOutputButton_->setMinimumHeight(38);
    browseOutputButton_->setCursor(Qt::PointingHandCursor);

    outputLayout->addWidget(outputLabel);
    outputLayout->addWidget(outputPathEdit_, 1);
    outputLayout->addWidget(browseOutputButton_);

    layout->addLayout(outputLayout);

    // Load saved output folder
    QString savedFolder =
        AppContext::instance().settings().getImageOutputFolder();

    if (savedFolder.isEmpty() || !QDir(savedFolder).exists()) {
        savedFolder = QStandardPaths::writableLocation(
            QStandardPaths::PicturesLocation
        );
    }

    if (savedFolder.isEmpty()) {
        savedFolder = QDir::homePath();
    }

    outputPathEdit_->setText(
        QDir::toNativeSeparators(savedFolder)
    );

    connect(
        browseOutputButton_,
        &QPushButton::clicked,
        this,
        &PhotoPage::browseOutputFolder
    );

    connect(
        outputPathEdit_,
        &QLineEdit::editingFinished,
        this,
        &PhotoPage::saveOutputFolder
    );

    // File list
    fileList_ = new DropListWidget(this);

    fileList_->setFilterMode(
        DropListWidget::FilterMode::Image
    );

    fileList_->setSelectionMode(
        QAbstractItemView::ExtendedSelection
    );

    fileList_->setMinimumHeight(300);
    fileList_->setViewMode(QListView::ListMode);
    fileList_->setFlow(QListView::TopToBottom);
    fileList_->setWrapping(false);
    fileList_->setIconSize(QSize(kThumbSize, kThumbSize));
    fileList_->setSpacing(4);
    fileList_->setUniformItemSizes(false);

    fileList_->setVerticalScrollMode(
        QAbstractItemView::ScrollPerPixel
    );

    fileList_->viewport()->setAutoFillBackground(false);

    fileList_->setStyleSheet(R"QSS(
        QListWidget {
            background: transparent;
            border: 2px dashed rgba(120,118,113,0.55);
            border-radius: 16px;
            padding: 8px;
            outline: none;
            font-family: "Inter", "Segoe UI", sans-serif;
            font-size: 14px;
            font-weight: 500;
        }

        QListWidget::item {
            background: rgba(255,255,255,0.90);
            color: #1a1a1a;
            border: 1px solid #e5e3df;
            border-radius: 12px;
            padding: 8px 12px;
            margin: 2px;
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
            background: rgba(120,118,113,0.55);
            border-radius: 4px;
            min-height: 30px;
        }

        QScrollBar::handle:vertical:hover {
            background: rgba(86,69,212,0.85);
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
        &PhotoPage::addFiles
    );

    layout->addWidget(fileList_, 1);

    // Progress bar
    progressBar_ = new QProgressBar;
    progressBar_->setRange(0, 100);
    progressBar_->setValue(0);
    progressBar_->setTextVisible(true);
    progressBar_->setVisible(false);

    layout->addWidget(progressBar_);

    // Bottom controls
    auto *bottom = new QHBoxLayout;
    bottom->setSpacing(8);

    statusLabel_ = new QLabel("Belum ada gambar dipilih.");
    statusLabel_->setStyleSheet(
        "font-size:13px;color:#737373;"
    );

    convertButton_ = new QPushButton("Convert");
    convertButton_->setObjectName("primaryButton");
    convertButton_->setCursor(Qt::PointingHandCursor);
    convertButton_->setMinimumHeight(38);
    convertButton_->setMinimumWidth(105);
    convertButton_->setEnabled(false);

    cancelButton_ = new QPushButton("Batalkan");
    cancelButton_->setObjectName("cancelButton");
    cancelButton_->setCursor(Qt::PointingHandCursor);
    cancelButton_->setMinimumHeight(38);
    cancelButton_->setMinimumWidth(105);
    cancelButton_->setEnabled(false);

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

    connect(
        convertButton_,
        &QPushButton::clicked,
        this,
        &PhotoPage::startBatchConversion
    );

    connect(
        cancelButton_,
        &QPushButton::clicked,
        this,
        &PhotoPage::cancelConversion
    );

    bottom->addWidget(statusLabel_);
    bottom->addStretch();
    bottom->addWidget(cancelButton_);
    bottom->addWidget(convertButton_);

    layout->addLayout(bottom);

    updateStatus();
}

void PhotoPage::browseOutputFolder()
{
    const QString currentFolder =
        outputPathEdit_->text().trimmed();

    const QString folder = QFileDialog::getExistingDirectory(
        this,
        "Pilih Folder Penyimpanan Gambar",
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

void PhotoPage::saveOutputFolder()
{
    const QString folder = QDir::cleanPath(
        outputPathEdit_->text().trimmed()
    );

    if (folder.isEmpty() || !QDir(folder).exists()) {
        QMessageBox::warning(
            this,
            "Folder Tidak Valid",
            "Folder penyimpanan gambar tidak ditemukan."
        );
        return;
    }

    outputPathEdit_->setText(
        QDir::toNativeSeparators(folder)
    );

    AppContext::instance().settings().setImageOutputFolder(folder);
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

        QFileInfo fileInfo(path);

        auto *item = new QListWidgetItem(
            fileInfo.fileName(),
            fileList_
        );

        item->setData(Qt::UserRole, path);
        item->setToolTip(path);

        QImageReader reader(path);
        reader.setAutoTransform(true);

        QPixmap thumbnail = QPixmap::fromImage(reader.read());

        if (!thumbnail.isNull()) {
            QPixmap scaled = thumbnail.scaled(
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
        } else {
            item->setIcon(
                style()->standardIcon(QStyle::SP_FileIcon)
            );
        }

        item->setSizeHint(QSize(0, kRowHeight));
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
        pendingFiles_.isEmpty() &&
        formatCombo_->count() > 0
    );
}

QString PhotoPage::createOutputPath(
    const QString &inputPath
) const
{
    QFileInfo info(inputPath);

    QString extension =
        FormatRegistry::extensionForImageOutput(
            selectedOutputFormat()
        );

    if (extension.isEmpty()) {
        extension = selectedOutputFormat().toLower();
    }

    const QString baseName = info.completeBaseName();

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

    const QString folder = QDir::cleanPath(
        outputPathEdit_->text().trimmed()
    );

    if (folder.isEmpty() || !QDir(folder).exists()) {
        QMessageBox::warning(
            this,
            "Folder Tidak Valid",
            "Pilih folder penyimpanan gambar yang valid terlebih dahulu."
        );
        return;
    }

    if (!QFileInfo(folder).isWritable()) {
        QMessageBox::warning(
            this,
            "Folder Tidak Dapat Ditulis",
            "Aplikasi tidak memiliki izin untuk menyimpan file di folder ini."
        );
        return;
    }

    outputDirectory_ = folder;

    AppContext::instance().settings().setImageOutputFolder(folder);

    pendingFiles_ = files_;
    errors_.clear();

    completedFiles_ = 0;
    totalFiles_ = pendingFiles_.size();

    batchCancelled_ = false;
    batchFinished_ = false;

    progressBar_->setValue(0);
    progressBar_->setVisible(true);

    convertButton_->setEnabled(false);
    cancelButton_->setEnabled(true);

    formatCombo_->setEnabled(false);
    fileList_->setEnabled(false);
    outputPathEdit_->setEnabled(false);
    browseOutputButton_->setEnabled(false);

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
    if (!converter_ || !converter_->isRunning()) {
        batchCancelled_ = true;
        pendingFiles_.clear();
        finishBatchConversion();
        return;
    }

    batchCancelled_ = true;
    pendingFiles_.clear();

    statusLabel_->setText(
        "Menghentikan proses konversi..."
    );

    cancelButton_->setEnabled(false);

    converter_->cancel();
}

void PhotoPage::finishBatchConversion()
{
    if (batchFinished_) {
        return;
    }

    batchFinished_ = true;
    pendingFiles_.clear();

    progressBar_->setValue(
        totalFiles_ > 0
            ? completedFiles_ * 100 / totalFiles_
            : 0
    );

    fileList_->setEnabled(true);
    formatCombo_->setEnabled(true);
    outputPathEdit_->setEnabled(true);
    browseOutputButton_->setEnabled(true);

    cancelButton_->setEnabled(false);

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
}