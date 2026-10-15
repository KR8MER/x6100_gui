// HamQTH lookup and QRZ reply parsing
#include <catch2/catch_test_macros.hpp>

#include <cstring>
#include <string>

extern "C" {
#include "../src/callbook.h"
}

// The samples published at https://www.hamqth.com/developers (as given)
static const char *HQ_LOGIN_OK =
    "<?xml version=\"1.0\"?>\n"
    "<HamQTH version=\"2.8\" xmlns=\"https://www.hamqth.com\">\n"
    "<session>\n"
    "<session_id>09b0ae90050be03c452ad235a1f2915ad684393c</session_id>\n"
    "</session>\n"
    "</HamQTH>\n";

static const char *HQ_LOGIN_BAD =
    "<?xml version=\"1.0\"?>\n"
    "<HamQTH version=\"2.8\" xmlns=\"https://www.hamqth.com\">\n"
    "<session>\n"
    "<error>Wrong user name or password</error>\n"
    "</session> \n"
    "</HamQTH>\n";

static const char *HQ_EXPIRED =
    "<?xml version=\"1.0\"?>\n"
    "<HamQTH version=\"2.8\" xmlns=\"https://www.hamqth.com\">\n"
    "<session>\n"
    "<error>Session does not exist or expired</error>\n"
    "</session> \n"
    "</HamQTH>\n";

static const char *HQ_NOT_FOUND =
    "<?xml version=\"1.0\"?>\n"
    "<HamQTH version=\"2.8\" xmlns=\"https://www.hamqth.com\">\n"
    "<session>\n"
    "  <error>Callsign not found</error>\n"
    "</session>\n"
    "</HamQTH>\n";

static const char *HQ_SEARCH =
    "<?xml version=\"1.0\"?>\n"
    "<HamQTH version=\"2.8\" xmlns=\"https://www.hamqth.com\">\n"
    "<search> \n"
    "\t<callsign>ok2cqr</callsign>\n"
    "\t<nick>Petr</nick> \n"
    "\t<qth>Neratovice</qth> \n"
    "\t<country>Czech Republic</country>\n"
    "     \t<adif>503</adif>\n"
    "\t<itu>28</itu> \n"
    "\t<cq>15</cq> \n"
    "\t<grid>jo70gg</grid> \n"
    "\t<adr_name>Petr Hlozek</adr_name> \n"
    "\t<adr_street1>17. listopadu 1065</adr_street1> \n"
    "\t<adr_city>Neratovice</adr_city> \n"
    "\t<adr_zip>27711</adr_zip> \n"
    "\t<adr_country>Czech Republic</adr_country> \n"
    "     \t<adr_adif>503</adr_adif>\n"
    "\t<district>GZL</district>\n"
    "\t<lotw>Y</lotw> \n"
    "\t<qsl>Y</qsl> \n"
    "\t<qsldirect>Y</qsldirect> \n"
    "\t<eqsl>Y</eqsl> \n"
    "\t<email>petr@ok2cqr.com</email>\n"
    "\t<jabber>petr@ok2cqr.com</jabber>\n"
    "\t<skype>PetrHH</skype> \n"
    "\t<birth_year>1982</birth_year> \n"
    "\t<lic_year>1998</lic_year> \n"
    "\t<web>https://www.ok2cqr.com</web>\n"
    "\t<latitude>50.07</latitude>\n"
    "\t<longitude>14.42</longitude>\n"
    "\t<continent>EU</continent>\n"
    "        <utc_offset>-1</utc_offset>\n"
    "\t<picture>https://www.hamqth.com/userfiles/o/ok/ok2cqr/_profile/ok2cqr_nove.jpg</picture>\n"
    "    </search> \n"
    "    </HamQTH>\n";

TEST_CASE("HamQTH login reply", "[test_callbook]") {
    char id[64], err[64];

    CHECK(hamqth_parse_session(HQ_LOGIN_OK, id, sizeof(id), err, sizeof(err)) == CALLBOOK_OK);
    CHECK(std::string(id) == "09b0ae90050be03c452ad235a1f2915ad684393c");
    CHECK(err[0] == 0);

    CHECK(hamqth_parse_session(HQ_LOGIN_BAD, id, sizeof(id), err, sizeof(err)) == CALLBOOK_BAD_LOGIN);
    CHECK(id[0] == 0);
    CHECK(std::string(err) == "Wrong user name or password");

    CHECK(hamqth_parse_session(HQ_EXPIRED, id, sizeof(id), err, sizeof(err)) == CALLBOOK_EXPIRED);
    CHECK(std::string(err) == "Session does not exist or expired");

    // error text pointer may be NULL
    CHECK(hamqth_parse_session(HQ_LOGIN_BAD, id, sizeof(id), NULL, 0) == CALLBOOK_BAD_LOGIN);
}

