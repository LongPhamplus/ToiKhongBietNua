/*
 * controller/conn.h: kết nối TCP tới thiết bị (Minh). Chuẩn bị TIÊU CHÍ 6.
 *
 * Đây là CẦU NỐI DUY NHẤT giữa giao diện và mạng: mọi màn hình (của cả Minh và
 * Long) chỉ gọi conn_cmd(), không màn hình nào tự mở socket. Timeout, mất kết nối,
 * đọc đúng một dòng phản hồi đều được xử lý ở một chỗ.
 */
#ifndef CONN_H
#define CONN_H

#include <stddef.h>
#include "net_io.h"
#include "protocol.h"

/* Mã lỗi phía client (số âm, không trùng với mã phản hồi 200..500 của thiết bị) */
#define CONN_ERR_CLOSED    (-1)   /* thiết bị đã đóng kết nối */
#define CONN_ERR_TIMEOUT   (-2)   /* hết REQ_TIMEOUT_MS mà không có phản hồi */
#define CONN_ERR_IO        (-3)   /* lỗi socket khác */
#define CONN_ERR_NOSESSION (-4)   /* chưa kết nối tới thiết bị này */
#define CONN_ERR_BADRESP   (-5)   /* phản hồi không bắt đầu bằng mã 3 chữ số */

/* Một phiên làm việc với một thiết bị (bảng phiên dùng thật từ tuần 2) */
typedef struct {
    int       used;
    char      device_id[MAX_ID];
    char      ip[64];
    int       port;
    int       fd;
    linebuf_t lb;
    char      token[TOKEN_HEX_LEN + 1];
    int       connected;
} session_t;

void conn_init(const char *app_id);
void conn_shutdown(void);

/* Mở kết nối TCP (host là IP hoặc tên máy). Trả fd, hoặc -1 kèm err. */
int  conn_open(const char *host, int port, char *err, size_t errsz);

/* Gửi MỘT dòng lệnh và đọc MỘT dòng phản hồi (timeout REQ_TIMEOUT_MS).
 * Trả mã phản hồi (200, 404, ...) hoặc CONN_ERR_* (âm). */
int  conn_raw(int fd, linebuf_t *lb, const char *cmd, char *resp, size_t sz);

/* Hàm mọi màn hình sẽ dùng từ tuần 2. Người gọi chỉ viết "LỆNH tham số...",
 * conn_cmd tự chèn thành "LỆNH <device_id> <token> tham số...".
 * Tuần 1 chưa có CONNECT nên luôn trả CONN_ERR_NOSESSION. */
int  conn_cmd(const char *device_id, char *resp, size_t sz, const char *fmt, ...)
     __attribute__((format(printf, 4, 5)));

int  conn_connected_count(void);

/* Mục menu "[DEV] Gửi lệnh thô": công cụ tích hợp/debug của tuần 1 */
void conn_raw_console(void);

#endif /* CONN_H */
