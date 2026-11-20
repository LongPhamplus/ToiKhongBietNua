/*
 * controller/ui.c: cài đặt nhập/xuất an toàn (Minh)
 *
 * KHÔNG dùng scanf("%d"): gặp chữ thì scanf không đọc được, để nguyên chữ đó
 * trong stdin, vòng lặp hỏi lại đọc trúng chữ đó mãi mãi, tức là LẶP VÔ HẠN.
 * scanf("%s") còn có thể tràn bộ đệm.
 * Cách đúng: đọc cả dòng bằng fgets, rồi tự phân tích bằng parse_*_strict (kv.c).
 */
#include "ui.h"
#include "kv.h"
#include "protocol.h"
#include "util.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

int ui_read_line(const char *prompt, char *buf, size_t sz)
{
    for (;;) {
        if (prompt) {
            printf("%s", prompt);
            fflush(stdout);         /* prompt không có '\n' nên phải tự đẩy ra màn hình */
        }
        if (!fgets(buf, (int)sz, stdin)) {
            /* EOF (Ctrl+D). clearerr để lần đọc sau trên terminal vẫn hoạt động.
             * Nếu stdin là pipe đã hết thì lần sau lại trả -1, các menu thoát dần ra ngoài. */
            clearerr(stdin);
            printf("\n");
            return -1;
        }
        if (!strchr(buf, '\n') && !feof(stdin)) {
            /* Dòng dài hơn bộ đệm: đọc bỏ phần còn lại, nếu không nó sẽ bị
             * hiểu nhầm là câu trả lời cho câu hỏi tiếp theo */
            int ch;
            while ((ch = getchar()) != '\n' && ch != EOF)
                ;
            ui_err("Noi dung qua dai (toi da %d ky tu)", (int)sz - 2);
            continue;
        }
        trim(buf);
        return 0;
    }
}

int ui_read_int(const char *prompt, int min, int max, int *out)
{
    char buf[64];
    for (;;) {
        if (ui_read_line(prompt, buf, sizeof buf) < 0)
            return -1;
        if (buf[0] == '\0')
            return 1;
        int v;
        if (parse_int_strict(buf, &v) == 0 && v >= min && v <= max) {
            *out = v;
            return 0;
        }
        ui_err("Hay nhap so nguyen tu %d den %d", min, max);
    }
}

int ui_read_double(const char *prompt, double min, double max, double *out)
{
    char buf[64];
    for (;;) {
        if (ui_read_line(prompt, buf, sizeof buf) < 0)
            return -1;
        if (buf[0] == '\0')
            return 1;
        double v;
        if (parse_double_strict(buf, &v) == 0 && v >= min && v <= max) {
            *out = v;
            return 0;
        }
        ui_err("Hay nhap so tu %g den %g (dung dau cham, vd 28.5)", min, max);
    }
}

int ui_read_int_keep(const char *prompt, int cur, int min, int max, int *out)
{
    char p[128];
    snprintf(p, sizeof p, "%s [%d]: ", prompt, cur);
    int r = ui_read_int(p, min, max, out);
    if (r == 1) {                   /* Enter rỗng = giữ giá trị cũ */
        *out = cur;
        return 0;
    }
    return r;
}

int ui_read_double_keep(const char *prompt, double cur, double min, double max, double *out)
{
    char p[128];
    snprintf(p, sizeof p, "%s [%g]: ", prompt, cur);
    int r = ui_read_double(p, min, max, out);
    if (r == 1) {
        *out = cur;
        return 0;
    }
    return r;
}

int ui_read_str_keep(const char *prompt, const char *cur, char *buf, size_t sz)
{
    char p[128], tmp[MAX_LINE];
    snprintf(p, sizeof p, "%s [%s]: ", prompt, cur);
    if (ui_read_line(p, tmp, sizeof tmp) < 0)
        return -1;
    safe_copy(buf, sz, tmp[0] ? tmp : cur);
    return 0;
}

int ui_read_hhmm(const char *prompt, int *minute)
{
    char buf[32];
    for (;;) {
        if (ui_read_line(prompt, buf, sizeof buf) < 0)
            return -1;
        if (buf[0] == '\0')
            return 1;
        if (parse_hhmm(buf, minute) == 0)
            return 0;
        ui_err("Gio phai co dang HH:MM, vd 06:30");
    }
}