TEST_CASE("HamQTH session id must fit and be plain", "[test_callbook]") {
    char id[16], err[64];

    // 40 chars into 16: refused rather than cut short
    CHECK(hamqth_parse_session(HQ_LOGIN_OK, id, sizeof(id), err, sizeof(err)) == CALLBOOK_ERROR);
    CHECK(id[0] == 0);
    CHECK(err[0] != 0);

    char big[64];
    CHECK(hamqth_parse_session("<HamQTH><session><session_id>ab&amp;cd</session_id></session></HamQTH>", big,
                               sizeof(big), err, sizeof(err)) == CALLBOOK_ERROR);
}

TEST_CASE("HamQTH search reply: published sample", "[test_callbook]") {
    callbook_info_t info;
    char            err[64] = "x";

    REQUIRE(hamqth_parse_search(HQ_SEARCH, &info, err, sizeof(err)) == CALLBOOK_OK);
    CHECK(std::string(info.callsign) == "OK2CQR");
    CHECK(std::string(info.nick) == "Petr");
    CHECK(std::string(info.qth) == "Neratovice");
    CHECK(std::string(info.grid) == "JO70gg");
    CHECK(std::string(info.country) == "Czech Republic");
    CHECK(err[0] == 0);
}

TEST_CASE("HamQTH search reply: errors", "[test_callbook]") {
    callbook_info_t info;
    char            err[64];

    CHECK(hamqth_parse_search(HQ_NOT_FOUND, &info, err, sizeof(err)) == CALLBOOK_NOT_FOUND);
    CHECK(std::string(err) == "Callsign not found");
    CHECK(info.nick[0] == 0);

    CHECK(hamqth_parse_search(HQ_EXPIRED, &info, err, sizeof(err)) == CALLBOOK_EXPIRED);

    CHECK(hamqth_parse_search("<HamQTH><session><error>Server busy</error></session></HamQTH>", &info, err,
                              sizeof(err)) == CALLBOOK_ERROR);
    CHECK(std::string(err) == "Server busy");

    // not HamQTH at all, empty, truncated
    CHECK(hamqth_parse_search("<html><body>502 Bad Gateway</body></html>", &info, err, sizeof(err)) ==
          CALLBOOK_ERROR);
    CHECK(err[0] != 0);
    CHECK(hamqth_parse_search("", &info, err, sizeof(err)) == CALLBOOK_ERROR);
    CHECK(hamqth_parse_search(NULL, &info, err, sizeof(err)) == CALLBOOK_ERROR);
    CHECK(hamqth_parse_search("<HamQTH><search><callsign>ok2", &info, err, sizeof(err)) == CALLBOOK_ERROR);
    CHECK(hamqth_parse_search("<HamQTH><search><callsign attr=\"a>b", &info, err, sizeof(err)) ==
          CALLBOOK_ERROR);
}

TEST_CASE("HamQTH search reply: missing tags, attributes, nested entries", "[test_callbook]") {
    callbook_info_t info;

    // only a callsign: the rest stays empty
    REQUIRE(hamqth_parse_search("<HamQTH><search><callsign>w1aw</callsign></search></HamQTH>", &info, NULL, 0) ==
            CALLBOOK_OK);
    CHECK(std::string(info.callsign) == "W1AW");
    CHECK(info.nick[0] == 0);
    CHECK(info.qth[0] == 0);
    CHECK(info.grid[0] == 0);
    CHECK(info.country[0] == 0);

    // attributes, comments, a self-closing tag; the <callsign> in a nested
    // logbook entry must not be taken for the station's own, nor <qth_x>
    const char *xml =
        "<?xml version='1.0'?><!-- hi -->"
        "<HamQTH version=\"2.8\" xmlns='https://www.hamqth.com'>"
        "<search kind=\"a>b\">"
        "  <logbook><data><callsign>AA3B</callsign><nick>Wrong</nick></data></logbook>"
        "  <qth_x>Nope</qth_x><picture/>"
        "  <callsign >k8abc</callsign>"
        "  <nick lang=\"en\"><![CDATA[Jo <Jo>]]></nick>"
        "  <qth>\n   Ann   Arbor \n</qth>"
        "  <grid>en82</grid>"
        "</search></HamQTH>";
    REQUIRE(hamqth_parse_search(xml, &info, NULL, 0) == CALLBOOK_OK);
    CHECK(std::string(info.callsign) == "K8ABC");
    CHECK(std::string(info.nick) == "Jo <Jo>");
    CHECK(std::string(info.qth) == "Ann   Arbor");
    CHECK(std::string(info.grid) == "EN82");
    CHECK(info.country[0] == 0);
}

