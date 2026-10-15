/*
 *  SPDX-License-Identifier: LGPL-2.1-or-later
 *
 *  Xiegu X6100 LVGL GUI
 *
 *  HamQTH call sign lookup and QRZ Logbook replies
 *
 *  HamQTH's XML is small and flat, so instead of an XML library a tiny
 *  scanner walks it: it finds a child element by name at the top level of
 *  its parent (so the <callsign> inside a <logbook><data> entry is never
 *  taken for the station's own), skipping comments, CDATA, processing
 *  instructions and attributes.
 */

#include "callbook.h"

#include <stdint.h>
#include <string.h>

typedef struct {
    const char *p;
    const char *end;
} span_t;

/* ---- bounded output ---- */

typedef struct {
    char  *buf;
    size_t n;       // buffer size, room for the NUL included
    size_t len;
    bool   full;
} out_t;

static void out_init(out_t *o, char *buf, size_t n) {
    o->buf  = buf;
    o->n    = n;
    o->len  = 0;
    o->full = (n == 0);
    if (n) buf[0] = 0;
}

/* All of the bytes or none, so an entity's UTF-8 sequence is never split */
static void out_put(out_t *o, const char *s, size_t k) {
    if (o->full) return;
    if (o->len + k + 1 > o->n) {
        o->full = true;
        return;
    }
    memcpy(o->buf + o->len, s, k);
    o->len += k;
    o->buf[o->len] = 0;
}

/* A raw byte that ran out of room may have cut a UTF-8 character in two:
 * drop the incomplete tail, then trailing white space */
static size_t out_finish(out_t *o) {
    if (!o->n) return 0;
    if (o->full && o->len) {
        size_t i = o->len;
        while (i > 0 && (o->len - i) < 4 && ((unsigned char)o->buf[i - 1] & 0xC0) == 0x80) i--;
        if (i > 0) {
            unsigned char lead = (unsigned char)o->buf[i - 1];
            size_t        need = lead >= 0xF0 ? 4 : lead >= 0xE0 ? 3 : lead >= 0xC0 ? 2 : 1;
            if (o->len - (i - 1) < need) o->len = i - 1;
        }
    }
    while (o->len && (o->buf[o->len - 1] == ' ' || o->buf[o->len - 1] == '\t' || o->buf[o->len - 1] == '\r' ||
                      o->buf[o->len - 1] == '\n')) {
        o->len--;
    }
    o->buf[o->len] = 0;
    return o->len;
}

static void copy_text(char *dst, size_t n, const char *src) {
    out_t o;
    out_init(&o, dst, n);
    /* byte by byte: when too long, as much as fits (out_finish then cuts
     * back to a character boundary) */
    for (const char *p = src; p && *p && !o.full; p++) out_put(&o, p, 1);
    out_finish(&o);
}

/* ---- small string helpers (no locale) ---- */

static char lower(char c) {
    return (c >= 'A' && c <= 'Z') ? (char)(c - 'A' + 'a') : c;
}

static char upper(char c) {
    return (c >= 'a' && c <= 'z') ? (char)(c - 'a' + 'A') : c;
}

static bool is_space(char c) {
    return c == ' ' || c == '\t' || c == '\r' || c == '\n';
}

static bool contains_nocase(const char *s, const char *word) {
    size_t w = strlen(word);
    for (; *s; s++) {
        size_t i = 0;
        while (i < w && s[i] && lower(s[i]) == lower(word[i])) i++;
        if (i == w) return true;
    }
    return false;
}

static bool equals_nocase(const char *a, size_t a_len, const char *b) {
    size_t i = 0;
    for (; i < a_len && b[i]; i++) {
        if (lower(a[i]) != lower(b[i])) return false;
    }
    return i == a_len && !b[i];
}

static const char *find(const char *p, const char *end, const char *what) {
    size_t w = strlen(what);
    for (; p + w <= end; p++) {
        if (memcmp(p, what, w) == 0) return p;
    }
    return NULL;
}

static bool starts(const char *p, const char *end, const char *what) {
    size_t w = strlen(what);
    return (size_t)(end - p) >= w && memcmp(p, what, w) == 0;
}

/* ---- XML scanner ---- */

/* p is at '<' of a comment, CDATA, <?…?> or <!…>: the position after it,
 * or NULL when it never ends */
