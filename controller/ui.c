/*
 * =============================================================================
 * FILE        : controller/ui.c
 * PHỤ TRÁCH   : Minh
 * PHASE       : 1 (24/11 – 28/11)
 * =============================================================================
 *
 * VIỆC CẦN LÀM
 *   [ ] KHÔNG dùng scanf("%d") / scanf("%s") vì để lại '\n' trong stdin và tràn
 *       bộ đệm. Mọi nhập liệu đều qua fgets rồi parse_*_strict (kv.c).
 *       Tham khảo IT4062/1-Chap1/lab06_io_basic.c.
 *   [ ] Nếu dòng nhập dài hơn bộ đệm (không thấy '\n') thì đọc bỏ phần còn lại của dòng.
 *   [ ] ui_read_password: tcgetattr, tắt ECHO, đọc, khôi phục (phải khôi phục cả khi lỗi),
 *       in '\n'. Mật khẩu chứa dấu cách thì báo lỗi (giao thức tách theo dấu cách).
 *   [ ] ui_explain_code, ví dụ:
 *         401 "Sai mật khẩu."
 *         403 "Phiên làm việc đã hết hạn hoặc mật khẩu đã bị đổi. Hãy kết nối lại."
 *         404 "Thiết bị không tồn tại trên máy này."
 *         405 "Thiết bị này không hỗ trợ chức năng đó."
 *         409 "Thiết bị đang tắt nguồn."
 *         422 "Giá trị không hợp lệ: <key>"
 *         423 "Thiết bị tạm khoá do nhập sai nhiều lần, thử lại sau <n> giây."
 *         -2 (timeout) "Thiết bị không phản hồi."
 *   [ ] Bảng: tính độ rộng cột theo nội dung. Chuỗi UTF-8 tiếng Việt có độ dài
 *       byte khác độ rộng hiển thị, nên tiêu đề bảng có thể dùng không dấu cho dễ căn.
 *
 * KIỂM THỬ
 *   [ ] Nhập "12abc", "", "999999999999" vào ô số: hỏi lại.
 *   [ ] Ctrl+D ở bất kỳ ô nhập nào: quay về menu trước / thoát sạch, không lặp vô hạn.
 *   [ ] Nhập mật khẩu không hiện ký tự. Sau đó terminal vẫn hiện chữ bình thường.
 * =============================================================================
 */
