/*
 *  SPDX-License-Identifier: LGPL-2.1-or-later
 *
 *  Xiegu X6100 LVGL GUI
 */

#include "net_fetch.h"

#include <curl/curl.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>

#include "cfg/cfg_api.h"
#include "main.h"
#include "scheduler.h"

#define MAX_BODY (2 * 1024 * 1024)

/* Every failed request is noted here (DATA partition, download it from the
 * radio's web page), with curl's own detail */
#ifndef NET_LOG_PATH
#define NET_LOG_PATH "/mnt/net_errors.log"
#endif
#define NET_LOG_MAX  (64 * 1024)

static pthread_once_t  curl_once = PTHREAD_ONCE_INIT;
static pthread_mutex_t ua_mux    = PTHREAD_MUTEX_INITIALIZER;
static char            ua[128]   = "X6100-KR8MER/" VERSION;

static void curl_setup(void) {
    curl_global_init(CURL_GLOBAL_DEFAULT);
}

static size_t write_cb(char *ptr, size_t size, size_t nmemb, void *user) {
    net_result_t *r = user;
    size_t        n = size * nmemb;

    if (r->len + n > MAX_BODY) {
        return 0;
    }
    char *grown = realloc(r->body, r->len + n + 1);
    if (!grown) {
        return 0;
    }
    r->body = grown;
    memcpy(r->body + r->len, ptr, n);
    r->len += n;
    r->body[r->len] = 0;
    return n;
}

/* "www.hamqth.com" from the URL */
static void host_of(const char *url, char *out, size_t n) {
    const char *p = strstr(url, "://");
    p             = p ? p + 3 : url;
    size_t len    = strcspn(p, "/:?");
    snprintf(out, n, "%.*s", (int)(len < 60 ? len : 60), p);
}

/* What went wrong, in words: the first part says which kind of failure */
static void describe(CURLcode rc, const char *detail, const char *url, char *out, size_t n) {
    char        host[64];
    const char *why = detail[0] ? detail : curl_easy_strerror(rc);

    host_of(url, host, sizeof(host));
    switch (rc) {
        case CURLE_COULDNT_RESOLVE_HOST:
            snprintf(out, n, "No internet: can't look up %s (Wi-Fi connected?)", host);
            break;
        case CURLE_COULDNT_CONNECT:
        case CURLE_OPERATION_TIMEDOUT:
            snprintf(out, n, "Can't reach %s: %s", host, why);
            break;
        case CURLE_PEER_FAILED_VERIFICATION:
        case CURLE_SSL_CONNECT_ERROR:
        case CURLE_SSL_CACERT_BADFILE:
        case CURLE_SSL_CERTPROBLEM:
            snprintf(out, n, "Secure link to %s failed: %s", host, why);
            break;
        default:
            snprintf(out, n, "%s: %s", host, why);
            break;
    }
    /* one line */
    for (char *c = out; *c; c++) {
        if ((unsigned char)*c < ' ' || *c == '#') *c = ' ';
    }
}

static void log_failure(const char *url, CURLcode rc, long http, const char *detail) {
    struct stat st;
    if (stat(NET_LOG_PATH, &st) == 0 && st.st_size > NET_LOG_MAX) {
        rename(NET_LOG_PATH, NET_LOG_PATH ".old");
    }
    FILE *f = fopen(NET_LOG_PATH, "a");
    if (!f) return;
    time_t    now = time(NULL);
    struct tm tm;
    char      when[24];
    gmtime_r(&now, &tm);
    strftime(when, sizeof(when), "%Y-%m-%dT%H:%M:%SZ", &tm);
    fprintf(f, "%s %s curl=%d (%s) http=%ld %s\n", when, url, (int)rc, curl_easy_strerror(rc), http, detail);
    fclose(f);
}

