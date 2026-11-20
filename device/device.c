/*
 * device/device.c: nạp / lưu / khởi tạo thông số thiết bị (Long). TIÊU CHÍ 4.
 *
 * LUỒNG KHỞI ĐỘNG
 *   1. Đọc config/<tên>.conf: các dòng tcp_port= / udp_port= / speed= (thông số tiến trình).
 *   2. Nếu data/devices_<tên>.conf đã có: nạp thiết bị từ đó (ID, mật khẩu, thông số giữ nguyên).
 *      Nếu chưa có (lần chạy đầu): lấy các dòng "device ..." trong config, điền mặc định,
 *      sinh ID ngẫu nhiên, rồi LƯU NGAY ra data/.
 *   3. Kiểm tra ràng buộc thông số. Sai thì báo "file:dòng: lý do" và không chạy.
 */
#include "device.h"
#include "kv.h"
#include "util.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DEFAULT_PASSWORD "123456"
#define DATA_DIR         "data"

static device_t     g_devs[MAX_DEVICES];
static int          g_ndev = 0;
static sim_config_t g_cfg;
static char         g_state_path[256];

sim_config_t *sim_config(void)  { return &g_cfg; }
int           device_count(void) { return g_ndev; }
device_t     *device_at(int i)   { return (i >= 0 && i < g_ndev) ? &g_devs[i] : NULL; }

device_t *device_find(const char *id)
{
    for (int i = 0; i < g_ndev; i++)
        if (strcmp(g_devs[i].id, id) == 0)
            return &g_devs[i];
    return NULL;
}

device_t *device_find_first(dev_type_t type)
{
    for (int i = 0; i < g_ndev; i++)
        if (g_devs[i].type == type)
            return &g_devs[i];
    return NULL;
}

/* ------------------------------------------------------------------------
 * Giá trị mặc định: KE_HOACH.md bảng 4.1
 * ------------------------------------------------------------------------ */
static void default_schedule(feed_params_t *f)
{
    f->times[0] = 6 * 60;      /* 06:00 */
    f->times[1] = 11 * 60;     /* 11:00 */
    f->times[2] = 17 * 60;     /* 17:00 */
    f->n_times = 3;
}

void device_set_defaults(device_t *d, dev_type_t type)
{
    memset(d, 0, sizeof *d);   /* mọi trường khác về 0 / NULL */
    d->type  = type;
    d->power = PWR_ON;
    d->mode  = MODE_AUTO;
    safe_copy(d->password, sizeof d->password, DEFAULT_PASSWORD);

    switch (type) {
    case DEV_ENV_SENSOR:
        d->p.env.interval = 1;
        d->p.env.temp = 26.0;
        d->p.env.humidity = 65.0;
        d->p.env.last_read = -1;
        break;
    case DEV_EGG_COUNTER:
        d->p.egg.capacity = 300;
        d->p.egg.rate = 12.0;
        break;
    case DEV_FEEDER:
        d->p.feed.amount = 5.0;     /* W = 5 kg mỗi lần */
        d->p.feed.stock = 100.0;
        default_schedule(&d->p.feed);
        d->p.feed.last_run = -1;
        break;
    case DEV_WATERER:
        d->p.feed.amount = 10.0;    /* Vw = 10 lít mỗi lần */
        d->p.feed.stock = 500.0;
        default_schedule(&d->p.feed);
        d->p.feed.last_run = -1;
        break;
    case DEV_FAN:
        d->p.fan.Tmax = 32.0;
        d->p.fan.Tp1 = 28.0;
        break;
    case DEV_HEATER:
        d->p.heater.Tmin = 18.0;
        d->p.heater.Tp2 = 22.0;
        break;
    case DEV_MISTER:
        d->p.mister.Hmin = 55.0;
        d->p.mister.Hp = 70.0;
        d->p.mister.Vh = 2.0;
        break;
    default:
        break;
    }
}

/* ------------------------------------------------------------------------
 * Lịch "06:00,11:00,17:00" <-> mảng phút (sắp xếp tăng dần, không trùng)
 * ------------------------------------------------------------------------ */
static int cmp_int(const void *a, const void *b)
{
    return *(const int *)a - *(const int *)b;
}

