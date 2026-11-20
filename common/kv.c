/*
 * common/kv.c: cài đặt bộ phân tích cú pháp (Minh)
 */
#include "kv.h"

#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int split_words(char *line, char *argv[], int max)
{
    int argc = 0;
    char *save = NULL;
    /* strtok_r (có _r) giữ trạng thái trong biến save của mình.
     * strtok dùng biến tĩnh ẩn, nên lồng hai vòng strtok sẽ phá nhau. */
    for (char *w = strtok_r(line, " \t", &save); w; w = strtok_r(NULL, " \t", &save)) {
        if (argc >= max)
            return -1;
        argv[argc++] = w;
    }
    return argc;
}

const char *kv_find(int argc, char *argv[], const char *key)
{
    size_t klen = strlen(key);
    for (int i = 0; i < argc; i++) {
        /* Phải kiểm tra ký tự ngay sau key là '=', nếu không thì
         * tìm "Tp1" sẽ khớp nhầm "Tp10=5". */
        if (strncmp(argv[i], key, klen) == 0 && argv[i][klen] == '=')
            return argv[i] + klen + 1;
    }
    return NULL;
}

int kv_split_pair(char *word, char **key, char **val)
{
    char *eq = strchr(word, '=');
    if (!eq)
        return -1;
    *eq = '\0';
    *key = word;
    *val = eq + 1;
    return 0;
}

int kv_get_str(int argc, char *argv[], const char *key, char *out, size_t outsz)
{
    const char *v = kv_find(argc, argv, key);
    if (!v)
        return -1;
    if (strlen(v) >= outsz)
        return -2;
    memcpy(out, v, strlen(v) + 1);
    return 0;
}

int kv_get_double(int argc, char *argv[], const char *key, double *out)
{
    const char *v = kv_find(argc, argv, key);
    if (!v)
        return -1;
    return parse_double_strict(v, out) == 0 ? 0 : -2;
}

int kv_get_int(int argc, char *argv[], const char *key, int *out)
{
    const char *v = kv_find(argc, argv, key);
    if (!v)
        return -1;
    return parse_int_strict(v, out) == 0 ? 0 : -2;
}

int parse_double_strict(const char *s, double *out)
{
    if (!s || !*s || isspace((unsigned char)*s))
        return -1;
    char *end;
    errno = 0;
    double v = strtod(s, &end);
    /* end == s: không đọc được số nào. *end != 0: còn ký tự thừa ("12x", "28,5"). */
    if (end == s || *end != '\0' || errno == ERANGE || !isfinite(v))
        return -1;
    *out = v;
    return 0;
}

int parse_int_strict(const char *s, int *out)
{
    if (!s || !*s || isspace((unsigned char)*s))
        return -1;
    char *end;
    errno = 0;
    long v = strtol(s, &end, 10);
    if (end == s || *end != '\0' || errno == ERANGE || v < INT_MIN || v > INT_MAX)
        return -1;
    *out = (int)v;
    return 0;
}

int parse_hhmm(const char *s, int *minute)
{
    /* Chỉ nhận đúng 5 ký tự "HH:MM". Từ chối "6:00", "06-00", "24:00", "06:60". */
    if (!s || strlen(s) != 5 || s[2] != ':')
        return -1;
    if (!isdigit((unsigned char)s[0]) || !isdigit((unsigned char)s[1]) ||
        !isdigit((unsigned char)s[3]) || !isdigit((unsigned char)s[4]))
        return -1;
    int hh = (s[0] - '0') * 10 + (s[1] - '0');
    int mm = (s[3] - '0') * 10 + (s[4] - '0');
    if (hh > 23 || mm > 59)
        return -1;
    *minute = hh * 60 + mm;
    return 0;
}

void format_hhmm(int minute, char *buf, size_t sz)
{
    if (minute < 0) {
        snprintf(buf, sz, "--:--");
        return;
    }
    minute %= 1440;
    snprintf(buf, sz, "%02d:%02d", minute / 60, minute % 60);
}

