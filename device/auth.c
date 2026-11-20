/*
 * =============================================================================
 * FILE        : device/auth.c
 * PHỤ TRÁCH   : Long
 * PHASE       : 2 (01/12 – 05/12)
 * TIÊU CHÍ    : 3. Xác thực truy cập điều khiển (1 điểm)
 * =============================================================================
 *
 * MỤC ĐÍCH
 *   "Khi nhận được yêu cầu kết nối, kiểm tra mật khẩu. Nếu khớp thì gửi lại thông
 *   tin thiết bị. Để quản lý các ứng dụng đã xác thực, thiết bị tạo một token
 *   ngẫu nhiên gắn với ID của ứng dụng." (trích đề bài)
 *
 * VIỆC CẦN LÀM
 *   [ ] auth_connect:
 *         1. Nếu now < dev->lock_until thì 423 (kèm số giây còn lại).
 *         2. Kiểm tra định dạng app_id (8 hex) và password (MIN..MAX_PASS).
 *         3. So mật khẩu. Sai thì fail_count++, đủ LOCK_FAILS thì
 *            lock_until = now + LOCK_SEC và fail_count = 0. Log WARN
 *            "CONNECT fail app=... (3/5)", trả 401.
 *         4. Đúng thì fail_count = 0. Nếu app_id này đã có token thì cấp token mới
 *            thay cho token cũ (không để 1 app giữ nhiều token).
 *            Bảng đầy thì thay token có last_used cũ nhất.
 *            random_hex(16) (util.c) rồi lưu {token, app_id, now, now}. Log INFO.
 *   [ ] auth_authorize:
 *         device_find(id) == NULL thì 404. Duyệt bảng token của thiết bị đó;
 *         không thấy hoặc đã hết hạn thì 403. Thấy thì cập nhật last_used, trả 200.
 *         (Token của thiết bị A KHÔNG dùng được cho thiết bị B.)
 *   [ ] auth_change_password:
 *         Kiểm tra token (đã qua authorize), mật khẩu cũ đúng (sai thì 401, có
 *         đếm fail như CONNECT), mật khẩu mới hợp lệ và khác mật khẩu cũ (422).
 *         Đổi xong thì XOÁ mọi token khác token hiện tại, rồi devices_save()
 *         để lưu ngay, log "password changed by app=..." (KHÔNG ghi mật khẩu).
 *   [ ] auth_expire_tokens: xoá token có now - last_used > TOKEN_TTL_SEC, log.
 *   [ ] So sánh chuỗi bí mật: nên dùng hàm so sánh hết độ dài (không return sớm
 *       khi gặp ký tự khác) để chống tấn công đo thời gian. Đây là điểm cộng khi vấn đáp.
 *
 * GHI CHÚ AN TOÀN (đưa vào báo cáo, phần "hạn chế")
 *   - Mật khẩu đi trên mạng dạng rõ, Wireshark thấy được. Cải tiến: TLS (OpenSSL)
 *     hoặc challenge–response (thiết bị gửi nonce, client gửi hash(pass+nonce)).
 *   - Mật khẩu lưu dạng rõ trong data/. Cải tiến: lưu hash + salt.
 *
 * KIỂM THỬ
 *   [ ] Đúng mật khẩu thì nhận token 32 hex. Hai lần CONNECT cho 2 token khác nhau.
 *   [ ] INFO với token sai trả 403; token của FAN dùng cho HEATER trả 403; ID lạ trả 404.
 *   [ ] Sai 5 lần thì 423, chờ 60s thì kết nối lại được.
 *   [ ] App A đổi mật khẩu thì token app B bị huỷ, còn token app A vẫn dùng được.
 *   [ ] Khởi động lại thì mật khẩu mới vẫn đúng, mọi token cũ mất hiệu lực.
 * =============================================================================
 */
