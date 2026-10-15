// DX cluster spot parsing (HamQTH /api/v2/spots)
extern "C" {
#include "../src/dx_spots.h"
}

#include <catch2/catch_test_macros.hpp>

#include <cstring>
#include <ctime>
#include <string>
#include <vector>

// Ten real spots from HamQTH (2026-10-15), in the feed's order. Two are on
// 2 m and 70 cm, which the radio can't reach.
static const char *real_sample = R"({"spots":[
{"spotter":"W5ART","dxcall":"EA5ET","freq":28076.5,"band":"10M","mode":"DIGI","comment":"","time":"2026-10-15T16:31:00Z","dxcc":281,"country":"Spain","continent":"EU","lotw":true,"eqsl":true},
{"spotter":"DL6BF","dxcall":"SM6MUY","freq":144174.0,"band":"2M","mode":"DIGI","comment":"FT8 +30 dB  DG8YHH CALL","time":"2026-10-15T16:31:00Z","dxcc":284,"country":"Sweden","continent":"EU","lotw":true,"eqsl":false},
{"spotter":"VE1ZZ","dxcall":"N3A/2","freq":14061.5,"band":"20M","mode":"CW","comment":"","time":"2026-10-15T16:31:00Z","dxcc":291,"country":"United States","continent":"NA","lotw":false,"eqsl":false},
{"spotter":"IK5PWQ","dxcall":"II5GG","freq":10116.0,"band":"30M","mode":"CW","comment":"galileo award","time":"2026-10-15T16:31:00Z","dxcc":248,"country":"Italy","continent":"EU","lotw":false,"eqsl":false},
{"spotter":"SP3EFD","dxcall":"SN2S","freq":3758.0,"band":"80M","mode":"SSB","comment":"LSB","time":"2026-10-15T16:31:00Z","dxcc":269,"country":"Poland","continent":"EU","lotw":true,"eqsl":false},
{"spotter":"EA6SX","dxcall":"EA6SX/3","freq":5357.0,"band":"60M","mode":"DIGI","comment":"FT4 DME-50201 (Paracuellos de Jiloca) TPEA: Z","time":"2026-10-15T16:31:00Z","dxcc":281,"country":"Spain","continent":"EU","lotw":true,"eqsl":false},
{"spotter":"9K2HS","dxcall":"9K2HS","freq":24892.0,"band":"12M","mode":"CW","comment":"CQ CW","time":"2026-10-15T16:26:00Z","dxcc":348,"country":"Kuwait","continent":"AS","lotw":false,"eqsl":true},
{"spotter":"F5DQK","dxcall":"OZ1FKZ","freq":432174.0,"band":"70CM","mode":"SSB","comment":"qso not finished, AS ?","time":"2026-10-15T16:22:00Z","dxcc":221,"country":"Denmark","continent":"EU","lotw":false,"eqsl":false},
{"spotter":"VE9HF","dxcall":"VE1PZ","freq":50313.0,"band":"6M","mode":"DIGI","comment":"","time":"2026-10-15T16:15:00Z","dxcc":1,"country":"Canada","continent":"NA","lotw":true,"eqsl":true},
{"spotter":"IK0FFU","dxcall":"IZ0PAP","freq":7116.0,"band":"40M","mode":"SSB","comment":"monumenti ita. award","time":"2026-10-15T16:12:00Z","dxcc":248,"country":"Italy","continent":"EU","lotw":false,"eqsl":true}
]})";

static std::string spot_json(const char *call, double khz, const char *mode, const char *time,
                             const char *comment = "") {
    char buf[1024];
    snprintf(buf, sizeof(buf),
             R"({"spotter":"K1ABC","dxcall":"%s","freq":%.1f,"band":"20M","mode":%s,"comment":"%s","time":"%s",)"
             R"("dxcc":1,"country":"Canada","continent":"NA","lotw":false,"eqsl":false})",
             call, khz, mode, comment, time);
    return buf;
}

static std::string feed(const std::vector<std::string> &items) {
    std::string s = R"({"spots":[)";
    for (size_t i = 0; i < items.size(); i++) {
        if (i) s += ",";
        s += items[i];
    }
    return s + "]}";
}

static dx_spot_t make_spot(const char *mode, const char *comment) {
    dx_spot_t s;
    memset(&s, 0, sizeof(s));
    snprintf(s.mode, sizeof(s.mode), "%s", mode);
    snprintf(s.comment, sizeof(s.comment), "%s", comment);
    return s;
}

