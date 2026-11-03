#pragma once

#include <QString>
#include <QtGlobal>

struct CivilDate {
    int day = 1;
    int month = 1;
    qint64 year = 1;
};

enum class CivilCalendarType {
    Julian,
    Gregorian
};

class CivilCalendar {
public:
    static constexpr qint64 GREGORIAN_REFORM_JDN = 2299161;

    static bool isValidDate(const CivilDate &date);

    static bool isGregorianDate(const CivilDate &date);

    static bool toJulianDayNumber(
        const CivilDate &date,
        qint64 *jdn
    );

    static bool fromJulianDayNumber(
        qint64 jdn,
        CivilDate *date
    );

    static CivilCalendarType calendarType(
        const CivilDate &date
    );

    static int weekday(qint64 jdn);

    static QString weekdayName(int weekday);
};