static int parse_times(feed_params_t *f, const char *val, char *err, size_t errsz)
{
    int tmp[MAX_SCHED], n = 0;
    if (strcmp(val, "-") != 0) {               /* "-" = lịch rỗng */
        char copy[MAX_LINE];
        safe_copy(copy, sizeof copy, val);
        char *save = NULL;
        for (char *t = strtok_r(copy, ",", &save); t; t = strtok_r(NULL, ",", &save)) {
            if (n >= MAX_SCHED) {
                snprintf(err, errsz, "lich qua %d moc", MAX_SCHED);
                return -1;
            }
            if (parse_hhmm(t, &tmp[n]) != 0) {
                snprintf(err, errsz, "gio '%s' khong dung dang HH:MM", t);
                return -1;
            }
            n++;
        }
    }
    qsort(tmp, (size_t)n, sizeof tmp[0], cmp_int);
    for (int i = 1; i < n; i++)
        if (tmp[i] == tmp[i - 1]) {
            snprintf(err, errsz, "lich bi trung gio");
            return -1;
        }
    memcpy(f->times, tmp, sizeof(int) * (size_t)n);
    f->n_times = n;
    return 0;
}

static void format_times(const feed_params_t *f, char *buf, size_t sz)
{
    if (f->n_times == 0) {
        safe_copy(buf, sz, "-");
        return;
    }
    buf[0] = '\0';
    for (int i = 0; i < f->n_times; i++) {
        char hh[8];
        format_hhmm(f->times[i], hh, sizeof hh);
        size_t len = strlen(buf);
        snprintf(buf + len, sz - len, "%s%s", i ? "," : "", hh);
    }
}

/* ------------------------------------------------------------------------
 * Đặt một thông số theo tên key
 * ------------------------------------------------------------------------ */
static int set_double(double *dst, const char *key, const char *val, char *err, size_t errsz)
{
    if (parse_double_strict(val, dst) != 0) {
        snprintf(err, errsz, "%s='%s' khong phai so", key, val);
        return -1;
    }
    return 0;
}

static int set_int(int *dst, const char *key, const char *val, char *err, size_t errsz)
{
    if (parse_int_strict(val, dst) != 0) {
        snprintf(err, errsz, "%s='%s' khong phai so nguyen", key, val);
        return -1;
    }
    return 0;
}

int device_set_param(device_t *d, const char *key, const char *val, char *err, size_t errsz)
{
#define IS(k) (strcmp(key, k) == 0)
    switch (d->type) {
    case DEV_ENV_SENSOR:
        if (IS("interval")) return set_int(&d->p.env.interval, key, val, err, errsz);
        break;
    case DEV_EGG_COUNTER:
        if (IS("capacity")) return set_int(&d->p.egg.capacity, key, val, err, errsz);
        if (IS("rate"))     return set_double(&d->p.egg.rate, key, val, err, errsz);
        break;
    case DEV_FEEDER:
        if (IS("W"))        return set_double(&d->p.feed.amount, key, val, err, errsz);
        if (IS("stock"))    return set_double(&d->p.feed.stock, key, val, err, errsz);
        if (IS("times"))    return parse_times(&d->p.feed, val, err, errsz);
        break;
    case DEV_WATERER:
        if (IS("Vw"))       return set_double(&d->p.feed.amount, key, val, err, errsz);
        if (IS("tank"))     return set_double(&d->p.feed.stock, key, val, err, errsz);
        if (IS("times"))    return parse_times(&d->p.feed, val, err, errsz);
        break;
    case DEV_FAN:
        if (IS("Tmax"))     return set_double(&d->p.fan.Tmax, key, val, err, errsz);
        if (IS("Tp1"))      return set_double(&d->p.fan.Tp1, key, val, err, errsz);
        break;
    case DEV_HEATER:
        if (IS("Tmin"))     return set_double(&d->p.heater.Tmin, key, val, err, errsz);
        if (IS("Tp2"))      return set_double(&d->p.heater.Tp2, key, val, err, errsz);
        break;
    case DEV_MISTER:
        if (IS("Hmin"))     return set_double(&d->p.mister.Hmin, key, val, err, errsz);
        if (IS("Hp"))       return set_double(&d->p.mister.Hp, key, val, err, errsz);
        if (IS("Vh"))       return set_double(&d->p.mister.Vh, key, val, err, errsz);
        break;
    default:
        break;
    }
#undef IS
    snprintf(err, errsz, "thong so '%s' khong ap dung cho %s", key, dev_type_to_str(d->type));
    return -1;
}

