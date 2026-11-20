/*
 * =============================================================================
 * FILE        : device/server.c
 * PHỤ TRÁCH   : Long
 * PHASE       : 1 (24/11 – 28/11)
 * TIÊU CHÍ    : 2. Vào/ra socket server (2 điểm), 1. Truyền dòng (1 điểm)
 * =============================================================================
 *
 * MỤC ĐÍCH
 *   Quản lý các kết nối TCP: mỗi client có bộ đệm dòng RIÊNG. Nhận byte, cắt
 *   thành dòng, chuyển từng dòng cho handlers_dispatch(), gửi phản hồi về.
 *
 * VIỆC CẦN LÀM
 *   [ ] Mảng tĩnh `static struct client clients[MAX_CLIENTS]` và số lượng.
 *   [ ] server_accept: accept(listen_fd, &addr, &len), inet_ntop, ghi stdout:
 *       "[+] client 192.168.1.5:51234 (fd=5)".
 *   [ ] server_handle_readable: với mỗi client có trong set:
 *         r = linebuf_fill(...)
 *         r == 0 hoặc r < 0 (lỗi khác EAGAIN): server_close_client, in "[-] ...".
 *         Ngược lại: LẶP while (linebuf_next(...) != 0):
 *             -1: send_line "400 BAD_REQUEST LINE_TOO_LONG"
 *              1: dòng rỗng thì bỏ qua; còn lại thì
 *                 handlers_dispatch(&clients[i], line, resp, sizeof resp)
 *                 rồi send_line(fd, resp).
 *                 Gửi lỗi thì đánh dấu đóng.
 *         Sau vòng: nếu cờ "cần đóng" (BYE) thì đóng.
 *   [ ] Chú ý khi xoá client ngay TRONG vòng duyệt: nếu dồn mảng thì phải
 *       giảm i, hoặc dùng cách đánh dấu fd = -1 rồi dọn sau vòng.
 *   [ ] In ra stdout mỗi lệnh nhận (cắt mật khẩu: với CONNECT/CHPASS chỉ in
 *       tên lệnh + ID) để demo dễ quan sát.
 *   [ ] Phản hồi nhiều dòng (LOG, Phase 4): handlers trả cờ "multi-line" và
 *       con trỏ danh sách dòng; server gửi lần lượt rồi gửi "." cuối.
 *       Thống nhất cách làm với handlers.c.
 *
 * TRẢ LỜI VẤN ĐÁP
 *   Vì sao mỗi client cần bộ đệm riêng? Hai client có thể cùng gửi dở dang
 *   một dòng. Nếu dùng chung một bộ đệm thì byte của hai client trộn lẫn vào nhau.
 *
 * KIỂM THỬ: xem tests/README.md mục "Server I/O" và "Truyền dòng".
 * =============================================================================
 */
