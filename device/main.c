/*
 * device/main.c: chương trình device_sim (Long). TIÊU CHÍ 2 (cùng server.c).
 *
 * MỘT luồng, MỘT vòng select() phục vụ đồng thời:
 *   (1) TCP listener   : chấp nhận kết nối mới
 *   (2) các client TCP : nhận lệnh (server.c)
 *   (3) UDP 9999       : gói quét SCAN (tuần 2, module của Minh)
 *   (4) stdin          : lệnh quản trị để demo (status, clients, quit)
 *   (5) timeout 1 giây : sim_tick() cho mô phỏng chạy theo thời gian
 *
 * Vì chỉ có một luồng nên không có hai đoạn code cùng sửa một thiết bị cùng lúc,
 * không cần mutex, không có race condition. Đây là lý do chọn select() thay vì
 * fork()/pthread (IT4062/3-Chap3/L3, L4).
 */
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

#include "device.h"
#include "discovery.h"
#include "dlog.h"
#include "kv.h"
#include "server.h"
#include "util.h"

static volatile sig_atomic_t g_stop = 0;  /* đặt bởi trình xử lý tín hiệu, đọc ở vòng lặp */
static long g_ticks = 0;

static void on_signal(int sig)
{
    (void)sig;
    g_stop = 1;     /* trong signal handler chỉ nên đặt cờ, không printf/không free */
}

static void usage(const char *prog)
{
    printf("Cach dung: %s -c <file cau hinh> [-p cong_tcp] [-u cong_udp] [-s toc_do]\n", prog);
    printf("  vd: %s -c config/coop_A.conf\n", prog);
}

static int open_listener(int port)
{
    int fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (fd < 0)
        die("socket");

    /* SO_REUSEADDR: tắt rồi bật lại ngay vẫn bind được. Không có nó thì cổng còn kẹt ở
     * trạng thái TIME_WAIT khoảng 1 phút, báo "Address already in use". */
    int on = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof on);

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof addr);
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);   /* nhận kết nối trên mọi card mạng */
    addr.sin_port = htons((uint16_t)port);

    if (bind(fd, (struct sockaddr *)&addr, sizeof addr) < 0) {
        fprintf(stderr, "Khong bind duoc cong TCP %d: %s\n", port, strerror(errno));
        exit(1);
    }
    if (listen(fd, 16) < 0)
        die("listen");
    return fd;
}

/* Nhịp mô phỏng, gọi mỗi TICK_MS. Tuần 1 chỉ đếm số tick.
 * Tuần 3 thêm: env_tick(), climate_tick() (Long), feed_tick(), mister_tick() (Minh). */
static void sim_tick(void)
{
    g_ticks++;
}

static void print_devices(void)
{
    printf("  %-9s %-12s %-14s %-5s %-6s\n", "ID", "TYPE", "NAME", "POWER", "MODE");
    for (int i = 0; i < device_count(); i++) {
        device_t *d = device_at(i);
        printf("  %-9s %-12s %-14s %-5s %-6s\n", d->id, dev_type_to_str(d->type), d->name,
               d->power == PWR_ON ? "ON" : "OFF", d->mode == MODE_AUTO ? "AUTO" : "MANUAL");
    }
}

/* Lệnh quản trị gõ trên terminal của device_sim. Trả 1 nếu cần thoát. */
static int admin_command(char *line)
{
    trim(line);
    if (line[0] == '\0')
        return 0;
    if (strcmp(line, "status") == 0) {
        printf("tick=%ld  clients=%d\n", g_ticks, server_client_count());
        print_devices();
    } else if (strcmp(line, "clients") == 0) {
        server_print_clients();
    } else if (strcmp(line, "quit") == 0 || strcmp(line, "exit") == 0) {
        return 1;
    } else {
        printf("Lenh: status | clients | quit\n");
    }
    return 0;
}

/* Đọc stdin bằng read() (không dùng fgets): fgets đọc vào bộ đệm của stdio, có thể
 * "nuốt" nhiều dòng một lúc. Khi đó select() trên fd 0 không còn báo sẵn sàng nữa
 * dù vẫn còn dòng chưa xử lý. Trả -1 khi stdin đóng (EOF), 1 khi cần thoát, 0 bình thường. */
static int handle_stdin(void)
{
    char buf[512];
    ssize_t n = read(STDIN_FILENO, buf, sizeof buf - 1);
    if (n <= 0)
        return -1;
    buf[n] = '\0';
    char *save = NULL;
    for (char *l = strtok_r(buf, "\n", &save); l; l = strtok_r(NULL, "\n", &save))
        if (admin_command(l))
            return 1;
    printf("> ");
    fflush(stdout);
    return 0;
}

