/*
 * controller/coop.c: quản lý chuồng nuôi (Minh). TIÊU CHÍ 13.
 *
 * Định dạng <data>/coops.txt (đọc/ghi bằng kv.c, giống định dạng của giao thức):
 *   coop id=1 name=Chuong_A location=Khu_1 capacity=500 chickens=420 created=2026-11-25
 *   dev  coop=1 id=FAN-8A21 type=FAN ip=127.0.0.1 port=7000 name=Quat_1
 */
#include "coop.h"
#include "kv.h"
#include "ui.h"
#include "util.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static coop_t *g_coops = NULL;   /* mảng động, nhân đôi khi đầy (IT4062 lab03) */
static int     g_n = 0, g_cap = 0;
static int     g_next_id = 1;
static char    g_path[200];

int     coop_count(void) { return g_n; }
coop_t *coop_at(int i)   { return (i >= 0 && i < g_n) ? &g_coops[i] : NULL; }

coop_t *coop_find(int id)
{
    for (int i = 0; i < g_n; i++)
        if (g_coops[i].id == id)
            return &g_coops[i];
    return NULL;
}

static coop_t *find_by_name(const char *name)
{
    for (int i = 0; i < g_n; i++)
        if (strcmp(g_coops[i].name, name) == 0)
            return &g_coops[i];
    return NULL;
}

coop_t *coop_find_device(const char *device_id, coop_device_t **out)
{
    for (int i = 0; i < g_n; i++)
        for (int j = 0; j < g_coops[i].n_dev; j++)
            if (strcmp(g_coops[i].devs[j].id, device_id) == 0) {
                if (out)
                    *out = &g_coops[i].devs[j];
                return &g_coops[i];
            }
    return NULL;
}

/* Tên/vị trí được lưu dạng key=value cách nhau bởi dấu cách, nên:
 * dấu cách đổi thành '_', còn dấu '=' thì không cho phép. */
static void sanitize(char *s)
{
    for (; *s; s++)
        if (*s == ' ' || *s == '\t')
            *s = '_';
}

static int check_text(const char *what, const char *s, char *err, size_t errsz)
{
    if (s[0] == '\0') {
        snprintf(err, errsz, "%s khong duoc de trong", what);
        return -1;
    }
    if (strlen(s) >= MAX_NAME) {
        snprintf(err, errsz, "%s dai toi da %d ky tu", what, MAX_NAME - 1);
        return -1;
    }
    if (strchr(s, '=')) {
        snprintf(err, errsz, "%s khong duoc chua dau '='", what);
        return -1;
    }
    return 0;
}

static int validate(int self_id, const char *name, const char *loc, int cap, int chick,
                    char *err, size_t errsz)
{
    if (check_text("Ten", name, err, errsz) || check_text("Vi tri", loc, err, errsz))
        return -1;
    coop_t *same = find_by_name(name);
    if (same && same->id != self_id) {
        snprintf(err, errsz, "Da co chuong ten '%s'", name);
        return -1;
    }
    if (cap <= 0) {
        snprintf(err, errsz, "Suc chua phai > 0");
        return -1;
    }
    if (chick < 0 || chick > cap) {
        snprintf(err, errsz, "So ga phai trong khoang 0..%d (suc chua)", cap);
        return -1;
    }
    return 0;
}

static coop_t *push_coop(void)
{
    if (g_n == g_cap) {
        int ncap = g_cap ? g_cap * 2 : 4;
        /* realloc có thể DỜI mảng sang chỗ khác, nên mọi con trỏ coop_t* cũ đều
         * mất hiệu lực. Không được giữ con trỏ chuồng qua một lần thêm chuồng. */
        coop_t *p = realloc(g_coops, sizeof *p * (size_t)ncap);
        if (!p)
            return NULL;
        g_coops = p;
        g_cap = ncap;
    }
    coop_t *c = &g_coops[g_n++];
    memset(c, 0, sizeof *c);
    return c;
}

int coop_add(const char *name, const char *location, int capacity, int chickens,
             char *err, size_t errsz)
{
    char n[MAX_LINE], l[MAX_LINE];
    safe_copy(n, sizeof n, name);
    safe_copy(l, sizeof l, location);
    sanitize(n);
    sanitize(l);
    if (validate(0, n, l, capacity, chickens, err, errsz) != 0)
        return -1;

    coop_t *c = push_coop();
    if (!c) {
        snprintf(err, errsz, "Het bo nho");
        return -1;
    }
    c->id = g_next_id++;
    safe_copy(c->name, sizeof c->name, n);
    safe_copy(c->location, sizeof c->location, l);
    c->capacity = capacity;
    c->chickens = chickens;
    today_str(c->created, sizeof c->created);
    return c->id;
}

