/*
 * device/server.h: quản lý các kết nối TCP của device_sim (Long). TIÊU CHÍ 2.
 */
#ifndef SERVER_H
#define SERVER_H

#include <arpa/inet.h>
#include <sys/select.h>
#include <time.h>
#include "net_io.h"

typedef struct {
    int       fd;                       /* -1 = ô trống */
    char      ip[INET_ADDRSTRLEN];
    int       port;
    linebuf_t lb;                       /* bộ đệm dòng RIÊNG của client này */
    time_t    since;
    int       n_cmds;
    int       close_after;              /* 1 = đóng sau khi gửi phản hồi (lệnh BYE) */
} client_t;

void server_init(void);
void server_accept(int listen_fd);
void server_fill_fdset(fd_set *set, int *max_fd);
void server_handle_readable(fd_set *set);
int  server_client_count(void);
void server_print_clients(void);
void server_shutdown(void);

#endif /* SERVER_H */