TEST_CASE("Real HamQTH spots parse, newest first, HF and 6 m only", "[test_dxc]") {
    dx_spot_t out[20];
    int       n = dx_spots_parse(real_sample, out, 20);

    REQUIRE(n == 8);   // 2 m and 70 cm dropped
    const char *order[] = {"EA5ET", "N3A/2", "II5GG", "SN2S", "EA6SX/3", "9K2HS", "VE1PZ", "IZ0PAP"};
    for (int i = 0; i < n; i++) {
        CHECK(std::string(out[i].call) == order[i]);
        if (i) CHECK(out[i - 1].time >= out[i].time);
    }

    CHECK(out[0].freq_hz == 28076500);
    CHECK(std::string(out[0].band) == "10M");
    CHECK(std::string(out[0].mode) == "DIGI");
    CHECK(std::string(out[0].spotter) == "W5ART");
    CHECK(std::string(out[0].country) == "Spain");
    CHECK(std::string(out[0].continent) == "EU");
    CHECK(out[0].lotw);
    CHECK(out[0].time == 1792081860);   // 2026-10-15 16:31:00 UTC

    CHECK(out[1].freq_hz == 14061500);
    CHECK(std::string(out[1].mode) == "CW");
    CHECK_FALSE(out[1].lotw);
    CHECK(std::string(out[2].comment) == "galileo award");
    CHECK(out[6].freq_hz == 50313000);
    CHECK(std::string(out[6].band) == "6M");
    CHECK(out[7].time == 1792081860 - 19 * 60);
}

TEST_CASE("A spot without a mode has an empty one", "[test_dxc]") {
    dx_spot_t out[4];
    std::string j = feed({spot_json("W1AW", 14020.0, "null", "2026-10-15T16:31:00Z")});

    REQUIRE(dx_spots_parse(j.c_str(), out, 4) == 1);
    CHECK(std::string(out[0].mode) == "");
    CHECK(dx_spot_tune(&out[0]) == DX_TUNE_ONLY);
}

TEST_CASE("Spots out of range or without a call are dropped", "[test_dxc]") {
    dx_spot_t   out[8];
    std::string j = feed({
        spot_json("LOW1", 999.0, R"("CW")", "2026-10-15T16:31:00Z"),
        spot_json("EDGE1", 1000.0, R"("CW")", "2026-10-15T16:30:00Z"),
        spot_json("EDGE2", 60000.0, R"("CW")", "2026-10-15T16:29:00Z"),
        spot_json("HIGH1", 60000.1, R"("CW")", "2026-10-15T16:28:00Z"),
        spot_json("VHF1", 144174.0, R"("DIGI")", "2026-10-15T16:27:00Z"),
        spot_json("", 14020.0, R"("CW")", "2026-10-15T16:26:00Z"),
        R"({"spotter":"K1ABC","freq":14021.0,"mode":"CW","time":"2026-10-15T16:25:00Z"})",
        R"("not an object")",
    });

    REQUIRE(dx_spots_parse(j.c_str(), out, 8) == 2);
    CHECK(std::string(out[0].call) == "EDGE1");
    CHECK(std::string(out[1].call) == "EDGE2");
}

TEST_CASE("Not a spot list", "[test_dxc]") {
    dx_spot_t out[2];
    CHECK(dx_spots_parse("", out, 2) == -1);
    CHECK(dx_spots_parse("<html>busy</html>", out, 2) == -1);
    CHECK(dx_spots_parse("[]", out, 2) == -1);
    CHECK(dx_spots_parse(R"({"error":"rate limit"})", out, 2) == -1);
    CHECK(dx_spots_parse(R"({"spots":{}})", out, 2) == -1);
    CHECK(dx_spots_parse(R"({"spots":[]})", out, 2) == 0);
}

TEST_CASE("Out of order feed comes back newest first", "[test_dxc]") {
    dx_spot_t   out[4];
    std::string j = feed({
        spot_json("MID", 14020.0, R"("CW")", "2026-10-15T16:20:00Z"),
        spot_json("OLD", 14021.0, R"("CW")", "2026-10-15T16:10:00Z"),
        spot_json("NEW", 14022.0, R"("CW")", "2026-10-15T16:30:00Z"),
    });

    REQUIRE(dx_spots_parse(j.c_str(), out, 4) == 3);
    CHECK(std::string(out[0].call) == "NEW");
    CHECK(std::string(out[1].call) == "MID");
    CHECK(std::string(out[2].call) == "OLD");
}

TEST_CASE("Truncation keeps the newest, not the first in the feed", "[test_dxc]") {
    dx_spot_t   out[2];
    std::string j = feed({
        spot_json("OLD1", 14020.0, R"("CW")", "2026-10-15T16:00:00Z"),
        spot_json("OLD2", 14021.0, R"("CW")", "2026-10-15T16:05:00Z"),
        spot_json("NEW2", 14022.0, R"("CW")", "2026-10-15T16:25:00Z"),
        spot_json("NEW1", 14023.0, R"("CW")", "2026-10-15T16:30:00Z"),
    });

    REQUIRE(dx_spots_parse(j.c_str(), out, 2) == 2);
    CHECK(std::string(out[0].call) == "NEW1");
    CHECK(std::string(out[1].call) == "NEW2");
}

