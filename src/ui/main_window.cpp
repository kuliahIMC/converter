#include "main_window.h"

#include <QColor>
#include <QComboBox>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QPaintEvent>
#include <QPainter>
#include <QPushButton>
#include <QResizeEvent>
#include <QStackedWidget>
#include <QVBoxLayout>
#include <QWidget>

#include "audio_page.h"
#include "video_page.h"
#include "photo_page.h"
#include "compress_page.h"
#include "calculator_page.h"
#include "calendar_page.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      themeManager_(new ThemeManager(this))
{
    setupUI();
}

void MainWindow::setupUI()
{
    setWindowTitle("Marhaban Converter");
    resize(1180, 760);
    setMinimumSize(900, 600);

    auto *central = new QWidget(this);
    central->setObjectName("central");

    auto *root = new QHBoxLayout(central);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);

    // SIDEBAR
    auto *sidebar = new QFrame;
    sidebar->setObjectName("sidebar");
    sidebar->setFixedWidth(235);

    auto *sidebarLayout = new QVBoxLayout(sidebar);
    sidebarLayout->setContentsMargins(18, 24, 18, 20);
    sidebarLayout->setSpacing(8);

    auto *brand = new QLabel(
        "<span style='color:#2563eb;'>M</span> "
        "<b>Marhaban</b><br>"
        "<span style='font-size:11px;'>"
        "STUDIO · MEDIA TOOLS</span>"
    );

    brand->setObjectName("brandLabel");
    brand->setStyleSheet("font-size:20px; padding:4px 2px;");

    sidebarLayout->addWidget(brand);
    sidebarLayout->addSpacing(28);

    auto *navLabel = new QLabel("WORKSPACE");
    navLabel->setObjectName("sectionLabel");

    sidebarLayout->addWidget(navLabel);
    sidebarLayout->addSpacing(6);

    const QStringList names = {
        "Beranda",
        "Foto & Gambar",
        "Konversi Video",
        "Konversi Audio",
        "Kompres Media",
        "Kalkulator",
        "Kalender Converter"
    };

    for (int i = 0; i < names.size(); ++i) {
        auto *button = new QPushButton(names[i]);

        button->setObjectName("navButton");
        button->setCheckable(true);
        button->setCursor(Qt::PointingHandCursor);
        button->setMinimumHeight(42);

        sidebarLayout->addWidget(button);
        navButtons_.append(button);

        connect(button, &QPushButton::clicked, this,
                [this, i]() {
                    navigateTo(i);
                });
    }

    sidebarLayout->addStretch();

    auto *version = new QLabel("Marhaban Converter · v2.1.0");
    version->setObjectName("versionLabel");
    sidebarLayout->addWidget(version);

    // MAIN CONTENT
    auto *content = new QWidget;
    auto *contentLayout = new QVBoxLayout(content);

    contentLayout->setContentsMargins(32, 24, 32, 26);
    contentLayout->setSpacing(24);

    auto *header = new QHBoxLayout;

    pageTitle_ = new QLabel("Beranda");
    pageTitle_->setObjectName("pageTitle");

    header->addWidget(pageTitle_);
    header->addStretch();

    // THEME SELECTOR
    setupThemeSelector();
    header->addWidget(themeSelector_);

    auto *status = new QLabel("● SIAP DIGUNAKAN");
    status->setObjectName("statusLabel");

    header->addWidget(status);

    contentLayout->addLayout(header);

    auto *separator = new QFrame;
    separator->setFrameShape(QFrame::HLine);
    separator->setObjectName("separator");

    contentLayout->addWidget(separator);


