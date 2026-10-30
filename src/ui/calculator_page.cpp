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

// Jenis tombol, dipakai ThemeManager lewat selector [calcKey="..."]
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
    cardLayout->setContentsMargins(20, 14, 20, 18);
    cardLayout->setSpacing(8);

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
    display_->setMinimumHeight(52);
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