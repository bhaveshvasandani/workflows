/* SPDX-License-Identifier: Apache-2.0 */
/*
 * Copyright (C) 2015-2020 Micron Technology, Inc.  All rights reserved.
 */

#include <assert.h>

#include <hse_util/rest_client.h>
#include <hse_util/hse_err.h>
#include <hse_util/inttypes.h>
#include <hse_util/yaml.h>
#include <hse_util/rest_api.h>
#include <hse_util/timing.h>
#include <hse_util/parse_num.h>

#include <mpool/mpool.h>

#include <hse/hse.h>
#include <hse/hse_experimental.h> /* need 'hse_mpool_utype' */

static int
rest_kvs_list(struct yaml_context *yc, const char *mpool)
{
    char      sock[PATH_MAX];
    char      url[PATH_MAX];
    char *    buf, *next;
    size_t    bufsz = (32 * 1024);
    char *    c;
    hse_err_t err;

    snprintf(url, sizeof(url), "mpool/%s", mpool);

    snprintf(sock, sizeof(sock), "%s/%s/%s.sock", REST_SOCK_ROOT, mpool, mpool);

    buf = calloc(1, bufsz);
    if (!buf)
        return -ENOMEM;

    err = merr_to_hse_err(curl_get(url, sock, buf, bufsz));
    if (err) {
        free(buf);
        return hse_err_to_errno(err);
    }

    next = buf;
    next[strlen(next) - 1] = '\0'; /* Get rid of the trailing newline char */

    c = strsep(&next, "\n"); /* Advance next past 'kvs_list:' */
    while (next) {
        char path[128];

        c = strsep(&next, "\n");
        while (*c != '-')
            c++;

        snprintf(path, sizeof(path), "%s/%s", mpool, c + 2);
        yaml_element_list(yc, path);
    }

    yaml_end_element(yc);
    free(buf);

    return 0;
}

static hse_err_t
kvdb_list_props(const char *mpool, struct hse_params *params, struct yaml_context *yc)
{
    struct hse_kvdb *hdl;
    unsigned int     kvs_cnt;
    char **          kvs_list;
    hse_err_t        err;
    char             path[128];
    int              i;

    err = hse_kvdb_open(mpool, params, &hdl);
    if (err) {
        if (hse_err_to_errno(err) != EBUSY && hse_err_to_errno(err) != ENODATA)
            return err;

        yaml_start_element_type(yc, "kvslist");
        err = rest_kvs_list(yc, mpool);
        yaml_end_element_type(yc);
        return err;
    }

    err = hse_kvdb_get_names(hdl, &kvs_cnt, &kvs_list);
    if (err) {
        hse_kvdb_close(hdl);
        return err;
    }

    yaml_start_element_type(yc, "kvslist");

    for (i = 0; i < kvs_cnt; i++) {
        snprintf(path, sizeof(path), "%s/%s", mpool, kvs_list[i]);
        yaml_element_list(yc, path);
    }

    yaml_end_element(yc);
    yaml_end_element_type(yc); /* kvslist */

    hse_kvdb_free_names(hdl, kvs_list);
    hse_kvdb_close(hdl);

    return err;
}

int
kvdb_list_print(
    const char *         mpname,
    struct hse_params *  params,
    struct yaml_context *yc,
    bool                 verbose,
    int *                count)
{
    struct mpool_params *propv = NULL;

    int       propc = 0;
    hse_err_t err;
    int       i;

    err = merr_to_hse_err(mpool_list(&propc, &propv));
    if (err)
        return hse_err_to_errno(err);

    *count = 0;
    for (i = 0; i < propc; i++) {
        const struct mpool_params *props = propv + i;

        if (uuid_compare(props->mp_utype, hse_mpool_utype))
            continue;

        if (mpname && strcmp(props->mp_name, mpname))
            continue;

        *count += 1;
        if (*count == 1)
            yaml_start_element_type(yc, "kvdbs");

        yaml_start_element(yc, "name", props->mp_name);

        if (!verbose) {
            yaml_end_element(yc);
            continue;
        }

        err = kvdb_list_props(props->mp_name, params, yc);
        if (err) {
            char buf[256];
            hse_err_to_string(err, buf, sizeof(buf), NULL);
            yaml_field_fmt(yc, "error", "\"kvdb_list_props failed: %s\"", buf);
        }

        yaml_end_element(yc);
    }

    if (*count)
        yaml_end_element_type(yc);

    hse_params_destroy(params);
    free(propv);

    return 0;
}