int coop_edit(int id, const char *name, const char *location, int capacity, int chickens,
              char *err, size_t errsz)
{
    coop_t *c = coop_find(id);
    if (!c) {
        snprintf(err, errsz, "Khong co chuong %d", id);
        return -1;
    }
    char n[MAX_LINE], l[MAX_LINE];
    safe_copy(n, sizeof n, name);
    safe_copy(l, sizeof l, location);
    sanitize(n);
    sanitize(l);
    if (validate(id, n, l, capacity, chickens, err, errsz) != 0)
        return -1;
    safe_copy(c->name, sizeof c->name, n);
    safe_copy(c->location, sizeof c->location, l);
    c->capacity = capacity;
    c->chickens = chickens;
    return 0;
}

int coop_remove(int id)
{
    for (int i = 0; i < g_n; i++) {
        if (g_coops[i].id != id)
            continue;
        free(g_coops[i].devs);       /* giải phóng mảng thiết bị của chuồng trước */
        memmove(&g_coops[i], &g_coops[i + 1], sizeof(coop_t) * (size_t)(g_n - i - 1));
        g_n--;
        return 0;
    }
    return -1;
}

int coop_add_device(coop_t *c, const coop_device_t *d, char *err, size_t errsz)
{
    coop_t *owner = coop_find_device(d->id, NULL);
    if (owner) {
        snprintf(err, errsz, "Thiet bi %s da thuoc chuong '%s'", d->id, owner->name);
        return -1;
    }
    if (c->n_dev == c->cap_dev) {
        int ncap = c->cap_dev ? c->cap_dev * 2 : 4;
        coop_device_t *p = realloc(c->devs, sizeof *p * (size_t)ncap);
        if (!p) {
            snprintf(err, errsz, "Het bo nho");
            return -1;
        }
        c->devs = p;
        c->cap_dev = ncap;
    }
    c->devs[c->n_dev++] = *d;
    return 0;
}

int coop_remove_device(coop_t *c, const char *device_id)
{
    for (int i = 0; i < c->n_dev; i++) {
        if (strcmp(c->devs[i].id, device_id) != 0)
            continue;
        memmove(&c->devs[i], &c->devs[i + 1], sizeof(coop_device_t) * (size_t)(c->n_dev - i - 1));
        c->n_dev--;
        return 0;
    }
    return -1;
}

void coop_free_all(void)
{
    for (int i = 0; i < g_n; i++)
        free(g_coops[i].devs);
    free(g_coops);
    g_coops = NULL;
    g_n = g_cap = 0;
}

/* ------------------------------------------------------------------------
 * Lưu / nạp file
 * ------------------------------------------------------------------------ */
int coop_save(void)
{
    char tmp[220];
    snprintf(tmp, sizeof tmp, "%s.tmp", g_path);
    FILE *f = fopen(tmp, "w");
    if (!f)
        return -1;
    fprintf(f, "# Du lieu chuong nuoi cua controller (tu sinh)\n");
    for (int i = 0; i < g_n; i++) {
        coop_t *c = &g_coops[i];
        fprintf(f, "coop id=%d name=%s location=%s capacity=%d chickens=%d created=%s\n",
                c->id, c->name, c->location, c->capacity, c->chickens, c->created);
        for (int j = 0; j < c->n_dev; j++) {
            coop_device_t *d = &c->devs[j];
            fprintf(f, "dev coop=%d id=%s type=%s ip=%s port=%d name=%s\n",
                    c->id, d->id, dev_type_to_str(d->type), d->ip, d->port, d->name);
        }
    }
    if (fclose(f) != 0)
        return -1;
    /* Ghi file tạm rồi rename: tắt ngang lúc đang ghi cũng không hỏng file thật */
    return rename(tmp, g_path) == 0 ? 0 : -1;
}

static int parse_coop_line(int argc, char *argv[])
{
    coop_t tmp;
    memset(&tmp, 0, sizeof tmp);
    if (kv_get_int(argc, argv, "id", &tmp.id) || tmp.id <= 0 ||
        kv_get_str(argc, argv, "name", tmp.name, sizeof tmp.name) ||
        kv_get_str(argc, argv, "location", tmp.location, sizeof tmp.location) ||
        kv_get_int(argc, argv, "capacity", &tmp.capacity) ||
        kv_get_int(argc, argv, "chickens", &tmp.chickens) ||
        kv_get_str(argc, argv, "created", tmp.created, sizeof tmp.created))
        return -1;
    if (coop_find(tmp.id) || find_by_name(tmp.name))
        return -1;                  /* trùng id/tên: coi là dòng hỏng */
    coop_t *c = push_coop();
    if (!c)
        return -1;
    *c = tmp;
    if (tmp.id >= g_next_id)
        g_next_id = tmp.id + 1;
    return 0;
}

