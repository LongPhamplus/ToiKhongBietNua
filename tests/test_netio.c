/*
 * tests/test_netio.c: kiểm thử xử lý truyền dòng (TIÊU CHÍ 1)
 *
 * Dùng socketpair() để có 2 đầu socket nối với nhau ngay trong một chương trình:
 * ghi vào sv[0] thì đọc được ở sv[1]. Nhờ vậy giả lập được các tình huống
 * "cắt vụn", "dồn dòng" mà không cần mạng thật.
 */
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <unistd.h>
#include "net_io.h"
#include "test.h"
#include "util.h"

static void pair(int sv[2])
{
    if (socketpair(AF_UNIX, SOCK_STREAM, 0, sv) < 0)
        die("socketpair");
}

static void put(int fd, const char *s) { send_all(fd, s, strlen(s)); }

int main(void)
{
    signal(SIGPIPE, SIG_IGN);
    int sv[2];
    linebuf_t lb;
    char line[MAX_LINE];

    /* 1. Một dòng bị chia làm 2 lần gửi */
    pair(sv);
    linebuf_init(&lb);
    put(sv[0], "HEL");
    linebuf_fill(&lb, sv[1]);
    int r1 = linebuf_next(&lb, line, sizeof line);
    put(sv[0], "LO\n");
    linebuf_fill(&lb, sv[1]);
    int r2 = linebuf_next(&lb, line, sizeof line);
    CHECK(r1 == 0 && r2 == 1 && strcmp(line, "HELLO") == 0,
          "\"HEL\" + \"LO\\n\" (2 lan gui) -> 1 dong HELLO");
    close(sv[0]); close(sv[1]);

    /* 2. Ba dòng trong một lần gửi */
    pair(sv);
    linebuf_init(&lb);
    put(sv[0], "A\nB\nC\n");
    linebuf_fill(&lb, sv[1]);
    char got[3][8] = {{0}};
    int k = 0;
    while (k < 3 && linebuf_next(&lb, line, sizeof line) == 1)
        safe_copy(got[k++], sizeof got[0], line);
    CHECK(k == 3 && !strcmp(got[0], "A") && !strcmp(got[1], "B") && !strcmp(got[2], "C"),
          "\"A\\nB\\nC\\n\" (1 lan gui) -> 3 dong A, B, C");
    CHECK(linebuf_next(&lb, line, sizeof line) == 0, "het dong -> tra 0");
    close(sv[0]); close(sv[1]);

    /* 3. \r\n */
    pair(sv);
    linebuf_init(&lb);
    put(sv[0], "A\r\n");
    linebuf_fill(&lb, sv[1]);
    CHECK(linebuf_next(&lb, line, sizeof line) == 1 && strcmp(line, "A") == 0,
          "\"A\\r\\n\" -> \"A\"");
    close(sv[0]); close(sv[1]);

    /* 4. Dòng quá dài: báo lỗi 1 lần, dòng sau vẫn đọc được */
    pair(sv);
    linebuf_init(&lb);
    char *big = malloc(5001);
    memset(big, 'x', 5000);
    big[5000] = '\0';
    put(sv[0], big);
    put(sv[0], "\nHELLO\n");
    free(big);
    int too_long = 0, hello = 0;
    for (int i = 0; i < 10 && !hello; i++) {
        int r = read_line_blocking(sv[1], &lb, line, sizeof line, 500);
        if (r == -3) too_long++;
        else if (r == 1 && strcmp(line, "HELLO") == 0) hello = 1;
        else break;
    }
    CHECK(too_long == 1 && hello, "5000 byte + \"\\nHELLO\\n\" -> loi 1 lan, sau do doc duoc HELLO");
    close(sv[0]); close(sv[1]);

    /* 5. Bên kia đóng giữa chừng một dòng */
    pair(sv);
    linebuf_init(&lb);
    put(sv[0], "AB");
    close(sv[0]);
    ssize_t f1 = linebuf_fill(&lb, sv[1]);
    int n1 = linebuf_next(&lb, line, sizeof line);
    ssize_t f2 = linebuf_fill(&lb, sv[1]);
    CHECK(f1 == 2 && n1 == 0 && f2 == 0, "gui \"AB\" roi dong -> chua co dong, fill tra 0");
    close(sv[1]);

    /* 6. Hết giờ */
    pair(sv);
    linebuf_init(&lb);
    long long t0 = now_ms();
    int rt = read_line_blocking(sv[1], &lb, line, sizeof line, 300);
    long long dt = now_ms() - t0;
    CHECK(rt == -2 && dt >= 250 && dt < 1000, "read_line_blocking khong co du lieu -> -2 sau ~300ms");
    close(sv[0]); close(sv[1]);

    /* 7. send_all 100 KB: tiến trình con gửi, tiến trình cha đếm số byte nhận */
    pair(sv);
    enum { BIG = 100 * 1024 };
    pid_t pid = fork();
    if (pid == 0) {
        close(sv[1]);
        char *buf = calloc(1, BIG);
        int rc = send_all(sv[0], buf, BIG);
        free(buf);
        close(sv[0]);
        _exit(rc == 0 ? 0 : 1);
    }
    close(sv[0]);
    size_t total = 0;
    char tmp[4096];
    ssize_t r;
    while ((r = recv(sv[1], tmp, sizeof tmp, 0)) > 0)
        total += (size_t)r;
    close(sv[1]);
    int status = 0;
    waitpid(pid, &status, 0);
    CHECK(total == BIG && WIFEXITED(status) && WEXITSTATUS(status) == 0,
          "send_all 100 KB -> ben kia nhan du 100 KB");

    /* 8. send_line từ chối dòng quá dài */
    pair(sv);
    char *huge = malloc(MAX_LINE + 10);
    memset(huge, 'y', MAX_LINE + 9);
    huge[MAX_LINE + 9] = '\0';
    CHECK(send_line(sv[0], huge) == -1, "send_line tu choi dong >= MAX_LINE");
    free(huge);
    close(sv[0]); close(sv[1]);

    return TEST_DONE();
}
