/* POTA activation in the QSO log: counts, dupes, 2-fer P2P entries and the
 * upload file, on a real SQLite log in a temporary file. */

#include <catch2/catch_test_macros.hpp>

#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <sstream>
#include <string>
#include <unistd.h>

extern "C" {
#include "../src/adif.h"
#include "../src/qso_log.h"

/* what qso_log.c calls outside the log */
void msg_update_text_fmt(const char *, ...) {}
void msg_schedule_text_fmt(const char *, ...) {}
void _lv_log_add(int, const char *, int, const char *, const char *, ...) {}
}

static const time_t DAY = 1792022400;   // 2026-10-15 00:00 UTC

static int add(const char *call, time_t t, qso_log_mode_t mode, uint64_t freq, const char *mine, const char *theirs) {
    qso_log_record_t r = qso_log_record_create("KR8MER", call, t, mode, 59, 59, freq, NULL, NULL, "EN71", NULL);
    if (mine) {
        snprintf(r.my_sig, sizeof r.my_sig, "POTA");
        snprintf(r.my_sig_info, sizeof r.my_sig_info, "%s", mine);
    }
    if (theirs) {
        snprintf(r.sig, sizeof r.sig, "POTA");
        snprintf(r.sig_info, sizeof r.sig_info, "%s", theirs);
    }
    return qso_log_record_save(r);
}

static std::string slurp(const char *path) {
    std::ifstream f(path);
    std::stringstream s;
    s << f.rdbuf();
    return s.str();
}

static int count_of(const std::string &s, const std::string &what) {
    int n = 0;
    for (size_t p = s.find(what); p != std::string::npos; p = s.find(what, p + 1)) n++;
    return n;
}

/* Catch2 runs the test once per SECTION: the log is made the first time */
static void make_log(void) {
    static bool made;
    if (made) return;
    made = true;
    unlink(QSO_LOG_DB_PATH);
    REQUIRE(qso_log_init());

    /* a 2-fer activation of US-1234 and US-5678 */
    const char *mine = "US-1234,US-5678";
    REQUIRE(add("W1AW", DAY + 3600, MODE_SSB, 14285000, mine, NULL) == 1);
    REQUIRE(add("W1AW/P", DAY + 3700, MODE_SSB, 14286000, mine, NULL) == 1);   // dupe: same call, band, phone
    REQUIRE(add("W1AW", DAY + 3800, MODE_CW, 14040000, mine, NULL) == 1);      // CW: counts again
    REQUIRE(add("W1AW", DAY + 3900, MODE_SSB, 7185000, mine, NULL) == 1);      // 40m: counts again
    REQUIRE(add("K8ABC", DAY + 4000, MODE_FT8, 14074000, mine, NULL) == 1);
    REQUIRE(add("K8ABC", DAY + 4100, MODE_FT4, 14080000, mine, NULL) == 1);    // FT4 after FT8: data dupe
    /* P2P with a 2-fer: one entry per park, same time */
    REQUIRE(add("N0P", DAY + 4200, MODE_SSB, 14290000, mine, "US-1111") == 1);
    REQUIRE(add("N0P", DAY + 4200, MODE_SSB, 14290000, mine, "US-2222") == 1);
    REQUIRE(add("N0P", DAY + 4200, MODE_SSB, 14290000, mine, "US-2222") == 0);  // the same entry again
    /* yesterday and elsewhere */
    REQUIRE(add("W2XYZ", DAY - 600, MODE_SSB, 14285000, mine, NULL) == 1);
    REQUIRE(add("W3XYZ", DAY + 5000, MODE_SSB, 14285000, "US-9999", NULL) == 1);
    REQUIRE(add("W4XYZ", DAY + 5100, MODE_SSB, 14285000, NULL, NULL) == 1);
}