/* ------------------------------------------------------------------------
 * Kiểm tra ràng buộc
 * ------------------------------------------------------------------------ */
#define CHECK_RANGE(v, lo, hi, name)                                          \
    do {                                                                      \
        if ((v) < (lo) || (v) > (hi)) {                                       \
            snprintf(err, errsz, "%s=%g ngoai khoang [%g, %g]", name,         \
                     (double)(v), (double)(lo), (double)(hi));                \
            return -1;                                                        \
        }                                                                     \
    } while (0)

int device_validate(const device_t *d, char *err, size_t errsz)
{
    switch (d->type) {
    case DEV_ENV_SENSOR:
        CHECK_RANGE(d->p.env.interval, 1, 60, "interval");
        break;
    case DEV_EGG_COUNTER:
        CHECK_RANGE(d->p.egg.capacity, 1, 10000, "capacity");
        CHECK_RANGE(d->p.egg.rate, 0, 1000, "rate");
        break;
    case DEV_FEEDER:
        CHECK_RANGE(d->p.feed.amount, 0.1, 50, "W");
        CHECK_RANGE(d->p.feed.stock, 0, 10000, "stock");
        break;
    case DEV_WATERER:
        CHECK_RANGE(d->p.feed.amount, 0.1, 200, "Vw");
        CHECK_RANGE(d->p.feed.stock, 0, 10000, "tank");
        break;
    case DEV_FAN:
        CHECK_RANGE(d->p.fan.Tmax, 10, 45, "Tmax");
        CHECK_RANGE(d->p.fan.Tp1, 10, 45, "Tp1");
        if (d->p.fan.Tp1 >= d->p.fan.Tmax) {
            snprintf(err, errsz, "Tp1=%.1f phai nho hon Tmax=%.1f", d->p.fan.Tp1, d->p.fan.Tmax);
            return -1;
        }
        break;
    case DEV_HEATER:
        CHECK_RANGE(d->p.heater.Tmin, 5, 35, "Tmin");
        CHECK_RANGE(d->p.heater.Tp2, 5, 35, "Tp2");
        if (d->p.heater.Tmin >= d->p.heater.Tp2) {
            snprintf(err, errsz, "Tmin=%.1f phai nho hon Tp2=%.1f", d->p.heater.Tmin, d->p.heater.Tp2);
            return -1;
        }
        break;
    case DEV_MISTER:
        CHECK_RANGE(d->p.mister.Hmin, 10, 95, "Hmin");
        CHECK_RANGE(d->p.mister.Hp, 10, 95, "Hp");
        CHECK_RANGE(d->p.mister.Vh, 0.1, 20, "Vh");
        if (d->p.mister.Hmin >= d->p.mister.Hp) {
            snprintf(err, errsz, "Hmin=%.1f phai nho hon Hp=%.1f", d->p.mister.Hmin, d->p.mister.Hp);
            return -1;
        }
        break;
    default:
        snprintf(err, errsz, "kieu thiet bi khong hop le");
        return -1;
    }
    return 0;
}

/* Ràng buộc CHÉO giữa các thiết bị: đèn sưởi phải tắt (Tp2) trước ngưỡng quạt
 * tắt (Tp1). Nếu không, có khoảng nhiệt độ mà quạt và đèn cùng chạy, "đánh nhau". */
static int validate_cross(char *err, size_t errsz)
{
    device_t *fan = device_find_first(DEV_FAN);
    device_t *hea = device_find_first(DEV_HEATER);
    if (fan && hea && hea->p.heater.Tp2 >= fan->p.fan.Tp1) {
        snprintf(err, errsz, "Tp2 cua %s (%.1f) phai nho hon Tp1 cua %s (%.1f)",
                 hea->id, hea->p.heater.Tp2, fan->id, fan->p.fan.Tp1);
        return -1;
    }
    return 0;
}

/* ------------------------------------------------------------------------
 * Chuyển thông số riêng của loại thành chuỗi " key=value ..."
 * ------------------------------------------------------------------------ */
