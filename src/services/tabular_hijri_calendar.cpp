#include "tabular_hijri_calendar.h"
#include "tabular_hijri_calendar.h"

#include <QStringList>

namespace {

constexpr qint64 DAYS_PER_30_YEARS = 10631;

}

bool TabularHijriCalendar::isLeapYear(qint64 year)
{
    if (year < 1)
        return false;

    return ((11 * year + 14) % 30) < 11;
}

int TabularHijriCalendar::daysInMonth(int month, qint64 year)
{
    if (month < 1 || month > 12 || year < 1)
        return 0;

    // Bulan ganjil: 30 hari.
    // Bulan genap: 29 hari.
    if (month % 2 == 1)
        return 30;

    // Dzulhijjah memiliki 30 hari pada tahun kabisat.
    if (month == 12 && isLeapYear(year))
        return 30;

    return 29;
}

bool TabularHijriCalendar::isValidDate(const HijriDate &date)
{
    if (date.year < 1)
        return false;

    if (date.month < 1 || date.month > 12)
        return false;

    const int maximumDay =
        daysInMonth(date.month, date.year);

    return date.day >= 1 && date.day <= maximumDay;
}

qint64 TabularHijriCalendar::yearStart(qint64 year)
{
    if (year < 1)
        return -1;

    const qint64 completedYears = year - 1;

    const qint64 leapDays =
        (3 + 11 * year) / 30;

    return EPOCH_JDN
        + (354 * completedYears)
        + leapDays;
}

qint64 TabularHijriCalendar::monthStartOffset(int month)
{
    if (month < 1 || month > 12)
        return -1;

    const qint64 completedMonths = month - 1;

    // Pembulatan ke atas dari 29,5 hari per bulan.
    return (59 * completedMonths + 1) / 2;
}

bool TabularHijriCalendar::toJulianDayNumber(
    const HijriDate &date,
    qint64 *jdn)
{
    if (!jdn || !isValidDate(date))
        return false;

    *jdn = yearStart(date.year)
        + monthStartOffset(date.month)
        + date.day - 1;

    return true;
}

bool TabularHijriCalendar::fromJulianDayNumber(
    qint64 jdn,
    HijriDate *date)
{
    if (!date || jdn < EPOCH_JDN)
        return false;

    // Perkiraan tahun Hijriah.
    qint64 low = 1;
    qint64 high =
        ((jdn - EPOCH_JDN) / DAYS_PER_30_YEARS + 1) * 30 + 30;

    // Cari tahun yang memuat JDN.
    while (low <= high) {
        const qint64 middle = low + (high - low) / 2;
        const qint64 start = yearStart(middle);
        const qint64 nextStart = yearStart(middle + 1);

        if (jdn < start) {
            high = middle - 1;
        }
        else if (jdn >= nextStart) {
            low = middle + 1;
        }
        else {
            date->year = middle;
            break;
        }
    }

    if (low > high)
        return false;

    // Cari bulan dalam tahun tersebut.
    const qint64 startOfYear = yearStart(date->year);
    const qint64 dayOfYear = jdn - startOfYear;

    int month = 12;

    for (int candidate = 1; candidate <= 12; ++candidate) {
        const qint64 nextMonthOffset =
            candidate == 12
                ? 354 + (isLeapYear(date->year) ? 1 : 0)
                : monthStartOffset(candidate + 1);

        if (dayOfYear < nextMonthOffset) {
            month = candidate;
            break;
        }
    }

    const qint64 dayOffset =
        dayOfYear - monthStartOffset(month);

    date->month = month;
    date->day = static_cast<int>(dayOffset) + 1;

    return isValidDate(*date);
}

QString TabularHijriCalendar::monthName(int month)
{
    static const QStringList months = {
        "",
        "Muharram",
        "Safar",
        "Rabiul Awal",
        "Rabiul Akhir",
        "Jumadil Ula",
        "Jumadil Akhir",
        "Rajab",
        "Syaban",
        "Ramadhan",
        "Syawal",
        "Dzulqaidah",
        "Dzulhijjah"
    };

    if (month < 1 || month > 12)
        return {};

    return months.at(month);
}