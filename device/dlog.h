/*
 * device/dlog.h: ghi log hoạt động trên MỖI thiết bị (Long). TIÊU CHÍ 12.
 * Mỗi thiết bị một file logs/<id>.log. Mọi module phía server (kể cả của Minh) gọi dlog().
 */
#ifndef DLOG_H
#define DLOG_H

#include "device.h"

typedef enum { DLOG_DEBUG = 0, DLOG_INFO, DLOG_WARN, DLOG_ERROR } dlog_level_t;

/* Mở logs/<id>.log (ghi nối) cho mọi thiết bị. Trả 0 / -1. */
int  dlog_open_all(void);
void dlog_close_all(void);

/* Ghi một dòng vào log của dev. app_id = NULL nghĩa là sự kiện do mô phỏng tự sinh ("auto"). */
void dlog(device_t *dev, dlog_level_t lv, const char *app_id, const char *fmt, ...)
     __attribute__((format(printf, 4, 5)));

/* Ghi cùng một dòng vào log của TẤT CẢ thiết bị (vd. khởi động / tắt). */
void dlog_all(dlog_level_t lv, const char *fmt, ...) __attribute__((format(printf, 2, 3)));

void dlog_set_min_level(dlog_level_t lv);   /* bỏ qua các dòng thấp hơn mức này */
void dlog_set_echo(int on);                  /* 1 = in thêm ra stdout (mặc định 1) */

#endif /* DLOG_H */
