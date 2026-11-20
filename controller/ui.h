/*
 * =============================================================================
 * FILE        : controller/ui.h
 * PHỤ TRÁCH   : Minh        · NGƯỜI DÙNG: mọi file controller (cả ui_climate.c của Long)
 * PHASE       : 1 (bàn giao kèm H6, hạn 02/12)
 * =============================================================================
 *
 * CẦN KHAI BÁO (bộ công cụ nhập/xuất để mọi màn hình trông thống nhất)
 *   [ ] ui_read_line(prompt, buf, sz): fgets + trim. Trả -1 khi EOF (Ctrl+D).
 *   [ ] ui_read_int(prompt, min, max, &out): lặp hỏi lại đến khi hợp lệ hoặc người dùng bỏ (Enter rỗng).
 *   [ ] ui_read_double(prompt, min, max, &out)
 *   [ ] ui_read_double_keep(prompt, current, min, max, &out): hiện giá trị hiện tại,
 *       Enter rỗng nghĩa là giữ nguyên. Dùng cho mọi màn hình SET.
 *   [ ] ui_read_hhmm(prompt, &minute)
 *   [ ] ui_read_password(prompt, buf, sz): tắt echo (termios).
 *   [ ] ui_confirm(prompt): hỏi y/n.
 *   [ ] ui_menu(title, items[], n): in menu đánh số, trả lựa chọn (0 = quay lại).
 *   [ ] ui_table_*: in bảng căn cột (tiêu đề, dòng, đường kẻ).
 *   [ ] ui_ok(msg), ui_err(msg), ui_info(msg): in có tiền tố [OK] / [LỖI] / [i] (có thể tô màu ANSI).
 *   [ ] ui_explain_code(code): mã 400/401/403/... thành câu tiếng Việt dễ hiểu.
 *   [ ] ui_pause(): "Nhấn Enter để tiếp tục".
 * =============================================================================
 */
