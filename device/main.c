/*
 * device/main.c: chương trình device_sim (Long)
 * Bước 5: nạp thiết bị, in bảng rồi thoát (để kiểm tra device.c). Vòng select() làm ở bước 7.
 */
#include <stdio.h>
#include "device.h"
#include "dlog.h"
#include "kv.h"
#include "util.h"

int main(int argc, char *argv[])
{
    const char *config = argc > 1 ? argv[1] : "config/coop_A.conf";
    char err[256];

    if (ensure_dir("data") != 0)
        die("data");
    if (devices_load(config, err, sizeof err) != 0) {
        fprintf(stderr, "Loi cau hinh: %s\n", err);
        return 1;
    }

    sim_config_t *cfg = sim_config();
    printf("%s  config=%s  TCP=%d  devices=%d\n", PROTO_VERSION, cfg->conf_name,
           cfg->tcp_port, device_count());
    for (int i = 0; i < device_count(); i++) {
        device_t *d = device_at(i);
        char params[MAX_LINE] = "";
        device_params_to_kv(d, params, sizeof params);
        printf("  %-9s %-12s %-12s%s\n", d->id, dev_type_to_str(d->type), d->name, params);
    }

    if (dlog_open_all() != 0)
        die("logs");
    dlog_all(DLOG_INFO, "device_sim started (%s, tcp %d)", cfg->conf_name, cfg->tcp_port);
    dlog(device_find_first(DEV_FAN), DLOG_INFO, NULL, "thu ghi log rieng cho quat");
    dlog(device_find_first(DEV_FAN), DLOG_DEBUG, NULL, "dong DEBUG nay bi an");
    dlog_all(DLOG_INFO, "device_sim stopped");
    dlog_close_all();
    return 0;
}
