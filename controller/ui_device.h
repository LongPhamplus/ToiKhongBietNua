/*
 * =============================================================================
 * FILE        : controller/ui_device.h
 * PHỤ TRÁCH   : Minh        · NGƯỜI CẮM MÀN HÌNH VÀO: Long (ui_climate), Minh (ui_feed)
 * PHASE       : 2 (kết nối, đổi mật khẩu), 3 (bật/tắt, mode, tổng quan), 4 (xem log)
 * TIÊU CHÍ    : 6, 7, 8, 11 (tổng quan), 12 (xem log từ xa)
 * =============================================================================
 *
 * CẦN KHAI BÁO
 *   [ ] device_menu(coop): danh sách thiết bị trong chuồng, chọn một thiết bị,
 *       rồi menu thao tác cho thiết bị đó.
 *   [ ] ui_overview(coop): bảng tổng quan chuồng.
 *   [ ] BẢNG MÀN HÌNH THEO LOẠI (bàn giao P3.0b cho Long, hạn 09/12).
 *       Mỗi loại đăng ký bộ hàm:
 *         show_info(device_id, resp_line) : in INFO đẹp theo loại
 *         edit_params(device_id)          : màn hình thiết lập thông số
 *         direct_control(device_id)       : màn hình điều khiển trực tiếp
 *       Long đăng ký ENV_SENSOR/FAN/HEATER (ui_climate.h),
 *       Minh đăng ký FEEDER/WATERER/MISTER/EGG_COUNTER (ui_feed.h).
 *       Hàm NULL thì menu ẩn mục tương ứng (vd. cảm biến không có điều khiển trực tiếp).
 * =============================================================================
 */