static bool request(const char *url, const char *form, const char *ctype, net_result_t *res) {
    char agent[sizeof(ua)];

    memset(res, 0, sizeof(*res));
    pthread_once(&curl_once, curl_setup);
    pthread_mutex_lock(&ua_mux);
    memcpy(agent, ua, sizeof(agent));
    pthread_mutex_unlock(&ua_mux);

    CURL *curl = curl_easy_init();
    if (!curl) {
        snprintf(res->error, sizeof(res->error), "Network unavailable");
        return false;
    }
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, agent);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_cb);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, res);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 20L);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 8L);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);
    curl_easy_setopt(curl, CURLOPT_CAINFO, NET_CA_BUNDLE);
    curl_easy_setopt(curl, CURLOPT_CAPATH, NET_CA_PATH);
    char detail[CURL_ERROR_SIZE] = "";
    curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, detail);
    struct curl_slist *headers = NULL;
    if (form) {
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, form);
    }
    if (ctype) {
        char h[96];
        snprintf(h, sizeof(h), "Content-Type: %s", ctype);
        headers = curl_slist_append(headers, h);
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    }

    CURLcode rc = curl_easy_perform(curl);
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &res->http);
    curl_easy_cleanup(curl);
    curl_slist_free_all(headers);

    if (rc != CURLE_OK) {
        describe(rc, detail, url, res->error, sizeof(res->error));
        log_failure(url, rc, res->http, detail);
    } else if (res->http < 200 || res->http > 299) {
        /* the reply often says why (POTA: "Invalid park reference") */
        if (res->body && res->body[0]) {
            snprintf(res->error, sizeof(res->error), "Server said %ld: %.80s", res->http, res->body);
            /* one line, and no LVGL recolour marks */
            for (char *c = res->error; *c; c++) {
                if ((unsigned char)*c < ' ' || *c == '#') *c = ' ';
            }
        } else {
            snprintf(res->error, sizeof(res->error), "Server said %ld", res->http);
        }
    } else if (!res->body) {
        snprintf(res->error, sizeof(res->error), "Empty reply");
    } else {
        return true;
    }
    free(res->body);
    res->body = NULL;
    res->len  = 0;
    return false;
}

bool net_get(const char *url, net_result_t *res) {
    return request(url, NULL, NULL, res);
}

bool net_post_form(const char *url, const char *form, net_result_t *res) {
    return request(url, form ? form : "", NULL, res);
}

bool net_post_json(const char *url, const char *json, net_result_t *res) {
    return request(url, json ? json : "{}", "application/json", res);
}

void net_result_free(net_result_t *res) {
    free(res->body);
    res->body = NULL;
    res->len  = 0;
}

char *net_escape(const char *text) {
    pthread_once(&curl_once, curl_setup);
    char *e = curl_easy_escape(NULL, text ? text : "", 0);
    if (!e) {
        return NULL;
    }
    char *copy = strdup(e);   // free() for callers, not curl_free()
    curl_free(e);
    return copy;
}

void net_user_agent(char *buf, size_t n) {
    pthread_mutex_lock(&ua_mux);
    snprintf(buf, n, "%s", ua);
    pthread_mutex_unlock(&ua_mux);
}

/* ---- async ---- */

typedef struct {
    void (*work)(void *);
    void (*done)(void *);
    void *ctx;
} job_t;

static void job_done(void *arg) {
    job_t *job = *(job_t **)arg;
    job->done(job->ctx);
    free(job);
}

static void *job_thread(void *arg) {
    job_t *job = arg;

    pthread_detach(pthread_self());
    job->work(job->ctx);
    scheduler_put(job_done, &job, sizeof(job));
    return NULL;
}

/* Settings are read here, on the LVGL thread, never from a worker */
static void refresh_user_agent(void) {
    char call[24] = "";

    param_t_get_into(cfg.callsign(), call, sizeof(call));
    pthread_mutex_lock(&ua_mux);
    if (call[0]) {
        snprintf(ua, sizeof(ua), "X6100-KR8MER/%s (%s)", VERSION, call);
    } else {
        snprintf(ua, sizeof(ua), "X6100-KR8MER/%s", VERSION);
    }
    pthread_mutex_unlock(&ua_mux);
}

bool net_async(void (*work)(void *ctx), void (*done)(void *ctx), void *ctx) {
    job_t    *job = malloc(sizeof(*job));
    pthread_t thr;

    if (!job) {
        return false;
    }
    refresh_user_agent();
    job->work = work;
    job->done = done;
    job->ctx  = ctx;
    if (pthread_create(&thr, NULL, job_thread, job) != 0) {
        free(job);
        return false;
    }
    return true;
}
