/*
 * =============================================================================
 * FILE        : controller/ui_feed.c
 * PHỤ TRÁCH   : Minh
 * PHASE       : 3 (10/12 – 14/12)
 * TIÊU CHÍ    : 9a, 9d (giao diện), 10, 11
 * =============================================================================
 *
 * VIỆC CẦN LÀM: FEEDER / WATERER (viết chung, tham số hoá theo kg/lít)
 *   [ ] show_info: lượng mỗi lần, lịch (06:00, 11:00, 17:00), lượng còn trong bồn,
 *       lần chạy gần nhất, tổng hôm nay. Bồn < 2 lần nhả thì in cảnh báo "sắp hết".
 *   [ ] edit_params, menu con:
 *         1. Đổi lượng mỗi lần (W / Vw)     SET W=... / SET Vw=...
 *         2. Xem lịch                        SCHED LIST
 *         3. Thêm giờ                        ui_read_hhmm, rồi SCHED ADD hh:mm
 *         4. Xoá giờ                         chọn từ danh sách, rồi SCHED DEL hh:mm
 *         5. Nạp thêm bồn                    SET stock=... / SET tank=...
 *   [ ] direct_control: "Cho ăn ngay" / "Cho uống ngay" (hỏi lượng, Enter = mặc định),
 *       rồi ACT FEED_NOW kg=.. / ACT WATER_NOW l=..
 *
 * VIỆC CẦN LÀM: MISTER
 *   [ ] show_info: trạng thái, độ ẩm hiện tại, ngưỡng Hmin/Hp, Vh, nước đã dùng.
 *   [ ] edit_params: Hmin, Hp, Vh (ui_read_double_keep). Kiểm tra trước Hmin < Hp.
 *   [ ] direct_control: Bật ngay / Tắt ngay / Trả về tự động (giống ui_climate.c
 *       của Long, thống nhất câu chữ với Long).
 *
 * VIỆC CẦN LÀM: EGG_COUNTER
 *   [ ] show_info: "Trứng trong máng: 37 / 300", tổng đã thu, cảnh báo nếu đầy.
 *   [ ] edit_params: capacity, rate.
 *   [ ] direct_control: "Thu trứng" (ACT COLLECT, hỏi xác nhận). Thành công thì in
 *       số trứng vừa thu. (Tuỳ chọn) cộng vào thống kê chuồng (coop.c).
 *
 * KIỂM THỬ
 *   [ ] Tất cả kịch bản trong device/dev_feed.c và dev_mister.c chạy được từ giao diện.
 *   [ ] Nhập giờ "7:5", "abc": hỏi lại, không gửi lên server.
 * =============================================================================
 */
