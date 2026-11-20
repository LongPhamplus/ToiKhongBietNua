/*
 * controller/ui.h: nhập/xuất an toàn cho giao diện dòng lệnh (Minh)
 * Mọi màn hình (kể cả ui_climate.c của Long ở tuần 3) đều dùng các hàm này.
 *
 * QUY ƯỚC GIÁ TRỊ TRẢ VỀ của các hàm ui_read_*:
 *    0  đọc được giá trị hợp lệ
 *    1  người dùng nhấn Enter rỗng (huỷ / bỏ qua)
 *   -1  hết dữ liệu vào (Ctrl+D / EOF): người gọi nên quay lại menu trước
 */
#ifndef UI_H
#define UI_H

#include <stddef.h>

int  ui_read_line(const char *prompt, char *buf, size_t sz);
int  ui_read_int(const char *prompt, int min, int max, int *out);
int  ui_read_double(const char *prompt, double min, double max, double *out);

/* Các bản *_keep hiện giá trị hiện tại trong ngoặc []. Enter rỗng = giữ nguyên (trả 0). */
int  ui_read_int_keep(const char *prompt, int cur, int min, int max, int *out);
int  ui_read_double_keep(const char *prompt, double cur, double min, double max, double *out);
int  ui_read_str_keep(const char *prompt, const char *cur, char *buf, size_t sz);

int  ui_read_hhmm(const char *prompt, int *minute);

/* Nhập mật khẩu không hiện ký tự. Từ chối mật khẩu có dấu cách. */
int  ui_read_password(const char *prompt, char *buf, size_t sz);

int  ui_confirm(const char *prompt);         /* 1 = đồng ý (y), 0 = không */

/* In menu đánh số 1..n và mục "0. <back_label>". Trả lựa chọn 0..n, hoặc -1 nếu EOF. */
int  ui_menu(const char *title, const char *items[], int n, const char *back_label);

void ui_ok(const char *fmt, ...)   __attribute__((format(printf, 1, 2)));
void ui_err(const char *fmt, ...)  __attribute__((format(printf, 1, 2)));
void ui_info(const char *fmt, ...) __attribute__((format(printf, 1, 2)));

/* Mã phản hồi thành câu tiếng Việt dễ hiểu cho người dùng */
const char *ui_explain_code(int code);

void ui_hr(int width);                       /* in đường kẻ ngang */
void ui_pause(void);                         /* "Nhan Enter de tiep tuc" */

#endif /* UI_H */