// HOME PAGE
    pages_ = new QStackedWidget;

    auto *homePage = new QWidget;
    auto *homeLayout = new QVBoxLayout(homePage);

    homeLayout->setContentsMargins(0, 4, 0, 0);
    homeLayout->setSpacing(18);

    // ===== STYLE KHUSUS HALAMAN BERANDA (gaya Notion) =====
    // Dipasang di homePage supaya tidak mengganggu QSS tema di ThemeManager.
    // Warna memakai latar sendiri (kartu/hero), jadi tetap terbaca di tema foto.
    homePage->setStyleSheet(R"QSS(
        /* --- HERO (navy) --- */
        QFrame#homeHero {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
                                        stop:0 #0a1530, stop:1 #1a2a52);
            border-radius: 16px;
        }
        QFrame#homeHero QLabel {
            background: transparent;
        }
        QFrame#homeHero QLabel#welcomeLabel {
            color: #ffffff;
            font-family: "Inter", "Segoe UI", sans-serif;
            font-size: 30px;
            font-weight: 600;
        }
        QFrame#homeHero QLabel#descriptionLabel {
            color: #c9c6bf;
            font-family: "Inter", "Segoe UI", sans-serif;
            font-size: 15px;
        }
        QLabel#homeHeroDots {
            background: transparent;
            font-size: 14px;
        }

        /* --- JUDUL SEKSI --- */
        QLabel#sectionLabel {
            font-family: "Inter", "Segoe UI", sans-serif;
            font-size: 20px;
            font-weight: 600;
        }

        /* --- TAB ALAT (pill) --- */
        QPushButton#homeToolButton {
            background: rgba(255, 255, 255, 0.88);
            color: #37352f;
            border: 1px solid #e5e3df;
            border-radius: 22px;
            padding: 8px 14px;
            font-family: "Inter", "Segoe UI", sans-serif;
            font-size: 14px;
            font-weight: 500;
        }
        QPushButton#homeToolButton:hover {
            background: #ffffff;
            border-color: #c8c4be;
        }
        QPushButton#homeToolButton:checked {
            background: #000000;
            color: #ffffff;
            border-color: #000000;
        }

        /* --- PANEL DETAIL (kartu pastel) --- */
        QStackedWidget#homeToolDetails {
            background: transparent;
        }
        QWidget#homeDetailCard {
            border: 1px solid rgba(0, 0, 0, 0.06);
            border-radius: 14px;
            background: #f0eeec;
        }
        QWidget#homeDetailCard[tint="peach"]    { background: #ffe8d4; }
        QWidget#homeDetailCard[tint="sky"]      { background: #dcecfa; }
        QWidget#homeDetailCard[tint="mint"]     { background: #d9f3e1; }
        QWidget#homeDetailCard[tint="rose"]     { background: #fde0ec; }
        QWidget#homeDetailCard[tint="lavender"] { background: #e6e0f5; }

        QWidget#homeDetailCard QLabel {
            background: transparent;
            border: none;
            font-family: "Inter", "Segoe UI", sans-serif;
        }
        QWidget#homeDetailCard QLabel#homeDetailTitle {
            color: #1a1a1a;
            font-size: 24px;
            font-weight: 600;
        }
        QWidget#homeDetailCard QLabel#descriptionLabel {
            color: #5d5b54;
            font-size: 14px;
        }
        QWidget#homeDetailCard QLabel#homeDetailFeatures {
            color: #37352f;
            font-size: 14px;
            font-weight: 500;
        }

        /* --- TOMBOL BUKA (ungu utama) --- */
        QPushButton#homeOpenToolButton {
            background: #5645d4;
            color: #ffffff;
            border: none;
            border-radius: 8px;
            padding: 0 22px;
            font-family: "Inter", "Segoe UI", sans-serif;
            font-size: 14px;
            font-weight: 500;
        }
        QPushButton#homeOpenToolButton:hover   { background: #4534b3; }
        QPushButton#homeOpenToolButton:pressed { background: #3a2a99; }
    )QSS");

    // WELCOME SECTION (dibungkus hero navy)
    auto *hero = new QFrame;
    hero->setObjectName("homeHero");

    auto *heroLayout = new QVBoxLayout(hero);
    heroLayout->setContentsMargins(28, 24, 28, 26);
    heroLayout->setSpacing(8);

    // titik warna dekoratif ala sticky-note Notion
    auto *heroDots = new QLabel(
        "<span style='color:#ff64c8;'>●</span>&nbsp;"
        "<span style='color:#f5d75e;'>●</span>&nbsp;"
        "<span style='color:#2a9d99;'>●</span>&nbsp;"
        "<span style='color:#7b3ff2;'>●</span>"
    );
    heroDots->setObjectName("homeHeroDots");
    heroDots->setTextFormat(Qt::RichText);

    auto *welcome = new QLabel(
        "Selamat datang di Marhaban Converter."
    );
    welcome->setObjectName("welcomeLabel");
    welcome->setWordWrap(true);

    auto *description = new QLabel(
        "Solusi sederhana untuk mengonversi dan mengelola "
        "berbagai jenis media dalam satu aplikasi."
    );
    description->setObjectName("descriptionLabel");
    description->setWordWrap(true);

    heroLayout->addWidget(heroDots);
    heroLayout->addWidget(welcome);
    heroLayout->addWidget(description);

    homeLayout->addWidget(hero);

    // TOOLBAR SECTION
    auto *toolbarTitle = new QLabel("Jelajahi Fitur");
    toolbarTitle->setObjectName("sectionLabel");

    auto *toolbarDescription = new QLabel(
        "Pilih salah satu alat untuk mengetahui fungsi dan "
        "kemampuannya."
    );
    toolbarDescription->setObjectName("descriptionLabel");

    homeLayout->addWidget(toolbarTitle);
    homeLayout->addWidget(toolbarDescription);

    auto *toolbar = new QHBoxLayout;
    toolbar->setSpacing(8);

    const QStringList toolNames = {
        "Foto",
        "Video",
        "Audio",
        "Kompres",
        "Kalkulator"
    };

    const QStringList toolIcons = {
        "▧",
        "▷",
        "♫",
        "▤",
        "＋"
    };

    // warna pastel tiap kartu detail (urutan sama dengan toolNames)
    const QStringList toolTints = {
        "peach",
        "sky",
        "mint",
        "rose",
        "lavender"
    };

    QList<QPushButton *> toolButtons;

    for (int i = 0; i < toolNames.size(); ++i) {
        auto *button = new QPushButton(
            toolIcons[i] + "  " + toolNames[i]
        );

        button->setObjectName("homeToolButton");
        button->setCheckable(true);
        button->setCursor(Qt::PointingHandCursor);
        button->setMinimumHeight(44);
        button->setSizePolicy(
            QSizePolicy::Expanding,
            QSizePolicy::Preferred
        );

        toolbar->addWidget(button);
        toolButtons.append(button);
    }

    homeLayout->addLayout(toolbar);

    // TOOL DESCRIPTION PANEL
    auto *toolDetails = new QStackedWidget;
    toolDetails->setObjectName("homeToolDetails");
    toolDetails->setMinimumHeight(220);

    const QStringList toolDescriptions = {
        "Ubah gambar ke berbagai format seperti JPG, PNG, "
        "WEBP, BMP, TIFF, AVIF, JP2, TGA, ICO, dan SVG.",

        "Konversi video ke berbagai format untuk kebutuhan "
        "pemutaran, penyimpanan, dan kompatibilitas perangkat.",

        "Konversi file audio ke format MP3, WAV, FLAC, AAC, "
        "M4A, OGG, OPUS, dan AIFF.",

        "Kurangi ukuran file gambar maupun video agar lebih "
        "hemat ruang penyimpanan.",

        "Gunakan kalkulator untuk membantu perhitungan "
        "dengan cepat langsung dari aplikasi."
    };

    const QStringList toolFeatures = {
        "• Mendukung berbagai format gambar\n"
        "• Konversi gambar satu per satu\n"
        "• Mendukung pemrosesan beberapa file",

        "• Mendukung berbagai format video\n"
        "• Menggunakan FFmpeg\n"
        "• Mengatur format hasil konversi",

        "• Mendukung berbagai format audio\n"
        "• Konversi menggunakan FFmpeg\n"
        "• Mempermudah pengelolaan file audio",

        "• Mengoptimalkan ukuran media\n"
        "• Membantu menghemat penyimpanan\n"
        "• Mendukung file gambar dan video",

        "• Perhitungan dasar\n"
        "• Operasi aritmetika\n"
        "• Tersedia langsung di aplikasi"
    };

    for (int i = 0; i < toolNames.size(); ++i) {
        auto *detailPage = new QWidget;

        // supaya background QSS tergambar di QWidget biasa
        detailPage->setObjectName("homeDetailCard");
        detailPage->setAttribute(Qt::WA_StyledBackground, true);
        detailPage->setProperty("tint", toolTints[i]);

        auto *detailLayout = new QVBoxLayout(detailPage);

        detailLayout->setContentsMargins(24, 20, 24, 20);
        detailLayout->setSpacing(12);

        auto *title = new QLabel(toolNames[i]);
        title->setObjectName("homeDetailTitle");

        auto *detailDescription = new QLabel(
            toolDescriptions[i]
        );
        detailDescription->setObjectName("descriptionLabel");
        detailDescription->setWordWrap(true);

        auto *features = new QLabel(toolFeatures[i]);
        features->setObjectName("homeDetailFeatures");
        features->setWordWrap(true);

        auto *openButton = new QPushButton(
            "Buka " + toolNames[i]
        );
        openButton->setObjectName("homeOpenToolButton");
        openButton->setCursor(Qt::PointingHandCursor);
        openButton->setMinimumHeight(40);
        openButton->setMinimumWidth(170);

        detailLayout->addWidget(title);
        detailLayout->addWidget(detailDescription);
        detailLayout->addWidget(features);
        detailLayout->addStretch();
        detailLayout->addWidget(openButton, 0, Qt::AlignLeft);

        toolDetails->addWidget(detailPage);

        connect(
            openButton,
            &QPushButton::clicked,
            this,
            [this, i]() {
                navigateTo(i + 1);
            }
        );
    }

    homeLayout->addWidget(toolDetails);
    homeLayout->addStretch();

    // TOOLBAR INTERACTION
    for (int i = 0; i < toolButtons.size(); ++i) {
        connect(
            toolButtons[i],
            &QPushButton::clicked,
            this,
            [toolButtons, toolDetails, i]() {
                toolDetails->setCurrentIndex(i);

                for (int j = 0; j < toolButtons.size(); ++j) {
                    toolButtons[j]->setChecked(j == i);
                }
            }
        );
    }

    // Pilihan awal: Photo Converter
    toolButtons[0]->setChecked(true);
    toolDetails->setCurrentIndex(0);

    // REGISTER PAGES
    pages_->addWidget(homePage);
    pages_->addWidget(new PhotoPage);
    pages_->addWidget(new VideoPage);
    pages_->addWidget(new AudioPage);
    pages_->addWidget(new CompressPage);
    pages_->addWidget(new CalculatorPage);
    pages_->addWidget(new CalendarPage);


    contentLayout->addWidget(pages_, 1);

    root->addWidget(sidebar);
    root->addWidget(content, 1);

    setCentralWidget(central);

    // sambungkan dulu, baru applyTheme, supaya foto langsung termuat
    connect(themeManager_, &ThemeManager::themeChanged,
            this, [this](const QString &) { updateBackground(); });

    // APPLY SAVED THEME
    themeManager_->applyTheme(themeManager_->currentTheme());

    updateBackground();
    navigateTo(0);
}

