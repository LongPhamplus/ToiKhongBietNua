/*
 * common/util.h: tiện ích dùng chung cho cả 2 chương trình (Long)
 */
#ifndef UTIL_H
#define UTIL_H

#include <stddef.h>

/* Sinh 2*nbytes ký tự hex ngẫu nhiên (chữ thường) + '\0' vào out.
 * out phải có ít nhất 2*nbytes + 1 byte. Trả 0, hoặc -1 nếu dùng nguồn dự phòng rand(). */
int  random_hex(char *out, size_t nbytes);

/* Thời gian đơn điệu (không bị lùi khi chỉnh giờ hệ thống), tính bằng mili giây.
 * Dùng để đo khoảng thời gian: timeout, nhịp tick. */
long long now_ms(void);

/* Giờ địa phương dạng "YYYY-MM-DD HH:MM:SS" (dùng cho log). */
void now_str(char *buf, size_t sz);

/* Ngày hôm nay dạng "YYYY-MM-DD". */
void today_str(char *buf, size_t sz);

/* Bỏ khoảng trắng (cả \r \n \t) ở hai đầu, sửa trực tiếp. Trả lại s. */
char *trim(char *s);

/* Chép chuỗi có giới hạn, luôn kết thúc bằng '\0' (thay cho strcpy). */
void safe_copy(char *dst, size_t dstsz, const char *src);

/* Đổi chữ thường thành chữ hoa, sửa trực tiếp. */
void str_upper(char *s);

/* Tạo thư mục nếu chưa có. Trả 0 / -1. */
int  ensure_dir(const char *path);

/* perror(msg) rồi exit(1). Chỉ dùng cho lỗi lúc khởi động. */
void die(const char *msg);

#endif /* UTIL_H */
