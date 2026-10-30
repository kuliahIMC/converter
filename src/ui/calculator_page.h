#pragma once

#include <QWidget>
#include <QString>

class QLineEdit;
class QLabel;
class QPushButton;

class CalculatorPage : public QWidget
{
public:
    explicit CalculatorPage(QWidget *parent = nullptr);

private:
    QLineEdit *display_ = nullptr;
    QLabel *statusLabel_ = nullptr;

    bool degreeMode_ = true;
    double lastAnswer_ = 0.0;

    void setupUI();

    QPushButton *createButton(
        const QString &text,
        int row,
        int column
    );

    void handleButton(const QString &text);
    void calculate();
};