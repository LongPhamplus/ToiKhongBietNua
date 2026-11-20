/*
 * common/util.c: cài đặt tiện ích dùng chung (Long)
 */
#include "util.h"

#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

int random_hex(char *out, size_t nbytes)
{
    static const char HEX[] = "0123456789abcdef";
    unsigned char raw[64];
    int rc = 0;

    if (nbytes > sizeof(raw))
        nbytes = sizeof(raw);

    /* /dev/urandom là nguồn ngẫu nhiên an toàn mật mã của kernel. Token và
     * App ID lấy từ đây thì không đoán được. rand() đoán được nếu biết seed. */
    size_t got = 0;
    int fd = open("/dev/urandom", O_RDONLY);
    if (fd >= 0) {
        /* read() có thể trả về ít hơn số byte yêu cầu, nên phải lặp */
        while (got < nbytes) {
            ssize_t r = read(fd, raw + got, nbytes - got);
            if (r < 0 && errno == EINTR)
                continue;
            if (r <= 0)
                break;
            got += (size_t)r;
        }
        close(fd);
    }

    if (got < nbytes) {
        /* Dự phòng hiếm khi xảy ra: srand một lần duy nhất trong cả chương trình */
        static int seeded = 0;
        if (!seeded) {
            srand((unsigned)time(NULL) ^ (unsigned)getpid());
            seeded = 1;
        }
        for (size_t i = got; i < nbytes; i++)
            raw[i] = (unsigned char)(rand() & 0xff);
        fprintf(stderr, "[canh bao] khong doc duoc /dev/urandom, dung rand()\n");
        rc = -1;
    }

    for (size_t i = 0; i < nbytes; i++) {
        out[2 * i]     = HEX[raw[i] >> 4];
        out[2 * i + 1] = HEX[raw[i] & 0x0f];
    }
    out[2 * nbytes] = '\0';
    return rc;
}

long long now_ms(void)
{
    struct timespec ts;
    /* CLOCK_MONOTONIC không bị ảnh hưởng khi người dùng chỉnh đồng hồ hệ thống,
     * nên phù hợp để tính timeout. (time() có thể nhảy lùi.) */
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (long long)ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
}

static void format_now(char *buf, size_t sz, const char *fmt)
{
    time_t t = time(NULL);
    struct tm tmv;
    localtime_r(&t, &tmv);           /* bản _r an toàn, không dùng vùng nhớ tĩnh chung */
    if (strftime(buf, sz, fmt, &tmv) == 0 && sz > 0)
        buf[0] = '\0';
}

void now_str(char *buf, size_t sz)   { format_now(buf, sz, "%Y-%m-%d %H:%M:%S"); }
void today_str(char *buf, size_t sz) { format_now(buf, sz, "%Y-%m-%d"); }

char *trim(char *s)
{
    char *start = s;
    while (*start && isspace((unsigned char)*start))
        start++;

    size_t len = strlen(start);
    while (len > 0 && isspace((unsigned char)start[len - 1]))
        len--;

    /* memmove vì hai vùng nhớ chồng lên nhau (memcpy thì không được) */
    memmove(s, start, len);
    s[len] = '\0';
    return s;
}

void safe_copy(char *dst, size_t dstsz, const char *src)
{
    if (dstsz == 0)
        return;
    /* snprintf luôn tự thêm '\0' và tự cắt nếu src quá dài */
    snprintf(dst, dstsz, "%s", src ? src : "");
}

void str_upper(char *s)
{
    for (; *s; s++)
        *s = (char)toupper((unsigned char)*s);
}

int ensure_dir(const char *path)
{
    struct stat st;
    if (stat(path, &st) == 0)
        return S_ISDIR(st.st_mode) ? 0 : -1;   /* đã tồn tại nhưng là file thì báo lỗi */
    if (mkdir(path, 0755) == 0 || errno == EEXIST)
        return 0;
    return -1;
}

void die(const char *msg)
{
    perror(msg);
    exit(1);
}
