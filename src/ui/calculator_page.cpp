#include "calculator_page.h"

#include <QFrame>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QScrollArea>
#include <QVBoxLayout>

#include <cmath>
#include <stdexcept>

namespace {

constexpr double PI = 3.14159265358979323846;
constexpr double EULER = 2.71828182845904523536;

class ExpressionParser
{
public:
    ExpressionParser(const QString &input, bool degrees, double answer)
        : input_(input),
          degrees_(degrees),
          answer_(answer)
    {
    }

    double parse()
    {
        position_ = 0;

        double result = expression();
        skipSpaces();

        if (position_ < input_.size())
            throw std::runtime_error("Ekspresi tidak valid");

        return result;
    }

private:
    QString input_;
    int position_ = 0;
    bool degrees_;
    double answer_;

    void skipSpaces()
    {
        while (position_ < input_.size() &&
               input_[position_].isSpace()) {
            ++position_;
        }
    }

    bool match(QChar character)
    {
        skipSpaces();

        if (position_ < input_.size() &&
            input_[position_] == character) {
            ++position_;
            return true;
        }

        return false;
    }

    double expression()
    {
        double value = term();

        while (true) {
            if (match('+'))
                value += term();
            else if (match('-'))
                value -= term();
            else
                break;
        }

        return value;
    }

    double term()
    {
        double value = unary();

        while (true) {
            if (match('*')) {
                value *= unary();
            }
            else if (match('/')) {
                double divisor = unary();

                if (divisor == 0)
                    throw std::runtime_error("Tidak bisa dibagi nol");

                value /= divisor;
            }
            else {
                break;
            }
        }

        return value;
    }

    double unary()
    {
        if (match('+'))
            return unary();

        if (match('-'))
            return -unary();

        return power();
    }

    double power()
    {
        double value = postfix();

        if (match('^'))
            value = std::pow(value, unary());

        return value;
    }

    double postfix()
    {
        double value = primary();

        while (true) {
            if (match('!')) {
                if (value < 0 ||
                    value > 170 ||
                    std::floor(value) != value) {
                    throw std::runtime_error("Faktorial tidak valid");
                }

                double result = 1.0;

                for (int i = 2; i <= static_cast<int>(value); ++i)
                    result *= i;

                value = result;
            }
            else if (match('%')) {
                value /= 100.0;
            }
            else {
                break;
            }
        }

        return value;
    }

    double primary()
    {
        skipSpaces();

        if (match('(')) {
            double value = expression();

            if (!match(')'))
                throw std::runtime_error("Kurung tidak lengkap");

            return value;
        }

        if (position_ >= input_.size())
            throw std::runtime_error("Ekspresi kosong");

        if (input_[position_].isDigit() ||
            input_[position_] == '.') {

            int start = position_;

            while (position_ < input_.size() &&
                   (input_[position_].isDigit() ||
                    input_[position_] == '.')) {
                ++position_;
            }

            if (position_ < input_.size() &&
                (input_[position_] == 'e' ||
                 input_[position_] == 'E')) {

                ++position_;

                if (position_ < input_.size() &&
                    (input_[position_] == '+' ||
                     input_[position_] == '-')) {
                    ++position_;
                }

                while (position_ < input_.size() &&
                       input_[position_].isDigit()) {
                    ++position_;
                }
            }

            bool ok = false;

            double value = input_.mid(
                start,
                position_ - start
            ).toDouble(&ok);

            if (!ok)
                throw std::runtime_error("Angka tidak valid");

            return value;
        }

        if (input_[position_].isLetter()) {
            int start = position_;

            while (position_ < input_.size() &&
                   input_[position_].isLetter()) {
                ++position_;
            }

            QString name = input_.mid(
                start,
                position_ - start
            ).toLower();

            if (name == "pi")
                return PI;

            if (name == "e")
                return EULER;

            if (name == "ans")
                return answer_;

            if (!match('('))
                throw std::runtime_error("Format fungsi tidak valid");

            double value = expression();

            if (!match(')'))
                throw std::runtime_error("Kurung fungsi tidak lengkap");

            double angle = degrees_
                ? value * PI / 180.0
                : value;

            if (name == "sin") return std::sin(angle);
            if (name == "cos") return std::cos(angle);
            if (name == "tan") return std::tan(angle);

            if (name == "asin") {
                double result = std::asin(value);
                return degrees_ ? result * 180.0 / PI : result;
            }

            if (name == "acos") {
                double result = std::acos(value);
                return degrees_ ? result * 180.0 / PI : result;
            }

            if (name == "atan") {
                double result = std::atan(value);
                return degrees_ ? result * 180.0 / PI : result;
            }

            if (name == "sqrt") return std::sqrt(value);
            if (name == "log") return std::log10(value);
            if (name == "ln") return std::log(value);
            if (name == "exp") return std::exp(value);
            if (name == "abs") return std::abs(value);

            throw std::runtime_error("Fungsi tidak dikenal");
        }

        throw std::runtime_error("Ekspresi tidak valid");
    }
};

// Jenis tombol, dipakai stylesheet lewat selector [calcKey="..."]
QString keyKind(const QString &t)
{
    static const QStringList digits = {
        "0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "."
    };

    if (digits.contains(t))
        return "digit";

    if (t == "/" || t == "*" || t == "-" || t == "+")
        return "operator";

    if (t == "=")
        return "equals";

    if (t == "C" || t == "DEL" || t == "+/-" || t == "%")
        return "action";

    return "func";
}

// Teks yang tampil di tombol (yang dimasukkan ke layar tetap teks asli)
QString keyLabel(const QString &t)
{
    if (t == "/")    return QString::fromUtf8("\xC3\xB7");      // ÷
    if (t == "*")    return QString::fromUtf8("\xC3\x97");      // ×
    if (t == "-")    return QString::fromUtf8("\xE2\x88\x92");  // −
    if (t == "sqrt") return QString::fromUtf8("\xE2\x88\x9A");  // √
    if (t == "pi")   return QString::fromUtf8("\xCF\x80");      // π
    if (t == "+/-")  return QString::fromUtf8("\xC2\xB1");      // ±
    if (t == "DEL")  return QString::fromUtf8("\xE2\x8C\xAB");  // ⌫
    return t;
}

} // namespace

