/*
 * device/server.c: phục vụ nhiều client TCP (Long). TIÊU CHÍ 2.
 *
 * Việc của file này là KẾT NỐI: ai đang kết nối, bộ đệm của từng người, đọc byte,
 * cắt dòng, gửi phản hồi, dọn dẹp khi ngắt. Ý NGHĨA của từng lệnh thuộc về handlers.c.
 * Bước 7: tạm trả lời "200 OK echo=<dòng>" để kiểm tra phần kết nối.
 */
#include "server.h"

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

/* Mảng cố định, ô trống có fd = -1. Đóng client chỉ cần đặt fd = -1, không phải
 * dồn mảng, nên đóng ngay trong vòng duyệt cũng không làm lệch chỉ số. */
static client_t g_clients[MAX_CLIENTS];

void server_init(void)
{
    for (int i = 0; i < MAX_CLIENTS; i++)
        g_clients[i].fd = -1;
}

int server_client_count(void)
{
    int n = 0;
    for (int i = 0; i < MAX_CLIENTS; i++)
        if (g_clients[i].fd >= 0)
            n++;
    return n;
}

static void close_client(client_t *c, const char *why)
{
    printf("[-] client %s:%d (fd=%d) %s\n", c->ip, c->port, c->fd, why);
    close(c->fd);
    c->fd = -1;
}

void server_accept(int listen_fd)
{
    struct sockaddr_in addr;
    socklen_t len = sizeof addr;
    int fd = accept(listen_fd, (struct sockaddr *)&addr, &len);
    if (fd < 0) {
        if (errno != EINTR && errno != EAGAIN && errno != ECONNABORTED)
            perror("accept");
        return;
    }

    client_t *c = NULL;
    for (int i = 0; i < MAX_CLIENTS && !c; i++)
        if (g_clients[i].fd < 0)
            c = &g_clients[i];
    if (!c) {
        send_line(fd, "500 SERVER_ERROR server_full");
        close(fd);
        printf("[!] tu choi ket noi: da du %d client\n", MAX_CLIENTS);
        return;
    }

    memset(c, 0, sizeof *c);
    c->fd = fd;
    inet_ntop(AF_INET, &addr.sin_addr, c->ip, sizeof c->ip);
    c->port = ntohs(addr.sin_port);
    c->since = time(NULL);
    linebuf_init(&c->lb);
    printf("[+] client %s:%d (fd=%d)\n", c->ip, c->port, fd);
}

void server_fill_fdset(fd_set *set, int *max_fd)
{
    for (int i = 0; i < MAX_CLIENTS; i++) {
        int fd = g_clients[i].fd;
        if (fd >= 0) {
            FD_SET(fd, set);
            if (fd > *max_fd)
                *max_fd = fd;
        }
    }
}

/* In lệnh nhận được ra màn hình để quan sát khi demo.
 * CONNECT / CHPASS chứa mật khẩu nên chỉ in 2 từ đầu. */
static void print_command(const client_t *c, const char *line)
{
    if (strncmp(line, "CONNECT ", 8) == 0 || strncmp(line, "CHPASS ", 7) == 0) {
        char head[64];
        int n = 0, spaces = 0;
        while (line[n] && n < (int)sizeof head - 1) {
            if (line[n] == ' ' && ++spaces == 2)
                break;
            head[n] = line[n];
            n++;
        }
        head[n] = '\0';
        printf("[%d] %s ***\n", c->fd, head);
    } else {
        printf("[%d] %s\n", c->fd, line);
    }
}

static void handle_client(client_t *c)
{
    /* select() đã báo fd này có dữ liệu, nên recv đúng MỘT lần (không bị chặn) */
    ssize_t r = linebuf_fill(&c->lb, c->fd);
    if (r == 0) {
        close_client(c, "da ngat ket noi");
        return;
    }
    if (r < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR)
            return;
        close_client(c, "loi recv");
        return;
    }

    /* Một lần recv có thể chứa 0, 1 hoặc nhiều dòng, nên lặp đến khi hết */
    char line[MAX_LINE];
    int k;
    while ((k = linebuf_next(&c->lb, line, sizeof line)) != 0) {
        char resp[MAX_LINE];
        if (k < 0) {
            snprintf(resp, sizeof resp, "%d BAD_REQUEST LINE_TOO_LONG", RC_BAD_REQUEST);
        } else {
            if (line[0] == '\0')
                continue;           /* bỏ qua dòng trống (gõ Enter trong nc) */
            c->n_cmds++;
            print_command(c, line);
            /* Bước 7: tạm thời trả lời lại nguyên dòng. Bước 8 thay bằng handlers_dispatch(). */
            snprintf(resp, sizeof resp, "200 OK echo=%.900s", line);
        }
        if (send_line(c->fd, resp) != 0) {
            close_client(c, "loi send");
            return;
        }
        if (c->close_after)
            break;
    }
    if (c->close_after)
        close_client(c, "BYE");
}

void server_handle_readable(fd_set *set)
{
    for (int i = 0; i < MAX_CLIENTS; i++)
        if (g_clients[i].fd >= 0 && FD_ISSET(g_clients[i].fd, set))
            handle_client(&g_clients[i]);
}

void server_print_clients(void)
{
    int n = 0;
    time_t now = time(NULL);
    for (int i = 0; i < MAX_CLIENTS; i++) {
        const client_t *c = &g_clients[i];
        if (c->fd < 0)
            continue;
        printf("  fd=%-3d %s:%-5d  ket noi %lds  lenh=%d\n", c->fd, c->ip, c->port,
               (long)(now - c->since), c->n_cmds);
        n++;
    }
    printf("  (%d client)\n", n);
}

void server_shutdown(void)
{
    for (int i = 0; i < MAX_CLIENTS; i++)
        if (g_clients[i].fd >= 0)
            close_client(&g_clients[i], "server tat");
}