TEST_CASE("Long fields are cut and stay terminated", "[test_dxc]") {
    dx_spot_t   out[1];
    std::string call(60, 'K'), comment(300, 'x');
    std::string j = feed({spot_json(call.c_str(), 14020.0, R"("CW")", "2026-10-15T16:30:00Z", comment.c_str())});

    REQUIRE(dx_spots_parse(j.c_str(), out, 1) == 1);
    CHECK(strlen(out[0].call) == sizeof(out[0].call) - 1);
    CHECK(strlen(out[0].comment) == sizeof(out[0].comment) - 1);
}

TEST_CASE("Band names, edges included", "[test_dxc]") {
    char b[8];

    struct {
        int32_t     hz;
        const char *name;
    } in[] = {
        {1800000, "160M"},  {2000000, "160M"},  {3500000, "80M"},   {3999999, "80M"},   {5357000, "60M"},
        {7000000, "40M"},   {7300000, "40M"},   {10100000, "30M"},  {10150000, "30M"},  {14000000, "20M"},
        {14350000, "20M"},  {18068000, "17M"},  {18168000, "17M"},  {21000000, "15M"},  {21450000, "15M"},
        {24890000, "12M"},  {24990000, "12M"},  {28000000, "10M"},  {29700000, "10M"},  {50000000, "6M"},
        {54000000, "6M"},
    };
    for (auto &t : in) {
        CHECK(dx_band_name(t.hz, b, sizeof(b)));
        CHECK(std::string(b) == t.name);
    }

    int32_t out_of_band[] = {1799999, 2000001, 3499999, 4000001, 6999999, 7300001, 10099999, 10150001,
                             13999999, 14350001, 29700001, 49999999, 54000001, 0};
    for (int32_t hz : out_of_band) {
        CHECK_FALSE(dx_band_name(hz, b, sizeof(b)));
        CHECK(std::string(b) == "");
    }
}

TEST_CASE("Spots URL", "[test_dxc]") {
    char url[192];

    dx_spots_url(url, sizeof(url), NULL, NULL, 100);
    CHECK(std::string(url) == "https://www.hamqth.com/api/v2/spots?format=json&limit=100");
    dx_spots_url(url, sizeof(url), "20M", NULL, 100);
    CHECK(std::string(url) == "https://www.hamqth.com/api/v2/spots?format=json&limit=100&bands=20M");
    dx_spots_url(url, sizeof(url), "", "CW", 60);
    CHECK(std::string(url) == "https://www.hamqth.com/api/v2/spots?format=json&limit=60&modes=CW");
    dx_spots_url(url, sizeof(url), "6M", "DIGI", 100);
    CHECK(std::string(url) == "https://www.hamqth.com/api/v2/spots?format=json&limit=100&bands=6M&modes=DIGI");
    dx_spots_url(url, sizeof(url), NULL, NULL, 0);
    CHECK(std::string(url) == "https://www.hamqth.com/api/v2/spots?format=json&limit=1");
    dx_spots_url(url, sizeof(url), NULL, NULL, 500);
    CHECK(std::string(url) == "https://www.hamqth.com/api/v2/spots?format=json&limit=200");

    char tiny[16];
    dx_spots_url(tiny, sizeof(tiny), "20M", "SSB", 100);
    CHECK(strlen(tiny) == sizeof(tiny) - 1);
}

TEST_CASE("How to tune for a spot", "[test_dxc]") {
    dx_spot_t s = make_spot("CW", "CQ CW");
    CHECK(dx_spot_tune(&s) == DX_TUNE_CW);
    s = make_spot("SSB", "LSB");
    CHECK(dx_spot_tune(&s) == DX_TUNE_PHONE);
    s = make_spot("DIGI", "FT8 +30 dB  DG8YHH CALL");
    CHECK(dx_spot_tune(&s) == DX_TUNE_FT8);
    s = make_spot("DIGI", "IO81TI<>FF46QJ FT8");
    CHECK(dx_spot_tune(&s) == DX_TUNE_FT8);
    s = make_spot("DIGI", "cq ft8 -17dB");
    CHECK(dx_spot_tune(&s) == DX_TUNE_FT8);
    s = make_spot("DIGI", "FT4 DME-50201 (Paracuellos de Jiloca) TPEA: Z");
    CHECK(dx_spot_tune(&s) == DX_TUNE_FT4);
    s = make_spot("DIGI", "FT4 now, was FT8");
    CHECK(dx_spot_tune(&s) == DX_TUNE_FT4);
    s = make_spot("DIGI", "");
    CHECK(dx_spot_tune(&s) == DX_TUNE_ONLY);
    s = make_spot("DIGI", "FT2 JN71 Decodium");
    CHECK(dx_spot_tune(&s) == DX_TUNE_ONLY);
    s = make_spot("DIGI", "SOFT8 FT84");
    CHECK(dx_spot_tune(&s) == DX_TUNE_ONLY);
    s = make_spot("CW", "FT8");                 // the mode decides first
    CHECK(dx_spot_tune(&s) == DX_TUNE_CW);
    s = make_spot("", "FT8");
    CHECK(dx_spot_tune(&s) == DX_TUNE_ONLY);
}
