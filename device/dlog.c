/*
 * device/dlog.c: cài đặt log từng thiết bị (Long). TIÊU CHÍ 12.
 *
 * Định dạng một dòng:
 *   2026-11-26 20:10:01 | sim --:-- | INFO  | auto         | device_sim started
 *   thời gian thực       giờ mô phỏng  mức    nguồn          nội dung
 * Giờ mô phỏng tạm ghi "--:--", tuần 3 lấy từ env.c.
 */
#include "dlog.h"
#include "util.h"

#include <stdarg.h>
#include <stdio.h>

#define LOG_DIR "logs"

static dlog_level_t g_min = DLOG_INFO;
static int          g_echo = 1;

static const char *LEVEL_NAME[] = { "DEBUG", "INFO ", "WARN ", "ERROR" };

void dlog_set_min_level(dlog_level_t lv) { g_min = lv; }
void dlog_set_echo(int on)               { g_echo = on; }

int dlog_open_all(void)
{
    if (ensure_dir(LOG_DIR) != 0)
        return -1;
    for (int i = 0; i < device_count(); i++) {
        device_t *d = device_at(i);
        char path[64];
        snprintf(path, sizeof path, LOG_DIR "/%s.log", d->id);
        /* "a" = ghi nối vào cuối: log của các lần chạy trước được giữ lại */
        d->log = fopen(path, "a");
        if (!d->log)
            return -1;
    }
    return 0;
}

void dlog_close_all(void)
{
    for (int i = 0; i < device_count(); i++) {
        device_t *d = device_at(i);
        if (d->log) {
            fclose(d->log);
            d->log = NULL;
        }
    }
}

static void vlog(device_t *d, dlog_level_t lv, const char *app_id, const char *fmt, va_list ap)
{
    char ts[32], msg[MAX_LINE];
    now_str(ts, sizeof ts);
    vsnprintf(msg, sizeof msg, fmt, ap);

    char src[16];
    if (app_id)
        snprintf(src, sizeof src, "app=%s", app_id);
    else
        snprintf(src, sizeof src, "auto");

    if (d->log) {
        fprintf(d->log, "%s | sim --:-- | %s | %-12s | %s\n", ts, LEVEL_NAME[lv], src, msg);
        /* fflush ngay: nếu chương trình bị Ctrl+C / crash thì log vẫn đã nằm trên đĩa,
         * không bị kẹt trong bộ đệm của stdio */
        fflush(d->log);
    }
    if (g_echo)
        printf("[log %s] %s %s\n", d->id, LEVEL_NAME[lv], msg);
}

void dlog(device_t *d, dlog_level_t lv, const char *app_id, const char *fmt, ...)
{
    if (!d || lv < g_min)
        return;
    va_list ap;
    va_start(ap, fmt);
    vlog(d, lv, app_id, fmt, ap);
    va_end(ap);
}

void dlog_all(dlog_level_t lv, const char *fmt, ...)
{
    if (lv < g_min)
        return;
    int echo = g_echo;
    g_echo = 0;                     /* không in 7 dòng giống nhau ra màn hình */
    for (int i = 0; i < device_count(); i++) {
        va_list ap;
        va_start(ap, fmt);          /* va_list chỉ dùng được một lần, nên mỗi vòng phải va_start lại */
        vlog(device_at(i), lv, NULL, fmt, ap);
        va_end(ap);
    }
    g_echo = echo;
}