static const char *skip_special(const char *p, const char *end) {
    const char *e;

    if (starts(p, end, "<!--")) {
        e = find(p + 4, end, "-->");
        return e ? e + 3 : NULL;
    }
    if (starts(p, end, "<![CDATA[")) {
        e = find(p + 9, end, "]]>");
        return e ? e + 3 : NULL;
    }
    if (starts(p, end, "<?")) {
        e = find(p + 2, end, "?>");
        return e ? e + 2 : NULL;
    }
    e = memchr(p, '>', end - p);
    return e ? e + 1 : NULL;
}

typedef struct {
    bool        closing;
    bool        self_closing;
    const char *name;
    size_t      name_len;
    const char *after;  // first char after '>'
} tag_t;

/* p is at the '<' of an ordinary tag. Attribute values may hold '>' */
static bool parse_tag(const char *p, const char *end, tag_t *t) {
    const char *q = p + 1;
    char        quote = 0;

    t->closing = q < end && *q == '/';
    if (t->closing) q++;
    t->name = q;
    while (q < end && !is_space(*q) && *q != '>' && *q != '/') q++;
    t->name_len = q - t->name;
    for (; q < end; q++) {
        if (quote) {
            if (*q == quote) quote = 0;
        } else if (*q == '"' || *q == '\'') {
            quote = *q;
        } else if (*q == '>') {
            t->self_closing = !t->closing && q[-1] == '/';
            t->after        = q + 1;
            return t->name_len > 0;
        }
    }
    return false;
}

/* The content of the first element called `name` at the top level of `in` */
static bool child(span_t in, const char *name, span_t *out) {
    const char *q       = in.p;
    const char *content = NULL;
    int         depth   = 0;
    size_t      name_n  = strlen(name);

    while (q && q < in.end) {
        q = memchr(q, '<', in.end - q);
        if (!q) break;
        if (q + 1 < in.end && (q[1] == '!' || q[1] == '?')) {
            q = skip_special(q, in.end);
            continue;
        }
        tag_t t = {0};
        if (!parse_tag(q, in.end, &t)) return false;

        bool match = t.name_len == name_n && memcmp(t.name, name, name_n) == 0;
        if (t.closing) {
            if (content && depth == 1) {
                out->p   = content;
                out->end = q;
                return true;
            }
            if (--depth < 0) return false;
        } else if (t.self_closing) {
            if (depth == 0 && match) {
                out->p = out->end = t.after;
                return true;
            }
        } else {
            if (depth == 0 && match) content = t.after;
            depth++;
        }
        q = t.after;
    }
    return false;
}

static int hex_val(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return -1;
}

static size_t utf8_encode(uint32_t cp, char *u) {
    if (cp < 0x80) {
        u[0] = (char)cp;
        return 1;
    }
    if (cp < 0x800) {
        u[0] = (char)(0xC0 | (cp >> 6));
        u[1] = (char)(0x80 | (cp & 0x3F));
        return 2;
    }
    if (cp < 0x10000) {
        u[0] = (char)(0xE0 | (cp >> 12));
        u[1] = (char)(0x80 | ((cp >> 6) & 0x3F));
        u[2] = (char)(0x80 | (cp & 0x3F));
        return 3;
    }
    u[0] = (char)(0xF0 | (cp >> 18));
    u[1] = (char)(0x80 | ((cp >> 12) & 0x3F));
    u[2] = (char)(0x80 | ((cp >> 6) & 0x3F));
    u[3] = (char)(0x80 | (cp & 0x3F));
    return 4;
}

/* p is at '&': the decoded character and how much input it used, 0 when
 * it is not an entity we know (then the '&' is kept as it is) */
static size_t entity(const char *p, const char *end, char *u, size_t *u_len) {
    static const struct {
        const char *name;
        char        c;
    } named[] = {{"&amp;", '&'}, {"&lt;", '<'}, {"&gt;", '>'}, {"&quot;", '"'}, {"&apos;", '\''}};

    for (size_t i = 0; i < sizeof(named) / sizeof(named[0]); i++) {
        if (starts(p, end, named[i].name)) {
            u[0]   = named[i].c;
            *u_len = 1;
            return strlen(named[i].name);
        }
    }
    if (!starts(p, end, "&#")) return 0;

    const char *q   = p + 2;
    bool        hex = q < end && (*q == 'x' || *q == 'X');
    uint32_t    cp  = 0;
    int         digits = 0;

    if (hex) q++;
    for (; q < end && *q != ';'; q++, digits++) {
        int v = hex ? hex_val(*q) : (*q >= '0' && *q <= '9' ? *q - '0' : -1);
        if (v < 0 || digits >= 8) return 0;
        cp = cp * (hex ? 16 : 10) + v;
    }
    if (q >= end || !digits) return 0;
    /* no NUL, surrogates or values past Unicode */
    if (cp == 0 || (cp >= 0xD800 && cp <= 0xDFFF) || cp > 0x10FFFF) return 0;
    *u_len = utf8_encode(cp, u);
    return q + 1 - p;
}

