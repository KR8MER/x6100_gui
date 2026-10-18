#include <catch2/catch_test_macros.hpp>

#include <cstdlib>
#include <cstring>
#include <string>

#include <jansson.h>

extern "C" {
#include "../src/pota_ref.h"
}

/* pota.app's own checks (validReferenceRegex, validCallsignRegex and the
 * callsign helper of its spot form) were compared against these on 50 000
 * generated inputs; the cases below are the ones that matter in use. */

TEST_CASE("Park references as pota.app checks them", "[pota]") {
    CHECK(pota_ref_valid("US-1234"));
    CHECK(pota_ref_valid("VE-12345"));
    CHECK(pota_ref_valid("K-1234"));
    CHECK(pota_ref_valid("4X-0001"));
    CHECK_FALSE(pota_ref_valid("USA-1234"));   // prefix is 1 or 2
    CHECK_FALSE(pota_ref_valid("US-123"));
    CHECK_FALSE(pota_ref_valid("US-123456"));
    CHECK_FALSE(pota_ref_valid("us-1234"));    // upper case only
    CHECK_FALSE(pota_ref_valid("US1234"));     // the dash is required
    CHECK_FALSE(pota_ref_valid(""));
    CHECK_FALSE(pota_ref_valid(nullptr));
}

TEST_CASE("Callsigns pota.app takes on a spot", "[pota]") {
    CHECK(pota_call_valid("KR8MER"));
    CHECK(pota_call_valid("KR8MER/P"));
    CHECK(pota_call_valid("VE3/KR8MER"));
    CHECK(pota_call_valid("2E0ABC"));
    CHECK(pota_call_valid("K1A"));
    CHECK_FALSE(pota_call_valid("KR8MER//P"));
    CHECK_FALSE(pota_call_valid("/KR8MER"));
    CHECK_FALSE(pota_call_valid("KR8MER/"));
    CHECK_FALSE(pota_call_valid("K8"));         // must end in a letter
    CHECK_FALSE(pota_call_valid("KR8MER/12345"));
    CHECK_FALSE(pota_call_valid(""));
}

TEST_CASE("Typed park lists", "[pota]") {
    char refs[POTA_PARKS_MAX][POTA_REF_LEN];
    char bad[POTA_REF_LEN];

    REQUIRE(pota_refs_parse("us-1234, US-5678 us-1234", refs, POTA_PARKS_MAX, bad, sizeof(bad)) == 2);
    CHECK(std::string(refs[0]) == "US-1234");
    CHECK(std::string(refs[1]) == "US-5678");
    CHECK(pota_refs_parse("", refs, POTA_PARKS_MAX, bad, sizeof(bad)) == 0);
    CHECK(pota_refs_parse(" , ", refs, POTA_PARKS_MAX, bad, sizeof(bad)) == 0);
    CHECK(pota_refs_parse("US-1234,US12", refs, POTA_PARKS_MAX, bad, sizeof(bad)) == -1);
    CHECK(std::string(bad) == "US12");
    CHECK(pota_refs_parse("US-0001,US-0002,US-0003,US-0004,US-0005", refs, POTA_PARKS_MAX, bad, sizeof(bad)) == -1);
    CHECK(pota_refs_parse("US-1234567890123456789", refs, POTA_PARKS_MAX, bad, sizeof(bad)) == -1);

    char joined[64];
    REQUIRE(pota_refs_parse("US-1234 US-5678", refs, POTA_PARKS_MAX, nullptr, 0) == 2);
    pota_refs_join(refs, 2, joined, sizeof(joined));
    CHECK(std::string(joined) == "US-1234,US-5678");
}

TEST_CASE("Upload file names follow POTA's rule", "[pota]") {
    char name[96];
    time_t day = 1792022400 + 3600;   // 2026-10-15 01:00 UTC

    pota_upload_name("KR8MER", "US-1234", day, name, sizeof(name));
    CHECK(std::string(name) == "KR8MER@US-1234-20261015.adi");
    pota_upload_name("kr8mer/p", "US-1234", day, name, sizeof(name));
    CHECK(std::string(name) == "KR8MER_P@US-1234-20261015.adi");
}

static std::string field(const char *json, const char *key) {
    json_error_t err;
    json_t      *o = json_loads(json, 0, &err);
    REQUIRE(o);
    const char *v = json_string_value(json_object_get(o, key));
    std::string s = v ? v : "<not a string>";
    json_decref(o);
    return s;
}

TEST_CASE("Self-spot request body, as pota.app's spot form sends it", "[pota]") {
    char *j = pota_spot_json("KR8MER", "US-1234", 14074000, MODE_FT8, "", "X6100-KR8MER");
    REQUIRE(j);
    CHECK(field(j, "activator") == "KR8MER");
    CHECK(field(j, "spotter") == "KR8MER");
    CHECK(field(j, "frequency") == "14074");     // kHz
    CHECK(field(j, "reference") == "US-1234");
    CHECK(field(j, "mode") == "FT8");
    CHECK(field(j, "source") == "X6100-KR8MER");
    CHECK(field(j, "comments") == POTA_SPOT_DEFAULT_COMMENT);   // a comment is required
    free(j);

    j = pota_spot_json("KR8MER", "US-1234", 7185550, MODE_SSB, "QRT 5 min", "X");
    CHECK(field(j, "frequency") == "7185.5");    // to 100 Hz, cut
    CHECK(field(j, "mode") == "SSB");
    CHECK(field(j, "comments") == "QRT 5 min");
    free(j);

    j = pota_spot_json("KR8MER", "US-1234", 14349990, MODE_CW, nullptr, "X");
    CHECK(field(j, "frequency") == "14349.9");   // not rounded past the band edge
    free(j);
}

TEST_CASE("Spot modes", "[pota]") {
    CHECK(std::string(pota_spot_mode(MODE_FT4)) == "FT4");
    CHECK(std::string(pota_spot_mode(MODE_CW)) == "CW");
    CHECK(std::string(pota_spot_mode(MODE_OTHER)).empty());
}

TEST_CASE("Park look-up reply", "[pota]") {
    pota_park_t p;
    const char *reply =
        "{\"parkId\": 1234, \"reference\": \"US-1234\", \"name\": \"Pearl Lake\", \"grid6\": \"DN60ns\","
        " \"parktypeDesc\": \"State Park\", \"locationDesc\": \"US-CO\"}";

    REQUIRE(pota_park_parse(reply, &p));
    CHECK(std::string(p.name) == "Pearl Lake");
    CHECK(std::string(p.location) == "US-CO");
    CHECK(std::string(p.type) == "State Park");
    CHECK(std::string(p.grid) == "DN60ns");
    CHECK_FALSE(pota_park_parse("null", &p));     // unknown park
    CHECK_FALSE(pota_park_parse("<html>", &p));
    CHECK_FALSE(pota_park_parse(nullptr, &p));
}
