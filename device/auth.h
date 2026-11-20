/*
 * =============================================================================
 * FILE        : device/auth.h
 * PHỤ TRÁCH   : Long
 * PHASE       : 2 (01/12 – 05/12)
 * TIÊU CHÍ    : 3. Xác thực truy cập điều khiển (1đ), 6. Kết nối (phía server), 7. Đổi mật khẩu (phía server)
 * =============================================================================
 *
 * CẦN KHAI BÁO
 *   [ ] struct token_entry: token (33 ký tự), app_id (9 ký tự), created, last_used.
 *       (Mảng token nằm TRONG struct device ở device.h, tối đa MAX_TOKENS_PER_DEV.)
 *   [ ] auth_connect(dev, app_id, password, token_out): trả 200 / 401 / 423.
 *   [ ] auth_authorize(device_id, token, &dev_out): trả 200 / 403 / 404.
 *       Mọi lệnh cần quyền đều đi qua đây.
 *   [ ] auth_change_password(dev, token, old, new): trả 200 / 401 / 422.
 *   [ ] auth_revoke(dev, token): dùng cho DISCONNECT.
 *   [ ] auth_expire_tokens(): main.c gọi mỗi tick để huỷ token quá TOKEN_TTL_SEC.
 *   [ ] auth_app_of(dev, token): trả app_id gắn với token, để ghi log "app=...".
 * =============================================================================
 */
