/*
 * =============================================================================
 * FILE        : controller/conn.c
 * PHỤ TRÁCH   : Minh
 * PHASE       : 1 (khung: mở TCP, gửi/nhận 1 dòng), 2 (CONNECT + quản lý phiên)
 * TIÊU CHÍ    : 6. Kết nối thiết bị (2 điểm)
 * =============================================================================
 *
 * ĐỀ BÀI: "Người dùng yêu cầu kết nối tới thiết bị. Chương trình gửi mật khẩu do người
 *          dùng nhập và ID của ứng dụng."
 *
 * VIỆC CẦN LÀM
 *   [ ] Mở TCP: getaddrinfo(ip, port), socket, connect. Có thể đặt timeout cho
 *       connect (non-blocking + select) để IP sai không treo ~2 phút; tối thiểu
 *       thì báo lỗi rõ ràng.
 *       Tham khảo IT4062/2-Chap2/tcp_client.c.
 *   [ ] Dùng chung kết nối: 7 thiết bị của coop_A cùng ip:7000, nên chỉ mở MỘT
 *       socket và các phiên trỏ vào cùng fd + linebuf. Đóng socket khi không còn
 *       phiên nào dùng.
 *       (Cách đơn giản hơn nếu thiếu thời gian: mỗi thiết bị một socket. Server vẫn chịu được.)
 *   [ ] Gửi/nhận: send_line (net_io.c), rồi read_line_blocking với REQ_TIMEOUT_MS.
 *       Mỗi yêu cầu đọc ĐÚNG một dòng phản hồi. Phải đọc hết phản hồi của lệnh trước
 *       rồi mới gửi lệnh sau, nếu không phản hồi sẽ bị "lệch" sang lệnh khác.
 *   [ ] conn_connect:
 *         gửi "CONNECT <device_id> <app_id> <password>"
 *         200 thì lấy token=..., type=... (so với type đã lưu trong chuồng, khác thì cảnh báo)
 *         Không lưu mật khẩu sau khi gửi (xoá bộ đệm bằng memset).
 *   [ ] Phát hiện mất kết nối: read trả 0/-1 hoặc send lỗi thì đóng fd, đánh dấu
 *       MỌI phiên dùng fd đó là "mất kết nối", trả mã âm để UI báo
 *       "Mất kết nối tới thiết bị, hãy kết nối lại".
 *   [ ] Nhận 403 thì xoá token của phiên (đánh dấu chưa kết nối), vì có thể app khác
 *       đã đổi mật khẩu.
 *   [ ] conn_cmd: dùng vsnprintf ghép lệnh; kiểm tra tổng độ dài < MAX_LINE.
 *   [ ] conn_cmd_multiline: đọc dòng đầu "200 OK lines=k", sau đó đọc đến khi gặp ".".
 *       Dòng ".." thì đổi lại thành ".".
 *   [ ] conn_shutdown: với mỗi phiên gửi DISCONNECT, mỗi socket gửi BYE, rồi close.
 *
 * KIỂM THỬ (trước khi có server của Long, dùng `nc -l 7000` giả làm thiết bị)
 *   [ ] nc gõ "200 OK token=abc type=FAN": controller hiện "Đã kết nối".
 *   [ ] nc không gõ gì: sau 3 giây báo "Thiết bị không phản hồi".
 *   [ ] Ctrl+C nc: lệnh sau báo "Mất kết nối".
 *   [ ] Với server thật: kết nối 7 thiết bị coop_A, lệnh `clients` trên device_sim
 *       chỉ thấy 1 client (nếu dùng chung socket).
 * =============================================================================
 */