int ui_read_password(const char *prompt, char *buf, size_t sz)
{
    struct termios old, noecho;
    int tty = isatty(STDIN_FILENO);
    if (tty && tcgetattr(STDIN_FILENO, &old) == 0) {
        noecho = old;
        noecho.c_lflag &= ~(tcflag_t)ECHO;   /* tắt hiển thị ký tự gõ vào */
        tcsetattr(STDIN_FILENO, TCSAFLUSH, &noecho);
    } else {
        tty = 0;
    }

    int r;
    for (;;) {
        r = ui_read_line(prompt, buf, sz);
        if (tty)
            printf("\n");           /* Enter không hiện khi tắt echo, nên tự xuống dòng */
        if (r < 0 || strchr(buf, ' ') == NULL)
            break;
        ui_err("Mat khau khong duoc chua dau cach");
    }

    if (tty)
        tcsetattr(STDIN_FILENO, TCSAFLUSH, &old);   /* LUÔN khôi phục, kể cả khi EOF */
    return r;
}

int ui_confirm(const char *prompt)
{
    char buf[16], p[160];
    snprintf(p, sizeof p, "%s (y/n): ", prompt);
    if (ui_read_line(p, buf, sizeof buf) < 0)
        return 0;
    return buf[0] == 'y' || buf[0] == 'Y';
}

int ui_menu(const char *title, const char *items[], int n, const char *back_label)
{
    printf("\n=== %s ===\n", title);
    for (int i = 0; i < n; i++)
        printf("%d. %s\n", i + 1, items[i]);
    printf("0. %s\n", back_label ? back_label : "Quay lai");
    for (;;) {
        int choice;
        int r = ui_read_int("Chon: ", 0, n, &choice);
        if (r < 0)
            return -1;
        if (r == 0)
            return choice;
        /* Enter rỗng: hỏi lại */
    }
}

/* Tô màu chỉ khi stdout là terminal. Ghi ra file/pipe thì không chèn mã màu rác. */
static void vmsg(const char *color, const char *tag, const char *fmt, va_list ap)
{
    int tty = isatty(STDOUT_FILENO);
    printf("%s%s%s ", tty ? color : "", tag, tty ? "\033[0m" : "");
    vprintf(fmt, ap);
    printf("\n");
}

void ui_ok(const char *fmt, ...)   { va_list ap; va_start(ap, fmt); vmsg("\033[32m", "[OK]", fmt, ap); va_end(ap); }
void ui_err(const char *fmt, ...)  { va_list ap; va_start(ap, fmt); vmsg("\033[31m", "[LOI]", fmt, ap); va_end(ap); }
void ui_info(const char *fmt, ...) { va_list ap; va_start(ap, fmt); vmsg("\033[36m", "[i]", fmt, ap); va_end(ap); }

const char *ui_explain_code(int code)
{
    switch (code) {
    case RC_OK:             return "Thanh cong.";
    case RC_BAD_REQUEST:    return "Lenh sai cu phap.";
    case RC_AUTH_FAILED:    return "Sai mat khau.";
    case RC_INVALID_TOKEN:  return "Phien lam viec het han hoac mat khau da bi doi. Hay ket noi lai.";
    case RC_NO_SUCH_DEVICE: return "Thiet bi khong ton tai tren may nay.";
    case RC_NOT_SUPPORTED:  return "Thiet bi nay khong ho tro chuc nang do.";
    case RC_POWER_OFF:      return "Thiet bi dang tat nguon.";
    case RC_INVALID_PARAM:  return "Gia tri khong hop le.";
    case RC_LOCKED:         return "Thiet bi tam khoa do nhap sai nhieu lan.";
    case RC_SERVER_ERROR:   return "Loi phia thiet bi.";
    default:                return "Phan hoi khong xac dinh.";
    }
}

void ui_hr(int width)
{
    for (int i = 0; i < width; i++)
        putchar('-');
    putchar('\n');
}

void ui_pause(void)
{
    char buf[8];
    ui_read_line("Nhan Enter de tiep tuc...", buf, sizeof buf);
}
