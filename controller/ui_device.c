/*
 * =============================================================================
 * FILE        : controller/ui_device.c
 * PHỤ TRÁCH   : Minh
 * PHASE       : 2 → 4
 * TIÊU CHÍ    : 6 (kết nối), 7 (đổi mật khẩu), 8 (bật/tắt), 10 (chọn mode), 11, 12
 * =============================================================================
 *
 * VIỆC CẦN LÀM: device_menu(coop)
 *   [ ] In danh sách thiết bị của chuồng: STT | Loại | Tên | ID | ●/○ kết nối.
 *   [ ] Chọn thiết bị, rồi menu (mục nào cần kết nối mà chưa kết nối thì nhắc kết nối trước):
 *         a. Kết nối           (PHASE 2)
 *         b. Xem thông tin     (PHASE 3)  INFO, gọi show_info theo loại
 *         c. Bật / Tắt nguồn   (PHASE 3)
 *         d. Chế độ AUTO/MANUAL(PHASE 3)  ẩn với cảm biến và máy đếm trứng
 *         e. Thiết lập thông số(PHASE 3)  gọi edit_params theo loại
 *         f. Điều khiển trực tiếp (PHASE 3) gọi direct_control theo loại
 *         g. Đổi mật khẩu      (PHASE 2)
 *         h. Xem log           (PHASE 4)
 *         i. Đổi tên thiết bị  (tuỳ chọn) gửi SET name=..., cập nhật coops.txt
 *         j. Ngắt kết nối
 *
 *   [ ] a. Kết nối (tiêu chí 6): ui_read_password, conn_connect, rồi in
 *       "Đã kết nối <tên> (token ****<4 ký tự cuối>)" hoặc ui_explain_code.
 *   [ ] g. Đổi mật khẩu (tiêu chí 7): nhập mật khẩu cũ, mới, nhập lại (ẩn ký tự).
 *       Kiểm tra trước khi gửi: mới == nhập lại, độ dài MIN..MAX_PASS, không dấu cách,
 *       khác mật khẩu cũ. Rồi conn_cmd "CHPASS <cũ> <mới>".
 *       Thành công thì báo "Các ứng dụng khác sẽ phải kết nối lại".
 *   [ ] c. Bật/tắt (tiêu chí 8): hiện trạng thái hiện tại (lấy từ INFO), hỏi xác nhận,
 *       rồi "POWER ON|OFF".
 *   [ ] d. Mode: giải thích ngắn
 *       "AUTO = tự chạy theo ngưỡng/lịch; MANUAL = chỉ chạy khi bạn ra lệnh".
 *   [ ] h. Xem log: hỏi số dòng (mặc định 20), rồi conn_cmd_multiline "LOG <n>" và in ra.
 *
 * VIỆC CẦN LÀM: ui_overview(coop) (bổ sung tiêu chí 11)
 *   [ ] Với mỗi thiết bị ĐÃ kết nối trong chuồng: INFO, rồi rút các key quan trọng.
 *   [ ] In một bảng:
 *         Nhiệt độ | Độ ẩm | Quạt | Đèn | Phun | Trứng/Sức chứa | Thức ăn còn | Nước còn
 *       Thiết bị chưa kết nối thì hiện "--".
 *   [ ] Tuỳ chọn: chế độ "theo dõi" tự làm mới mỗi 2 giây đến khi nhấn Enter
 *       (select trên stdin có timeout, giống select_client.c). Rất ấn tượng khi demo.
 *
 * KIỂM THỬ
 *   [ ] Mọi mục a–j hoạt động với cả 7 loại; mục không áp dụng thì bị ẩn hoặc báo rõ.
 *   [ ] Hai controller: A đổi mật khẩu, B bấm "Xem thông tin" thì nhận 403, B được
 *       nhắc kết nối lại.
 * =============================================================================
 */
