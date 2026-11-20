/*
 * =============================================================================
 * FILE        : device/main.c   (chương trình device_sim)
 * PHỤ TRÁCH   : Long
 * PHASE       : 1 (vòng select), 2 (gắn UDP discovery của Minh), 3 (lệnh stdin)
 * TIÊU CHÍ    : 2. Cơ chế vào/ra socket trên server (2 điểm)
 * =============================================================================
 *
 * MỤC ĐÍCH
 *   Điểm vào của device_sim. Khởi tạo mọi thứ rồi chạy MỘT vòng lặp select()
 *   duy nhất, phục vụ đồng thời:
 *     (1) TCP listener: chấp nhận kết nối mới
 *     (2) các client TCP: nhận lệnh điều khiển
 *     (3) UDP socket: nhận gói SCAN (module của Minh)
 *     (4) stdin: lệnh quản trị để demo (settemp, sethum, speed, status, quit)
 *     (5) timeout 1s: gọi sim_tick() để mô phỏng chạy theo thời gian
 *
 * VIỆC CẦN LÀM
 *   [ ] Đọc tham số dòng lệnh: -c <file cấu hình> (bắt buộc), -p <cổng TCP>
 *       (ghi đè giá trị trong file), -u <cổng UDP>, -s <tốc độ mô phỏng>, -h.
 *       Có thể dùng getopt. Tham khảo IT4062/1-Chap1/lab04_pointer_to_pointer.c (argv).
 *   [ ] signal(SIGPIPE, SIG_IGN): client ngắt đột ngột thì send trả lỗi thay vì
 *       giết tiến trình. Bắt SIGINT để thoát sạch (lưu dữ liệu, đóng log).
 *   [ ] ensure_dir("data"), ensure_dir("logs").
 *   [ ] devices_load(...) (device.c), env_init(...) (env.c), mở log từng thiết bị.
 *   [ ] Tạo TCP listener: socket, setsockopt SO_REUSEADDR, bind INADDR_ANY:port,
 *       listen. Lỗi thì in rõ cổng nào và thoát.
 *       Tham khảo IT4062/2-Chap2/tcp_server.c, 3-Chap3/L1-NonBlocking/non_blocking_server.c
 *   [ ] [Phase 2] udp_fd = discovery_open(udp_port) (của Minh, bàn giao H5/H8).
 *   [ ] In banner: cổng TCP/UDP, bảng 7 thiết bị (ID, loại, tên).
 *   [ ] VÒNG LẶP CHÍNH, mỗi vòng:
 *         - FD_ZERO; FD_SET listener, udp_fd, STDIN_FILENO, mọi client đang mở.
 *         - Tính lại max_fd mỗi vòng.
 *         - Thời gian chờ = thời điểm tick kế tiếp - bây giờ (không âm).
 *           ĐẶT LẠI struct timeval MỖI VÒNG.
 *         - select(); nếu -1 và errno == EINTR thì quay lại vòng.
 *         - listener sẵn sàng: server_accept() (server.c).
 *         - udp_fd sẵn sàng: discovery_handle() (của Minh).
 *         - stdin sẵn sàng: admin_command() (xem dưới).
 *         - client sẵn sàng: server_handle_readable() (server.c).
 *         - Nếu now >= tick kế tiếp thì sim_tick() rồi cộng thêm TICK_MS.
 *   [ ] sim_tick(): env_tick() (env.c), sau đó gọi theo thứ tự
 *       climate_tick() (Long), feed_tick() (Minh), mister_tick() (Minh),
 *       rồi auth_expire_tokens(). Hook này là bàn giao H7 cho Minh.
 *   [ ] admin_command(): đọc 1 dòng stdin rồi xử lý:
 *         settemp <t> | sethum <h> | speed <phút mô phỏng / giây thực> |
 *         status (in bảng mọi thiết bị) | clients (in danh sách client) | quit
 *   [ ] Khi thoát: devices_save, đóng mọi socket, đóng log.
 *
 * LỖI CẦN TRÁNH (so với IT4062/3-Chap3/L5-IOMultiPlexing/select_server.c)
 *   - Client ngắt kết nối mà không bị xoá khỏi danh sách. Ở đây việc xoá do server.c làm.
 *   - max_fd không được tính lại.
 *   - timeout không đặt lại mỗi vòng.
 *
 * TRẢ LỜI VẤN ĐÁP: vì sao chọn select() thay vì fork()/pthread?
 *   - Một luồng duy nhất nên không có dữ liệu dùng chung bị tranh chấp, không cần mutex.
 *   - Dùng được timeout làm bộ đếm thời gian cho mô phỏng.
 *   - Quản lý cả UDP, TCP và stdin trong cùng một chỗ.
 *   - Giới hạn: FD_SETSIZE (1024), và một handler chạy chậm sẽ chặn mọi client
 *     khác (handler của mình rất nhanh nên chấp nhận được).
 *
 * KIỂM THỬ
 *   [ ] 3 nc + 2 controller cùng lúc đều được phục vụ.
 *   [ ] Không client nào kết nối thì `status` vẫn thấy nhiệt độ thay đổi theo thời gian.
 *   [ ] Ctrl+C rồi chạy lại: không bị "Address already in use".
 *   [ ] kill -9 một controller đang kết nối: server vẫn chạy.
 * =============================================================================
 */
