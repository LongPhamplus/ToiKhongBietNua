/*
 * =============================================================================
 * FILE        : device/device.c
 * PHỤ TRÁCH   : Long
 * PHASE       : 1 (26/11 – 30/11)
 * TIÊU CHÍ    : 4. Khởi tạo các thông số cho thiết bị (1 điểm)
 * =============================================================================
 *
 * MỤC ĐÍCH
 *   Tạo danh sách thiết bị khi khởi động. Lần đầu lấy từ config/*.conf + giá trị
 *   mặc định; các lần sau lấy từ data/devices_*.conf để ID, mật khẩu, thông số
 *   được giữ nguyên.
 *
 * VIỆC CẦN LÀM
 *   [ ] devices_load(config_path):
 *         1. Lấy tên cấu hình từ đường dẫn ("config/coop_A.conf" thành "coop_A").
 *         2. Nếu data/devices_coop_A.conf tồn tại thì nạp từ đó (bỏ qua bước 3).
 *         3. Nếu không thì đọc config_path từng dòng (fgets, trim, bỏ dòng rỗng
 *            và dòng bắt đầu bằng #):
 *              - "tcp_port=", "udp_port=", "speed=": thông số tiến trình.
 *              - "device type=... name=... password=... <key>=...":
 *                device_set_defaults(type), rồi ghi đè bằng key có trong dòng,
 *                rồi sinh ID.
 *            Sau đó gọi devices_save() ngay.
 *         4. Kiểm tra mọi ràng buộc (Tp1 < Tmax, ...). Vi phạm thì in lỗi kèm
 *            số dòng rồi thoát. Không được chạy với cấu hình sai.
 *   [ ] Sinh ID: tiền tố theo loại + random_hex(2) viết hoa (vd. "FAN-8A21").
 *       Lặp đến khi không trùng với thiết bị đã có.
 *   [ ] devices_save(): ghi ra file tạm "....tmp" rồi rename() đè file thật,
 *       để không hỏng file nếu bị tắt giữa chừng. Mỗi thiết bị một dòng
 *       "device id=... type=... name=... password=... power=... mode=... <thông số>".
 *       Lịch: times=06:00,11:00,17:00.
 *       Trạng thái tức thời (running, temp) KHÔNG cần lưu.
 *   [ ] device_set_defaults: đúng bảng 4.1 trong KE_HOACH.md. Mọi thiết bị mới
 *       ở trạng thái power=ON, mode=AUTO.
 *   [ ] Dùng kv.c (Minh) để đọc key=value. Nếu H3 chưa bàn giao thì tạm tự tách,
 *       sau đó thay thế.
 *
 * THAM KHẢO
 *   IT4062/1-Chap1/lab07_file_stdio.c (fopen/fgets/fprintf), lab11_filesystem.c (rename, stat)
 *
 * KIỂM THỬ
 *   [ ] make reset-data rồi chạy: 7 thiết bị có ID mới, file data/ được tạo.
 *   [ ] Chạy lại: ID giống hệt lần trước.
 *   [ ] SET Tmax=31 qua nc, tắt, chạy lại: INFO vẫn ra Tmax=31.0.
 *   [ ] Sửa config cho Tp1=40 > Tmax: chương trình báo lỗi rõ ràng rồi thoát.
 * =============================================================================
 */
