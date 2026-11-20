/*
 * controller/conn.c: kết nối TCP, gửi lệnh / nhận phản hồi có timeout (Minh)
 */
#include "conn.h"
#include "kv.h"
#include "ui.h"
#include "util.h"

#include <errno.h>
#include <netdb.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

static char      g_app_id[APP_ID_HEX_LEN + 1];
static session_t g_sess[MAX_DEVICES * 4];

void conn_init(const char *app_id)
{
    safe_copy(g_app_id, sizeof g_app_id, app_id);
    memset(g_sess, 0, sizeof g_sess);
    for (size_t i = 0; i < sizeof g_sess / sizeof g_sess[0]; i++)
        g_sess[i].fd = -1;
}

int conn_connected_count(void)
{
    int n = 0;
    for (size_t i = 0; i < sizeof g_sess / sizeof g_sess[0]; i++)
        if (g_sess[i].used && g_sess[i].connected)
            n++;
    return n;
}

void conn_shutdown(void)
{
    for (size_t i = 0; i < sizeof g_sess / sizeof g_sess[0]; i++) {
        session_t *s = &g_sess[i];
        if (s->used && s->fd >= 0) {
            char resp[MAX_LINE];
            conn_raw(s->fd, &s->lb, CMD_BYE, resp, sizeof resp);   /* lịch sự báo thiết bị */
            close(s->fd);
        }
        s->used = 0;
        s->fd = -1;
    }
}

int conn_open(const char *host, int port, char *err, size_t errsz)
{
    /* getaddrinfo nhận được cả IP ("127.0.0.1") lẫn tên máy ("localhost").
     * Tham khảo IT4062/2-Chap2/domain2ip.c */
    struct addrinfo hints, *res = NULL;
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    char portstr[16];
    snprintf(portstr, sizeof portstr, "%d", port);
    int gai = getaddrinfo(host, portstr, &hints, &res);
    if (gai != 0) {
        snprintf(err, errsz, "Khong phan giai duoc '%s': %s", host, gai_strerror(gai));
        return -1;
    }

    int fd = -1, last_errno = 0;
    for (struct addrinfo *p = res; p; p = p->ai_next) {
        fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (fd < 0) {
            last_errno = errno;
            continue;
        }
        if (connect(fd, p->ai_addr, p->ai_addrlen) == 0)
            break;
        last_errno = errno;
        close(fd);
        fd = -1;
    }
    freeaddrinfo(res);

    if (fd < 0) {
        if (last_errno == ECONNREFUSED)
            snprintf(err, errsz, "Khong ket noi duoc %s:%d (device_sim da chay chua?)", host, port);
        else
            snprintf(err, errsz, "Khong ket noi duoc %s:%d: %s", host, port, strerror(last_errno));
    }
    return fd;
}

int conn_raw(int fd, linebuf_t *lb, const char *cmd, char *resp, size_t sz)
{
    resp[0] = '\0';
    if (send_line(fd, cmd) != 0)
        return errno == EPIPE || errno == ECONNRESET ? CONN_ERR_CLOSED : CONN_ERR_IO;

    /* Mỗi lệnh đọc ĐÚNG MỘT dòng phản hồi. Phải đọc xong phản hồi của lệnh này
     * rồi mới gửi lệnh sau, nếu không phản hồi sẽ bị "lệch" sang lệnh khác. */
    int r = read_line_blocking(fd, lb, resp, sz, REQ_TIMEOUT_MS);
    switch (r) {
    case 1: {
        int code = resp_code(resp);
        return code < 0 ? CONN_ERR_BADRESP : code;
    }
    case 0:  return CONN_ERR_CLOSED;
    case -2: return CONN_ERR_TIMEOUT;
    default: return CONN_ERR_IO;
    }
}

static session_t *find_session(const char *device_id)
{
    for (size_t i = 0; i < sizeof g_sess / sizeof g_sess[0]; i++)
        if (g_sess[i].used && strcmp(g_sess[i].device_id, device_id) == 0)
            return &g_sess[i];
    return NULL;
}

int conn_cmd(const char *device_id, char *resp, size_t sz, const char *fmt, ...)
{
    session_t *s = find_session(device_id);
    if (!s || !s->connected)
        return CONN_ERR_NOSESSION;          /* tuần 1: luôn rơi vào đây */

    char body[MAX_LINE], line[MAX_LINE];
    va_list ap;
    va_start(ap, fmt);
    int n = vsnprintf(body, sizeof body, fmt, ap);
    va_end(ap);
    if (n < 0 || n >= (int)sizeof body)
        return CONN_ERR_IO;

    /* "SET Tmax=31" -> "SET <id> <token> Tmax=31" */
    char *rest = strchr(body, ' ');
    if (rest)
        *rest++ = '\0';
    n = snprintf(line, sizeof line, "%s %s %s%s%s", body, s->device_id, s->token,
                 rest ? " " : "", rest ? rest : "");
    if (n < 0 || n >= (int)sizeof line)
        return CONN_ERR_IO;

    int code = conn_raw(s->fd, &s->lb, line, resp, sz);
    if (code == CONN_ERR_CLOSED || code == CONN_ERR_IO || code == CONN_ERR_TIMEOUT)
        s->connected = 0;                   /* tuần 2: đóng fd, báo UI kết nối lại */
    return code;
}

void conn_raw_console(void)
{
    char host[64], cmd[MAX_LINE], resp[MAX_LINE], err[160];
    int port;

    if (ui_read_str_keep("IP/ten may", "127.0.0.1", host, sizeof host) < 0 ||
        ui_read_int_keep("Port", DEFAULT_TCP_PORT, 1, 65535, &port) != 0)
        return;

    int fd = conn_open(host, port, err, sizeof err);
    if (fd < 0) {
        ui_err("%s", err);
        return;
    }
    ui_ok("Da ket noi %s:%d. Go lenh (HELLO, INFO <id>, BYE...), 'exit' de thoat.", host, port);

    linebuf_t lb;
    linebuf_init(&lb);
    for (;;) {
        if (ui_read_line("Lenh> ", cmd, sizeof cmd) < 0 || strcmp(cmd, "exit") == 0)
            break;
        if (cmd[0] == '\0')
            continue;

        int code = conn_raw(fd, &lb, cmd, resp, sizeof resp);
        if (code < 0) {
            /* Hết giờ thì vẫn ĐÓNG kết nối: nếu phản hồi muộn tới sau, nó sẽ bị
             * đọc nhầm thành phản hồi của lệnh kế tiếp. */
            ui_err("%s", ui_explain_code(code));
            close(fd);
            return;
        }
        printf("%s\n", resp);
        if (code != RC_OK)
            ui_info("%s", ui_explain_code(code));
        if (code == RC_OK && strcmp(cmd, CMD_BYE) == 0) {
            close(fd);              /* server đã đóng phía nó */
            return;
        }
    }
    conn_raw(fd, &lb, CMD_BYE, resp, sizeof resp);
    close(fd);
}