static int parse_dev_line(int argc, char *argv[])
{
    int coop_id;
    char type_s[MAX_NAME];
    coop_device_t d;
    memset(&d, 0, sizeof d);
    if (kv_get_int(argc, argv, "coop", &coop_id) ||
        kv_get_str(argc, argv, "id", d.id, sizeof d.id) ||
        kv_get_str(argc, argv, "type", type_s, sizeof type_s) ||
        kv_get_str(argc, argv, "ip", d.ip, sizeof d.ip) ||
        kv_get_int(argc, argv, "port", &d.port) ||
        kv_get_str(argc, argv, "name", d.name, sizeof d.name))
        return -1;
    d.type = dev_type_from_str(type_s);
    coop_t *c = coop_find(coop_id);
    char err[128];
    if (d.type == DEV_INVALID || !c || coop_add_device(c, &d, err, sizeof err) != 0)
        return -1;
    return 0;
}

int coop_load(const char *data_dir)
{
    snprintf(g_path, sizeof g_path, "%s/coops.txt", data_dir);
    FILE *f = fopen(g_path, "r");
    if (!f)
        return 0;                   /* chưa có file: chưa có chuồng nào, không phải lỗi */

    char line[MAX_LINE];
    int lineno = 0, bad = 0;
    while (fgets(line, sizeof line, f)) {
        lineno++;
        trim(line);
        if (line[0] == '\0' || line[0] == '#')
            continue;
        char *argv[MAX_ARGS];
        int argc = split_words(line, argv, MAX_ARGS);
        int rc = -1;
        if (argc > 0 && strcmp(argv[0], "coop") == 0)
            rc = parse_coop_line(argc, argv);
        else if (argc > 0 && strcmp(argv[0], "dev") == 0)
            rc = parse_dev_line(argc, argv);
        if (rc != 0) {
            /* Người dùng có thể sửa tay file: dòng hỏng thì cảnh báo rồi bỏ qua,
             * không làm cả chương trình dừng lại */
            ui_err("%s:%d: bo qua dong hong", g_path, lineno);
            bad++;
        }
    }
    fclose(f);
    if (bad)
        ui_info("Da nap %d chuong (bo qua %d dong hong)", g_n, bad);
    return 0;
}

/* ------------------------------------------------------------------------
 * Giao diện
 * ------------------------------------------------------------------------ */
static void print_coops(void)
{
    if (g_n == 0) {
        ui_info("Chua co chuong nao.");
        return;
    }
    printf("%-3s | %-16s | %-12s | %-11s | %-8s | %s\n",
           "ID", "Ten", "Vi tri", "Ga/Suc chua", "Thiet bi", "Ngay tao");
    ui_hr(72);
    for (int i = 0; i < g_n; i++) {
        coop_t *c = &g_coops[i];
        char ga[24];
        snprintf(ga, sizeof ga, "%d/%d", c->chickens, c->capacity);
        printf("%-3d | %-16s | %-12s | %-11s | %-8d | %s\n",
               c->id, c->name, c->location, ga, c->n_dev, c->created);
    }
}

static void print_devices(const coop_t *c)
{
    printf("Thiet bi trong chuong '%s':\n", c->name);
    if (c->n_dev == 0) {
        ui_info("(chua co thiet bi)");
        return;
    }
    printf("%-3s | %-9s | %-20s | %-14s | %s\n", "STT", "ID", "Loai", "Ten", "IP:Port");
    ui_hr(72);
    for (int i = 0; i < c->n_dev; i++) {
        const coop_device_t *d = &c->devs[i];
        printf("%-3d | %-9s | %-20s | %-14s | %s:%d\n", i + 1, d->id,
               dev_type_label(d->type), d->name, d->ip, d->port);
    }
}

/* In danh sách rồi hỏi ID chuồng. Trả NULL nếu huỷ hoặc không có. */
static coop_t *choose_coop(void)
{
    print_coops();
    if (g_n == 0)
        return NULL;
    int id;
    if (ui_read_int("Nhap ID chuong (Enter = huy): ", 1, 1000000, &id) != 0)
        return NULL;
    coop_t *c = coop_find(id);
    if (!c)
        ui_err("Khong co chuong %d", id);
    return c;
}

static void save_or_warn(void)
{
    if (coop_save() != 0)
        ui_err("Khong ghi duoc %s", g_path);
}

static void menu_add(void)
{
    char name[MAX_LINE], loc[MAX_LINE], err[128];
    int cap, chick;
    if (ui_read_line("Ten chuong: ", name, sizeof name) < 0 ||
        ui_read_line("Vi tri: ", loc, sizeof loc) < 0 ||
        ui_read_int("Suc chua (so ga toi da): ", 1, 1000000, &cap) != 0 ||
        ui_read_int("So ga hien co: ", 0, 1000000, &chick) != 0)
        return;
    int id = coop_add(name, loc, cap, chick, err, sizeof err);
    if (id < 0) {
        ui_err("%s", err);
        return;
    }
    save_or_warn();
    ui_ok("Da them chuong ID %d", id);
}