TEST_CASE("XML entities", "[test_callbook]") {
    char out[64];

    auto text = [&](const char *s) {
        callbook_xml_text(s, strlen(s), out, sizeof(out));
        return std::string(out);
    };
    CHECK(text("Tom &amp; Jerry") == "Tom & Jerry");
    CHECK(text("&lt;a&gt; &quot;b&quot; &apos;c&apos;") == "<a> \"b\" 'c'");
    CHECK(text("&#65;&#x42;&#X43;") == "ABC");
    CHECK(text("M&#252;nchen") == "M\xC3\xBCnchen");
    CHECK(text("&#x20AC;5") == "\xE2\x82\xAC" "5");
    CHECK(text("&#x1F600;") == "\xF0\x9F\x98\x80");
    // unknown, unterminated or invalid entities are left as they are
    CHECK(text("a &nbsp; b") == "a &nbsp; b");
    CHECK(text("R&D") == "R&D");
    CHECK(text("&#;&#x;&#0;&#xD800;&#99999999999;") == "&#;&#x;&#0;&#xD800;&#99999999999;");
    CHECK(text("&amp") == "&amp");
    CHECK(text("  \t\n ") == "");

    callbook_info_t info;
    REQUIRE(hamqth_parse_search("<HamQTH><search><callsign>g4abc</callsign><qth>Stoke &amp; Trent</qth>"
                                "<country>C&#244;te d&apos;Ivoire</country></search></HamQTH>",
                                &info, NULL, 0) == CALLBOOK_OK);
    CHECK(std::string(info.qth) == "Stoke & Trent");
    CHECK(std::string(info.country) == "C\xC3\xB4te d'Ivoire");
}

TEST_CASE("Oversize values are cut short safely", "[test_callbook]") {
    callbook_info_t info;
    std::string     longname(500, 'N');
    std::string     xml = "<HamQTH><search><callsign>" + std::string(300, 'k') + "</callsign><nick>" + longname +
                      "</nick><qth>" + std::string(47, 'a') + "&#252;</qth></search></HamQTH>";

    REQUIRE(hamqth_parse_search(xml.c_str(), &info, NULL, 0) == CALLBOOK_OK);
    CHECK(strlen(info.callsign) == sizeof(info.callsign) - 1);
    CHECK(std::string(info.callsign) == std::string(sizeof(info.callsign) - 1, 'K'));
    CHECK(strlen(info.nick) == sizeof(info.nick) - 1);
    // the two-byte u-umlaut does not fit in the last byte: dropped, not split
    CHECK(std::string(info.qth) == std::string(47, 'a'));

    // raw UTF-8 cut at the end of the buffer: never half a character
    char out[6];
    const char *s = "abcd\xC3\xBC";     // 4 + 2 bytes into 5 usable
    callbook_xml_text(s, strlen(s), out, sizeof(out));
    CHECK(std::string(out) == "abcd");
    const char *t = "ab\xE2\x82\xAC\xE2\x82\xAC";
    callbook_xml_text(t, strlen(t), out, sizeof(out));
    CHECK(std::string(out) == "ab\xE2\x82\xAC");

    // tiny and zero-size buffers
    char one[1] = {'x'};
    callbook_xml_text("abc", 3, one, sizeof(one));
    CHECK(one[0] == 0);
    char none = 'x';
    callbook_xml_text("abc", 3, &none, 0);
    CHECK(none == 'x');

    char id[8], err[8];
    CHECK(hamqth_parse_session(HQ_LOGIN_BAD, id, sizeof(id), err, sizeof(err)) == CALLBOOK_BAD_LOGIN);
    CHECK(std::string(err) == "Wrong u");
}