TEST_CASE("POTA activation in the log", "[pota]") {
    make_log();
    time_t now = DAY + 50000;

    SECTION("count leaves out dupes, per park and UTC day") {
        CHECK(qso_log_activation_count("US-1234", now) == 5);   // W1AW x3 (ph 20, cw 20, ph 40), K8ABC, N0P
        CHECK(qso_log_activation_count("us-5678", now) == 5);
        CHECK(qso_log_activation_count("US-9999", now) == 1);
        CHECK(qso_log_activation_count("US-1234", DAY - 60) == 1);   // yesterday: W2XYZ
        CHECK(qso_log_activation_count("US-12", now) == 0);          // no partial match
    }

    SECTION("dupes") {
        CHECK(qso_log_activation_dupe("W1AW", BAND_20M, MODE_SSB, "US-1234", now));
        CHECK(qso_log_activation_dupe("W1AW/M", BAND_20M, MODE_FM, "US-5678", now));   // phone group
        CHECK(qso_log_activation_dupe("K8ABC", BAND_20M, MODE_RTTY, "US-1234", now));  // data group
        CHECK_FALSE(qso_log_activation_dupe("K8ABC", BAND_20M, MODE_CW, "US-1234", now));
        CHECK_FALSE(qso_log_activation_dupe("W1AW", BAND_17M, MODE_SSB, "US-1234", now));
        CHECK_FALSE(qso_log_activation_dupe("W2XYZ", BAND_20M, MODE_SSB, "US-1234", now));  // that was yesterday
    }

    SECTION("summary per band and mode group") {
        qso_log_band_count_t rows[8];
        int n = qso_log_activation_summary("US-1234", now, rows, 8);
        REQUIRE(n == 4);
        CHECK((rows[0].band == BAND_40M && rows[0].group == MODE_GROUP_PHONE && rows[0].count == 1));
        CHECK((rows[1].band == BAND_20M && rows[1].group == MODE_GROUP_CW && rows[1].count == 1));
        CHECK((rows[2].band == BAND_20M && rows[2].group == MODE_GROUP_PHONE && rows[2].count == 2));
        CHECK((rows[3].band == BAND_20M && rows[3].group == MODE_GROUP_DATA && rows[3].count == 1));
    }

    SECTION("upload file: one park, every QSO of the day at it") {
        const char *path = "test_pota_upload.adi";
        unlink(path);
        CHECK(qso_log_write_activation("US-5678", now, path) == 8);
        std::string f = slurp(path);
        CHECK(count_of(f, "<EOR>") == 8);
        CHECK(count_of(f, "<MY_SIG_INFO:7>US-5678") == 8);   // this park only, not the list
        CHECK(count_of(f, "<MY_SIG:4>POTA") == 8);
        CHECK(count_of(f, "US-1234") == 0);
        CHECK(count_of(f, "<SIG_INFO:7>US-1111") == 1);       // the P2P entries
        CHECK(count_of(f, "<SIG_INFO:7>US-2222") == 1);
        CHECK(count_of(f, "W2XYZ") == 0);                     // yesterday
        CHECK(count_of(f, "<QSO_DATE:8>20261015") == 8);
        CHECK(count_of(f, "<PROGRAMVERSION:") == 1);
        CHECK(count_of(f, ":0>") == 0);                       // no empty fields
        CHECK(access((std::string(path) + ".tmp").c_str(), F_OK) != 0);
        unlink(path);
        CHECK(qso_log_write_activation("US-4321", now, path) == 0);   // nothing at that park
        CHECK(access(path, F_OK) != 0);
        FILE *old = fopen(path, "w");                                  // a stale file goes
        REQUIRE(old);
        fclose(old);
        CHECK(qso_log_write_activation("US-4321", now, path) == 0);
        CHECK(access(path, F_OK) != 0);
    }

    SECTION("the whole log carries both parks, and reads back") {
        const char *path = "test_pota_all.adi";
        unlink(path);
        REQUIRE(qso_log_export_adif(path) == 11);   // 12 added, one repeat refused
        std::string f = slurp(path);
        CHECK(count_of(f, "<MY_SIG_INFO:7>US-1234") == 9);           // one value
        CHECK(count_of(f, "<MY_SIG_INFO:15>") == 0);
        CHECK(count_of(f, "<MY_POTA_REF:15>US-1234,US-5678") == 9);
        CHECK(count_of(f, "<POTA_REF:7>US-2222") == 1);

        qso_log_record_t *recs = NULL;   // adif_read allocates it
        int n = adif_read(path, &recs);
        REQUIRE(n >= 11);
        int with_park = 0;
        for (int i = 0; i < 11; i++) {
            if (std::string(recs[i].my_sig_info) == "US-1234,US-5678" && std::string(recs[i].my_sig) == "POTA") {
                with_park++;
            }
        }
        CHECK(with_park == 9);
        free(recs);
        unlink(path);
    }
}

TEST_CASE("QRZ gets a 2-fer P2P once", "[pota]") {
    make_log();
    qso_log_row_t rows[32];
    int before = qso_log_qrz_unsent_count();
    int n = qso_log_qrz_unsent(rows, 32);
    REQUIRE(n == before);
    CHECK(n == 10);   // 11 rows, the N0P 2-fer is one QSO
    int n0p = 0;
    for (int i = 0; i < n; i++) {
        if (std::string(rows[i].rec.remote_call) == "N0P") {
            n0p++;
            CHECK(qso_log_mark_qrz_sent(rows[i].id));
        }
    }
    CHECK(n0p == 1);
    CHECK(qso_log_qrz_unsent_count() == 9);   // both N0P rows are now sent
}

TEST_CASE("Worked-before history, however the call was written", "[log]") {
    make_log();
    qso_log_record_t last;
    CHECK(qso_log_history("W1AW", &last) == 4);
    CHECK(std::string(last.remote_call) == "W1AW");      // the latest: 40m SSB
    CHECK(last.band == BAND_40M);
    CHECK(qso_log_history("W1AW/QRP", nullptr) == 4);    // portable forms are the same station
    CHECK(qso_log_history("N0P", &last) == 2);           // the 2-fer P2P is two entries
    CHECK(qso_log_history("ZZ9ZZZ", &last) == 0);
}
