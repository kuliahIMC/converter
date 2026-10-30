#include "theme_manager.h"

#include <QApplication>
#include <QColor>
#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QSettings>
#include <QWidget>

#include <cmath>

ThemeManager::ThemeManager(QObject *parent)
    : QObject(parent)
{
    loadSavedTheme();
}

QStringList ThemeManager::availableThemes() const
{
    return {
        "Light",
        "Dark",
        "Bochi",
        "Frieren",
        "Kurumi",
        "Mao Mao",
        "Ram & Rem"
    };
}

QString ThemeManager::currentTheme() const
{
    return currentTheme_;
}

QString ThemeManager::themeImagePath(const QString &theme) const
{
    QString fileName;

    if (theme == "Frieren") {
        fileName = "friren";
    } else if (theme == "Ram & Rem") {
        fileName = "ram-rem";
    } else {
        fileName = theme.toLower().replace(" ", "-");
    }

    const QString filePath = fileName + ".png";

    const QStringList locations = {
        QCoreApplication::applicationDirPath()
            + "/../resources/images/themes/" + filePath,

        QCoreApplication::applicationDirPath()
            + "/resources/images/themes/" + filePath,

        QDir::currentPath()
            + "/resources/images/themes/" + filePath
    };

    for (const QString &path : locations) {
        const QString absolutePath = QDir::cleanPath(path);

        if (QFileInfo::exists(absolutePath))
            return absolutePath;
    }

    return {};
}

void ThemeManager::applyTheme(const QString &theme)
{
    if (!availableThemes().contains(theme))
        return;

    currentTheme_ = theme;

    saveTheme(theme);

    if (qApp)
        qApp->setStyleSheet(getThemeStyleSheet(theme));

    emit themeChanged(theme);
}

void ThemeManager::applyToWidget(QWidget *widget)
{
    if (!widget)
        return;

    widget->setStyleSheet(getThemeStyleSheet(currentTheme_));
}

void ThemeManager::loadSavedTheme()
{
    QSettings settings;

    currentTheme_ = settings.value(
        "appearance/theme",
        "Dark"
    ).toString();

    if (!availableThemes().contains(currentTheme_))
        currentTheme_ = "Dark";
}

void ThemeManager::saveTheme(const QString &theme)
{
    QSettings settings;
    settings.setValue("appearance/theme", theme);
}

