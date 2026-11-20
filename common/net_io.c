/*
 * common/net_io.c: cài đặt vào/ra theo dòng (Long). TIÊU CHÍ 1.
 *
 * So với code mẫu IT4062 (tcp_server.c, select_server.c): code mẫu gọi recv() MỘT lần
 * rồi coi như đã nhận đủ thông điệp, và gọi send() MỘT lần rồi coi như đã gửi hết.
 * Cả hai giả định đó đều SAI với TCP khi mạng chậm hoặc dữ liệu lớn.
 */
#include "net_io.h"
#include "util.h"

#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>

#ifndef MSG_NOSIGNAL
#define MSG_NOSIGNAL 0   /* macOS không có cờ này; ở đó dựa vào signal(SIGPIPE, SIG_IGN) */
#endif

void linebuf_init(linebuf_t *lb)
{
    lb->len = 0;
    lb->discarding = 0;
}

/* Bỏ n byte đầu bộ đệm, dồn phần còn lại về đầu. */
static void consume(linebuf_t *lb, size_t n)
{
    /* memmove (không phải memcpy) vì vùng nguồn và đích chồng lên nhau */
    memmove(lb->data, lb->data + n, lb->len - n);
    lb->len -= n;
}

ssize_t linebuf_fill(linebuf_t *lb, int fd)
{
    size_t space = sizeof(lb->data) - lb->len;
    if (space == 0) {
        /* Không xảy ra nếu người gọi luôn gọi linebuf_next() đến khi hết dòng,
         * vì linebuf_next đã bỏ dòng quá dài từ khi chạm MAX_LINE. Phòng thủ: */
        lb->len = 0;
        lb->discarding = 1;
        space = sizeof(lb->data);
    }

    for (;;) {
        ssize_t r = recv(fd, lb->data + lb->len, space, 0);
        if (r < 0 && errno == EINTR)
            continue;               /* bị tín hiệu cắt ngang, thử lại */
        if (r > 0)
            lb->len += (size_t)r;
        return r;
    }
}

int linebuf_next(linebuf_t *lb, char *out, size_t outsz)
{
    for (;;) {
        /* memchr chứ không phải strchr: dữ liệu mạng có thể chứa byte 0,
         * strchr sẽ dừng ở byte 0 đó. Phải dựa vào độ dài lb->len. */
        char *nl = memchr(lb->data, '\n', lb->len);

        if (lb->discarding) {
            /* Đang vứt phần đuôi của một dòng quá dài: bỏ đến '\n' kế tiếp */
            if (!nl) {
                lb->len = 0;
                return 0;
            }
            consume(lb, (size_t)(nl - lb->data) + 1);
            lb->discarding = 0;
            continue;               /* sau '\n' có thể còn dòng bình thường */
        }

        if (!nl) {
            if (lb->len >= MAX_LINE) {
                /* Đã có >= MAX_LINE byte mà chưa thấy '\n', nên dòng này quá dài.
                 * Báo lỗi ĐÚNG MỘT LẦN, rồi vứt phần còn lại đến '\n'. */
                lb->discarding = 1;
                lb->len = 0;
                return -1;
            }
            return 0;               /* chưa đủ một dòng, chờ recv tiếp */
        }

        size_t n = (size_t)(nl - lb->data);   /* độ dài dòng, không tính '\n' */
        if (n >= MAX_LINE || n >= outsz) {
            consume(lb, n + 1);
            return -1;
        }
        memcpy(out, lb->data, n);
        if (n > 0 && out[n - 1] == '\r')      /* chấp nhận "\r\n" của telnet/nc -C */
            n--;
        out[n] = '\0';
        consume(lb, (size_t)(nl - lb->data) + 1);
        return 1;
    }
}

int read_line_blocking(int fd, linebuf_t *lb, char *out, size_t outsz, int timeout_ms)
{
    long long deadline = now_ms() + timeout_ms;

    for (;;) {
        /* Có thể dòng đã nằm sẵn trong bộ đệm từ lần recv trước */
        int r = linebuf_next(lb, out, outsz);
        if (r == 1)
            return 1;
        if (r == -1)
            return -3;

        long long left = deadline - now_ms();
        if (left <= 0)
            return -2;

        fd_set rs;
        FD_ZERO(&rs);
        FD_SET(fd, &rs);
        /* Đặt lại timeval MỖI VÒNG: trên Linux select() sửa nội dung timeval.
         * Đây là lỗi trong IT4062/3-Chap3/L5-IOMultiPlexing/select_client.c. */
        struct timeval tv;
        tv.tv_sec  = (time_t)(left / 1000);
        tv.tv_usec = (suseconds_t)((left % 1000) * 1000);

        int s = select(fd + 1, &rs, NULL, NULL, &tv);
        if (s < 0) {
            if (errno == EINTR)
                continue;
            return -1;
        }
        if (s == 0)
            return -2;

        ssize_t n = linebuf_fill(lb, fd);
        if (n == 0)
            return 0;
        if (n < 0)
            return -1;
    }
}

int send_all(int fd, const void *buf, size_t len)
{
    const char *p = buf;
    size_t sent = 0;
    while (sent < len) {
        /* MSG_NOSIGNAL: nếu bên kia đã đóng, send trả lỗi EPIPE thay vì
         * gửi tín hiệu SIGPIPE giết chết tiến trình */
        ssize_t r = send(fd, p + sent, len - sent, MSG_NOSIGNAL);
        if (r < 0) {
            if (errno == EINTR)
                continue;
            return -1;
        }
        sent += (size_t)r;          /* có thể gửi thiếu, nên lặp gửi tiếp phần còn lại */
    }
    return 0;
}

int send_line(int fd, const char *s)
{
    char buf[MAX_LINE + 1];
    size_t n = strlen(s);
    if (n >= MAX_LINE)
        return -1;
    /* Ghép "\n" vào CÙNG bộ đệm rồi gửi một lần. Gửi 2 lần riêng ("abc", "\n")
     * vẫn đúng, nhưng thuật toán Nagle + delayed ACK của TCP có thể làm
     * gói thứ hai bị trễ ~40ms. */
    memcpy(buf, s, n);
    buf[n] = '\n';
    return send_all(fd, buf, n + 1);
}

int send_linef(int fd, const char *fmt, ...)
{
    char buf[MAX_LINE];
    va_list ap;
    va_start(ap, fmt);
    int n = vsnprintf(buf, sizeof buf, fmt, ap);
    va_end(ap);
    if (n < 0 || n >= (int)sizeof buf)
        return -1;                  /* bị cắt thì không gửi một dòng hỏng */
    return send_line(fd, buf);
}
