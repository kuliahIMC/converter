#include "calendar_page.h"

#include "../services/calendar_converter_service.h"

#include <QComboBox>
#include <QFrame>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QScrollArea>
#include <QSpinBox>
#include <QVBoxLayout>
#include <QClipboard>
#include <QApplication>

namespace {

QString civilMonthName(int month)
{
    static const QStringList months = {
        "Januari", "Februari", "Maret", "April",
        "Mei", "Juni", "Juli", "Agustus",
        "September", "Oktober", "November", "Desember"
    };

    if (month < 1 || month > 12)
        return {};

    return months[month - 1];
}

QString formatHijriDate(const HijriDate &date)
{
    return QString("%1 %2 %3 H")
        .arg(date.day)
        .arg(TabularHijriCalendar::monthName(date.month))
        .arg(date.year);
}

QString formatCivilDate(const CivilDate &date)
{
    return QString("%1 %2 %3 M")
        .arg(date.day)
        .arg(civilMonthName(date.month))
        .arg(date.year);
}

} // namespace

CalendarPage::CalendarPage(QWidget *parent)
    : QWidget(parent)
{
    setupUI();
    updateMode();
}

void CalendarPage::setupUI()
{
    // ===== STYLE HALAMAN (gaya Notion) =====
    // Alur dibuat jelas: 1) pilih arah, 2) isi tanggal, 3) lihat hasil.
    // Kartu input putih, kartu hasil navy supaya hasil langsung menonjol.
    setStyleSheet(R"QSS(
        /* Judul halaman: warna mengikuti tema (tidak dipaksa gelap) */
        QLabel#calendarHeading {
            font-family: "Inter", "Segoe UI", sans-serif;
            font-size: 26px;
            font-weight: 700;
        }

        /* Deskripsi: warna mengikuti tema supaya terbaca di atas foto */
        QLabel#calendarDescription {
            font-size: 14px;
        }

        /* Area scroll transparan */
        QScrollArea {
            background: transparent;
            border: none;
        }
        QScrollArea > QWidget > QWidget {
            background: transparent;
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

        /* ---------- KARTU INPUT (putih) ---------- */
        QFrame#calendarCard {
            background: rgba(255, 255, 255, 0.94);
            border: 1px solid #e5e3df;
            border-radius: 18px;
        }

        QFrame#calendarCard QLabel {
            background: transparent;
            font-family: "Inter", "Segoe UI", sans-serif;
        }

        /* Nomor langkah (1, 2) */
        QFrame#calendarCard QLabel#calendarStepBadge {
            background: #5645d4;
            color: #ffffff;
            border-radius: 12px;
            font-size: 12px;
            font-weight: 600;
        }

        QFrame#calendarCard QLabel#calendarFieldLabel {
            color: #37352f;
            font-size: 13px;
            font-weight: 600;
        }

        QSpinBox, QComboBox {
            background: #f6f5f4;
            color: #1a1a1a;
            border: 1px solid #e5e3df;
            border-radius: 9px;
            padding: 9px;
            min-height: 25px;
            font-size: 14px;
        }

        QSpinBox:hover, QComboBox:hover {
            border-color: #c8c4be;
        }

        QSpinBox:focus, QComboBox:focus {
            border: 1px solid #5645d4;
            background: #ffffff;
        }

        QComboBox QAbstractItemView {
            background: #ffffff;
            color: #1a1a1a;
            border: 1px solid #e5e3df;
            selection-background-color: #e6e0f5;
            selection-color: #391c57;
            outline: none;
        }

        /* Tombol arah konversi (pill, aktif = hitam) */
        QPushButton#calendarModeButton {
            background: #f6f5f4;
            color: #37352f;
            border: 1px solid #e5e3df;
            border-radius: 21px;
            padding: 8px 14px;
            font-size: 13px;
            font-weight: 500;
        }

        QPushButton#calendarModeButton:hover {
            background: #ffffff;
            border-color: #c8c4be;
        }

        QPushButton#calendarModeButton:checked {
            background: #000000;
            color: #ffffff;
            border-color: #000000;
        }

        /* Tombol konversi (ungu utama) */
        QPushButton#calendarConvertButton {
            background: #5645d4;
            color: #ffffff;
            border: none;
            border-radius: 10px;
            padding: 12px;
            font-size: 14px;
            font-weight: 600;
        }

        QPushButton#calendarConvertButton:hover {
            background: #4534b3;
        }

        QPushButton#calendarConvertButton:pressed {
            background: #3a2a99;
        }

        /* Pesan status (warna diatur dari kode: merah/hijau) */
        QLabel#calendarStatus {
            color: #b42318;
            font-size: 13px;
            font-weight: 500;
        }

        /* ---------- KARTU HASIL (navy) ---------- */
        QFrame#calendarResultCard {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
                                        stop:0 #0a1530, stop:1 #1a2a52);
            border: none;
            border-radius: 18px;
        }

        QFrame#calendarResultCard QLabel {
            background: transparent;
            font-family: "Inter", "Segoe UI", sans-serif;
        }

        /* Nomor langkah (3) */
        QFrame#calendarResultCard QLabel#calendarStepBadgeLight {
            background: #ffffff;
            color: #0a1530;
            border-radius: 12px;
            font-size: 12px;
            font-weight: 600;
        }

        QFrame#calendarResultCard QLabel#calendarResultTitle {
            color: #c9c6bf;
            font-size: 13px;
            font-weight: 600;
        }

        QFrame#calendarResultCard QLabel#calendarResultDate {
            color: #ffffff;
            font-size: 28px;
            font-weight: 600;
        }

        QFrame#calendarResultCard QLabel#calendarResultInfo {
            color: #c9c6bf;
            font-size: 13px;
        }

        QPushButton#calendarCopyButton {
            background: #e6e0f5;
            color: #391c57;
            border: none;
            border-radius: 9px;
            padding: 9px 16px;
            font-size: 13px;
            font-weight: 500;
        }

        QPushButton#calendarCopyButton:hover {
            background: #d6b6f6;
        }

        QPushButton#calendarCopyButton:pressed {
            background: #c4a0ee;
        }
    )QSS");

    // Lingkaran nomor langkah (tampilan saja)
    auto makeBadge = [](const QString &number,
                        const QString &objectName) {
        auto *badge = new QLabel(number);
        badge->setObjectName(objectName);
        badge->setAlignment(Qt::AlignCenter);
        badge->setFixedSize(24, 24);
        return badge;
    };

    auto *pageLayout = new QVBoxLayout(this);
    pageLayout->setContentsMargins(0, 0, 0, 0);

    // Dibungkus scroll area: kalau jendela kurang tinggi, halaman bisa
    // di-scroll dan isinya tidak saling menumpuk
    auto *scroll = new QScrollArea;
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scroll->viewport()->setAutoFillBackground(false);

    auto *inner = new QWidget;
    inner->setAutoFillBackground(false);

    scroll->setWidget(inner);
    pageLayout->addWidget(scroll);

    auto *outer = new QVBoxLayout(inner);
    outer->setContentsMargins(0, 0, 8, 20);
    outer->setSpacing(0);

    // Satu kolom di tengah halaman: judul, kartu input, dan kartu hasil
    // sama lebarnya dan rata kiri satu garis
    auto *column = new QWidget;
    column->setMinimumWidth(460);
    column->setMaximumWidth(650);
    column->setSizePolicy(
        QSizePolicy::Expanding,
        QSizePolicy::Preferred
    );

    auto *columnLayout = new QVBoxLayout(column);
    columnLayout->setContentsMargins(0, 0, 0, 0);
    columnLayout->setSpacing(18);

    auto *heading = new QLabel("Kalender Converter");
    heading->setObjectName("calendarHeading");

    auto *description = new QLabel(
        "Konversi tanggal Hijriah dan Masehi secara dua arah."
    );
    description->setObjectName("calendarDescription");

    columnLayout->addWidget(heading);
    columnLayout->addWidget(description);
    columnLayout->addSpacing(4);

    // INPUT CARD
    auto *card = new QFrame;
    card->setObjectName("calendarCard");

    auto *cardLayout = new QVBoxLayout(card);
    cardLayout->setContentsMargins(24, 22, 24, 24);
    cardLayout->setSpacing(18);

    // Langkah 1: arah konversi
    auto *modeTitle = new QLabel("Pilih arah konversi");
    modeTitle->setObjectName("calendarFieldLabel");

    auto *modeRow = new QHBoxLayout;
    modeRow->setSpacing(8);
    modeRow->addWidget(makeBadge("1", "calendarStepBadge"));
    modeRow->addWidget(modeTitle);
    modeRow->addStretch();

    auto *modeLayout = new QHBoxLayout;
    modeLayout->setSpacing(8);

    hijriModeButton_ = new QPushButton("Hijriah → Masehi");
    civilModeButton_ = new QPushButton("Masehi → Hijriah");

    for (auto *button : {
             hijriModeButton_,
             civilModeButton_
         }) {
        button->setObjectName("calendarModeButton");
        button->setCheckable(true);
        button->setCursor(Qt::PointingHandCursor);
        button->setMinimumHeight(42);
    }

    hijriModeButton_->setChecked(true);

    modeLayout->addWidget(hijriModeButton_);
    modeLayout->addWidget(civilModeButton_);

    connect(hijriModeButton_, &QPushButton::clicked,
            this, [this]() {
        hijriMode_ = true;
        updateMode();
    });

    connect(civilModeButton_, &QPushButton::clicked,
            this, [this]() {
        hijriMode_ = false;
        updateMode();
    });

    cardLayout->addLayout(modeRow);
    cardLayout->addLayout(modeLayout);

    // Langkah 2: isi tanggal
    inputTitle_ = new QLabel("Tanggal Hijriah");
    inputTitle_->setObjectName("calendarFieldLabel");

    auto *inputRow = new QHBoxLayout;
    inputRow->setSpacing(8);
    inputRow->addWidget(makeBadge("2", "calendarStepBadge"));
    inputRow->addWidget(inputTitle_);
    inputRow->addStretch();

    auto *inputGrid = new QGridLayout;
    inputGrid->setHorizontalSpacing(10);
    inputGrid->setVerticalSpacing(6);

    // kolom Bulan lebih lebar karena nama bulan lebih panjang
    inputGrid->setColumnStretch(0, 2);
    inputGrid->setColumnStretch(1, 3);
    inputGrid->setColumnStretch(2, 3);

    auto *dayLabel = new QLabel("Hari");
    auto *monthLabel = new QLabel("Bulan");
    auto *yearLabel = new QLabel("Tahun");

    dayInput_ = new QSpinBox;
    dayInput_->setRange(1, 31);
    dayInput_->setValue(12);

    monthInput_ = new QComboBox;

    yearInput_ = new QSpinBox;
    yearInput_->setRange(1, 2147483647);
    yearInput_->setValue(11);

    // tinggi tetap supaya kolom input tidak terjepit atau terpotong
    dayInput_->setMinimumHeight(46);
    monthInput_->setMinimumHeight(46);
    yearInput_->setMinimumHeight(46);

    dayInput_->setObjectName("calendarDayInput");
    monthInput_->setObjectName("calendarMonthInput");
    yearInput_->setObjectName("calendarYearInput");

    dayLabel->setObjectName("calendarFieldLabel");
    monthLabel->setObjectName("calendarFieldLabel");
    yearLabel->setObjectName("calendarFieldLabel");

    inputGrid->addWidget(dayLabel, 0, 0);
    inputGrid->addWidget(monthLabel, 0, 1);
    inputGrid->addWidget(yearLabel, 0, 2);

    inputGrid->addWidget(dayInput_, 1, 0);
    inputGrid->addWidget(monthInput_, 1, 1);
    inputGrid->addWidget(yearInput_, 1, 2);

    cardLayout->addSpacing(8);
    cardLayout->addLayout(inputRow);
    cardLayout->addLayout(inputGrid);

    convertButton_ = new QPushButton("Konversi Tanggal");
    convertButton_->setObjectName("calendarConvertButton");
    convertButton_->setCursor(Qt::PointingHandCursor);
    convertButton_->setMinimumHeight(46);

    connect(convertButton_, &QPushButton::clicked,
            this, &CalendarPage::convertDate);

    cardLayout->addSpacing(2);
    cardLayout->addWidget(convertButton_);

    statusLabel_ = new QLabel;
    statusLabel_->setObjectName("calendarStatus");
    statusLabel_->setWordWrap(true);
    // tinggi tetap supaya kartu tidak "loncat" saat pesan muncul
    statusLabel_->setMinimumHeight(20);

    cardLayout->addWidget(statusLabel_);

    columnLayout->addWidget(card);

    // RESULT CARD (langkah 3)
    auto *resultCard = new QFrame;
    resultCard->setObjectName("calendarResultCard");

    auto *resultLayout = new QVBoxLayout(resultCard);
    resultLayout->setContentsMargins(22, 20, 22, 20);
    resultLayout->setSpacing(10);

    resultTitle_ = new QLabel("Hasil Konversi");
    resultTitle_->setObjectName("calendarResultTitle");

    auto *resultRow = new QHBoxLayout;
    resultRow->setSpacing(8);
    resultRow->addWidget(makeBadge("3", "calendarStepBadgeLight"));
    resultRow->addWidget(resultTitle_);
    resultRow->addStretch();

    resultDate_ = new QLabel("Belum ada hasil");
    resultDate_->setObjectName("calendarResultDate");
    resultDate_->setWordWrap(true);

    resultWeekday_ = new QLabel;
    resultWeekday_->setObjectName("calendarResultInfo");

    resultCalendar_ = new QLabel;
    resultCalendar_->setObjectName("calendarResultInfo");

    resultMethod_ = new QLabel(
        "Metode: Hijriah Tabular (aritmetis)"
    );
    resultMethod_->setObjectName("calendarResultInfo");

    auto *copyButton = new QPushButton("Salin Hasil");
    copyButton->setObjectName("calendarCopyButton");
    copyButton->setCursor(Qt::PointingHandCursor);
    copyButton->setMinimumHeight(36);

    connect(copyButton, &QPushButton::clicked, this, [this]() {
        QString text = resultDate_->text();

        if (text == "Belum ada hasil")
            return;

        text += "\n" + resultWeekday_->text();
        text += "\n" + resultCalendar_->text();
        text += "\n" + resultMethod_->text();

        QApplication::clipboard()->setText(text);
        statusLabel_->setStyleSheet("color: #16803c;");
        statusLabel_->setText("Hasil berhasil disalin.");
    });

    resultLayout->addLayout(resultRow);
    resultLayout->addWidget(resultDate_);
    resultLayout->addWidget(resultWeekday_);
    resultLayout->addWidget(resultCalendar_);
    resultLayout->addWidget(resultMethod_);
    resultLayout->addSpacing(4);
    resultLayout->addWidget(copyButton, 0, Qt::AlignLeft);

    columnLayout->addWidget(resultCard);

    outer->addWidget(column, 0, Qt::AlignHCenter);
    outer->addStretch();

    updateMonthList();
}