static hse_err_t
rest_kvdb_comp(const char *mpool, const char *kvdb, const char *policy)
{
    char      sock[PATH_MAX];
    char      url[PATH_MAX];
    char *    buf;
    size_t    bufsz = (4 * 1024);
    hse_err_t err;

    snprintf(url, sizeof(url), "mpool/%s/compact/request?policy=%s", mpool, policy);

    snprintf(sock, sizeof(sock), "%s/%s/%s.sock", REST_SOCK_ROOT, mpool, mpool);

    buf = calloc(1, bufsz);
    if (!buf)
        return ENOMEM;

    err = merr_to_hse_err(curl_put(url, sock, 0, 0, buf, bufsz));
    free(buf);

    return err;
}

static hse_err_t
rest_kvdb_status(const char *mpool, const char *kvdb, size_t bufsz, char *buf)
{
    char      sock[PATH_MAX];
    char      url[PATH_MAX];
    hse_err_t err;

    snprintf(url, sizeof(url), "mpool/%s/compact/status", mpool);

    snprintf(sock, sizeof(sock), "%s/%s/%s.sock", REST_SOCK_ROOT, mpool, mpool);

    err = merr_to_hse_err(curl_get(url, sock, buf, bufsz));
    if (err)
        return err;

    return 0UL;
}

static hse_err_t
rest_kvdb_params(const char *mpool, size_t bufsz, char *buf)
{
    char      sock[PATH_MAX];
    char      url[PATH_MAX];
    hse_err_t err;

    snprintf(url, sizeof(url), "data/config/kvdb/%s", mpool);

    snprintf(sock, sizeof(sock), "%s/%s/%s.sock", REST_SOCK_ROOT, mpool, mpool);

    err = merr_to_hse_err(curl_get(url, sock, buf, bufsz));
    if (err)
        return err;

    return 0UL;
}

/**
 * rest_status_parse() - A simple parser for converting status from yaml
 *                       struct hse_kvdb_compact_status.
 * @buf:    input buffer containing yaml.
 * @status: output status struct.
 */
static int
rest_status_parse(const char *buf, struct hse_kvdb_compact_status *status)
{
    /* Example contents of 'buf':
     *
     *   status:
     *   samp_lwm_pct: 117
     *   samp_hwm_pct: 137
     *   samp_curr_pct: 1371
     *   request_active: 0
     */

    struct {
        const char *  name;
        unsigned int *value;
    } items[] = {
        { "samp_lwm_pct:", &status->kvcs_samp_lwm },
        { "samp_hwm_pct:", &status->kvcs_samp_hwm },
        { "samp_curr_pct:", &status->kvcs_samp_curr },
        { "request_active:", &status->kvcs_active },
        { "request_canceled:", &status->kvcs_canceled },
    };

    hse_err_t err;

    memset(status, 0, sizeof(*status));

    for (int i = 0; i < NELEM(items); i++) {

        const char *p;
        char *      end;
        size_t      n;
        u64         v;

        p = strstr(buf, items[i].name);

        if (!p)
            return -1;

        /* name should be at buf[0] or preceded by whitespace */
        if (p != buf && p[-1] != '\n')
            return -2;

        /* skip over white space after name */
        n = strlen(items[i].name);
        p += n;
        p += strspn(p, " ");

        /* parse an integer */
        err = merr_to_hse_err(parse_u64_range(p, &end, 0, UINT_MAX, &v));
        if (err)
            return -3;
        if (*end != '\0' && *end != '\n')
            return -4;

        *items[i].value = (unsigned int)v;
    }

    return 0;
}

/**
 * rest_params_print() - Print the KVDB params
 * @mpool:  mpool name
 * @buf:    input buffer containing yaml
 */
static void
rest_params_print(const char *mpool, const char *buf)
{
    char        pfx[PATH_MAX], key[PATH_MAX];
    char        value[32];
    const char *p, *start;
    char *      pos;

    start = p = buf;

    snprintf(pfx, sizeof(pfx), "kvdb/%s/", mpool);

    while (*p != '\0') {
        start = strstr(p, "path:");
        if (!start)
            return;

        start += strlen("path:");
        start += strspn(start, " ");
        start += strlen(pfx);

        p = start;

        while (*p != '\0' && *p != ' ' && *p != '\n')
            p++;

        snprintf(key, sizeof(key), "%.*s", (int)(p - start), start);
        pos = strchr(key, '/');
        if (pos) {
            *pos = '.';
            printf("kvs.%s", key);
        } else {
            printf("kvdb.%s", key);
        }

        if (*p != '\0') {
            start = strstr(p, "current:");
            if (!start)
                return;

            start += strlen("current:");
            start += strspn(start, " ");
            p = start;

            while (*p != '\0' && *p != '\n')
                p++;

            snprintf(value, sizeof(value), "%.*s", (int)(p - start), start);
            printf(": %s\n", value);
        }
    }
}

