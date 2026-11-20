/*
 * device/handlers.h: bảng điều phối lệnh TCP (Long)
 */
#ifndef HANDLERS_H
#define HANDLERS_H

#include <stddef.h>
#include "server.h"

/* Mọi hàm xử lý lệnh có CÙNG chữ ký, nên đặt chung được vào một bảng.
 * argv[0] là tên lệnh. Hàm ghi đúng MỘT dòng phản hồi vào resp và trả mã phản hồi. */
typedef int (*handler_fn)(client_t *c, int argc, char *argv[], char *resp, size_t sz);

/* Điểm vào duy nhất server.c gọi: tách dòng, tra bảng, gọi hàm. Trả mã phản hồi. */
int handlers_dispatch(client_t *c, const char *line, char *resp, size_t sz);

#endif /* HANDLERS_H */
