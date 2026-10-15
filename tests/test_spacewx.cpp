// space weather parsing
#include <catch2/catch_test_macros.hpp>

#include <cstdlib>
#include <ctime>
#include <string>

#include "../src/space_weather.h"

namespace {

// Trimmed real replies (2026-10-15)
const char *FLUX_REAL = R"([{"flux":123,"time_tag":"2026-10-14T20:00:00"}])";

const char *KP_REAL =
    R"([{"time_tag":"2026-10-14T09:00:00","Kp":2.33,"a_running":9,"station_count":8},)"
    R"({"time_tag":"2026-10-14T12:00:00","Kp":2.00,"a_running":7,"station_count":8},)"
    R"({"time_tag":"2026-10-14T15:00:00","Kp":2.00,"a_running":7,"station_count":8},)"
    R"({"time_tag":"2026-10-14T18:00:00","Kp":2.67,"a_running":12,"station_count":8},)"
    R"({"time_tag":"2026-10-14T21:00:00","Kp":2.67,"a_running":12,"station_count":8},)"
    R"({"time_tag":"2026-10-15T00:00:00","Kp":2.00,"a_running":7,"station_count":8},)"
    R"({"time_tag":"2026-10-15T03:00:00","Kp":0.67,"a_running":3,"station_count":8},)"
    R"({"time_tag":"2026-10-15T06:00:00","Kp":1.33,"a_running":5,"station_count":8},)"
    R"({"time_tag":"2026-10-15T09:00:00","Kp":3.00,"a_running":15,"station_count":8},)"
    R"({"time_tag":"2026-10-15T12:00:00","Kp":2.67,"a_running":12,"station_count":8}])";

const time_t T_2026_10_14_2000 = 1792008000;
const time_t T_2026_10_15_0900 = 1792054800;
const time_t T_2026_10_15_1200 = 1792065600;

sw_data_t empty_data() {
    sw_data_t d = {};
    d.a_index   = -1;
    return d;
}

}  // namespace

TEST_CASE("space weather: real 10.7 cm flux reply", "[test_spacewx]") {
    sw_data_t d = empty_data();

    REQUIRE(sw_parse_flux(FLUX_REAL, &d));
    CHECK(d.has_flux);
    CHECK(d.flux == 123);
    CHECK(d.flux_time == T_2026_10_14_2000);
    CHECK_FALSE(d.has_kp);
}

TEST_CASE("space weather: real K index reply", "[test_spacewx]") {
    sw_data_t d = empty_data();

    REQUIRE(sw_parse_kp(KP_REAL, &d));
    CHECK(d.has_kp);
    // latest is the LAST element, not the largest
    CHECK(d.kp == 2.67);
    CHECK(d.kp_time == T_2026_10_15_1200);
    CHECK(d.a_index == 12);
    // 24 h ending 12:00: 09:00 the day before (2.33) is outside, 15:00 onward inside
    CHECK(d.kp_max_24h == 3.00);
    CHECK(d.kp_max_time == T_2026_10_15_0900);
    CHECK_FALSE(d.has_flux);
}

TEST_CASE("space weather: 24 h window bounds", "[test_spacewx]") {
    sw_data_t d = empty_data();

    // exactly 24 h before the latest is outside; 21 h before is inside
    const char *json = R"([{"time_tag":"2026-10-14T12:00:00","Kp":8.00,"a_running":200},)"
                       R"({"time_tag":"2026-10-14T15:00:00","Kp":5.33,"a_running":56},)"
                       R"({"time_tag":"2026-10-15T12:00:00","Kp":1.00,"a_running":4}])";
    REQUIRE(sw_parse_kp(json, &d));
    CHECK(d.kp == 1.00);
    CHECK(d.kp_max_24h == 5.33);

    // the latest itself can be the maximum
    const char *rising = R"([{"time_tag":"2026-10-15T09:00:00","Kp":2.00,"a_running":7},)"
                         R"({"time_tag":"2026-10-15T12:00:00","Kp":6.00,"a_running":80}])";
    REQUIRE(sw_parse_kp(rising, &d));
    CHECK(d.kp_max_24h == 6.00);
    CHECK(d.kp_max_time == T_2026_10_15_1200);
}

