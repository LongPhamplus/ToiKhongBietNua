/*
 * =============================================================================
 * FILE        : device/device.h
 * PHỤ TRÁCH   : Long        · NGƯỜI DÙNG: mọi file trong device/ (cả file của Minh)
 * PHASE       : 1 (bàn giao kèm H4, hạn 30/11)
 * TIÊU CHÍ    : 4. Khởi tạo các thông số cho thiết bị (1 điểm)
 * =============================================================================
 *
 * MỤC ĐÍCH
 *   Mô hình dữ liệu thiết bị. Đây là file Minh phải đọc kỹ nhất phía server, vì
 *   dev_feed.c và dev_mister.c truy cập trực tiếp các struct ở đây.
 *   Thống nhất tên trường với Minh TRƯỚC khi viết (họp cuối Phase 0).
 *
 * CẦN KHAI BÁO
 *   [ ] struct riêng cho từng loại (giá trị theo KE_HOACH.md bảng 4.1):
 *         env_params    : interval, temp, humidity, last_read
 *         egg_params    : capacity, rate, count (số thực để cộng dồn phần lẻ), total_collected, full
 *         feed_params   : amount (W hoặc Vw), stock (stock hoặc tank),
 *                         times[MAX_SCHED], n_times, last_run_day[MAX_SCHED],
 *                         today_total, last_run, running_until
 *                         (FEEDER và WATERER dùng CHUNG struct này)
 *         fan_params    : Tmax, Tp1, running, run_minutes
 *         heater_params : Tmin, Tp2, running, run_minutes
 *         mister_params : Hmin, Hp, Vh, running, water_used
 *   [ ] struct device:
 *         id, type (enum ở protocol.h), name, password,
 *         power, mode,
 *         tokens[MAX_TOKENS_PER_DEV], fail_count, lock_until,
 *         FILE *log (hoặc đường dẫn log),
 *         union { env, egg, feed, fan, heater, mister } p;
 *   [ ] devices_load(config_path): trả 0 / -1.
 *   [ ] devices_save(): ghi data/devices_<tên>.conf (gọi sau mỗi thay đổi).
 *   [ ] device_find(id): trả device* hoặc NULL.
 *   [ ] device_count(), device_at(i): để duyệt (discovery.c, status, tick).
 *   [ ] device_find_first(type): vd. HEATER tìm FAN cùng tiến trình để kiểm tra ràng buộc chéo.
 *   [ ] device_set_defaults(dev, type): điền mặc định của bảng 4.1.
 *   [ ] Thông tin chung của tiến trình: tcp_port, udp_port, tên cấu hình
 *       (vd. "coop_A"), để discovery.c và main.c dùng.
 * =============================================================================
 */
