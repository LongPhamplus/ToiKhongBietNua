/*
 * =============================================================================
 * FILE        : device/dev_mister.c
 * PHỤ TRÁCH   : Minh       · PHỤ THUỘC: env.h của Long (bàn giao H7)
 * PHASE       : 3 (08/12 – 14/12)
 * TIÊU CHÍ    : 9d (1 điểm), 10 (một phần), 11 (một phần)
 * =============================================================================
 *
 * ĐỀ BÀI: "Bộ phận phun sương tự động khởi động với tốc độ phun Vh (lít/giờ) khi
 *          độ ẩm thấp hơn Hmin và tự động tắt khi độ ẩm tăng lên Hp."
 *
 * VIỆC CẦN LÀM
 *   [ ] tick (power ON), với h = độ ẩm (theo quy ước với Long: lấy từ cảm biến
 *       nếu có, nếu không thì env_humidity()):
 *         AUTO   : running == 0 và h < Hmin thì running = 1, log "auto ON (hum=.. < Hmin=..)"
 *                  running == 1 và h >= Hp thì running = 0, log "auto OFF"
 *         MANUAL : giữ nguyên running.
 *         running: lít = Vh * phút / 60; water_used += lít;
 *                  env_add_humidity(lít * hệ_số). Chọn hệ số cùng Long để Vh = 2
 *                  cho khoảng +1%/phút.
 *       power OFF: running = 0.
 *   [ ] set: Hmin, Hp, Vh. Áp dụng trên bản tạm, kiểm tra
 *       10 <= Hmin < Hp <= 95 và 0.1 <= Vh <= 20. Sai thì 422 kèm key.
 *       Đúng thì chép vào thật. Log "cũ -> mới" (handlers.c đã log chung, chỉ cần
 *       thống nhất ai log để không bị log 2 lần).
 *   [ ] act: RUN / STOP thì mode = MANUAL. Khác thì 405.
 *   [ ] info: "running=1 Hmin=55.0 Hp=70.0 Vh=2.0 humidity=52.3 water_used=1.25"
 *
 * KIỂM THỬ
 *   [ ] sethum 40: phun bật, độ ẩm tăng dần. Đến >= Hp thì tắt.
 *   [ ] Vh = 6 thì độ ẩm tăng nhanh gấp 3 lần Vh = 2. water_used tăng tương ứng.
 *   [ ] SET Hmin=80 khi Hp=70: 422.
 *   [ ] ACT RUN khi độ ẩm 90: vẫn phun (MANUAL). MODE AUTO: tắt ở tick sau.
 * =============================================================================
 */