QString ThemeManager::getThemeStyleSheet(const QString &theme) const
{
    QString background, surface, sidebar, popup, text, muted,
            accent, border, hover, active, input;

    if (theme == "Light") {
        background = "#f5f6fa";
        surface    = "#ffffff";
        sidebar    = "#ffffff";
        popup      = "#ffffff";
        text       = "#1f2430";
        muted      = "#6b7280";
        accent     = "#6750d8";
        border     = "#e1e4ec";
        hover      = "#f0edff";
        active     = "#e4defc";
        input      = "#ffffff";
    }
    else if (theme == "Dark") {
        background = "#111318";
        surface    = "#1b1e26";
        sidebar    = "#171a21";
        popup      = "#1b1e26";
        text       = "#f2f3f5";
        muted      = "#9aa1ae";
        accent     = "#8b78ff";
        border     = "#2b303b";
        hover      = "#282c37";
        active     = "#343044";
        input      = "#151820";
    }
    // BOCHI - SAKURA PINK
    else if (theme == "Bochi") {
        background = "#1a1119";
        surface    = "rgba(40, 24, 38, 150)";
        sidebar    = "rgba(35, 24, 38, 150)";
        popup      = "#2a1c2b";
        text       = "#F9F1F7";
        muted      = "#B7A0B2";
        accent     = "#D0679C";
        border     = "#6B4A68";
        hover      = "rgba(98, 56, 88, 190)";
        active     = "rgba(130, 66, 112, 215)";
        input      = "rgba(24, 18, 27, 215)";
    }
    // FRIEREN - FANTASY BLUE
    else if (theme == "Frieren") {
        background = "#101722";
        surface    = "rgba(22, 32, 48, 150)";
        sidebar    = "rgba(24, 32, 48, 150)";
        popup      = "#18202f";
        text       = "#F4F5FA";
        muted      = "#9AA6BB";
        accent     = "#7BA3DB";
        border     = "#52627D";
        hover      = "rgba(72, 91, 120, 150)";
        active     = "rgba(87, 111, 151, 205)";
        input      = "rgba(19, 27, 40, 215)";
    }
    // KURUMI - GOTHIC CRIMSON
    else if (theme == "Kurumi") {
        background = "#0d080d";
        surface    = "rgba(30, 14, 22, 155)";
        sidebar    = "rgba(16, 10, 16, 150)";
        popup      = "#1b0f17";
        text       = "#F5EEF2";
        muted      = "#9C8790";
        accent     = "#D12A4D";
        border     = "#5A2236";
        hover      = "rgba(58, 24, 38, 190)";
        active     = "rgba(92, 28, 48, 220)";
        input      = "rgba(15, 10, 14, 210)";
    }
    // MAO MAO - HERBAL GREEN
    else if (theme == "Mao Mao") {
        background = "#0c1410";
        surface    = "rgba(20, 32, 24, 150)";
        sidebar    = "rgba(20, 30, 24, 150)";
        popup      = "#16221a";
        text       = "#F2F4E9";
        muted      = "#9CA896";
        accent     = "#8FB063";
        border     = "#4A614B";
        hover      = "rgba(54, 73, 58, 190)";
        active     = "rgba(70, 95, 72, 215)";
        input      = "rgba(17, 25, 20, 215)";
    }
    // RAM & REM
    else {
        background = "#10131f";
        surface    = "rgba(26, 30, 50, 150)";
        sidebar    = "rgba(31, 35, 55, 150)";
        popup      = "#1a1f33";
        text       = "#F4F2F8";
        muted      = "#9AA3BA";
        accent     = "#7F9BD6";
        border     = "#56658A";
        hover      = "rgba(71, 82, 117, 180)";
        active     = "rgba(92, 108, 151, 215)";
        input      = "rgba(19, 23, 38, 215)";
    }

    const bool transparentTheme = theme != "Light" && theme != "Dark";

    // transparan agar gambar tema terlihat di MainWindow/central
    const QString rootBackground = transparentTheme ? "transparent" : background;

    // warna teks di atas aksen: hitam/putih otomatis sesuai kecerahan
    const auto luminance = [](const QColor &c) {
        auto lin = [](qreal v) {
            return v <= 0.03928 ? v / 12.92 : std::pow((v + 0.055) / 1.055, 2.4);
        };
        return 0.2126 * lin(c.redF()) + 0.7152 * lin(c.greenF()) + 0.0722 * lin(c.blueF());
    };
    const QString onAccent = luminance(QColor(accent)) > 0.19 ? "#111318" : "#ffffff";

    return QString(R"(
        /* GLOBAL */

        QWidget {
            color: %1;
            font-family: "Segoe UI";
            font-size: 13px;
        }

        QMainWindow,
        QWidget#central,
        QStackedWidget {
            background-color: %2;
        }

        QDialog {
            background-color: %12;
        }

        /* biar foto latar tembus ke widget di atasnya */

        QLabel,
        QCheckBox,
        QRadioButton {
            background: transparent;
        }

        QStackedWidget > QWidget {
            background: transparent;
        }

        QScrollArea,
        QScrollArea > QWidget > QWidget {
            background: transparent;
        }

        /* SIDEBAR */

        QFrame#sidebar {
            background-color: %3;
            border: none;
            border-right: 1px solid %7;
        }

        QLabel#brandLabel {
            color: %1;
            font-size: 20px;
            font-weight: 600;
            padding: 4px 2px;
        }

        QLabel#sectionLabel {
            color: %8;
            font-size: 10px;
            font-weight: 700;
            letter-spacing: 1px;
            padding: 4px 2px;
        }

        QLabel#versionLabel {
            color: %8;
            font-size: 11px;
            padding-top: 12px;
        }

        /* NAVIGATION */

        QPushButton#navButton {
            background-color: transparent;
            color: %8;
            border: 1px solid transparent;
            border-radius: 8px;
            text-align: left;
            padding: 10px 13px;
            font-size: 13px;
        }

        QPushButton#navButton:hover {
            background-color: %9;
            color: %1;
        }

        QPushButton#navButton:checked {
            background-color: %10;
            color: %6;
            border: 1px solid %7;
            font-weight: 600;
        }

        /* HEADER */

        QLabel#pageTitle {
            color: %1;
            font-size: 23px;
            font-weight: 700;
        }

        QLabel#statusLabel {
            color: %6;
            font-size: 11px;
            font-weight: 600;
            padding: 6px 10px;
        }

        QFrame#separator {
            background-color: %7;
            border: none;
            max-height: 1px;
        }

        QLabel#welcomeLabel {
            color: %1;
            font-size: 25px;
            font-weight: 700;
        }

        QLabel#descriptionLabel {
            color: %8;
            font-size: 14px;
        }

        /* THEME SELECTOR */

        QComboBox#themeSelector {
            background-color: %11;
            color: %1;
            border: 1px solid %7;
            border-radius: 8px;
            padding: 8px 12px;
            min-height: 20px;
        }

        QComboBox#themeSelector:hover {
            border-color: %6;
        }

        QComboBox::drop-down {
            border: none;
            width: 25px;
        }

        QComboBox QAbstractItemView {
            background-color: %5;
            color: %1;
            border: 1px solid %7;
            selection-background-color: %6;
            selection-color: %13;
            padding: 5px;
        }

        QLabel {
            color: %1;
        }

        QLabel#sectionLabel,
        QLabel#versionLabel,
        QLabel#descriptionLabel {
            color: %8;
        }

        /* GENERAL SURFACES */

        QFrame,
        QGroupBox {
            border-color: %7;
        }

        QGroupBox {
            background-color: %4;
            border: 1px solid %7;
            border-radius: 10px;
            margin-top: 12px;
            padding: 12px;
            font-weight: 600;
        }

        QGroupBox::title {
            subcontrol-origin: margin;
            left: 12px;
            padding: 0 5px;
        }

        /* BUTTONS */

        QPushButton {
            background-color: %4;
            color: %1;
            border: 1px solid %7;
            border-radius: 8px;
            padding: 9px 15px;
        }

        QPushButton:hover {
            background-color: %9;
            border-color: %6;
        }

        QPushButton:pressed {
            background-color: %6;
            color: %13;
        }

        QPushButton:disabled {
            color: %8;
            background-color: %3;
            border-color: %7;
        }

        /* INPUTS */

        QLineEdit,
        QTextEdit,
        QPlainTextEdit,
        QSpinBox,
        QDoubleSpinBox,
        QDateEdit,
        QTimeEdit {
            background-color: %11;
            color: %1;
            border: 1px solid %7;
            border-radius: 8px;
            padding: 8px 10px;
            selection-background-color: %6;
            selection-color: %13;
        }

        QLineEdit {
            placeholder-text-color: %8;
        }

        QLineEdit:focus,
        QTextEdit:focus,
        QPlainTextEdit:focus,
        QSpinBox:focus,
        QComboBox:focus {
            border: 1px solid %6;
        }

        /* PROGRESS */

        QProgressBar {
            background-color: %4;
            color: %1;
            border: 1px solid %7;
            border-radius: 5px;
            text-align: center;
            min-height: 9px;
        }

        QProgressBar::chunk {
            background-color: %6;
            border-radius: 5px;
        }

        /* SLIDER */

        QSlider::groove:horizontal {
            background-color: %7;
            height: 5px;
            border-radius: 2px;
        }

        QSlider::handle:horizontal {
            background-color: %6;
            width: 14px;
            height: 14px;
            margin: -5px 0;
            border-radius: 7px;
        }

        QSlider::sub-page:horizontal {
            background-color: %6;
            border-radius: 2px;
        }

        /* CHECKBOX */

        QCheckBox {
            spacing: 8px;
        }

        QCheckBox::indicator {
            width: 16px;
            height: 16px;
            border: 1px solid %7;
            border-radius: 4px;
            background-color: %11;
        }

        QCheckBox::indicator:checked {
            background-color: %6;
            border-color: %6;
        }

        /* LIST AND TABLE */

        QListWidget,
        QTreeWidget,
        QTableWidget,
        QTableView {
            background-color: %4;
            color: %1;
            border: 1px solid %7;
            border-radius: 8px;
            gridline-color: %7;
            selection-background-color: %10;
            selection-color: %1;
        }

        QListWidget {
            outline: none;
            padding: 6px;
        }

        QListWidget::item {
            padding: 8px 10px;
            border-bottom: 1px solid %7;
            border-radius: 4px;
        }

        QListWidget::item:selected {
            background-color: %10;
            color: %1;
        }

        QHeaderView::section {
            background-color: %3;
            color: %8;
            border: none;
            border-bottom: 1px solid %7;
            padding: 9px;
            font-weight: 600;
        }

        /* SCROLLBAR */

        QScrollBar:vertical {
            background-color: transparent;
            width: 8px;
            margin: 0;
        }

        QScrollBar::handle:vertical {
            background-color: %7;
            border-radius: 4px;
            min-height: 25px;
        }

        QScrollBar::handle:vertical:hover {
            background-color: %6;
        }

        QScrollBar::add-line:vertical,
        QScrollBar::sub-line:vertical {
            height: 0;
        }

        QScrollBar:horizontal {
            background-color: transparent;
            height: 8px;
        }

        QScrollBar::handle:horizontal {
            background-color: %7;
            border-radius: 4px;
            min-width: 25px;
        }

        /* MENU */

        QMenu {
            background-color: %5;
            color: %1;
            border: 1px solid %7;
            padding: 5px;
        }

        QMenu::item {
            padding: 7px 22px;
            border-radius: 5px;
        }

        QMenu::item:selected {
            background-color: %6;
            color: %13;
        }

        /* CALCULATOR */

        QFrame#calcCard {
            background-color: %4;
            border: 1px solid %7;
            border-radius: 18px;
        }

        QLineEdit#calcDisplay {
            background-color: transparent;
            border: none;
            border-radius: 0;
            font-size: 34px;
            font-weight: 600;
            padding: 2px 8px;
        }

        QLineEdit#calcDisplay:focus {
            border: none;
        }

        QLabel#calcHistory {
            color: %8;
            font-size: 14px;
            padding-right: 8px;
        }

        QLabel#calcModeChip {
            color: %6;
            font-size: 11px;
            font-weight: 700;
            letter-spacing: 1px;
        }

        QPushButton[calcKey="digit"],
        QPushButton[calcKey="action"],
        QPushButton[calcKey="operator"],
        QPushButton[calcKey="equals"] {
            border-radius: 14px;
            font-size: 18px;
            font-weight: 600;
            padding: 0;
        }

        QPushButton[calcKey="digit"] {
            background-color: %11;
            border: 1px solid %7;
        }

        QPushButton[calcKey="action"] {
            background-color: transparent;
            border: 1px solid %7;
            color: %6;
        }

        QPushButton[calcKey="operator"] {
            background-color: %10;
            border: 1px solid transparent;
            color: %6;
            font-size: 20px;
        }

        QPushButton[calcKey="equals"] {
            background-color: %6;
            border: 1px solid transparent;
            color: %13;
            font-size: 22px;
        }

        QPushButton[calcKey="func"] {
            background-color: transparent;
            border: 1px solid %7;
            border-radius: 10px;
            color: %8;
            font-size: 13px;
            font-weight: 600;
            padding: 0;
        }

        QPushButton[calcKey="chip"] {
            background-color: transparent;
            border: 1px solid %7;
            border-radius: 12px;
            color: %8;
            font-size: 11px;
            font-weight: 600;
            padding: 5px 12px;
        }

        QPushButton[calcKey="chip"]:checked {
            background-color: %10;
            color: %6;
        }

        QPushButton[calcKey]:hover {
            background-color: %9;
            border-color: %6;
        }

        QPushButton[calcKey="equals"]:hover {
            background-color: %6;
        }

        QPushButton[calcKey]:pressed {
            background-color: %6;
            color: %13;
        }

        /* TOOLTIP */

        QToolTip {
            background-color: %5;
            color: %1;
            border: 1px solid %7;
            padding: 5px;
        }
    )")
        .arg(text)           // %1
        .arg(rootBackground) // %2
        .arg(sidebar)        // %3
        .arg(surface)        // %4
        .arg(popup)          // %5
        .arg(accent)         // %6
        .arg(border)         // %7
        .arg(muted)          // %8
        .arg(hover)          // %9
        .arg(active)         // %10
        .arg(input)          // %11
        .arg(background)     // %12
        .arg(onAccent);      // %13
}