static void menu_edit(void)
{
    coop_t *c = choose_coop();
    if (!c)
        return;
    char name[MAX_NAME], loc[MAX_NAME], err[128];
    int cap, chick, id = c->id;
    ui_info("Enter de giu gia tri cu");
    if (ui_read_str_keep("Ten", c->name, name, sizeof name) < 0 ||
        ui_read_str_keep("Vi tri", c->location, loc, sizeof loc) < 0 ||
        ui_read_int_keep("Suc chua", c->capacity, 1, 1000000, &cap) != 0 ||
        ui_read_int_keep("So ga", c->chickens, 0, 1000000, &chick) != 0)
        return;
    if (coop_edit(id, name, loc, cap, chick, err, sizeof err) != 0) {
        ui_err("%s", err);
        return;
    }
    save_or_warn();
    ui_ok("Da cap nhat chuong %d", id);
}

static void menu_remove(void)
{
    coop_t *c = choose_coop();
    if (!c)
        return;
    char q[128];
    snprintf(q, sizeof q, "Xoa chuong '%s' (%d thiet bi)?", c->name, c->n_dev);
    if (!ui_confirm(q))
        return;
    /* Tuần 2: ngắt kết nối các thiết bị của chuồng này trước khi xoá (conn_disconnect) */
    int id = c->id;
    coop_remove(id);
    save_or_warn();
    ui_ok("Da xoa chuong %d", id);
}

static void menu_add_device(void)
{
    coop_t *c = choose_coop();
    if (!c)
        return;
    int cid = c->id;
    coop_device_t d;
    memset(&d, 0, sizeof d);
    char buf[MAX_LINE], err[128];

    ui_info("Tuan 1: nhap tay. Tuan 2 se them truc tiep tu ket qua quet.");
    if (ui_read_line("ID thiet bi (vd FAN-8A21): ", buf, sizeof buf) < 0 || buf[0] == '\0')
        return;
    if (strlen(buf) >= sizeof d.id || strchr(buf, ' ') || strchr(buf, '=')) {
        ui_err("ID khong hop le");
        return;
    }
    safe_copy(d.id, sizeof d.id, buf);

    const char *types[DEV_TYPE_COUNT];
    for (int i = 0; i < DEV_TYPE_COUNT; i++)
        types[i] = dev_type_label((dev_type_t)i);
    int t = ui_menu("Loai thiet bi", types, DEV_TYPE_COUNT, "Huy");
    if (t <= 0)
        return;
    d.type = (dev_type_t)(t - 1);

    if (ui_read_str_keep("IP", "127.0.0.1", d.ip, sizeof d.ip) < 0 ||
        ui_read_int_keep("Port", DEFAULT_TCP_PORT, 1, 65535, &d.port) != 0 ||
        ui_read_str_keep("Ten", dev_type_to_str(d.type), buf, sizeof buf) < 0)
        return;
    sanitize(buf);
    if (check_text("Ten", buf, err, sizeof err) != 0) {
        ui_err("%s", err);
        return;
    }
    safe_copy(d.name, sizeof d.name, buf);

    c = coop_find(cid);
    if (coop_add_device(c, &d, err, sizeof err) != 0) {
        ui_err("%s", err);
        return;
    }
    save_or_warn();
    ui_ok("Da them %s vao chuong '%s'", d.id, c->name);
}

static void menu_remove_device(void)
{
    coop_t *c = choose_coop();
    if (!c)
        return;
    print_devices(c);
    if (c->n_dev == 0)
        return;
    int k;
    if (ui_read_int("STT thiet bi can xoa (Enter = huy): ", 1, c->n_dev, &k) != 0)
        return;
    char id[MAX_ID];
    safe_copy(id, sizeof id, c->devs[k - 1].id);
    coop_remove_device(c, id);
    save_or_warn();
    ui_ok("Da xoa %s khoi chuong", id);
}

void coop_menu(void)
{
    static const char *items[] = {
        "Danh sach chuong", "Them chuong", "Sua thong tin chuong", "Xoa chuong",
        "Xem thiet bi trong chuong", "Them thiet bi vao chuong (nhap tay)",
        "Xoa thiet bi khoi chuong",
    };
    for (;;) {
        int c = ui_menu("QUAN LY CHUONG NUOI", items, 7, NULL);
        if (c <= 0)
            return;
        switch (c) {
        case 1: print_coops(); break;
        case 2: menu_add(); break;
        case 3: menu_edit(); break;
        case 4: menu_remove(); break;
        case 5: { coop_t *x = choose_coop(); if (x) print_devices(x); break; }
        case 6: menu_add_device(); break;
        case 7: menu_remove_device(); break;
        }
    }
}
