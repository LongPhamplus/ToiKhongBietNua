/*
 * device/device.h: mô hình dữ liệu thiết bị
 * Phụ trách: Long. Minh phải đọc kỹ vì dev_feed.c và dev_mister.c (tuần 3) dùng trực tiếp.
 *
 * TẠI SAO DÙNG union
 *   Mỗi thiết bị chỉ thuộc MỘT loại, nên chỉ cần thông số của loại đó. union cho
 *   phép các struct thông số dùng chung một vùng nhớ: dev->p.fan.Tmax, dev->p.feed.amount, ...
 *   Truy cập sai nhánh (đọc p.fan của một FEEDER) là lỗi logic, nên luôn kiểm tra dev->type trước.
 */
#ifndef DEVICE_H
#define DEVICE_H

#include <stdio.h>
#include <time.h>
#include "protocol.h"

/* ---- Thông số theo loại (giá trị mặc định: KE_HOACH.md bảng 4.1) -------- */
typedef struct {
    int    interval;      /* phút mô phỏng giữa 2 lần đo */
    double temp;          /* số đo gần nhất (trạng thái, không lưu file) */
    double humidity;
    int    last_read;     /* phút trong ngày của lần đo cuối, -1 = chưa đo */
} env_params_t;

typedef struct {
    int    capacity;      /* sức chứa máng */
    double rate;          /* trứng / giờ */
    double count;         /* double để cộng dồn phần lẻ mỗi tick */
    int    total_collected;
    int    full;
} egg_params_t;

/* FEEDER và WATERER dùng CHUNG struct này:
 *   amount = W (kg) hoặc Vw (lít) mỗi lần; stock = lượng trong bồn (kg hoặc lít) */
typedef struct {
    double amount;
    double stock;
    int    times[MAX_SCHED];        /* phút trong ngày, sắp xếp tăng dần */
    int    n_times;
    int    last_run_day[MAX_SCHED]; /* ngày mô phỏng đã chạy mốc i (tránh chạy 2 lần) */
    double today_total;
    int    last_run;                /* phút trong ngày, -1 = chưa chạy */
    int    running;
} feed_params_t;

typedef struct { double Tmax, Tp1; int running; int run_minutes; } fan_params_t;
typedef struct { double Tmin, Tp2; int running; int run_minutes; } heater_params_t;
typedef struct { double Hmin, Hp, Vh; int running; double water_used; } mister_params_t;

/* Token cấp cho một App sau khi CONNECT đúng mật khẩu (dùng từ tuần 2) */
typedef struct {
    char   token[TOKEN_HEX_LEN + 1];
    char   app_id[APP_ID_HEX_LEN + 1];
    time_t created;
    time_t last_used;
} token_entry_t;

typedef struct device {
    char        id[MAX_ID];
    dev_type_t  type;
    char        name[MAX_NAME];
    char        password[MAX_PASS + 1];
    power_t     power;
    dev_mode_t  mode;

    token_entry_t tokens[MAX_TOKENS_PER_DEV];
    int         n_tokens;
    int         fail_count;   /* số lần sai mật khẩu liên tiếp */
    time_t      lock_until;

    FILE       *log;          /* logs/<id>.log, do dlog.c mở */

    union {
        env_params_t    env;
        egg_params_t    egg;
        feed_params_t   feed;
        fan_params_t    fan;
        heater_params_t heater;
        mister_params_t mister;
    } p;
} device_t;

/* Thông số của cả tiến trình device_sim (đọc từ đầu file cấu hình) */
typedef struct {
    char conf_name[64];   /* "coop_A" lấy từ "config/coop_A.conf" */
    int  tcp_port;
    int  udp_port;
    int  speed;           /* phút mô phỏng / giây thực (dùng từ tuần 3) */
} sim_config_t;

/* ---- API (cài đặt trong device.c) --------------------------------------- */

/* Nạp thiết bị. Trả 0 nếu thành công, -1 nếu lỗi (thông báo ghi vào err). */
int  devices_load(const char *config_path, char *err, size_t errsz);

/* Ghi toàn bộ trạng thái ra data/devices_<conf_name>.conf. Trả 0 / -1. */
int  devices_save(void);

sim_config_t *sim_config(void);
int        device_count(void);
device_t  *device_at(int i);
device_t  *device_find(const char *id);
device_t  *device_find_first(dev_type_t type);

void device_set_defaults(device_t *dev, dev_type_t type);

/* Đặt 1 thông số theo tên key (vd. "Tmax", "W", "times"). Trả 0, hoặc -1 kèm err.
 * Dùng cho file cấu hình (tuần 1) và lệnh SET (tuần 3). */
int  device_set_param(device_t *dev, const char *key, const char *val,
                      char *err, size_t errsz);

/* Kiểm tra ràng buộc thông số của một thiết bị. Trả 0, hoặc -1 kèm err. */
int  device_validate(const device_t *dev, char *err, size_t errsz);

/* Nối " key=value ..." các thông số riêng của loại vào buf (dùng cho lưu file và INFO).
 * Trả 0, hoặc -1 nếu buf không đủ chỗ. */
int  device_params_to_kv(const device_t *dev, char *buf, size_t sz);

#endif /* DEVICE_H */
