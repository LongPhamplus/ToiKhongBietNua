/*
 * =============================================================================
 * FILE        : device/env.h
 * PHỤ TRÁCH   : Long        · NGƯỜI DÙNG: dev_climate.c (Long), dev_feed.c + dev_mister.c (Minh), dlog.c
 * PHASE       : 3 (bàn giao H7 cho Minh hạn 05/12, có thể làm bản giả trước)
 * TIÊU CHÍ    : không chấm riêng; là "vật lý" để tiêu chí 9, 10, 11 demo được
 * =============================================================================
 *
 * CẦN KHAI BÁO (đây là API hứa với Minh, không đổi tên sau khi bàn giao)
 *   [ ] env_init(speed): khởi tạo nhiệt độ/độ ẩm ban đầu, đồng hồ mô phỏng = 06:00 ngày 1.
 *   [ ] env_tick(): main.c gọi mỗi giây thực, tăng đồng hồ thêm `speed` phút mô phỏng.
 *   [ ] env_minutes_per_tick(): số phút mô phỏng của tick vừa rồi (để tính lượng
 *       nước phun, trứng, ...).
 *   [ ] env_clock_minute(): phút trong ngày (0..1439) của đồng hồ mô phỏng.
 *   [ ] env_clock_prev_minute(): phút trong ngày ở tick TRƯỚC. Minh dùng để
 *       phát hiện "đã vượt qua mốc lịch" khi tốc độ cao nhảy nhiều phút một lúc.
 *   [ ] env_day(): số thứ tự ngày mô phỏng (để reset bộ đếm theo ngày).
 *   [ ] env_temp(), env_humidity(): giá trị hiện tại.
 *   [ ] env_add_temp(delta), env_add_humidity(delta): thiết bị tác động lên môi trường.
 *   [ ] env_set_temp(t), env_set_humidity(h), env_set_speed(s): cho lệnh quản trị.
 *   [ ] env_print_status(): cho lệnh `status`.
 * =============================================================================
 */
