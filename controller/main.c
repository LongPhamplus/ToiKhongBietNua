/*
 * =============================================================================
 * FILE        : controller/main.c   (chương trình controller)
 * PHỤ TRÁCH   : Minh
 * PHASE       : 1 (App ID + menu), 2 (gắn quét/kết nối), 3 (gắn màn hình thiết bị)
 * TIÊU CHÍ    : "Mỗi ứng dụng cài đặt được khởi tạo một giá trị ID ngẫu nhiên" (phần của 6)
 * =============================================================================
 *
 * VIỆC CẦN LÀM
 *   [ ] signal(SIGPIPE, SIG_IGN): thiết bị đóng kết nối thì send trả lỗi, không làm chết app.
 *   [ ] ensure_dir("data").
 *   [ ] App ID:
 *         Nếu data/app.conf có dòng "app_id=<8 hex>" hợp lệ thì dùng.
 *         Nếu không thì random_hex(4) (util.c), ghi file, báo "Đã khởi tạo App ID mới".
 *         Xoá file này nghĩa là "cài đặt lại" và sẽ có App ID mới.
 *   [ ] coop_load() (coop.c), conn_init() (conn.c).
 *   [ ] Vòng menu chính (xem phác thảo ở KE_HOACH.md Phase 0):
 *         1. Quản lý chuồng nuôi     -> coop_menu()          (coop.c)
 *         2. Quét thiết bị           -> scan_menu()          (scanner.c)
 *         3. Vào một chuồng          -> chọn chuồng -> device_menu(coop) (ui_device.c)
 *         4. Tổng quan chuồng        -> ui_overview(coop)    (ui_device.c)
 *         0. Thoát
 *       Đầu menu in: App ID, số chuồng, số thiết bị đang kết nối.
 *   [ ] Thoát: gửi DISCONNECT cho mọi phiên đang mở, rồi BYE, đóng socket (conn_shutdown),
 *       coop_save().
 *   [ ] Tham số dòng lệnh tuỳ chọn: -d <thư mục data> để chạy 2 controller với
 *       2 App ID khác nhau trên cùng một máy khi demo đổi mật khẩu.
 *
 * KIỂM THỬ
 *   [ ] Chạy lần đầu sinh App ID. Chạy lại vẫn App ID cũ.
 *   [ ] Chạy với -d data2: App ID khác.
 *   [ ] Nhập lựa chọn rác ("abc", "99", Enter rỗng): menu báo lỗi, không crash.
 * =============================================================================
 */
