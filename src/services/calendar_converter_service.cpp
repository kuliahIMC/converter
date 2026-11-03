#include "calendar_converter_service.h"

CalendarConversionResult CalendarConverterService::hijriToCivil(
    const HijriDate &date
) {
    CalendarConversionResult result;

    qint64 jdn = 0;

    if (!TabularHijriCalendar::isValidDate(date)) {
        result.errorMessage = "Tanggal Hijriah tidak valid.";
        return result;
    }

    if (!TabularHijriCalendar::toJulianDayNumber(date, &jdn)) {
        result.errorMessage = "Gagal menghitung tanggal Hijriah.";
        return result;
    }

    CivilDate civilDate;

    if (!CivilCalendar::fromJulianDayNumber(jdn, &civilDate)) {
        result.errorMessage = "Tanggal Masehi berada di luar rentang yang didukung.";
        return result;
    }

    result.success = true;
    result.hijriDate = date;
    result.civilDate = civilDate;
    result.julianDayNumber = jdn;
    result.weekday = CivilCalendar::weekday(jdn);
    result.weekdayName = CivilCalendar::weekdayName(result.weekday);

    result.calendarName =
        CivilCalendar::calendarType(civilDate) == CivilCalendarType::Julian
            ? "Julian"
            : "Gregorian";

    return result;
}

CalendarConversionResult CalendarConverterService::civilToHijri(
    const CivilDate &date
) {
    CalendarConversionResult result;

    qint64 jdn = 0;

    if (!CivilCalendar::isValidDate(date)) {
        result.errorMessage = "Tanggal Masehi tidak valid.";
        return result;
    }

    if (!CivilCalendar::toJulianDayNumber(date, &jdn)) {
        result.errorMessage = "Gagal menghitung tanggal Masehi.";
        return result;
    }

    HijriDate hijriDate;

    if (!TabularHijriCalendar::fromJulianDayNumber(jdn, &hijriDate)) {
        result.errorMessage = "Gagal mengonversi tanggal ke Hijriah.";
        return result;
    }

    CivilDate normalizedDate;

    if (!CivilCalendar::fromJulianDayNumber(jdn, &normalizedDate)) {
        result.errorMessage = "Gagal membaca kembali tanggal Masehi.";
        return result;
    }

    result.success = true;
    result.hijriDate = hijriDate;
    result.civilDate = normalizedDate;
    result.julianDayNumber = jdn;
    result.weekday = CivilCalendar::weekday(jdn);
    result.weekdayName = CivilCalendar::weekdayName(result.weekday);

    result.calendarName =
        CivilCalendar::calendarType(normalizedDate) == CivilCalendarType::Julian
            ? "Julian"
            : "Gregorian";

    return result;
}