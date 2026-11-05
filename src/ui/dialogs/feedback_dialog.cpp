#include "feedback_dialog.h"

#include "../../themes/theme_manager.h"
#include <QDebug>

#include <QDialog>
#include <QLabel>
#include <QPixmap>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QScrollArea>
#include <QFrame>
#include <QFileInfo>
#include <QApplication>

void FeedbackDialog::showResult(
    QWidget *parent,
    ThemeManager *themeManager,
    int successCount,
    int failedCount,
    int skippedCount,
    const QStringList &errors,
    bool cancelled
) {
    QDialog dialog(parent);
    dialog.setWindowTitle("Hasil Pemrosesan");
    dialog.setMinimumWidth(440);
    dialog.setMaximumWidth(520);
    dialog.setModal(true);

    const QString theme = themeManager
        ? themeManager->currentTheme()
        : "Dark";

    QString stickerPath;

    if (theme == "Light")
        stickerPath = ":/assets/light-stk.png";
    else if (theme == "Dark")
        stickerPath = ":/assets/dark-stk.png";
    else if (theme == "Bochi")
        stickerPath = ":/assets/bochi-stk.png";
    else if (theme == "Frieren")
        stickerPath = ":/assets/friren-stk.png";
    else if (theme == "Kurumi")
        stickerPath = ":/assets/kurumi-stk.png";
    else if (theme == "Mao Mao")
        stickerPath = ":/assets/mao-mao-stk.png";
    else if (theme == "Ram & Rem")
        stickerPath = ":/assets/ram-rem-stk.png";
    if (stickerPath.isEmpty()) {
        stickerPath = ":/assets/dark-stk.png";
    }

    const int total = successCount + failedCount + skippedCount;

    QString title;
    QString description;

    if (cancelled) {
        title = "Proses Dihentikan";
        description = "Pemrosesan dibatalkan sebelum semua file selesai.";
    }
    else if (failedCount == 0) {
        title = "Semua Berhasil!";
        description = "Seluruh file berhasil diproses.";
    }
    else if (successCount == 0) {
        title = "Belum Berhasil";
        description = "Tidak ada file yang berhasil diproses.";
    }
    else {
        title = "Proses Selesai";
        description = "Sebagian file berhasil, tetapi ada yang gagal.";
    }

    auto *layout = new QVBoxLayout(&dialog);
    layout->setContentsMargins(26, 24, 26, 22);
    layout->setSpacing(12);

    auto *sticker = new QLabel;
    sticker->setAlignment(Qt::AlignCenter);
    sticker->setMinimumHeight(150);

    QPixmap pixmap(stickerPath);

    if (pixmap.isNull()) {
        qWarning() << "[FeedbackDialog] Gagal memuat stiker:"
                   << stickerPath;

        pixmap.load(":/assets/dark-stk.png");
    }

    if (pixmap.isNull()) {
        qWarning() << "[FeedbackDialog] Stiker fallback juga gagal dimuat.";
    } else {
        qDebug() << "[FeedbackDialog] Stiker berhasil dimuat:"
                 << stickerPath
                 << pixmap.size();
    }

    if (!pixmap.isNull()) {
        sticker->setPixmap(
            pixmap.scaled(
                170,
                170,
                Qt::KeepAspectRatio,
                Qt::SmoothTransformation
            )
        );
    }

    layout->addWidget(sticker);

    auto *heading = new QLabel(title);
    heading->setAlignment(Qt::AlignCenter);
    heading->setStyleSheet(
        "font-size:22px;font-weight:700;"
    );

    layout->addWidget(heading);

    auto *subtitle = new QLabel(description);
    subtitle->setAlignment(Qt::AlignCenter);
    subtitle->setWordWrap(true);
    subtitle->setStyleSheet(
        "font-size:13px;color:#888;"
    );

    layout->addWidget(subtitle);

    auto *separator = new QFrame;
    separator->setFrameShape(QFrame::HLine);
    layout->addWidget(separator);

    auto *stats = new QHBoxLayout;
    stats->setSpacing(8);

    auto createStat = [](const QString &label, int count) {
        auto *container = new QFrame;
        container->setStyleSheet(
            "QFrame {"
            "background:rgba(128,128,128,0.10);"
            "border-radius:10px;"
            "}"
        );

        auto *box = new QVBoxLayout(container);

        auto *number = new QLabel(QString::number(count));
        number->setAlignment(Qt::AlignCenter);
        number->setStyleSheet(
            "font-size:24px;font-weight:700;"
        );

        auto *text = new QLabel(label);
        text->setAlignment(Qt::AlignCenter);
        text->setStyleSheet("font-size:12px;");

        box->addWidget(number);
        box->addWidget(text);

        return container;
    };

    stats->addWidget(createStat("Berhasil", successCount));
    stats->addWidget(createStat("Gagal", failedCount));
    stats->addWidget(createStat("Dilewati", skippedCount));

    layout->addLayout(stats);

    if (!errors.isEmpty()) {
        auto *errorTitle = new QLabel("Detail Kegagalan");
        errorTitle->setStyleSheet(
            "font-size:13px;font-weight:700;"
        );

        layout->addWidget(errorTitle);

        auto *errorList = new QLabel;
        errorList->setWordWrap(true);
        errorList->setTextFormat(Qt::PlainText);

        QString errorText;

        for (const QString &error : errors) {
            errorText += "• " + error + "\n";
        }

        errorList->setText(errorText.trimmed());
        errorList->setStyleSheet(
            "font-size:12px;padding:10px;"
            "background:rgba(220,38,38,0.08);"
            "border-radius:8px;"
        );

        auto *scroll = new QScrollArea;
        scroll->setWidget(errorList);
        scroll->setWidgetResizable(true);
        scroll->setMaximumHeight(140);
        scroll->setFrameShape(QFrame::NoFrame);

        layout->addWidget(scroll);
    }

    auto *footer = new QLabel(
        QString("Total file: %1").arg(total)
    );

    footer->setAlignment(Qt::AlignCenter);
    footer->setStyleSheet("font-size:11px;color:#888;");

    layout->addWidget(footer);

    auto *closeButton = new QPushButton("Selesai");
    closeButton->setCursor(Qt::PointingHandCursor);
    closeButton->setMinimumHeight(38);
    closeButton->setStyleSheet(
        "QPushButton {"
        "background:#5645d4;"
        "color:white;"
        "border:none;"
        "border-radius:8px;"
        "padding:8px 20px;"
        "font-weight:600;"
        "}"
        "QPushButton:hover {background:#4534b3;}"
    );

    QObject::connect(
        closeButton,
        &QPushButton::clicked,
        &dialog,
        &QDialog::accept
    );

    layout->addWidget(closeButton);

    dialog.exec();
}