size_t callbook_xml_text(const char *src, size_t len, char *dst, size_t n) {
    const char *p   = src;
    const char *end = src + len;
    out_t       o;

    out_init(&o, dst, n);
    if (!src) return out_finish(&o);
    while (p < end && is_space(*p)) p++;

    while (p < end && !o.full) {
        if (*p == '<') {
            if (starts(p, end, "<![CDATA[")) {
                const char *e = find(p + 9, end, "]]>");
                const char *s = p + 9;
                if (!e) e = end;
                for (; s < e && !o.full; s++) out_put(&o, s, 1);
                p = e + (e < end ? 3 : 0);
                continue;
            }
            /* markup inside a value is not text */
            const char *e = (p + 1 < end && (p[1] == '!' || p[1] == '?')) ? skip_special(p, end) : NULL;
            if (!e) {
                e = memchr(p, '>', end - p);
                e = e ? e + 1 : end;
            }
            p = e;
            continue;
        }
        if (*p == '&') {
            char   u[4];
            size_t u_len = 0;
            size_t used  = entity(p, end, u, &u_len);
            if (used) {
                out_put(&o, u, u_len);
                p += used;
                continue;
            }
        }
        out_put(&o, p, 1);
        p++;
    }
    return out_finish(&o);
}

/* Text of the element at the end of a path of child names, "" when absent */
static bool path_text(span_t in, const char *const *path, char *dst, size_t n) {
    span_t s = in;

    if (n) dst[0] = 0;
    for (; *path; path++) {
        if (!child(s, *path, &s)) return false;
    }
    callbook_xml_text(s.p, s.end - s.p, dst, n);
    return true;
}

static callbook_status_t classify(const char *error) {
    if (contains_nocase(error, "not found")) return CALLBOOK_NOT_FOUND;
    if (contains_nocase(error, "expired") || contains_nocase(error, "does not exist")) return CALLBOOK_EXPIRED;
    if (contains_nocase(error, "wrong user") || contains_nocase(error, "password")) return CALLBOOK_BAD_LOGIN;
    return CALLBOOK_ERROR;
}

/* <HamQTH><session><error>…</error></session></HamQTH> */
static callbook_status_t session_error(span_t root, char *error, size_t error_n) {
    static const char *const path[] = {"session", "error", NULL};
    char                     text[128];

    if (path_text(root, path, text, sizeof(text)) && text[0]) {
        if (error) copy_text(error, error_n, text);
        return classify(text);
    }
    if (error) copy_text(error, error_n, "Unexpected reply from HamQTH");
    return CALLBOOK_ERROR;
}

static bool root_of(const char *xml, span_t *root) {
    span_t all;

    if (!xml) return false;
    all.p   = xml;
    all.end = xml + strlen(xml);
    return child(all, "HamQTH", root);
}

callbook_status_t hamqth_parse_session(const char *xml, char *session_id, size_t id_n, char *error, size_t error_n) {
    static const char *const path[] = {"session", "session_id", NULL};
    span_t                   root;
    char                     id[128];

    if (id_n) session_id[0] = 0;
    if (error && error_n) error[0] = 0;
    if (!root_of(xml, &root)) {
        if (error) copy_text(error, error_n, "Unexpected reply from HamQTH");
        return CALLBOOK_ERROR;
    }
    if (path_text(root, path, id, sizeof(id)) && id[0]) {
        /* a cut-short or odd id would only fail later: refuse it now */
        bool ok = strlen(id) < id_n;
        for (const char *p = id; *p && ok; p++) {
            ok = (*p >= '0' && *p <= '9') || (*p >= 'a' && *p <= 'z') || (*p >= 'A' && *p <= 'Z');
        }
        if (!ok) {
            if (error) copy_text(error, error_n, "Bad session id from HamQTH");
            return CALLBOOK_ERROR;
        }
        copy_text(session_id, id_n, id);
        return CALLBOOK_OK;
    }
    return session_error(root, error, error_n);
}

/* Field and square in capitals, subsquare in small letters: "JO70gg" */
static void normalize_grid(char *g) {
    for (size_t i = 0; g[i]; i++) {
        if (i < 2) {
            g[i] = upper(g[i]);
        } else if (i >= 4 && i < 6) {
            g[i] = lower(g[i]);
        }
    }
}

