/*
 * =============================================================================
 * FILE        : controller/ui_climate.c
 * PHỤ TRÁCH   : Long
 * PHASE       : 3 (10/12 – 14/12)
 * TIÊU CHÍ    : 9b, 9c (giao diện), 10, 11
 * =============================================================================
 *
 * MỤC ĐÍCH
 *   Long tự làm giao diện cho thiết bị của mình. Chỉ dùng các hàm có sẵn:
 *   conn_cmd (conn.h), ui_* (ui.h), kv_* (kv.h). Không mở socket ở đây.
 *
 * VIỆC CẦN LÀM
 *   [ ] *_show_info: tách phản hồi INFO, in dạng:
 *         Quạt điều hoà "Quat_1" [FAN-8A21]
 *           Nguồn      : BẬT        Chế độ : TỰ ĐỘNG
 *           Trạng thái : ĐANG CHẠY  (đã chạy 125 phút)
 *           Nhiệt độ   : 31.6 °C
 *           Ngưỡng     : bật khi > 32.0 °C, tắt khi <= 28.0 °C
 *       Cảm biến: nhiệt độ, độ ẩm, lần đo cuối, chu kỳ đo.
 *   [ ] fan_edit_params: lấy INFO để biết giá trị hiện tại;
 *       ui_read_double_keep cho Tmax rồi Tp1. Kiểm tra trước Tp1 < Tmax (phía client),
 *       chỉ gửi key thay đổi trong MỘT lệnh SET. 422 thì in lý do server trả.
 *       Hiển thị gợi ý ràng buộc chéo với đèn sưởi (Tp2 < Tp1).
 *   [ ] heater_edit_params: Tmin, Tp2, tương tự.
 *   [ ] env_sensor_edit_params: interval.
 *   [ ] *_direct_control: menu 1. Bật ngay  2. Tắt ngay  3. Trả về tự động.
 *       Gửi ACT RUN / ACT STOP / MODE AUTO. Cảnh báo "thiết bị sẽ chuyển sang
 *       MANUAL, không còn tự chạy theo ngưỡng".
 *       Thiết bị đang POWER OFF (409) thì gợi ý bật nguồn trước.
 *
 * KIỂM THỬ
 *   [ ] Tất cả kịch bản trong device/dev_climate.c chạy được từ giao diện, không cần nc.
 * =============================================================================
 */
