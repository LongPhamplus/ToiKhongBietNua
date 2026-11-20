/*
 * =============================================================================
 * FILE        : device/dev_climate.c
 * PHỤ TRÁCH   : Long
 * PHASE       : 3 (08/12 – 14/12)
 * TIÊU CHÍ    : 9b, 9c (2 điểm), 10 (một phần), 11 (một phần)
 * =============================================================================
 *
 * ĐỀ BÀI
 *   - Quạt tự khởi động khi nhiệt độ vượt Tmax và tự tắt khi giảm xuống Tp1.
 *   - Đèn sưởi tự khởi động khi nhiệt độ dưới Tmin và tự tắt khi tăng lên Tp2.
 *
 * VIỆC CẦN LÀM: ENV_SENSOR
 *   [ ] tick: cứ mỗi `interval` phút mô phỏng thì chép env_temp()/env_humidity()
 *       vào temp/humidity và cập nhật last_read. Power OFF thì không đo
 *       (INFO trả giá trị cũ + "stale=1").
 *   [ ] info: "temp=30.4 humidity=62.0 interval=1 last_read=07:41"
 *   [ ] set: chỉ nhận interval (1..60). Key khác thì 405.
 *   [ ] Quạt/đèn/phun dùng giá trị đo của CẢM BIẾN (không đọc thẳng env)
 *       nếu có cảm biến đang ON. Nếu không có thì đọc env trực tiếp.
 *       Thống nhất điều này với Minh cho phần phun sương.
 *
 * VIỆC CẦN LÀM: FAN (tiêu chí 9b)
 *   [ ] tick (power ON):
 *         AUTO   : running == 0 và temp > Tmax thì running = 1, log "auto ON (temp=.. > Tmax=..)"
 *                  running == 1 và temp <= Tp1 thì running = 0, log "auto OFF"
 *         MANUAL : giữ nguyên running do người dùng đặt.
 *         running: env_add_temp(-0.3 * phút), run_minutes += phút.
 *       power OFF: running = 0.
 *   [ ] set: nhận Tmax, Tp1 (một hoặc cả hai). Cách làm an toàn:
 *         chép thông số ra bản tạm, áp dụng mọi key vào bản tạm, kiểm tra ràng buộc
 *         10 <= Tp1 < Tmax <= 45 và ràng buộc chéo Tp2(HEATER) < Tp1. Hợp lệ thì
 *         chép bản tạm vào thật, sai thì giữ nguyên và trả 422 kèm key vi phạm.
 *         Key lạ thì 405.
 *   [ ] act: RUN thì mode = MANUAL, running = 1. STOP thì mode = MANUAL, running = 0.
 *       Khác thì 405. Log kèm app_id.
 *   [ ] info: "running=1 Tmax=32.0 Tp1=28.0 temp=31.6 run_minutes=125"
 *
 * VIỆC CẦN LÀM: HEATER (tiêu chí 9c)
 *   [ ] Đối xứng với FAN: bật khi temp < Tmin, tắt khi temp >= Tp2,
 *       running thì env_add_temp(+0.4 * phút).
 *   [ ] Ràng buộc: 5 <= Tmin < Tp2 <= 35 và Tp2 < Tp1(FAN).
 *   [ ] act, info tương tự FAN.
 *
 * TRẢ LỜI VẤN ĐÁP: vì sao cần 2 ngưỡng (Tmax/Tp1)?
 *   Đây là điều khiển trễ (hysteresis). Nếu chỉ có một ngưỡng thì quạt bật/tắt
 *   liên tục khi nhiệt độ dao động quanh ngưỡng, làm hỏng thiết bị.
 *   Vẽ sơ đồ trạng thái OFF/ON cho báo cáo.
 *
 * KIỂM THỬ (đưa vào docs/test_cases.md)
 *   [ ] settemp 33 (Tmax=32): quạt ON. Chờ: nhiệt độ giảm, đến <= 28 thì quạt OFF.
 *   [ ] settemp 15 (Tmin=18): đèn ON. Đến >= 22 thì đèn OFF.
 *   [ ] SET Tp1=35 (Tmax=32): 422, Tp1 vẫn là 28.
 *   [ ] SET Tmax=36 Tp1=33 cùng lúc: hợp lệ (kiểm tra sau khi áp dụng cả hai).
 *   [ ] ACT RUN khi temp = 20: quạt chạy (MANUAL). MODE AUTO: tick sau quạt tắt.
 *   [ ] POWER OFF rồi settemp 40: quạt không chạy.
 * =============================================================================
 */
