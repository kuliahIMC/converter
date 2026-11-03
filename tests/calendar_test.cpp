#include "../src/services/tabular_hijri_calendar.h"
#include "../src/services/civil_calendar.h"
#include <iostream>
#include <cstdlib>
#include "../src/services/calendar_converter_service.h"

int passed = 0;
int failed = 0;

void test(const char *name, bool condition)
{
    if (condition) {
        std::cout << "[PASS] " << name << '\n';
        ++passed;
    } else {
        std::cout << "[FAIL] " << name << '\n';
        ++failed;
    }
}

int main()
{
    // Epoch
    test(
        "Hijri epoch JDN",
        TabularHijriCalendar::EPOCH_JDN == 1948440
    );

    // Tahun kabisat
    test(
        "Year 2 is leap",
        TabularHijriCalendar::isLeapYear(2)
    );

    test(
        "Year 3 is not leap",
        !TabularHijriCalendar::isLeapYear(3)
    );

    test(
        "Year 29 is leap",
        TabularHijriCalendar::isLeapYear(29)
    );

test(
    "Year 30 is not leap",
    !TabularHijriCalendar::isLeapYear(30)
);
    // Jumlah hari bulan
    test(
        "Muharram has 30 days",
        TabularHijriCalendar::daysInMonth(1, 1) == 30
    );

    test(
        "Safar has 29 days",
        TabularHijriCalendar::daysInMonth(2, 1) == 29
    );

    test(
        "Dzulhijjah leap year has 30 days",
        TabularHijriCalendar::daysInMonth(12, 2) == 30
    );

    test(
        "Dzulhijjah normal year has 29 days",
        TabularHijriCalendar::daysInMonth(12, 1) == 29
    );

    // Validasi tanggal
    test(
        "Valid Hijri date",
        TabularHijriCalendar::isValidDate({1, 1, 1})
    );

    test(
        "Invalid Hijri day",
        !TabularHijriCalendar::isValidDate({31, 1, 1})
    );

    test(
        "Invalid Hijri month",
        !TabularHijriCalendar::isValidDate({1, 13, 1})
    );

    // Konversi ke JDN
    qint64 jdn = 0;

    test(
        "Convert Hijri epoch to JDN",
        TabularHijriCalendar::toJulianDayNumber(
            {1, 1, 1}, &jdn
        ) && jdn == 1948440
    );

    // Konversi kembali
    HijriDate result;

    test(
        "Convert JDN back to Hijri",
        TabularHijriCalendar::fromJulianDayNumber(
            1948440, &result
        ) &&
        result.day == 1 &&
        result.month == 1 &&
        result.year == 1
    );

    // Round-trip satu siklus 30 tahun
    bool roundTrip = true;

    for (qint64 year = 1; year <= 30; ++year) {
        for (int month = 1; month <= 12; ++month) {
            for (int day = 1;
                 day <= TabularHijriCalendar::daysInMonth(
                     month, year
                 );
                 ++day) {

                HijriDate original{day, month, year};
                HijriDate converted;
                qint64 currentJdn = 0;

                if (!TabularHijriCalendar::toJulianDayNumber(
                        original, &currentJdn) ||
                    !TabularHijriCalendar::fromJulianDayNumber(
                        currentJdn, &converted) ||
                    original.day != converted.day ||
                    original.month != converted.month ||
                    original.year != converted.year) {

                    roundTrip = false;
                    break;
                }
            }

            if (!roundTrip)
                break;
        }

        if (!roundTrip)
            break;
    }

    test(
        "Round-trip all dates in 30-year cycle",
        roundTrip
    );

    // Julian-Gregorian calendar tests

qint64 julianJdn = 0;
qint64 gregorianJdn = 0;

// Reformasi kalender
test(
    "4 October 1582 is valid",
    CivilCalendar::isValidDate({4, 10, 1582})
);

test(
    "15 October 1582 is valid",
    CivilCalendar::isValidDate({15, 10, 1582})
);

// Tanggal yang dilewati
test(
    "5 October 1582 is invalid",
    !CivilCalendar::isValidDate({5, 10, 1582})
);

test(
    "10 October 1582 is invalid",
    !CivilCalendar::isValidDate({10, 10, 1582})
);

test(
    "14 October 1582 is invalid",
    !CivilCalendar::isValidDate({14, 10, 1582})
);

// Konversi tanggal reformasi
test(
    "4 October 1582 JDN",
    CivilCalendar::toJulianDayNumber(
        {4, 10, 1582}, &julianJdn
    ) && julianJdn == 2299160
);

test(
    "15 October 1582 JDN",
    CivilCalendar::toJulianDayNumber(
        {15, 10, 1582}, &gregorianJdn
    ) && gregorianJdn == 2299161
);

// Kedua tanggal berurutan
test(
    "Julian-Gregorian transition is consecutive",
    gregorianJdn - julianJdn == 1
);

// Nama hari
test(
    "4 October 1582 was Thursday",
    CivilCalendar::weekday(julianJdn) == 4
);

test(
    "15 October 1582 was Friday",
    CivilCalendar::weekday(gregorianJdn) == 5
);

// Tahun 2000
qint64 modernJdn = 0;

test(
    "1 January 2000 JDN",
    CivilCalendar::toJulianDayNumber(
        {1, 1, 2000}, &modernJdn
    ) && modernJdn == 2451545
);

test(
    "1 January 2000 was Saturday",
    CivilCalendar::weekday(modernJdn) == 6
);

// Konversi JDN kembali
CivilDate convertedDate;

test(
    "JDN converts back to 4 October 1582",
    CivilCalendar::fromJulianDayNumber(
        2299160, &convertedDate
    ) &&
    convertedDate.day == 4 &&
    convertedDate.month == 10 &&
    convertedDate.year == 1582
);

test(
    "JDN converts back to 15 October 1582",
    CivilCalendar::fromJulianDayNumber(
        2299161, &convertedDate
    ) &&
    convertedDate.day == 15 &&
    convertedDate.month == 10 &&
    convertedDate.year == 1582
);

    // Calendar Converter Service Tests

    // Hijriah ke Masehi
    CalendarConversionResult hijriResult =
        CalendarConverterService::hijriToCivil({12, 3, 11});

    test(
        "Hijri to Civil conversion succeeds",
        hijriResult.success
    );

    test(
        "Hijri conversion preserves original date",
        hijriResult.hijriDate.day == 12 &&
        hijriResult.hijriDate.month == 3 &&
        hijriResult.hijriDate.year == 11
    );

    test(
        "Hijri conversion returns valid civil date",
        hijriResult.success &&
        CivilCalendar::isValidDate(hijriResult.civilDate)
    );

    test(
        "Hijri conversion returns weekday",
        hijriResult.success &&
        !hijriResult.weekdayName.isEmpty()
    );

    // Masehi ke Hijriah
    CalendarConversionResult civilResult =
        CalendarConverterService::civilToHijri(
            hijriResult.civilDate
        );

    test(
        "Civil to Hijri conversion succeeds",
        civilResult.success
    );

    test(
        "Round-trip conversion preserves Hijri date",
        civilResult.success &&
        civilResult.hijriDate.day == 12 &&
        civilResult.hijriDate.month == 3 &&
        civilResult.hijriDate.year == 11
    );

    // Validasi input
    CalendarConversionResult invalidHijri =
        CalendarConverterService::hijriToCivil({31, 1, 1});

    test(
        "Converter rejects invalid Hijri date",
        !invalidHijri.success
    );

    CalendarConversionResult invalidCivil =
        CalendarConverterService::civilToHijri({31, 2, 2026});

    test(
        "Converter rejects invalid civil date",
        !invalidCivil.success
    );

    std::cout << "\n============================\n";
    std::cout << "Passed: " << passed << '\n';
    std::cout << "Failed: " << failed << '\n';
    std::cout << "============================\n";

    return failed == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}