#pragma once

#include <QWidget>

class QSpinBox;
class QComboBox;
class QLabel;
class QPushButton;

class CalendarPage : public QWidget
{
public:
    explicit CalendarPage(QWidget *parent = nullptr);

private:
    QSpinBox *dayInput_ = nullptr;
    QSpinBox *yearInput_ = nullptr;
    QComboBox *monthInput_ = nullptr;

    QLabel *inputTitle_ = nullptr;
    QLabel *resultTitle_ = nullptr;
    QLabel *resultDate_ = nullptr;
    QLabel *resultWeekday_ = nullptr;
    QLabel *resultCalendar_ = nullptr;
    QLabel *resultMethod_ = nullptr;
    QLabel *statusLabel_ = nullptr;

    QPushButton *hijriModeButton_ = nullptr;
    QPushButton *civilModeButton_ = nullptr;
    QPushButton *convertButton_ = nullptr;

    bool hijriMode_ = true;

    void setupUI();
    void updateMode();
    void updateMonthList();
    void convertDate();
    void setError(const QString &message);
};