TEST_CASE("QRZ Logbook replies", "[test_callbook]") {
    char id[32], why[128];

    CHECK(qrz_parse_reply("RESULT=OK&LOGID=130877825&COUNT=1", id, sizeof(id), why, sizeof(why)) == QRZ_RESULT_OK);
    CHECK(std::string(id) == "130877825");
    CHECK(why[0] == 0);

    // QRZ's own example spells it LOGIDS
    CHECK(qrz_parse_reply("RESULT=OK&LOGIDS=130877825&COUNT=1\n", id, sizeof(id), why, sizeof(why)) ==
          QRZ_RESULT_OK);
    CHECK(std::string(id) == "130877825");
    CHECK(qrz_parse_reply("COUNT=2&LOGIDS=11,12&RESULT=OK\r\n", id, sizeof(id), NULL, 0) == QRZ_RESULT_OK);
    CHECK(std::string(id) == "11");

    CHECK(qrz_parse_reply("RESULT=FAIL&REASON=Unable+to+add+QSO+to+database%3A+duplicate&EXTENDED=", id, sizeof(id),
                          why, sizeof(why)) == QRZ_RESULT_DUPLICATE);
    CHECK(std::string(why) == "Unable to add QSO to database: duplicate");
    CHECK(id[0] == 0);
    CHECK(qrz_parse_reply("result=fail&reason=Duplicate%20QSO", NULL, 0, why, sizeof(why)) ==
          QRZ_RESULT_DUPLICATE);

    CHECK(qrz_parse_reply("RESULT=FAIL&REASON=invalid+api+key+ABCD%26EF&COUNT=0", id, sizeof(id), why,
                          sizeof(why)) == QRZ_RESULT_FAIL);
    CHECK(std::string(why) == "invalid api key ABCD&EF");

    CHECK(qrz_parse_reply("RESULT=AUTH", id, sizeof(id), why, sizeof(why)) == QRZ_RESULT_FAIL);
    CHECK(std::string(why) == "API key not accepted");
    CHECK(qrz_parse_reply("RESULT=FAIL", id, sizeof(id), why, sizeof(why)) == QRZ_RESULT_FAIL);
    CHECK(std::string(why) == "No reason given");
    CHECK(qrz_parse_reply("<html>oops</html>", id, sizeof(id), why, sizeof(why)) == QRZ_RESULT_FAIL);
    CHECK(std::string(why) == "Unexpected reply from QRZ");
    CHECK(qrz_parse_reply("", id, sizeof(id), why, sizeof(why)) == QRZ_RESULT_FAIL);
    CHECK(qrz_parse_reply(NULL, id, sizeof(id), why, sizeof(why)) == QRZ_RESULT_FAIL);
    // "RESULT=OKAY" is not OK
    CHECK(qrz_parse_reply("RESULT=OKAY", id, sizeof(id), why, sizeof(why)) == QRZ_RESULT_FAIL);

    // a long reason is cut, a bad %-escape kept as it is, %00 dropped
    std::string reply = "RESULT=FAIL&REASON=" + std::string(400, 'x');
    CHECK(qrz_parse_reply(reply.c_str(), id, sizeof(id), why, sizeof(why)) == QRZ_RESULT_FAIL);
    CHECK(strlen(why) == sizeof(why) - 1);
    char small[16];
    CHECK(qrz_parse_reply("RESULT=FAIL&REASON=100%25+bad%zz%00x%4", NULL, 0, small, sizeof(small)) ==
          QRZ_RESULT_FAIL);
    CHECK(std::string(small) == "100% bad%zzx%4");
}

TEST_CASE("URL decoding", "[test_callbook]") {
    char out[32];

    CHECK(callbook_url_decode("a+b%2Cc%3d", 10, out, sizeof(out)) == 6);
    CHECK(std::string(out) == "a b,c=");
    // only len bytes are read
    CHECK(callbook_url_decode("abc%41", 4, out, sizeof(out)) == 4);
    CHECK(std::string(out) == "abc%");
    CHECK(callbook_url_decode("%C3%BC%C3%BC", 12, out, 4) == 2);
    CHECK(std::string(out) == "\xC3\xBC");
}
