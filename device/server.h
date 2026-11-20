/*
 * =============================================================================
 * FILE        : device/server.h
 * PHỤ TRÁCH   : Long
 * PHASE       : 1
 * TIÊU CHÍ    : 2. Vào/ra socket server; 1. Truyền dòng
 * =============================================================================
 *
 * CẦN KHAI BÁO
 *   [ ] struct client: fd, địa chỉ IP dạng chuỗi + cổng, struct linebuf (net_io.h),
 *       thời điểm kết nối, số lệnh đã xử lý, cờ "cần đóng sau khi gửi" (cho BYE).
 *   [ ] server_init(): đặt mảng client về rỗng.
 *   [ ] server_accept(listen_fd): accept, lưu địa chỉ (inet_ntop), thêm vào mảng.
 *       Đầy thì gửi "500 SERVER_FULL" rồi đóng.
 *   [ ] server_fill_fdset(&set, &max_fd): main.c gọi để thêm fd client vào select.
 *   [ ] server_handle_readable(&set): duyệt client có trong set, đọc và xử lý.
 *   [ ] server_close_client(i): đóng fd, xoá khỏi mảng (dồn mảng hoặc đánh dấu trống).
 *   [ ] server_print_clients(): cho lệnh quản trị `clients`.
 *   [ ] server_shutdown(): đóng tất cả.
 * =============================================================================
 */