static void
rest_status_yaml(struct hse_kvdb_compact_status *status, char *buf, size_t bufsz)
{
    struct yaml_context yc = {
        .yaml_indent = 0,
        .yaml_offset = 0,
        .yaml_buf = buf,
        .yaml_buf_sz = bufsz,
        .yaml_emit = NULL,
    };

    uint lwm = status->kvcs_samp_lwm;
    uint hwm = status->kvcs_samp_hwm;
    uint cur = status->kvcs_samp_curr;

    yaml_start_element_type(&yc, "compact_status");

    yaml_field_fmt(&yc, "samp_lwm", "%u.%02u", lwm / 100, lwm % 100);
    yaml_field_fmt(&yc, "samp_hwm", "%u.%02u", hwm / 100, hwm % 100);
    yaml_field_fmt(&yc, "samp_curr", "%u.%02u", cur / 100, cur % 100);
    yaml_field_fmt(&yc, "request_active", "%u", status->kvcs_active);
    yaml_field_fmt(&yc, "request_canceled", "%u", status->kvcs_canceled);

    yaml_end_element(&yc);
    yaml_end_element_type(&yc);
}

int
kvdb_compact_request(
    const char *       mpool,
    struct hse_params *params,
    const char *       request_type,
    u32                timeout_sec)
{
    hse_err_t                      err;
    struct hse_kvdb *              handle = 0;
    struct hse_kvdb_compact_status status;

    char   stat_buf[256];
    u64    stop_ts;
    uint   sleep_secs = 2;
    char **kvs_list;
    uint   kvs_cnt;

    err = hse_kvdb_open(mpool, params, &handle);
    if (err) {
        handle = 0;
        if (hse_err_to_errno(err) != EBUSY) {
            char buf[256];
            hse_err_to_string(err, buf, sizeof(buf), NULL);
            fprintf(stderr, "kvdb open %s failed: %s\n", mpool, buf);
            goto err_out;
        }
    }

    /* If this process has opened the KVDB, open all KVSes too. */
    if (handle) {
        int i;

        err = hse_err_to_errno(hse_kvdb_get_names(handle, &kvs_cnt, &kvs_list));
        if (err) {
            char buf[256];
            hse_err_to_string(err, buf, sizeof(buf), NULL);
            fprintf(stderr, "unable to get %s kvs names: %s\n", mpool, buf);
            goto err_out;
        }

        for (i = 0; i < kvs_cnt; i++) {
            struct hse_kvs *k;

            err = hse_kvdb_kvs_open(handle, kvs_list[i], 0, &k);
            if (err) {
                char buf[256];
                hse_err_to_string(err, buf, sizeof(buf), NULL);
                fprintf(stderr, "kvs open %s failed: %s\n", kvs_list[i], buf);
                goto err_out;
            }
        }
    }

    if (strcmp(request_type, "request") == 0) {

        const char *policy = "samp_lwm";

        printf("issuing compaction request with timeout of %u seconds\n", timeout_sec);

        err = rest_kvdb_comp(mpool, mpool, policy);
        if (err) {
            char buf[256];
            hse_err_to_string(err, buf, sizeof(buf), NULL);
            fprintf(stderr, "rest_kvdb_comp(%s) failed: %s\n", policy, buf);
            goto err_out;
        }

        err = rest_kvdb_status(mpool, mpool, sizeof(stat_buf), stat_buf);
        if (err) {
            char buf[256];
            hse_err_to_string(err, buf, sizeof(buf), NULL);
            fprintf(stderr, "rest_kvdb_status failed: %s\n", buf);
            goto err_out;
        }

        rest_status_parse(stat_buf, &status);

        if (!status.kvcs_active) {
            printf("no active compaction request in progress\n");
            err = 0;
            goto err_out;
        }

        stop_ts = get_time_ns() + (timeout_sec * 1000ul * 1000ul * 1000ul);
        while (status.kvcs_active) {

            err = rest_kvdb_status(mpool, mpool, sizeof(stat_buf), stat_buf);
            if (err) {
                char buf[256];
                hse_err_to_string(err, buf, sizeof(buf), NULL);
                fprintf(stderr, "rest_kvdb_status failed: %s\n", buf);
                goto err_out;
            }

            rest_status_parse(stat_buf, &status);

            if (status.kvcs_active)
                sleep(sleep_secs);

            if (get_time_ns() > stop_ts) {

                fprintf(stderr, "compact kvdb %s timed out\n", mpool);

                err = rest_kvdb_comp(mpool, mpool, "cancel");
                if (err) {
                    char buf[256];
                    hse_err_to_string(err, buf, sizeof(buf), NULL);
                    fprintf(stderr, "rest_kvdb_comp cancel failed: %s\n", buf);
                } else {
                    err = ETIMEDOUT;
                }
                goto err_out;
            }
        }

        printf("compact kvdb %s %s\n", mpool, status.kvcs_canceled ? "canceled" : "successful");

    } else if (strcmp(request_type, "cancel") == 0) {

        err = rest_kvdb_comp(mpool, mpool, "cancel");
        if (err) {
            char buf[256];
            hse_err_to_string(err, buf, sizeof(buf), NULL);
            fprintf(stderr, "rest_kvdb_comp cancel failed: %s\n", buf);
            goto err_out;
        }

        printf("compact kvdb %s canceled\n", mpool);

    } else if (strcmp(request_type, "status") == 0) {

        err = rest_kvdb_status(mpool, mpool, sizeof(stat_buf), stat_buf);
        if (err) {
            char buf[256];
            hse_err_to_string(err, buf, sizeof(buf), NULL);
            fprintf(stderr, "rest_kvdb_status failed: %s\n", buf);
            goto err_out;
        }

        rest_status_parse(stat_buf, &status);

        rest_status_yaml(&status, stat_buf, sizeof(stat_buf));

        printf("%s", stat_buf);
    }

err_out:
    if (handle) {
        hse_kvdb_free_names(handle, kvs_list);
        hse_kvdb_close(handle);
    }

    hse_params_destroy(params);

    return hse_err_to_errno(err);
}