int device_params_to_kv(const device_t *d, char *buf, size_t sz)
{
    char times[MAX_SCHED * 6 + 2];
    int rc = 0;
    switch (d->type) {
    case DEV_ENV_SENSOR:
        rc |= kv_append(buf, sz, "interval", "%d", d->p.env.interval);
        break;
    case DEV_EGG_COUNTER:
        rc |= kv_append(buf, sz, "capacity", "%d", d->p.egg.capacity);
        rc |= kv_append(buf, sz, "rate", "%.1f", d->p.egg.rate);
        break;
    case DEV_FEEDER:
    case DEV_WATERER: {
        int fed = d->type == DEV_FEEDER;
        format_times(&d->p.feed, times, sizeof times);
        rc |= kv_append(buf, sz, fed ? "W" : "Vw", "%.1f", d->p.feed.amount);
        rc |= kv_append(buf, sz, fed ? "stock" : "tank", "%.1f", d->p.feed.stock);
        rc |= kv_append(buf, sz, "times", "%s", times);
        break;
    }
    case DEV_FAN:
        rc |= kv_append(buf, sz, "Tmax", "%.1f", d->p.fan.Tmax);
        rc |= kv_append(buf, sz, "Tp1", "%.1f", d->p.fan.Tp1);
        break;
    case DEV_HEATER:
        rc |= kv_append(buf, sz, "Tmin", "%.1f", d->p.heater.Tmin);
        rc |= kv_append(buf, sz, "Tp2", "%.1f", d->p.heater.Tp2);
        break;
    case DEV_MISTER:
        rc |= kv_append(buf, sz, "Hmin", "%.1f", d->p.mister.Hmin);
        rc |= kv_append(buf, sz, "Hp", "%.1f", d->p.mister.Hp);
        rc |= kv_append(buf, sz, "Vh", "%.1f", d->p.mister.Vh);
        break;
    default:
        break;
    }
    return rc ? -1 : 0;
}

/* ------------------------------------------------------------------------
 * Đọc một dòng "device key=value ..." thành một device_t
 * ------------------------------------------------------------------------ */
static int parse_device_words(int argc, char *argv[], device_t *d, char *err, size_t errsz)
{
    const char *type_s = kv_find(argc, argv, "type");
    if (!type_s) {
        snprintf(err, errsz, "thieu type=");
        return -1;
    }
    dev_type_t t = dev_type_from_str(type_s);
    if (t == DEV_INVALID) {
        snprintf(err, errsz, "type='%s' khong hop le", type_s);
        return -1;
    }
    device_set_defaults(d, t);      /* điền mặc định TRƯỚC, sau đó key nào có thì ghi đè */

    for (int i = 1; i < argc; i++) {
        char word[MAX_LINE];
        char *key, *val;
        safe_copy(word, sizeof word, argv[i]);
        if (kv_split_pair(word, &key, &val) != 0) {
            snprintf(err, errsz, "'%s' khong dung dang key=value", argv[i]);
            return -1;
        }
        if (strcmp(key, "type") == 0) {
            continue;
        } else if (strcmp(key, "id") == 0) {
            if (strlen(val) == 0 || strlen(val) >= sizeof d->id) {
                snprintf(err, errsz, "id='%s' khong hop le", val);
                return -1;
            }
            safe_copy(d->id, sizeof d->id, val);
        } else if (strcmp(key, "name") == 0) {
            if (strlen(val) == 0 || strlen(val) >= sizeof d->name) {
                snprintf(err, errsz, "name dai toi da %d ky tu", (int)sizeof d->name - 1);
                return -1;
            }
            safe_copy(d->name, sizeof d->name, val);
        } else if (strcmp(key, "password") == 0) {
            size_t n = strlen(val);
            if (n < MIN_PASS || n > MAX_PASS) {
                snprintf(err, errsz, "password phai dai %d-%d ky tu", MIN_PASS, MAX_PASS);
                return -1;
            }
            safe_copy(d->password, sizeof d->password, val);
        } else if (strcmp(key, "power") == 0) {
            if (strcmp(val, "ON") && strcmp(val, "OFF")) {
                snprintf(err, errsz, "power phai la ON/OFF");
                return -1;
            }
            d->power = strcmp(val, "ON") == 0 ? PWR_ON : PWR_OFF;
        } else if (strcmp(key, "mode") == 0) {
            if (strcmp(val, "AUTO") && strcmp(val, "MANUAL")) {
                snprintf(err, errsz, "mode phai la AUTO/MANUAL");
                return -1;
            }
            d->mode = strcmp(val, "AUTO") == 0 ? MODE_AUTO : MODE_MANUAL;
        } else if (device_set_param(d, key, val, err, errsz) != 0) {
            return -1;
        }
    }
    return 0;
}