TEST_CASE("space weather: null and missing fields", "[test_spacewx]") {
    sw_data_t d = empty_data();

    // a trailing entry with null Kp is skipped: the previous one is the latest
    const char *json = R"([{"time_tag":"2026-10-15T09:00:00","Kp":3.00,"a_running":null},)"
                       R"({"time_tag":"2026-10-15T12:00:00","Kp":null,"a_running":12},)"
                       R"({"Kp":4.00}])";
    REQUIRE(sw_parse_kp(json, &d));
    CHECK(d.kp == 3.00);
    CHECK(d.kp_time == T_2026_10_15_0900);
    CHECK(d.a_index == -1);

    CHECK_FALSE(sw_parse_kp(R"([{"time_tag":null,"Kp":2}])", &d));
    CHECK_FALSE(sw_parse_flux(R"([{"flux":null,"time_tag":"2026-10-14T20:00:00"}])", &d));
    CHECK_FALSE(sw_parse_flux(R"([{"flux":120}])", &d));
    CHECK_FALSE(sw_parse_flux(R"([null, 5, "x"])", &d));

    // numeric strings (older SWPC format) are read
    REQUIRE(sw_parse_kp(R"([{"time_tag":"2026-10-15 12:00:00","Kp":"5.33","a_running":"56"}])", &d));
    CHECK(d.kp == 5.33);
    CHECK(d.a_index == 56);
    CHECK(d.kp_time == T_2026_10_15_1200);
}

TEST_CASE("space weather: empty and garbage replies leave data alone", "[test_spacewx]") {
    sw_data_t d = empty_data();

    REQUIRE(sw_parse_flux(FLUX_REAL, &d));
    REQUIRE(sw_parse_kp(KP_REAL, &d));

    const char *bad[] = {"[]", "{}", "", "<html>503 Service Unavailable</html>", "[{\"flux\":", "null", "42"};
    for (const char *b : bad) {
        CHECK_FALSE(sw_parse_flux(b, &d));
        CHECK_FALSE(sw_parse_kp(b, &d));
    }
    CHECK_FALSE(sw_parse_flux(nullptr, &d));
    CHECK_FALSE(sw_parse_kp(nullptr, &d));
    CHECK_FALSE(sw_parse_kp(R"([{"time_tag":"2026-10-15T12:00:00","Kp":12}])", &d));

    CHECK(d.flux == 123);
    CHECK(d.kp == 2.67);
    CHECK(d.kp_max_24h == 3.00);
}

TEST_CASE("space weather: NOAA G scale boundaries", "[test_spacewx]") {
    CHECK(sw_g_level(0.0) == 0);
    CHECK(sw_g_level(4.0) == 0);
    CHECK(sw_g_level(4.67) == 0);
    CHECK(sw_g_level(5.0) == 1);
    CHECK(sw_g_level(5.33) == 1);
    CHECK(sw_g_level(5.67) == 1);
    CHECK(sw_g_level(6.0) == 2);
    CHECK(sw_g_level(7.0) == 3);
    CHECK(sw_g_level(8.0) == 4);
    CHECK(sw_g_level(8.67) == 4);
    CHECK(sw_g_level(9.0) == 5);
    CHECK(sw_g_level(-1.0) == 0);

    CHECK(std::string(sw_g_name(0)) == "");
    CHECK(std::string(sw_g_name(1)) == "Minor");
    CHECK(std::string(sw_g_name(2)) == "Moderate");
    CHECK(std::string(sw_g_name(3)) == "Strong");
    CHECK(std::string(sw_g_name(4)) == "Severe");
    CHECK(std::string(sw_g_name(5)) == "Extreme");
    CHECK(std::string(sw_g_name(6)) == "");
}

TEST_CASE("space weather: timestamps are UTC whatever the local zone", "[test_spacewx]") {
    const char *old = getenv("TZ");
    std::string saved = old ? old : "";
    time_t      t     = 0;

    setenv("TZ", "America/New_York", 1);
    tzset();
    REQUIRE(sw_parse_time("2026-10-15T12:00:00", &t));
    CHECK(t == T_2026_10_15_1200);
    REQUIRE(sw_parse_time("1970-01-01T00:00:00", &t));
    CHECK(t == 0);

    CHECK_FALSE(sw_parse_time("2026-13-01T00:00:00", &t));
    CHECK_FALSE(sw_parse_time("2026-10-15X12:00:00", &t));
    CHECK_FALSE(sw_parse_time("yesterday", &t));
    CHECK_FALSE(sw_parse_time(nullptr, &t));

    if (old) {
        setenv("TZ", saved.c_str(), 1);
    } else {
        unsetenv("TZ");
    }
    tzset();
}
