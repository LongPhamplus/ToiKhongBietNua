/*
 * =============================================================================
 * FILE        : device/dlog.c
 * PHỤ TRÁCH   : Long
 * PHASE       : 1 (ghi log), 4 (dlog_tail cho lệnh LOG, rà soát đủ sự kiện)
 * TIÊU CHÍ    : 12. Ghi log hoạt động trên mỗi thiết bị (1 điểm)
 * =============================================================================
 *
 * VIỆC CẦN LÀM
 *   [ ] dlog: định dạng một dòng (xem logs/README.md):
 *         "<now_str> | sim <hh:mm> | <MỨC> | app=<id>/auto | <nội dung>"
 *       Dùng va_list + vfprintf. Gọi fflush sau mỗi dòng để Ctrl+C không mất log.
 *       Giờ mô phỏng lấy từ env_clock_minute() (env.h).
 *       Đồng thời in ra stdout nếu mức >= INFO, để terminal device_sim "kể chuyện" khi demo.
 *   [ ] Nội dung log KHÔNG được chứa mật khẩu hay token đầy đủ. Token chỉ in
 *       4 ký tự đầu + "...".
 *   [ ] dlog_tail: cách đơn giản là đọc cả file bằng fgets, giữ n dòng cuối trong
 *       mảng vòng (ring buffer). Đủ nhanh vì file log nhỏ.
 *
 *   DANH SÁCH SỰ KIỆN BẮT BUỘC (Phase 4: Long rà phía server, Minh rà phần của Minh)
 *   [ ] Bị quét (SCAN) bởi app nào                       : discovery.c (Minh)
 *   [ ] CONNECT thành công / thất bại / bị khoá          : auth.c
 *   [ ] DISCONNECT, token hết hạn                        : auth.c
 *   [ ] Đổi mật khẩu (không ghi mật khẩu)                : auth.c
 *   [ ] POWER, MODE (cũ -> mới)                          : handlers.c
 *   [ ] Mọi SET (key: cũ -> mới), SCHED ADD/DEL          : handlers.c, dev_feed.c
 *   [ ] ACT (ai bấm, hành động gì)                       : dev_*.c
 *   [ ] Tự bật/tắt quạt, đèn, phun sương (kèm số đo)     : dev_climate.c, dev_mister.c
 *   [ ] Mỗi lần cho ăn/uống theo lịch, bồn cạn           : dev_feed.c
 *   [ ] Máng trứng đầy, thu trứng                        : dev_feed.c
 *   [ ] Khởi động / tắt device_sim                       : main.c
 *
 * KIỂM THỬ
 *   [ ] Sau một phiên demo, mỗi file log đều có đủ các loại sự kiện ở trên.
 *   [ ] grep mật khẩu mặc định "123456" trong logs/: không thấy.
 * =============================================================================
 */