/* Sinh ID "<tiền tố><4 hex hoa>" chưa có thiết bị nào dùng.
 * LƯU Ý: phải sinh vào biến tạm `cand` rồi mới chép ra `out`. Nếu ghi thẳng vào
 * out (= d->id của chính thiết bị đang xét) thì device_find(out) luôn tìm thấy
 * chính nó, nên vòng lặp không bao giờ dừng (lỗi gặp ở bước 5). */
static void gen_unique_id(dev_type_t t, char *out, size_t sz)
{
    char cand[MAX_ID];
    for (;;) {
        char hex[5];
        random_hex(hex, 2);
        str_upper(hex);
        snprintf(cand, sizeof cand, "%s%s", dev_type_prefix(t), hex);
        if (!device_find(cand))
            break;
    }
    safe_copy(out, sz, cand);
}

/* Lấy "coop_A" từ "config/coop_A.conf" */
static void conf_name_from_path(const char *path, char *out, size_t sz)
{
    const char *base = strrchr(path, '/');
    base = base ? base + 1 : path;
    safe_copy(out, sz, base);
    char *dot = strrchr(out, '.');
    if (dot && dot != out)
        *dot = '\0';
}

/* ------------------------------------------------------------------------
 * Đọc một file theo dòng: bỏ dòng rỗng và chú thích '#'.
 * want_devices = 1: lấy các dòng "device ..." ; 0: bỏ qua chúng.
 * want_process = 1: lấy các dòng tcp_port= ... ; 0: bỏ qua.
 * ------------------------------------------------------------------------ */
static int load_file(const char *path, int want_process, int want_devices, int need_id,
                     char *err, size_t errsz)
{
    FILE *f = fopen(path, "r");
    if (!f) {
        snprintf(err, errsz, "khong mo duoc %s", path);
        return -1;
    }

    char line[MAX_LINE * 2];
    char msg[256];
    int lineno = 0;
    while (fgets(line, sizeof line, f)) {
        lineno++;
        if (!strchr(line, '\n') && !feof(f)) {
            snprintf(err, errsz, "%s:%d: dong qua dai", path, lineno);
            fclose(f);
            return -1;
        }
        trim(line);
        if (line[0] == '\0' || line[0] == '#')
            continue;

        char *argv[MAX_ARGS];
        int argc = split_words(line, argv, MAX_ARGS);
        if (argc < 0) {
            snprintf(err, errsz, "%s:%d: qua nhieu tu tren mot dong", path, lineno);
            fclose(f);
            return -1;
        }

        if (strcmp(argv[0], "device") == 0) {
            if (!want_devices)
                continue;
            if (g_ndev >= MAX_DEVICES) {
                snprintf(err, errsz, "%s:%d: qua %d thiet bi", path, lineno, MAX_DEVICES);
                fclose(f);
                return -1;
            }
            device_t *d = &g_devs[g_ndev];
            if (parse_device_words(argc, argv, d, msg, sizeof msg) != 0 ||
                device_validate(d, msg, sizeof msg) != 0) {
                snprintf(err, errsz, "%s:%d: %s", path, lineno, msg);
                fclose(f);
                return -1;
            }
            if (need_id && d->id[0] == '\0') {
                snprintf(err, errsz, "%s:%d: thieu id=", path, lineno);
                fclose(f);
                return -1;
            }
            if (d->id[0] && device_find(d->id)) {
                snprintf(err, errsz, "%s:%d: trung id %s", path, lineno, d->id);
                fclose(f);
                return -1;
            }
            g_ndev++;
        } else {
            if (!want_process)
                continue;
            /* Dòng thông số tiến trình: mỗi từ là một cặp key=value */
            for (int i = 0; i < argc; i++) {
                char *key, *val;
                int v;
                if (kv_split_pair(argv[i], &key, &val) != 0 || parse_int_strict(val, &v) != 0) {
                    snprintf(err, errsz, "%s:%d: '%s' khong hop le", path, lineno, argv[i]);
                    fclose(f);
                    return -1;
                }
                if (strcmp(key, "tcp_port") == 0)      g_cfg.tcp_port = v;
                else if (strcmp(key, "udp_port") == 0) g_cfg.udp_port = v;
                else if (strcmp(key, "speed") == 0)    g_cfg.speed = v;
                else {
                    snprintf(err, errsz, "%s:%d: khong biet thong so '%s'", path, lineno, key);
                    fclose(f);
                    return -1;
                }
            }
        }
    }
    fclose(f);
    return 0;
}

