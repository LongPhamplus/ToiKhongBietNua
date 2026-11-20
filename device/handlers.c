/*
 * device/handlers.c: bảng điều phối lệnh (Long)
 *
 * Bảng { tên lệnh, hàm xử lý, cần token? }, giống IT4062/1-Chap1/lab05_function_pointer.c.
 * Thêm lệnh mới chỉ cần thêm một dòng vào bảng, không phải viết thêm chuỗi if/else.
 *
 * Tuần 1: HELLO, INFO (chưa kiểm tra token), BYE.
 * Tuần 2: CONNECT, CHPASS, DISCONNECT + bật kiểm tra token.
 * Tuần 3: POWER, MODE, SET, ACT, SCHED.  Tuần 4: LOG.
 */
#include "handlers.h"
#include "device.h"
#include "kv.h"
#include "util.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

/* Ghi "<mã> <TÊN> [chi tiết]" vào resp, trả lại mã */
static int reply(char *resp, size_t sz, int code, const char *fmt, ...)
{
    int n = snprintf(resp, sz, "%d %s", code, rc_name(code));
    if (fmt && n > 0 && (size_t)n < sz) {
        resp[n++] = ' ';
        resp[n] = '\0';
        va_list ap;
        va_start(ap, fmt);
        vsnprintf(resp + n, sz - (size_t)n, fmt, ap);
        va_end(ap);
    }
    return code;
}

/* HELLO: client kiểm tra "đầu bên kia đúng là device_sim" */
static int h_hello(client_t *c, int argc, char *argv[], char *resp, size_t sz)
{
    (void)c; (void)argc; (void)argv;
    return reply(resp, sz, RC_OK, "devices=%d server=%s", device_count(), PROTO_VERSION);
}

/* INFO <device_id>   (tuần 2 đổi thành INFO <device_id> <token>) */
static int h_info(client_t *c, int argc, char *argv[], char *resp, size_t sz)
{
    (void)c;
    if (argc < 2)
        return reply(resp, sz, RC_BAD_REQUEST, "missing_device_id");
    device_t *d = device_find(argv[1]);
    if (!d)
        return reply(resp, sz, RC_NO_SUCH_DEVICE, NULL);

    reply(resp, sz, RC_OK, "id=%s type=%s name=%s power=%s mode=%s",
          d->id, dev_type_to_str(d->type), d->name,
          d->power == PWR_ON ? "ON" : "OFF", d->mode == MODE_AUTO ? "AUTO" : "MANUAL");
    /* Thông số riêng theo loại (dùng chung hàm với devices_save) */
    if (device_params_to_kv(d, resp, sz) != 0)
        return reply(resp, sz, RC_SERVER_ERROR, "response_too_long");
    return RC_OK;
}

/* BYE: trả lời rồi server.c đóng kết nối */
static int h_bye(client_t *c, int argc, char *argv[], char *resp, size_t sz)
{
    (void)argc; (void)argv;
    c->close_after = 1;
    return reply(resp, sz, RC_OK, NULL);
}

static const struct {
    const char *name;
    handler_fn  fn;
    int         need_token;   /* tuần 1 chưa dùng; tuần 2 kiểm tra trước khi gọi fn */
} COMMANDS[] = {
    { CMD_HELLO, h_hello, 0 },
    { CMD_INFO,  h_info,  0 },   /* tuần 2: đổi thành 1 */
    { CMD_BYE,   h_bye,   0 },
};

int handlers_dispatch(client_t *c, const char *line, char *resp, size_t sz)
{
    /* split_words sửa trực tiếp chuỗi, nên làm trên bản sao để không phá dòng gốc */
    char copy[MAX_LINE];
    safe_copy(copy, sizeof copy, line);

    char *argv[MAX_ARGS];
    int argc = split_words(copy, argv, MAX_ARGS);
    if (argc < 0)
        return reply(resp, sz, RC_BAD_REQUEST, "too_many_args");
    if (argc == 0)
        return reply(resp, sz, RC_BAD_REQUEST, "empty");

    for (size_t i = 0; i < sizeof COMMANDS / sizeof COMMANDS[0]; i++)
        if (strcmp(argv[0], COMMANDS[i].name) == 0)
            return COMMANDS[i].fn(c, argc, argv, resp, sz);

    return reply(resp, sz, RC_BAD_REQUEST, "unknown_command");
}
