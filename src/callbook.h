/*
 *  SPDX-License-Identifier: LGPL-2.1-or-later
 *
 *  Xiegu X6100 LVGL GUI
 *
 *  HamQTH call sign lookup and QRZ Logbook replies (parsing only).
 *
 *  No network, settings or GUI here, so the parsers can be tested on the
 *  host with the replies the services document. Every copy is bounded and
 *  always NUL-terminated; a value too long for its field is cut short at a
 *  character boundary.
 */

#pragma once

#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    CALLBOOK_OK,
    CALLBOOK_NOT_FOUND,     // "Callsign not found"
    CALLBOOK_EXPIRED,       // "Session does not exist or expired": log in again
    CALLBOOK_BAD_LOGIN,     // "Wrong user name or password"
    CALLBOOK_ERROR,         // any other error, or a reply we cannot read
} callbook_status_t;

/* What the log uses from a HamQTH <search> reply */
typedef struct {
    char callsign[16];
    char nick[32];          // the name used on the air
    char qth[48];
    char grid[12];          // "JO70gg" form
    char country[40];
} callbook_info_t;

/* Login reply: the session id, or the error HamQTH gave (error may be NULL) */
callbook_status_t hamqth_parse_session(const char *xml, char *session_id, size_t id_n, char *error, size_t error_n);

/* Search reply: fills info (all fields empty when absent), or the error */
callbook_status_t hamqth_parse_search(const char *xml, callbook_info_t *info, char *error, size_t error_n);

/* Text of an XML element with entities decoded (&amp; &lt; &gt; &quot;
 * &apos; &#NN; &#xHH;) and surrounding white space trimmed. Returns the
 * length written to dst. */
size_t callbook_xml_text(const char *src, size_t len, char *dst, size_t n);

typedef enum {
    QRZ_RESULT_OK,          // inserted
    QRZ_RESULT_DUPLICATE,   // already in the logbook: counts as uploaded
    QRZ_RESULT_FAIL,
} qrz_result_t;

/* QRZ Logbook reply ("RESULT=OK&LOGID=…&COUNT=1", "RESULT=FAIL&REASON=…").
 * logid and reason may be NULL; reason is URL-decoded. */
qrz_result_t qrz_parse_reply(const char *reply, char *logid, size_t logid_n, char *reason, size_t reason_n);

/* application/x-www-form-urlencoded value decoding ('+' and %XX) */
size_t callbook_url_decode(const char *src, size_t len, char *dst, size_t n);

#ifdef __cplusplus
}
#endif
