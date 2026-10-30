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
        "Kalkulator"
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

    auto *version = new QLabel("Marhaban Converter · v2.0.0");
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

    auto *welcome = new QLabel(
        "Selamat datang di Marhaban Converter"
    );
    welcome->setObjectName("welcomeLabel");

    auto *description = new QLabel(
        "Semua alat konversi media dalam satu aplikasi."
    );
    description->setObjectName("descriptionLabel");

    homeLayout->addWidget(welcome);
    homeLayout->addWidget(description);
    homeLayout->addStretch();

    pages_->addWidget(homePage);
    pages_->addWidget(new PhotoPage);
    pages_->addWidget(new VideoPage);
    pages_->addWidget(new AudioPage);
    pages_->addWidget(new CompressPage);
    pages_->addWidget(new CalculatorPage);

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