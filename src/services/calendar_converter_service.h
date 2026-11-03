#pragma once

#include "tabular_hijri_calendar.h"
#include "civil_calendar.h"

#include <QString>
#include <QtGlobal>

struct CalendarConversionResult {
    bool success = false;

    HijriDate hijriDate;
    CivilDate civilDate;

    qint64 julianDayNumber = 0;
    int weekday = 0;

    QString weekdayName;
    QString calendarName;
    QString errorMessage;
};

class CalendarConverterService {
public:
    static CalendarConversionResult hijriToCivil(
        const HijriDate &date
    );

    static CalendarConversionResult civilToHijri(
        const CivilDate &date
    );
};