/*
 * controller/main.c: chương trình controller (Minh)
 *
 * Đề bài: "Mỗi ứng dụng cài đặt được khởi tạo một giá trị ID ngẫu nhiên".
 * App ID là DANH TÍNH của controller. Thiết bị gắn token với App ID này (tuần 2),
 * nên phải sinh MỘT lần rồi giữ cố định (lưu <data>/app.conf).
 * Xoá file đó thì coi như cài lại app.
 *
 * Tham số -d <thư mục data>: chạy 2 controller với 2 App ID khác nhau trên cùng một
 * máy (tuần 2 cần để demo "app A đổi mật khẩu thì app B bị yêu cầu kết nối lại").
 */
#include <ctype.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "protocol.h"
#include "ui.h"
#include "util.h"

static char g_data_dir[128] = "data";
static char g_app_id[APP_ID_HEX_LEN + 1];

static int valid_app_id(const char *s)
{
    if (strlen(s) != APP_ID_HEX_LEN)
        return 0;
    for (; *s; s++)
        if (!isxdigit((unsigned char)*s))
            return 0;
    return 1;
}

/* Đọc app_id= trong <data>/app.conf. Chưa có hoặc sai định dạng thì sinh mới rồi lưu. */
static int load_or_create_app_id(void)
{
    char path[160];
    snprintf(path, sizeof path, "%s/app.conf", g_data_dir);

    FILE *f = fopen(path, "r");
    if (f) {
        char line[128];
        while (fgets(line, sizeof line, f)) {
            trim(line);
            if (strncmp(line, "app_id=", 7) == 0 && valid_app_id(line + 7)) {
                safe_copy(g_app_id, sizeof g_app_id, line + 7);
                fclose(f);
                return 0;
            }
        }
        fclose(f);
        ui_err("%s khong hop le, sinh App ID moi", path);
    }

    random_hex(g_app_id, APP_ID_HEX_LEN / 2);     /* 4 byte = 8 ký tự hex */
    f = fopen(path, "w");
    if (!f) {
        ui_err("Khong ghi duoc %s", path);
        return -1;
    }
    fprintf(f, "# App ID cua controller - xoa file nay = cai dat lai\napp_id=%s\n", g_app_id);
    fclose(f);
    ui_info("Da khoi tao App ID moi: %s", g_app_id);
    return 0;
}

static void not_yet(const char *what, int week)
{
    ui_info("Chuc nang '%s' se lam o tuan %d.", what, week);
}

int main(int argc, char *argv[])
{
    int opt;
    while ((opt = getopt(argc, argv, "d:h")) != -1) {
        if (opt == 'd') {
            safe_copy(g_data_dir, sizeof g_data_dir, optarg);
        } else {
            printf("Cach dung: %s [-d thu_muc_du_lieu]\n", argv[0]);
            return opt == 'h' ? 0 : 1;
        }
    }

    /* Thiết bị đóng kết nối mà app vẫn gửi thì không bị SIGPIPE giết */
    signal(SIGPIPE, SIG_IGN);

    if (ensure_dir(g_data_dir) != 0) {
        ui_err("Khong tao duoc thu muc %s", g_data_dir);
        return 1;
    }
    if (load_or_create_app_id() != 0)
        return 1;

    static const char *items[] = {
        "Quan ly chuong nuoi",
        "Quet thiet bi",
        "Vao mot chuong",
        "Tong quan chuong",
        "[DEV] Gui lenh tho",
    };
    for (;;) {
        char title[96];
        snprintf(title, sizeof title, "CHICKEN FARM CONTROLLER | App ID: %s", g_app_id);
        int c = ui_menu(title, items, 5, "Thoat");
        if (c <= 0)
            break;                      /* 0 = Thoát, -1 = hết dữ liệu vào */
        switch (c) {
        case 1: not_yet("Quan ly chuong", 1); break;   /* bước 11 */
        case 2: not_yet("Quet thiet bi", 2); break;
        case 3: not_yet("Vao mot chuong", 2); break;
        case 4: not_yet("Tong quan chuong", 3); break;
        case 5: not_yet("Gui lenh tho", 1); break;     /* bước 12 */
        }
    }
    printf("Tam biet.\n");
    return 0;
}