int devices_load(const char *config_path, char *err, size_t errsz)
{
    g_ndev = 0;
    memset(&g_cfg, 0, sizeof g_cfg);
    g_cfg.tcp_port = DEFAULT_TCP_PORT;
    g_cfg.udp_port = DEFAULT_UDP_PORT;
    g_cfg.speed = 1;
    conf_name_from_path(config_path, g_cfg.conf_name, sizeof g_cfg.conf_name);
    snprintf(g_state_path, sizeof g_state_path, "%s/devices_%s.conf", DATA_DIR, g_cfg.conf_name);

    FILE *probe = fopen(g_state_path, "r");
    int have_state = probe != NULL;
    if (probe)
        fclose(probe);

    if (have_state) {
        /* Lần chạy sau: thông số tiến trình lấy từ config, thiết bị lấy từ data/ */
        if (load_file(config_path, 1, 0, 0, err, errsz) != 0)
            return -1;
        if (load_file(g_state_path, 0, 1, 1, err, errsz) != 0)
            return -1;
        printf("[i] Nap trang thai tu %s\n", g_state_path);
    } else {
        /* Lần chạy đầu: lấy cả hai từ config, sinh ID, lưu ngay */
        if (load_file(config_path, 1, 1, 0, err, errsz) != 0)
            return -1;
        for (int i = 0; i < g_ndev; i++) {
            device_t *d = &g_devs[i];
            if (d->id[0] == '\0')
                gen_unique_id(d->type, d->id, sizeof d->id);
            if (d->name[0] == '\0')
                snprintf(d->name, sizeof d->name, "%s_%d", dev_type_to_str(d->type), i + 1);
        }
        printf("[i] Lan chay dau: sinh ID moi cho %d thiet bi\n", g_ndev);
    }

    if (g_ndev == 0) {
        snprintf(err, errsz, "%s: khong co thiet bi nao", config_path);
        return -1;
    }
    if (validate_cross(err, errsz) != 0)
        return -1;
    if (!have_state && devices_save() != 0) {
        snprintf(err, errsz, "khong ghi duoc %s", g_state_path);
        return -1;
    }
    return 0;
}

int devices_save(void)
{
    /* Ghi ra file tạm rồi rename() đè lên file thật. rename là thao tác nguyên tử,
     * nên dù chương trình bị tắt giữa chừng, file thật vẫn là bản cũ nguyên vẹn,
     * không bao giờ bị ghi dở một nửa. */
    char tmp[300];
    snprintf(tmp, sizeof tmp, "%s.tmp", g_state_path);
    FILE *f = fopen(tmp, "w");
    if (!f)
        return -1;

    fprintf(f, "# Trang thai device_sim (%s) - tu sinh, khong sua khi dang chay\n", g_cfg.conf_name);
    for (int i = 0; i < g_ndev; i++) {
        const device_t *d = &g_devs[i];
        char line[MAX_LINE];
        snprintf(line, sizeof line, "device id=%s type=%s name=%s password=%s power=%s mode=%s",
                 d->id, dev_type_to_str(d->type), d->name, d->password,
                 d->power == PWR_ON ? "ON" : "OFF", d->mode == MODE_AUTO ? "AUTO" : "MANUAL");
        device_params_to_kv(d, line, sizeof line);
        fprintf(f, "%s\n", line);
    }
    if (fclose(f) != 0) {
        remove(tmp);
        return -1;
    }
    return rename(tmp, g_state_path) == 0 ? 0 : -1;
}
