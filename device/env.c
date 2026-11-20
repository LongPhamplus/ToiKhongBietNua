/*
 * =============================================================================
 * FILE        : device/env.c
 * PHỤ TRÁCH   : Long
 * PHASE       : 3 (08/12 – 10/12). Làm BẢN GIẢ trước 05/12 (nhiệt độ/độ ẩm cố
 *               định, đồng hồ chạy) để Minh làm lịch cho ăn và phun sương.
 * =============================================================================
 *
 * MỤC ĐÍCH
 *   Mô hình nhiệt độ/độ ẩm của MỘT chuồng (một tiến trình device_sim) và đồng hồ
 *   mô phỏng tăng tốc. Không cần chính xác vật lý, chỉ cần HỢP LÝ và QUAN SÁT ĐƯỢC
 *   khi demo.
 *
 * VIỆC CẦN LÀM
 *   [ ] Biến static: sim_minute (tổng số phút từ đầu), speed, temp, humidity.
 *   [ ] env_tick, với m = speed phút mô phỏng:
 *         - Nhiệt độ ngoài trời theo giờ: 24 + 8*sin(...) với đỉnh lúc 14:00.
 *         - Nhiệt độ chuồng tiến về nhiệt độ ngoài: temp += (out - temp) * k * m
 *           (k nhỏ, vd. 0.02/phút) + nhiễu ±0.05.
 *         - Độ ẩm giảm tự nhiên: humidity -= 0.2 * m (có chặn dưới, vd. 20%).
 *         - Chặn giá trị: temp trong [0, 50], humidity trong [0, 100].
 *       Tác động của quạt/đèn/phun KHÔNG viết ở đây. Mỗi thiết bị tự gọi
 *       env_add_temp/env_add_humidity trong tick của nó, để module độc lập.
 *   [ ] Chọn hệ số sao cho với speed = 1 (1 giây = 1 phút) thì:
 *         - quạt bật làm nhiệt độ giảm khoảng 0.3°C/phút, nên từ 33 về 28 mất khoảng 17 giây
 *         - đèn bật làm nhiệt độ tăng khoảng 0.4°C/phút
 *         - phun sương (Vh = 2) làm độ ẩm tăng khoảng 1%/phút
 *       Thử nghiệm và chỉnh để demo không quá nhanh cũng không quá chậm.
 *   [ ] env_print_status: in giờ mô phỏng, nhiệt độ, độ ẩm, tốc độ.
 *
 * KIỂM THỬ
 *   [ ] Không có thiết bị nào chạy: nhiệt độ dao động theo ngày, độ ẩm giảm dần.
 *   [ ] settemp 35 rồi status: thấy 35.0.
 *   [ ] speed 60: một giờ mô phỏng trôi qua mỗi giây.
 * =============================================================================
 */
