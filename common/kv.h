/*
 * common/kv.h: tách lệnh, đọc/ghi "key=value" (Minh). Dùng chung cho cả 2 chương trình.
 *
 * Mọi thông điệp và mọi file dữ liệu đều có dạng:
 *     LỆNH arg1 arg2 key1=val1 key2=val2
 * Một bộ phân tích dùng chung giúp hai bên hiểu cú pháp giống hệt nhau.
 */
#ifndef KV_H
#define KV_H

#include <stddef.h>
#include "protocol.h"

/* Tách line thành các từ tại dấu cách/tab. SỬA TRỰC TIẾP line (chèn '\0').
 * argv[i] trỏ vào bên trong line. Trả số từ, hoặc -1 nếu nhiều hơn max từ. */
int  split_words(char *line, char *argv[], int max);

/* Tìm từ dạng "key=..." trong argv, trả con trỏ tới phần giá trị hoặc NULL. */
const char *kv_find(int argc, char *argv[], const char *key);

/* Tách word "a=b" tại dấu '=' đầu tiên (sửa word). Trả 0, hoặc -1 nếu không có '='. */
int  kv_split_pair(char *word, char **key, char **val);

/* Lấy giá trị theo kiểu. Trả 0 được | -1 không có key | -2 sai kiểu/quá dài */
int  kv_get_str(int argc, char *argv[], const char *key, char *out, size_t outsz);
int  kv_get_double(int argc, char *argv[], const char *key, double *out);
int  kv_get_int(int argc, char *argv[], const char *key, int *out);

/* Đổi chuỗi thành số, chặt chẽ: từ chối "", "abc", "12x", "28,5", số tràn, nan/inf.
 * Trả 0 / -1. */
int  parse_double_strict(const char *s, double *out);
int  parse_int_strict(const char *s, int *out);

/* "06:30" <-> 390 (phút trong ngày). Chỉ nhận đúng dạng HH:MM. Trả 0 / -1. */
int  parse_hhmm(const char *s, int *minute);
void format_hhmm(int minute, char *buf, size_t sz);

/* Nối " key=<giá trị theo fmt>" vào cuối buf. Trả 0, hoặc -1 nếu không đủ chỗ
 * (khi đó buf giữ nguyên như trước lúc gọi). */
int  kv_append(char *buf, size_t sz, const char *key, const char *fmt, ...)
     __attribute__((format(printf, 4, 5)));

/* Kiểu thiết bị <-> chuỗi */
dev_type_t  dev_type_from_str(const char *s);   /* "FAN" -> DEV_FAN, sai -> DEV_INVALID */
const char *dev_type_to_str(dev_type_t t);      /* DEV_FAN -> "FAN" */
const char *dev_type_prefix(dev_type_t t);      /* DEV_FAN -> "FAN-" */
const char *dev_type_label(dev_type_t t);       /* DEV_FAN -> "Quat dieu hoa" (hiển thị) */

/* Mã phản hồi: "200 OK ..." -> 200; dòng không bắt đầu bằng 3 chữ số -> -1 */
int  resp_code(const char *line);
const char *rc_name(int code);                  /* 403 -> "INVALID_TOKEN" */

#endif /* KV_H */
