#include "civil_calendar.h"
#include "civil_calendar.h"

#include <QStringList>
namespace {

bool isLeapYearJulian(qint64 year)
{
    return year % 4 == 0;
}

bool isLeapYearGregorian(qint64 year)
{
    return year % 4 == 0 &&
           (year % 100 != 0 || year % 400 == 0);
}

int daysInMonth(int month, qint64 year, bool gregorian)
{
    static const int days[] = {
        31, 28, 31, 30, 31, 30,
        31, 31, 30, 31, 30, 31
    };

    if (month < 1 || month > 12)
        return 0;

    if (month == 2) {
        const bool leap = gregorian
            ? isLeapYearGregorian(year)
            : isLeapYearJulian(year);

        return leap ? 29 : 28;
    }

    return days[month - 1];
}

qint64 julianToJdn(const CivilDate &date)
{
    const qint64 a = (14 - date.month) / 12;
    const qint64 y = date.year + 4800 - a;
    const qint64 m = date.month + 12 * a - 3;

    return date.day
        + (153 * m + 2) / 5
        + 365 * y
        + y / 4
        - 32083;
}

qint64 gregorianToJdn(const CivilDate &date)
{
    const qint64 a = (14 - date.month) / 12;
    const qint64 y = date.year + 4800 - a;
    const qint64 m = date.month + 12 * a - 3;

    return date.day
        + (153 * m + 2) / 5
        + 365 * y
        + y / 4
        - y / 100
        + y / 400
        - 32045;
}

}

CivilCalendarType CivilCalendar::calendarType(
    const CivilDate &date)
{
    if (date.year < 1582 ||
        (date.year == 1582 &&
         (date.month < 10 ||
          (date.month == 10 && date.day < 15)))) {
        return CivilCalendarType::Julian;
    }

    return CivilCalendarType::Gregorian;
}

bool CivilCalendar::isGregorianDate(const CivilDate &date)
{
    return calendarType(date) == CivilCalendarType::Gregorian;
}

bool CivilCalendar::isValidDate(const CivilDate &date)
{
    if (date.year < 1 || date.month < 1 || date.month > 12)
        return false;

    // Tanggal 5–14 Oktober 1582 tidak ada
    // dalam sistem reformasi kalender ini.
    if (date.year == 1582 &&
        date.month == 10 &&
        date.day >= 5 &&
        date.day <= 14) {
        return false;
    }

    const bool gregorian = isGregorianDate(date);

    return date.day >= 1 &&
        date.day <= daysInMonth(
            date.month,
            date.year,
            gregorian
        );
}

bool CivilCalendar::toJulianDayNumber(
    const CivilDate &date,
    qint64 *jdn)
{
    if (!jdn || !isValidDate(date))
        return false;

    *jdn = isGregorianDate(date)
        ? gregorianToJdn(date)
        : julianToJdn(date);

    return true;
}

bool CivilCalendar::fromJulianDayNumber(
    qint64 jdn,
    CivilDate *date)
{
    if (!date || jdn < 1721424)
        return false;

    qint64 a, b, c, d, e, m;

    if (jdn >= GREGORIAN_REFORM_JDN) {
        a = jdn + 32044;
        b = (4 * a + 3) / 146097;
        c = a - (146097 * b) / 4;
    } else {
        b = 0;
        c = jdn + 32082;
    }

    d = (4 * c + 3) / 1461;
    e = c - (1461 * d) / 4;
    m = (5 * e + 2) / 153;

    date->day = static_cast<int>(
        e - (153 * m + 2) / 5 + 1
    );

    date->month = static_cast<int>(
        m + 3 - 12 * (m / 10)
    );

    date->year = 100 * b + d - 4800 + m / 10;

    return isValidDate(*date);
}

int CivilCalendar::weekday(qint64 jdn)
{
    // 0 = Minggu, 1 = Senin, ..., 6 = Sabtu
    return static_cast<int>((jdn + 1) % 7);
}

QString CivilCalendar::weekdayName(int weekday)
{
    static const QStringList names = {
        "Minggu",
        "Senin",
        "Selasa",
        "Rabu",
        "Kamis",
        "Jumat",
        "Sabtu"
    };

    if (weekday < 0 || weekday > 6)
        return {};

    return names.at(weekday);
}