CalculatorPage::CalculatorPage(QWidget *parent)
    : QWidget(parent)
{
    setupUI();
}

void CalculatorPage::setupUI()
{
    // ===== STYLE HALAMAN (gaya Notion) =====
    // Dipasang di halaman ini saja, memakai objectName dan properti
    // calcKey yang sudah ada. Kartu punya latar sendiri sehingga tetap
    // terbaca di semua tema, termasuk tema berfoto.
    setStyleSheet(R"QSS(
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

        /* Kartu kalkulator */
        QFrame#calcCard {
            background: rgba(255, 255, 255, 0.92);
            border: 1px solid #e5e3df;
            border-radius: 20px;
        }
        QFrame#calcCard QLabel {
            background: transparent;
            font-family: "Inter", "Segoe UI", sans-serif;
        }

        /* Chip mode sudut (DEG / RAD) */
        QLabel#calcModeChip {
            background: #e6e0f5;
            color: #391c57;
            border-radius: 10px;
            padding: 3px 12px;
            font-size: 12px;
            font-weight: 600;
        }

        /* Riwayat / pesan error */
        QLabel#calcHistory {
            color: #787671;
            font-size: 13px;
        }

        /* Layar navy */
        QLineEdit#calcDisplay {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
                                        stop:0 #0a1530, stop:1 #1a2a52);
            color: #ffffff;
            border: none;
            border-radius: 14px;
            padding: 6px 18px;
            font-family: "Inter", "Segoe UI", sans-serif;
            font-size: 30px;
            font-weight: 600;
            selection-background-color: #5645d4;
            selection-color: #ffffff;
            placeholder-text-color: #a4a097;
        }

        /* Tombol chip (Ilmiah, DEG / RAD) */
        QPushButton[calcKey="chip"] {
            background: #ffffff;
            color: #37352f;
            border: 1px solid #c8c4be;
            border-radius: 14px;
            padding: 5px 14px;
            font-family: "Inter", "Segoe UI", sans-serif;
            font-size: 12px;
            font-weight: 500;
        }
        QPushButton[calcKey="chip"]:hover {
            border-color: #787671;
        }
        QPushButton[calcKey="chip"]:checked {
            background: #000000;
            color: #ffffff;
            border-color: #000000;
        }

        /* Tombol angka */
        QPushButton[calcKey="digit"] {
            background: #f6f5f4;
            color: #1a1a1a;
            border: 1px solid #e5e3df;
            border-radius: 12px;
            font-family: "Inter", "Segoe UI", sans-serif;
            font-size: 18px;
            font-weight: 500;
        }
        QPushButton[calcKey="digit"]:hover   { background: #ffffff; border-color: #c8c4be; }
        QPushButton[calcKey="digit"]:pressed { background: #ede9e4; }

        /* Tombol operator (lavender) */
        QPushButton[calcKey="operator"] {
            background: #e6e0f5;
            color: #391c57;
            border: none;
            border-radius: 12px;
            font-family: "Inter", "Segoe UI", sans-serif;
            font-size: 20px;
            font-weight: 600;
        }
        QPushButton[calcKey="operator"]:hover   { background: #d6b6f6; }
        QPushButton[calcKey="operator"]:pressed { background: #c4a0ee; }

        /* Tombol sama dengan (ungu utama) */
        QPushButton[calcKey="equals"] {
            background: #5645d4;
            color: #ffffff;
            border: none;
            border-radius: 12px;
            font-family: "Inter", "Segoe UI", sans-serif;
            font-size: 22px;
            font-weight: 600;
        }
        QPushButton[calcKey="equals"]:hover   { background: #4534b3; }
        QPushButton[calcKey="equals"]:pressed { background: #3a2a99; }

        /* Tombol aksi: C, DEL, ±, % (peach) */
        QPushButton[calcKey="action"] {
            background: #ffe8d4;
            color: #793400;
            border: none;
            border-radius: 12px;
            font-family: "Inter", "Segoe UI", sans-serif;
            font-size: 16px;
            font-weight: 600;
        }
        QPushButton[calcKey="action"]:hover   { background: #ffdcbf; }
        QPushButton[calcKey="action"]:pressed { background: #ffcfa8; }

        /* Tombol fungsi ilmiah (sky) */
        QPushButton[calcKey="func"] {
            background: #dcecfa;
            color: #1a2a52;
            border: none;
            border-radius: 10px;
            font-family: "Inter", "Segoe UI", sans-serif;
            font-size: 13px;
            font-weight: 500;
        }
        QPushButton[calcKey="func"]:hover   { background: #c9e0f5; }
        QPushButton[calcKey="func"]:pressed { background: #b5d3ef; }
    )QSS");

    // Bungkus dengan scroll area: kalau jendela kurang tinggi,
    // kalkulator bisa di-scroll dan tombolnya tidak saling menumpuk
    auto *outer = new QVBoxLayout(this);
    outer->setContentsMargins(0, 0, 0, 0);

    auto *scroll = new QScrollArea;
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scroll->viewport()->setAutoFillBackground(false);

    auto *inner = new QWidget;
    inner->setAutoFillBackground(false);

    scroll->setWidget(inner);
    outer->addWidget(scroll);

    auto *mainLayout = new QVBoxLayout(inner);
    mainLayout->setContentsMargins(0, 0, 8, 0);
    mainLayout->setSpacing(10);

    // ---------- KARTU KALKULATOR ----------
    auto *card = new QFrame;
    card->setObjectName("calcCard");
    card->setMinimumWidth(440);
    card->setMaximumWidth(600);

    auto *cardLayout = new QVBoxLayout(card);
    cardLayout->setContentsMargins(20, 16, 20, 20);
    cardLayout->setSpacing(10);

    // Baris atas: mode sudut + tombol chip
    auto *modeLayout = new QHBoxLayout;
    modeLayout->setSpacing(8);

    statusLabel_ = new QLabel("DEG");
    statusLabel_->setObjectName("calcModeChip");

    auto *sciToggle = new QPushButton("Ilmiah");
    sciToggle->setProperty("calcKey", "chip");
    sciToggle->setCheckable(true);
    sciToggle->setChecked(true);
    sciToggle->setCursor(Qt::PointingHandCursor);
    sciToggle->setFocusPolicy(Qt::NoFocus);

    auto *angleButton = new QPushButton("DEG / RAD");
    angleButton->setProperty("calcKey", "chip");
    angleButton->setCursor(Qt::PointingHandCursor);
    angleButton->setFocusPolicy(Qt::NoFocus);

    connect(angleButton, &QPushButton::clicked, this, [this]() {
        degreeMode_ = !degreeMode_;
        statusLabel_->setText(degreeMode_ ? "DEG" : "RAD");
    });

    modeLayout->addWidget(statusLabel_);
    modeLayout->addStretch();
    modeLayout->addWidget(sciToggle);
    modeLayout->addWidget(angleButton);

    cardLayout->addLayout(modeLayout);

    // Layar: riwayat kecil + input besar
    auto *history = new QLabel;
    history->setObjectName("calcHistory");
    history->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    history->setMinimumHeight(18);

    display_ = new QLineEdit;
    display_->setObjectName("calcDisplay");
    display_->setPlaceholderText("0");
    display_->setMinimumHeight(64);
    display_->setAlignment(Qt::AlignRight);
    display_->setClearButtonEnabled(false);

    connect(
        display_,
        &QLineEdit::returnPressed,
        this,
        [this]() {
            calculate();
        }
    );

    cardLayout->addWidget(history);
    cardLayout->addWidget(display_);

    // Panel ilmiah
    auto *sciPanel = new QWidget;
    auto *sciGrid = new QGridLayout(sciPanel);
    sciGrid->setContentsMargins(0, 2, 0, 2);
    sciGrid->setSpacing(6);

    const QStringList sciButtons = {
        "sin", "cos", "tan", "asin", "acos", "atan",
        "sqrt", "log", "ln", "exp", "pi", "e",
        "(", ")", "!", "^", "Ans", "abs"
    };

    constexpr int sciColumns = 6;

    for (int i = 0; i < sciButtons.size(); ++i) {
        const int row = i / sciColumns;
        const int column = i % sciColumns;

        sciGrid->addWidget(
            createButton(sciButtons[i], row, column),
            row,
            column
        );
    }

    cardLayout->addWidget(sciPanel);

    connect(sciToggle, &QPushButton::toggled, sciPanel, &QWidget::setVisible);

    // Keypad utama 4 kolom
    auto *padGrid = new QGridLayout;
    padGrid->setSpacing(8);

    const QStringList padButtons = {
        "C",   "DEL", "%", "/",
        "7",   "8",   "9", "*",
        "4",   "5",   "6", "-",
        "1",   "2",   "3", "+",
        "+/-", "0",   ".", "="
    };

    constexpr int padColumns = 4;

    for (int i = 0; i < padButtons.size(); ++i) {
        const int row = i / padColumns;
        const int column = i % padColumns;

        padGrid->addWidget(
            createButton(padButtons[i], row, column),
            row,
            column
        );
    }

    cardLayout->addLayout(padGrid);

    // Kartu di tengah halaman
    mainLayout->addWidget(card, 0, Qt::AlignHCenter);
    mainLayout->addStretch();

    display_->setFocus();
}

QPushButton *CalculatorPage::createButton(
    const QString &text,
    int row,
    int column
)
{
    Q_UNUSED(row);
    Q_UNUSED(column);

    const QString kind = keyKind(text);

    auto *button = new QPushButton(keyLabel(text));

    button->setProperty("calcKey", kind);
    button->setMinimumHeight(kind == "func" ? 34 : 46);
    button->setCursor(Qt::PointingHandCursor);
    button->setFocusPolicy(Qt::NoFocus);

    connect(
        button,
        &QPushButton::clicked,
        this,
        [this, text]() {
            handleButton(text);
        }
    );

    return button;
}

void CalculatorPage::handleButton(const QString &text)
{
    // fokus balik ke layar supaya keyboard tetap bisa dipakai
    display_->setFocus();

    if (text == "C") {
        display_->clear();

        if (auto *history = findChild<QLabel *>("calcHistory"))
            history->clear();

        return;
    }

    if (text == "DEL") {
        QString value = display_->text();
        value.chop(1);
        display_->setText(value);
        return;
    }

    if (text == "=") {
        calculate();
        return;
    }

    if (text == "+/-") {
        QString value = display_->text();

        if (value.startsWith("-(") && value.endsWith(")")) {
            value = value.mid(2, value.size() - 3);
        } else {
            value = "-(" + value + ")";
        }

        display_->setText(value);
        return;
    }

    QString value = text;

    if (text == "Ans")
        value = "ans";
    else if (text == "sqrt" ||
             text == "sin" ||
             text == "cos" ||
             text == "tan" ||
             text == "asin" ||
             text == "acos" ||
             text == "atan" ||
             text == "log" ||
             text == "ln" ||
             text == "exp" ||
             text == "abs") {
        value += "(";
    }

    display_->insert(value);
}

void CalculatorPage::calculate()
{
    auto *history = findChild<QLabel *>("calcHistory");
    const QString expressionText = display_->text();

    try {
        ExpressionParser parser(
            expressionText,
            degreeMode_,
            lastAnswer_
        );

        double result = parser.parse();

        if (!std::isfinite(result))
            throw std::runtime_error("Hasil tidak terdefinisi");

        lastAnswer_ = result;

        if (history)
            history->setText(expressionText + " =");

        display_->setText(
            QString::number(result, 'g', 14)
        );
    }
    catch (const std::exception &error) {
        // error tampil di baris riwayat, ketikan tetap utuh
        if (history)
            history->setText(error.what());
    }
}