void MainWindow::setupThemeSelector()
{
    themeSelector_ = new QComboBox;
    themeSelector_->setObjectName("themeSelector");
    themeSelector_->setMinimumWidth(145);
    themeSelector_->setCursor(Qt::PointingHandCursor);

    themeSelector_->addItems(themeManager_->availableThemes());
    themeSelector_->setCurrentText(themeManager_->currentTheme());

    connect(
        themeSelector_,
        &QComboBox::currentTextChanged,
        this,
        [this](const QString &theme) {
            themeManager_->applyTheme(theme);
        }
    );
}

void MainWindow::navigateTo(int index)
{
    if (!pages_ || index < 0 || index >= pages_->count()) {
        return;
    }

    pages_->setCurrentIndex(index);
    pageTitle_->setText(navButtons_[index]->text());

    for (int i = 0; i < navButtons_.size(); ++i) {
        navButtons_[i]->setChecked(i == index);
    }
}

// ---------- BACKGROUND FOTO ----------

void MainWindow::updateBackground()
{
    const QString theme = themeManager_->currentTheme();

    if (theme == "Light" || theme == "Dark") {
        bgScaled_ = QPixmap();
        bgOriginal_ = QPixmap();
        bgPath_.clear();
        update();
        return;
    }

    const QString path = themeManager_->themeImagePath(theme);

    if (path != bgPath_) {
        bgPath_ = path;
        bgOriginal_ = QPixmap();
        if (!path.isEmpty())
            bgOriginal_.load(path);
    }

    if (!bgOriginal_.isNull() && !size().isEmpty()) {
        bgScaled_ = bgOriginal_.scaled(
            size(),
            Qt::KeepAspectRatioByExpanding,
            Qt::SmoothTransformation);
    } else {
        bgScaled_ = QPixmap();
    }

    update();
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    QMainWindow::resizeEvent(event);
    updateBackground();
}

void MainWindow::paintEvent(QPaintEvent *event)
{
    QPainter p(this);

    if (!bgScaled_.isNull()) {
        const QPoint topLeft((width()  - bgScaled_.width())  / 2,
                             (height() - bgScaled_.height()) / 2);
        p.drawPixmap(topLeft, bgScaled_);

        // lapisan gelap tipis supaya teks terbaca
        p.fillRect(rect(), QColor(0, 0, 0, 110));
    }

    QMainWindow::paintEvent(event);
}