void CalendarPage::updateMode()
{
    hijriModeButton_->setChecked(hijriMode_);
    civilModeButton_->setChecked(!hijriMode_);

    inputTitle_->setText(
        hijriMode_ ? "Tanggal Hijriah" : "Tanggal Masehi"
    );

    yearInput_->setValue(hijriMode_ ? 11 : 632);

    dayInput_->setValue(12);

    updateMonthList();

    statusLabel_->clear();
    resultDate_->setText("Belum ada hasil");
    resultWeekday_->clear();
    resultCalendar_->clear();
}

void CalendarPage::updateMonthList()
{
    monthInput_->clear();

    if (hijriMode_) {
        for (int month = 1; month <= 12; ++month) {
            monthInput_->addItem(
                TabularHijriCalendar::monthName(month),
                month
            );
        }

        monthInput_->setCurrentIndex(2);
    } else {
        for (int month = 1; month <= 12; ++month) {
            monthInput_->addItem(
                civilMonthName(month),
                month
            );
        }

        monthInput_->setCurrentIndex(0);
    }
}

void CalendarPage::setError(const QString &message)
{
    statusLabel_->setStyleSheet("color: #b42318;");
    statusLabel_->setText(message);

    resultDate_->setText("Konversi gagal");
    resultWeekday_->clear();
    resultCalendar_->clear();
}

void CalendarPage::convertDate()
{
    statusLabel_->clear();

    const int day = dayInput_->value();
    const int month = monthInput_->currentData().toInt();
    const qint64 year = yearInput_->value();

    CalendarConversionResult result;

    if (hijriMode_) {
        result = CalendarConverterService::hijriToCivil(
            {day, month, year}
        );
    } else {
        result = CalendarConverterService::civilToHijri(
            {day, month, year}
        );
    }

    if (!result.success) {
        setError(result.errorMessage);
        return;
    }

    resultDate_->setText(
        hijriMode_
            ? formatCivilDate(result.civilDate)
            : formatHijriDate(result.hijriDate)
    );

    resultWeekday_->setText(
        "Hari: " + result.weekdayName
    );

    resultCalendar_->setText(
        "Kalender: " + result.calendarName
    );

    resultMethod_->setText(
        "Metode: Hijriah Tabular (aritmetis)"
    );

    statusLabel_->setStyleSheet("color: #16803c;");
    statusLabel_->setText("Konversi berhasil.");
}