callbook_status_t hamqth_parse_search(const char *xml, callbook_info_t *info, char *error, size_t error_n) {
    span_t root, search, s;

    memset(info, 0, sizeof(*info));
    if (error && error_n) error[0] = 0;
    if (!root_of(xml, &root)) {
        if (error) copy_text(error, error_n, "Unexpected reply from HamQTH");
        return CALLBOOK_ERROR;
    }
    if (!child(root, "search", &search)) {
        return session_error(root, error, error_n);
    }

    if (child(search, "callsign", &s)) callbook_xml_text(s.p, s.end - s.p, info->callsign, sizeof(info->callsign));
    if (child(search, "nick", &s)) callbook_xml_text(s.p, s.end - s.p, info->nick, sizeof(info->nick));
    if (child(search, "qth", &s)) callbook_xml_text(s.p, s.end - s.p, info->qth, sizeof(info->qth));
    if (child(search, "grid", &s)) callbook_xml_text(s.p, s.end - s.p, info->grid, sizeof(info->grid));
    if (child(search, "country", &s)) callbook_xml_text(s.p, s.end - s.p, info->country, sizeof(info->country));

    for (char *p = info->callsign; *p; p++) *p = upper(*p);
    normalize_grid(info->grid);
    return CALLBOOK_OK;
}

/* ---- QRZ Logbook ---- */

size_t callbook_url_decode(const char *src, size_t len, char *dst, size_t n) {
    out_t o;

    out_init(&o, dst, n);
    for (size_t i = 0; src && i < len && !o.full; i++) {
        char c = src[i];
        if (c == '+') {
            c = ' ';
        } else if (c == '%' && i + 2 < len && hex_val(src[i + 1]) >= 0 && hex_val(src[i + 2]) >= 0) {
            c = (char)(hex_val(src[i + 1]) * 16 + hex_val(src[i + 2]));
            i += 2;
            if (c == 0) continue;   // never an early end of string
        }
        out_put(&o, &c, 1);
    }
    return out_finish(&o);
}

qrz_result_t qrz_parse_reply(const char *reply, char *logid, size_t logid_n, char *reason, size_t reason_n) {
    char result[16]  = "";
    char why[256]    = "";
    char id[32]      = "";
    bool have_result = false;

    if (logid && logid_n) logid[0] = 0;
    if (reason && reason_n) reason[0] = 0;

    for (const char *p = reply; p && *p;) {
        const char *amp = strchr(p, '&');
        const char *end = amp ? amp : p + strlen(p);
        const char *eq  = memchr(p, '=', end - p);

        if (eq) {
            const char *k = p, *k_end = eq;
            while (k < k_end && is_space(*k)) k++;
            while (k_end > k && is_space(k_end[-1])) k_end--;

            const char *v = eq + 1;
            size_t      v_len = end - v;
            size_t      k_len = k_end - k;

            if (equals_nocase(k, k_len, "RESULT")) {
                callbook_url_decode(v, v_len, result, sizeof(result));
                have_result = true;
            } else if (equals_nocase(k, k_len, "REASON")) {
                callbook_url_decode(v, v_len, why, sizeof(why));
            } else if (equals_nocase(k, k_len, "LOGID") || equals_nocase(k, k_len, "LOGIDS")) {
                callbook_url_decode(v, v_len, id, sizeof(id));
                char *comma = strchr(id, ',');   // LOGIDS may list several
                if (comma) *comma = 0;
            }
        }
        p = amp ? amp + 1 : end;
    }

    /* the value may carry the reply's trailing new line */
    char *r = result;
    while (is_space(*r)) r++;

    if (logid) copy_text(logid, logid_n, id);

    if (equals_nocase(r, strlen(r), "OK") || equals_nocase(r, strlen(r), "REPLACE")) {
        return QRZ_RESULT_OK;
    }
    if (equals_nocase(r, strlen(r), "FAIL") && contains_nocase(why, "duplicate")) {
        if (reason) copy_text(reason, reason_n, why);
        return QRZ_RESULT_DUPLICATE;
    }
    if (reason) {
        if (why[0]) {
            copy_text(reason, reason_n, why);
        } else if (!have_result) {
            copy_text(reason, reason_n, "Unexpected reply from QRZ");
        } else if (equals_nocase(r, strlen(r), "AUTH")) {
            copy_text(reason, reason_n, "API key not accepted");
        } else {
            copy_text(reason, reason_n, "No reason given");
        }
    }
    return QRZ_RESULT_FAIL;
}
