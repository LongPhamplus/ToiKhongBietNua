/*
 * =============================================================================
 * FILE        : device/dev_feed.c
 * PHỤ TRÁCH   : Minh
 * PHASE       : 3 (08/12 – 14/12)
 * TIÊU CHÍ    : 9a (1 điểm), 10 (một phần), 11 (một phần)
 * =============================================================================
 *
 * ĐỀ BÀI: "Máy cho ăn uống tự động, hoạt động theo các thời điểm trong ngày với
 *          lượng thức ăn W (kg), lượng nước Vw (lít) mỗi lần."
 *   Theo thang điểm, máy cho ăn và máy cho uống là 2 loại thiết bị riêng.
 *
 * VIỆC CẦN LÀM: LỊCH (dùng chung FEEDER + WATERER)
 *   [ ] Lưu tối đa MAX_SCHED mốc, đơn vị phút trong ngày, luôn SẮP XẾP tăng dần, không trùng.
 *   [ ] sched LIST: "200 OK times=06:00,11:00,17:00" (rỗng thì "times=-")
 *   [ ] sched ADD hh:mm: parse_hhmm (kv.c). Sai định dạng thì 422. Trùng thì 422.
 *       Đầy thì 422. Thành công thì devices_save() và log.
 *   [ ] sched DEL hh:mm: không có thì 422.
 *   [ ] Có thể thêm "sched SET 06:00,12:00" thay toàn bộ lịch (tuỳ chọn).
 *
 * VIỆC CẦN LÀM: TICK FEEDER / WATERER
 *   [ ] Chỉ chạy khi power ON. Lịch chạy ở cả AUTO; ở MANUAL thì KHÔNG chạy lịch.
 *       Ghi rõ quy ước này vào PROTOCOL.md.
 *   [ ] Phát hiện mốc: với mỗi mốc t, nếu t nằm trong (prev_minute, cur_minute]
 *       (env_clock_prev_minute / env_clock_minute, chú ý trường hợp qua nửa đêm)
 *       và last_run_day[t] != env_day() thì chạy và đặt last_run_day[t] = env_day().
 *       Lý do: với speed lớn, đồng hồ nhảy nhiều phút một tick, so sánh == sẽ bỏ lỡ.
 *   [ ] "Chạy" = nhả amount: nếu stock < amount thì nhả phần còn lại và log WARN "bồn cạn".
 *       stock -= lượng nhả, today_total += lượng nhả, last_run = giờ mô phỏng.
 *       Có thể đặt running = 1 trong vài phút mô phỏng để INFO thấy "running=1".
 *   [ ] Sang ngày mới (env_day đổi) thì today_total = 0.
 *
 * VIỆC CẦN LÀM: SET / ACT / INFO
 *   [ ] FEEDER set: W (0.1..50), stock (0..10000). WATERER set: Vw (0.1..200), tank (0..10000).
 *       Kiểm tra trên bản tạm giống dev_climate.c. Key lạ thì 405.
 *   [ ] FEEDER act: FEED_NOW [kg=x] (mặc định x = W, 0 < x <= 50).
 *       WATERER act: WATER_NOW [l=x].
 *       Chạy ở cả AUTO và MANUAL (cho ăn thêm ngoài lịch). Log kèm app_id.
 *       (Tiêu chí 10: "điều khiển trực tiếp bên cạnh các thông số lập lịch".)
 *   [ ] FEEDER info: "W=5.0 times=06:00,11:00,17:00 stock=85.0 last_feed=11:00 fed_today=10.0 running=0"
 *       WATERER info: "Vw=10.0 times=... tank=480.0 last_water=... water_today=... running=0"
 *
 * VIỆC CẦN LÀM: EGG_COUNTER ("báo số lượng trứng trong máng chứa")
 *   [ ] tick (power ON): count += rate * phút / 60, chặn ở capacity.
 *       Vừa đầy thì full = 1, log WARN một lần (không log lặp mỗi tick).
 *   [ ] set: capacity (1..10000), rate (0..1000).
 *   [ ] act: COLLECT thì total_collected += (int)count, count = 0, full = 0, log.
 *       (EGG_COUNTER không có MODE, handlers trả 405 cho MODE.)
 *   [ ] info: "count=37 capacity=300 rate=12.0 total_collected=120 full=0"
 *
 * KIỂM THỬ
 *   [ ] speed 1, đồng hồ 06:00, SCHED ADD 06:03: 3 giây sau fed_today += W, stock -= W.
 *   [ ] speed 60 (1 giờ/giây): mọi mốc trong ngày vẫn chạy đúng 1 lần, không bỏ lỡ, không chạy 2 lần.
 *   [ ] stock=3, W=5: nhả 3, log "bồn cạn", stock=0.
 *   [ ] FEED_NOW kg=2 ngoài giờ: fed_today += 2.
 *   [ ] SCHED ADD 25:00 / 6:00 / trùng: 422.
 *   [ ] Trứng: rate=600, sau ~30 phút mô phỏng count ~ 300 thì full=1. COLLECT về 0.
 *   [ ] Khởi động lại: lịch và W vẫn giữ (devices_save của Long phải lưu được times).
 * =============================================================================
 */
