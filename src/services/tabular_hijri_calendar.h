#pragma once

#include <QString>
#include <QtGlobal>

struct HijriDate {
    int day = 1;
    int month = 1;
    qint64 year = 1;
};

class TabularHijriCalendar {
public:
    // Epoch: 1 Muharram 1 H = 16 Juli 622 Julian.
    static constexpr qint64 EPOCH_JDN = 1948440;

    static bool isLeapYear(qint64 year);

    static int daysInMonth(int month, qint64 year);

    static bool isValidDate(const HijriDate &date);

    static bool toJulianDayNumber(
        const HijriDate &date,
        qint64 *jdn
    );

    static bool fromJulianDayNumber(
        qint64 jdn,
        HijriDate *date
    );

    static QString monthName(int month);

private:
    static qint64 yearStart(qint64 year);

    static qint64 monthStartOffset(int month);
};