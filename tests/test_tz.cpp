#include "../src/tz_display.h"

#include <catch2/catch_test_macros.hpp>

#include <cstring>
#include <ctime>

/* Zone indices from tz_display.c (stored in settings: never renumbered) */
enum { UTC = 0, US_EASTERN = 1, ARIZONA = 4, UK = 10, CET = 11, SYDNEY = 17, AUCKLAND = 18, NEWFOUNDLAND = 9 };

static time_t utc(int y, int mo, int d, int h, int mi) {
    struct tm t = {};
    t.tm_year = y - 1900;
    t.tm_mon  = mo - 1;
    t.tm_mday = d;
    t.tm_hour = h;
    t.tm_min  = mi;
    return timegm(&t);
}

static void check(time_t t, int zone, int hour, int min, const char *abbr) {
    struct tm   out;
    const char *a;
    tz_convert(t, zone, &out, &a);
    CHECK(out.tm_hour == hour);
    CHECK(out.tm_min == min);
    CHECK(std::strcmp(a, abbr) == 0);
}

TEST_CASE("UTC passes through", "[tz]") {
    check(utc(2026, 10, 14, 18, 42), UTC, 18, 42, "UTC");
}

TEST_CASE("US daylight saving boundaries", "[tz]") {
    // 2026: DST starts Sun 8 Mar 07:00 UTC, ends Sun 1 Nov 06:00 UTC
    check(utc(2026, 3, 8, 6, 59), US_EASTERN, 1, 59, "EST");
    check(utc(2026, 3, 8, 7, 0), US_EASTERN, 3, 0, "EDT");
    check(utc(2026, 11, 1, 5, 59), US_EASTERN, 1, 59, "EDT");
    check(utc(2026, 11, 1, 6, 0), US_EASTERN, 1, 0, "EST");
    check(utc(2026, 7, 1, 12, 0), ARIZONA, 5, 0, "MST");
    check(utc(2026, 7, 1, 12, 0), NEWFOUNDLAND, 9, 30, "NDT");
}

TEST_CASE("EU daylight saving boundaries", "[tz]") {
    // 2026: last Sundays are 29 Mar and 25 Oct, at 01:00 UTC
    check(utc(2026, 3, 29, 0, 59), UK, 0, 59, "GMT");
    check(utc(2026, 3, 29, 1, 0), UK, 2, 0, "BST");
    check(utc(2026, 10, 25, 0, 59), CET, 2, 59, "CEST");
    check(utc(2026, 10, 25, 1, 0), CET, 2, 0, "CET");
}

TEST_CASE("Southern hemisphere wraps the new year", "[tz]") {
    check(utc(2026, 1, 1, 0, 0), SYDNEY, 11, 0, "AEDT");
    check(utc(2026, 7, 1, 0, 0), SYDNEY, 10, 0, "AEST");
    check(utc(2026, 12, 31, 23, 0), AUCKLAND, 12, 0, "NZDT");
}

TEST_CASE("Dropdown options list every zone", "[tz]") {
    const char *opts = tz_dropdown_options(true);
    int lines = 1;
    for (const char *p = opts; *p; p++) {
        lines += *p == '\n';
    }
    CHECK(lines == TZ_ZONE_COUNT + 1);
}