int main(int argc, char *argv[])
{
    const char *config = NULL;
    int opt_tcp = 0, opt_udp = 0, opt_speed = 0, opt;
    while ((opt = getopt(argc, argv, "c:p:u:s:h")) != -1) {
        switch (opt) {
        case 'c': config = optarg; break;
        case 'p': opt_tcp = atoi(optarg); break;
        case 'u': opt_udp = atoi(optarg); break;
        case 's': opt_speed = atoi(optarg); break;
        default:  usage(argv[0]); return opt == 'h' ? 0 : 1;
        }
    }
    if (!config) {
        usage(argv[0]);
        return 1;
    }

    /* Khi stdout được chuyển hướng ra file/pipe, stdio mặc định gom đầy 4 KB mới ghi,
     * nên các dòng "[+] client ..." không hiện ra ngay. Chuyển sang đẩy theo từng dòng. */
    setvbuf(stdout, NULL, _IOLBF, 0);

    /* SIGPIPE: ghi vào socket mà client đã đóng thì mặc định tiến trình BỊ GIẾT.
     * Bỏ qua tín hiệu này thì send() chỉ trả lỗi EPIPE và server chạy tiếp. */
    signal(SIGPIPE, SIG_IGN);

    /* Không đặt SA_RESTART, để select() đang chờ bị ngắt ngay (EINTR) khi Ctrl+C */
    struct sigaction sa;
    memset(&sa, 0, sizeof sa);
    sa.sa_handler = on_signal;
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);

    char err[256];
    if (ensure_dir("data") != 0)
        die("data");
    if (devices_load(config, err, sizeof err) != 0) {
        fprintf(stderr, "Loi cau hinh: %s\n", err);
        return 1;
    }
    sim_config_t *cfg = sim_config();
    if (opt_tcp > 0)   cfg->tcp_port = opt_tcp;
    if (opt_udp > 0)   cfg->udp_port = opt_udp;
    if (opt_speed > 0) cfg->speed = opt_speed;

    if (dlog_open_all() != 0)
        die("logs");

    int listener = open_listener(cfg->tcp_port);
    server_init();
    int udp_fd = discovery_open(cfg->udp_port);   /* tuần 1: bản giả trả -1 */

    printf("%s  config=%s  TCP=%d  devices=%d\n", PROTO_VERSION, cfg->conf_name,
           cfg->tcp_port, device_count());
    print_devices();
    dlog_all(DLOG_INFO, "device_sim started (%s, tcp %d)", cfg->conf_name, cfg->tcp_port);
    printf("> ");
    fflush(stdout);

    int stdin_open = 1;
    long long next_tick = now_ms() + TICK_MS;

    while (!g_stop) {
        /* fd_set phải dựng lại MỖI VÒNG vì select() xoá các fd chưa sẵn sàng */
        fd_set rs;
        FD_ZERO(&rs);
        FD_SET(listener, &rs);
        int max_fd = listener;
        if (stdin_open) {
            FD_SET(STDIN_FILENO, &rs);
            if (STDIN_FILENO > max_fd)
                max_fd = STDIN_FILENO;
        }
        if (udp_fd >= 0) {
            FD_SET(udp_fd, &rs);
            if (udp_fd > max_fd)
                max_fd = udp_fd;
        }
        server_fill_fdset(&rs, &max_fd);     /* tính lại max_fd mỗi vòng */

        /* Chỉ chờ đến lúc tick kế tiếp. timeval đặt lại mỗi vòng. */
        long long wait = next_tick - now_ms();
        if (wait < 0)
            wait = 0;
        struct timeval tv = { (time_t)(wait / 1000), (suseconds_t)((wait % 1000) * 1000) };

        int n = select(max_fd + 1, &rs, NULL, NULL, &tv);
        if (n < 0) {
            if (errno == EINTR)
                continue;               /* bị tín hiệu cắt ngang (vd Ctrl+C), vòng lại kiểm tra g_stop */
            perror("select");
            break;
        }
        if (n > 0) {
            if (FD_ISSET(listener, &rs))
                server_accept(listener);
            if (udp_fd >= 0 && FD_ISSET(udp_fd, &rs))
                discovery_handle(udp_fd);
            if (stdin_open && FD_ISSET(STDIN_FILENO, &rs)) {
                int r = handle_stdin();
                if (r == 1)
                    break;
                if (r < 0)
                    stdin_open = 0;     /* chạy nền (stdin = /dev/null): thôi theo dõi stdin */
            }
            server_handle_readable(&rs);
        }

        long long now = now_ms();
        if (now >= next_tick) {
            sim_tick();
            next_tick += TICK_MS;
            if (now - next_tick > 5 * TICK_MS)   /* bị trễ quá nhiều (máy treo) thì không "đuổi" */
                next_tick = now + TICK_MS;
        }
    }

    printf("\nDang tat: luu du lieu, dong ket noi...\n");
    dlog_all(DLOG_INFO, "device_sim stopped");
    devices_save();
    server_shutdown();
    discovery_close(udp_fd);
    close(listener);
    dlog_close_all();
    return 0;
}
