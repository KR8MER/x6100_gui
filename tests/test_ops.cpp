/* Nexus-style operating: cockpit rules, log rules, the pending hunt and the
 * POTA spot parser. */
extern "C" {
#include "../src/cockpit.h"
#include "../src/hunt_target.h"
#include "../src/log_rules.h"
#include "../src/pota_spots.h"
}

#include <catch2/catch_test_macros.hpp>

#include <cstring>
#include <string>

enum { LSB = 0, LSB_DIG = 1, USB = 2, USB_DIG = 3, CW = 4, CWR = 5, AM = 6, NFM = 7 };

TEST_CASE("Phone sideband follows the band", "[cockpit]") {
    CHECK(cockpit_phone_sideband(1900000) == LSB);
    CHECK(cockpit_phone_sideband(3850000) == LSB);
    CHECK(cockpit_phone_sideband(5357000) == USB);   // 60 m is USB by rule
    CHECK(cockpit_phone_sideband(7200000) == LSB);
    CHECK(cockpit_phone_sideband(14250000) == USB);
    CHECK(cockpit_phone_sideband(28400000) == USB);
    CHECK(cockpit_phone_sideband(50125000) == USB);
}

TEST_CASE("Log mode and report from the radio", "[log]") {
    CHECK(log_mode_for(USB, COCKPIT_PHONE) == MODE_SSB);
    CHECK(log_mode_for(LSB, COCKPIT_NONE) == MODE_SSB);
    CHECK(log_mode_for(CWR, COCKPIT_CW) == MODE_CW);
    CHECK(log_mode_for(NFM, COCKPIT_NONE) == MODE_FM);
    CHECK(log_mode_for(LSB_DIG, COCKPIT_RTTY) == MODE_RTTY);
    CHECK(log_mode_for(USB_DIG, COCKPIT_DIGITAL) == MODE_OTHER);
    CHECK(log_default_rst(MODE_SSB) == 59);
    CHECK(log_default_rst(MODE_CW) == 599);
    CHECK(log_default_rst(MODE_RTTY) == 599);
}

TEST_CASE("References are told apart", "[log]") {
    CHECK(std::string(log_sig_for_ref("US-1391")) == "POTA");
    CHECK(std::string(log_sig_for_ref("K-0001")) == "POTA");
    CHECK(std::string(log_sig_for_ref("VE-12345")) == "POTA");
    CHECK(std::string(log_sig_for_ref("W8O/NE-001")) == "SOTA");
    CHECK(std::string(log_sig_for_ref("G/LD-001")) == "SOTA");
    CHECK(std::string(log_sig_for_ref("")) == "");
    CHECK(std::string(log_sig_for_ref("US-12")) == "");
    CHECK(std::string(log_sig_for_ref("hello")) == "");
}

TEST_CASE("Base call drops portable and prefix parts", "[hunt]") {
    char out[32];
    hunt_base_call("w1aw/p", out, sizeof(out));
    CHECK(std::string(out) == "W1AW");
    hunt_base_call("VE3/W1AW", out, sizeof(out));
    CHECK(std::string(out) == "W1AW");
    hunt_base_call("KR8MER", out, sizeof(out));
    CHECK(std::string(out) == "KR8MER");
}

TEST_CASE("A hunt tags one QSO with that activator, then is gone", "[hunt]") {
    char sig[8], ref[24];
    time_t t0 = 1792000000;

    hunt_target_set("W1AW", "POTA", "US-1234", t0);
    CHECK_FALSE(hunt_target_take("K1ABC", t0 + 60, sig, sizeof(sig), ref, sizeof(ref)));
    REQUIRE(hunt_target_take("W1AW/P", t0 + 60, sig, sizeof(sig), ref, sizeof(ref)));
    CHECK(std::string(sig) == "POTA");
    CHECK(std::string(ref) == "US-1234");
    CHECK_FALSE(hunt_target_take("W1AW", t0 + 120, sig, sizeof(sig), ref, sizeof(ref)));
}

TEST_CASE("A hunt expires after four hours", "[hunt]") {
    char sig[8], ref[24], who[32];
    time_t t0 = 1792000000;

    hunt_target_set("N8XYZ", "POTA", "US-0001", t0);
    CHECK(hunt_target_peek(t0 + HUNT_TTL_SEC - 1, who, sizeof(who), ref, sizeof(ref)));
    CHECK_FALSE(hunt_target_take("N8XYZ", t0 + HUNT_TTL_SEC + 1, sig, sizeof(sig), ref, sizeof(ref)));
}

TEST_CASE("POTA spots parse, newest first, bad rows dropped", "[pota]") {
    const char *json = R"([
      {"activator":"N0CALL","frequency":"14055.5","mode":"CW","reference":"US-0001",
       "name":"Example State Park","locationDesc":"US-OH","spotTime":"2026-10-15T00:10:00"},
      {"activator":"N0CALL/P","frequency":"7185","mode":"SSB","reference":"US-0002",
       "name":"Another Park","locationDesc":"US-MI","spotTime":"2026-10-15T00:20:00"},
      {"activator":"N0VHF","frequency":"146520","mode":"FM","reference":"US-0003",
       "name":"Too high for the radio","spotTime":"2026-10-15T00:30:00"},
      {"activator":"","frequency":"14074","mode":"FT8","reference":"US-0004"}
    ])";
    pota_spot_t spots[8];
    int         n = pota_spots_parse(json, spots, 8);

    REQUIRE(n == 2);
    CHECK(std::string(spots[0].activator) == "N0CALL/P");   // newest first
    CHECK(spots[0].freq_hz == 7185000);
    CHECK(std::string(spots[0].mode) == "SSB");
    CHECK(spots[1].freq_hz == 14055500);
    CHECK(std::string(spots[1].ref) == "US-0001");
    CHECK(std::string(spots[1].park) == "Example State Park");

    CHECK(pota_spots_parse("{\"not\":\"a list\"}", spots, 8) == -1);
    CHECK(pota_spots_parse("garbage", spots, 8) == -1);
}