int kv_append(char *buf, size_t sz, const char *key, const char *fmt, ...)
{
    size_t len = strlen(buf);
    if (len >= sz)
        return -1;

    int n = snprintf(buf + len, sz - len, " %s=", key);
    if (n < 0 || (size_t)n >= sz - len) {
        buf[len] = '\0';            /* hoàn tác: buf giữ nguyên như cũ */
        return -1;
    }

    va_list ap;
    va_start(ap, fmt);
    int m = vsnprintf(buf + len + n, sz - len - (size_t)n, fmt, ap);
    va_end(ap);
    if (m < 0 || (size_t)m >= sz - len - (size_t)n) {
        buf[len] = '\0';
        return -1;
    }
    return 0;
}

/* ---- Bảng kiểu thiết bị: một chỗ duy nhất ánh xạ enum <-> chuỗi ------- */
static const struct {
    dev_type_t  type;
    const char *name;
    const char *prefix;
    const char *label;
} DEV_TABLE[] = {
    { DEV_ENV_SENSOR,  "ENV_SENSOR",  "ENV-", "Cam bien moi truong" },
    { DEV_EGG_COUNTER, "EGG_COUNTER", "EGG-", "May dem trung"       },
    { DEV_FEEDER,      "FEEDER",      "FED-", "May cho an"          },
    { DEV_WATERER,     "WATERER",     "WAT-", "May cho uong"        },
    { DEV_FAN,         "FAN",         "FAN-", "Quat dieu hoa"       },
    { DEV_HEATER,      "HEATER",      "HEA-", "Den suoi"            },
    { DEV_MISTER,      "MISTER",      "MIS-", "Phun suong"          },
};
#define DEV_TABLE_N (sizeof DEV_TABLE / sizeof DEV_TABLE[0])

dev_type_t dev_type_from_str(const char *s)
{
    for (size_t i = 0; i < DEV_TABLE_N; i++)
        if (s && strcmp(s, DEV_TABLE[i].name) == 0)
            return DEV_TABLE[i].type;
    return DEV_INVALID;
}

const char *dev_type_to_str(dev_type_t t)
{
    for (size_t i = 0; i < DEV_TABLE_N; i++)
        if (DEV_TABLE[i].type == t)
            return DEV_TABLE[i].name;
    return "UNKNOWN";
}

const char *dev_type_prefix(dev_type_t t)
{
    for (size_t i = 0; i < DEV_TABLE_N; i++)
        if (DEV_TABLE[i].type == t)
            return DEV_TABLE[i].prefix;
    return "DEV-";
}

const char *dev_type_label(dev_type_t t)
{
    for (size_t i = 0; i < DEV_TABLE_N; i++)
        if (DEV_TABLE[i].type == t)
            return DEV_TABLE[i].label;
    return "Khong ro";
}

int resp_code(const char *line)
{
    if (!line || !isdigit((unsigned char)line[0]) || !isdigit((unsigned char)line[1]) ||
        !isdigit((unsigned char)line[2]))
        return -1;
    if (line[3] != '\0' && line[3] != ' ')
        return -1;
    return (line[0] - '0') * 100 + (line[1] - '0') * 10 + (line[2] - '0');
}

const char *rc_name(int code)
{
    switch (code) {
    case RC_OK:             return "OK";
    case RC_BAD_REQUEST:    return "BAD_REQUEST";
    case RC_AUTH_FAILED:    return "AUTH_FAILED";
    case RC_INVALID_TOKEN:  return "INVALID_TOKEN";
    case RC_NO_SUCH_DEVICE: return "NO_SUCH_DEVICE";
    case RC_NOT_SUPPORTED:  return "NOT_SUPPORTED";
    case RC_POWER_OFF:      return "POWER_OFF";
    case RC_INVALID_PARAM:  return "INVALID_PARAM";
    case RC_LOCKED:         return "LOCKED";
    case RC_SERVER_ERROR:   return "SERVER_ERROR";
    default:                return "UNKNOWN";
    }
}