int
hse_kvdb_params(const char *kvdb, bool get)
{
    bool                 match = false;
    hse_err_t            err = 0;
    char *               buf;
    size_t               bufsz = (32 * 1024);
    int                  allc, i;
    struct mpool_params *allv = 0;

    err = mpool_list(&allc, &allv);
    if (err) {
        char buf[256];
        hse_err_to_string(err, buf, sizeof(buf), NULL);
        fprintf(stderr, "mpool_list failed: %s\n", buf);
        return -EINVAL;
    }

    for (i = 0; i < allc; ++i) {
        if (!strcmp(allv[i].mp_name, kvdb)) {
            match = true;
            break;
        }
    }

    /* free mem allocated in mpool_list() */
    free(allv);
    allv = 0;

    if (!match) {
        err = mpool_scan(&allc, &allv);
        if (err) {
            char buf[256];
            hse_err_to_string(err, buf, sizeof(buf), NULL);
            fprintf(stderr, "mpool_scan failed: %s\n", buf);
            return -EINVAL;
        }

        for (i = 0; i < allc; ++i) {

            if (!strcmp(allv[i].mp_name, kvdb)) {
                match = true;
                break;
            }
        }

        /* free mem allocated n mpool_scan() */
        free(allv);
        allv = 0;

        if (!match) {
            fprintf(stderr, "KVDB '%s' not found\n", kvdb);
        } else {
            fprintf(
                stderr,
                "Mpool '%s' is inactive. Must activate the mpool prior to accessing the KVDB.\n"
                "Try: mpool activate '%s'\n",
                kvdb,
                kvdb);
        }

        return -EINVAL;
    }

    buf = calloc(1, bufsz);
    if (!buf)
        return -ENOMEM;

    if (get) {
        err = rest_kvdb_params(kvdb, bufsz, buf);
        if (err) {
            char buf[256];
            hse_err_to_string(err, buf, sizeof(buf), NULL);
            fprintf(
                stderr,
                "rest_kvdb_params failed: %s\n"
                "Ensure that KVDB '%s' is open in a process before querying its params.\n",
                buf,
                kvdb);
            goto err_out;
        }

        rest_params_print(kvdb, buf);
    }

err_out:
    free(buf);

    return hse_err_to_errno(err);
}
