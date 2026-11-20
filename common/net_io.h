/*
 * common/net_io.h: vào/ra TCP theo DÒNG (Long). TIÊU CHÍ 1: xử lý truyền dòng.
 *
 * VẤN ĐỀ CẦN GIẢI QUYẾT
 *   TCP là LUỒNG BYTE, không giữ ranh giới thông điệp:
 *     - gửi "INFO FAN-1\n" một lần, bên kia có thể nhận "INF" rồi "O FAN-1\n"   (bị cắt vụn)
 *     - gửi 3 lệnh liên tiếp, bên kia có thể nhận cả 3 trong MỘT lần recv()      (bị dồn)
 *   Nên mỗi kết nối cần một bộ đệm: gom byte lại, chỉ trả ra khi đã có một dòng
 *   hoàn chỉnh (kết thúc '\n'), phần dư giữ lại cho lần sau.
 */
#ifndef NET_IO_H
#define NET_IO_H

#include <stddef.h>
#include <sys/types.h>
#include "protocol.h"

typedef struct {
    char   data[2 * MAX_LINE]; /* chứa được 1 dòng tối đa + phần đầu của dòng sau */
    size_t len;                /* số byte đang có trong data */
    int    discarding;         /* 1 = đang vứt bỏ phần còn lại của một dòng quá dài */
} linebuf_t;

void    linebuf_init(linebuf_t *lb);

/* Gọi recv() MỘT lần, nối byte nhận được vào cuối bộ đệm.
 * Trả: >0 số byte nhận | 0 bên kia đã đóng kết nối | -1 lỗi (xem errno).
 * Server gọi hàm này sau khi select() báo fd sẵn sàng, nên recv không bị chặn. */
ssize_t linebuf_fill(linebuf_t *lb, int fd);

/* Lấy MỘT dòng hoàn chỉnh (đã bỏ '\n' và '\r' cuối) ra out.
 * Trả: 1 có dòng | 0 chưa đủ một dòng | -1 dòng quá dài (>= MAX_LINE), đã bị bỏ.
 * Một lần recv có thể chứa nhiều dòng, nên phải gọi LẶP đến khi trả 0. */
int     linebuf_next(linebuf_t *lb, char *out, size_t outsz);

/* Dành cho CLIENT: chờ đến khi có đủ một dòng hoặc hết timeout_ms.
 * Trả: 1 có dòng | 0 bên kia đóng kết nối | -1 lỗi | -2 hết giờ | -3 dòng quá dài */
int     read_line_blocking(int fd, linebuf_t *lb, char *out, size_t outsz, int timeout_ms);

/* Gửi đến hết len byte (send() có thể gửi thiếu). Trả 0 / -1. */
int     send_all(int fd, const void *buf, size_t len);

/* Gửi chuỗi s kèm '\n'. Trả 0 / -1 (lỗi hoặc dòng dài hơn MAX_LINE-1). */
int     send_line(int fd, const char *s);

/* Như printf nhưng gửi thành một dòng lên socket. Trả 0 / -1. */
int     send_linef(int fd, const char *fmt, ...) __attribute__((format(printf, 2, 3)));

#endif /* NET_IO_H */
