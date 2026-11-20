/*
 * =============================================================================
 * FILE        : common/protocol.h
 * PHỤ TRÁCH   : Long + Minh (cùng viết ở Phase 0, sửa phải báo người kia)
 * PHASE       : 0 (hạn 23/11)
 * TIÊU CHÍ    : nền tảng cho mọi tiêu chí
 * =============================================================================
 *
 * MỤC ĐÍCH
 *   Nơi DUY NHẤT định nghĩa các hằng số của giao thức. Cả device_sim và
 *   controller cùng include file này, nên hai bên không bao giờ "lệch" nhau
 *   về tên lệnh, cổng hay mã lỗi.
 *   Chỉ chứa #define / enum. KHÔNG có hàm, KHÔNG có biến.
 *
 * VIỆC CẦN LÀM
 *   [ ] Header guard.
 *   [ ] Cổng mặc định: UDP quét = 9999, TCP điều khiển = 7000.
 *   [ ] Giới hạn độ dài:
 *         MAX_LINE (1024), MAX_ID (độ dài device_id, vd. 16), MAX_NAME (32),
 *         MAX_PASS (32), MIN_PASS (4), TOKEN_HEX_LEN (32), APP_ID_HEX_LEN (8),
 *         MAX_CLIENTS (64), MAX_DEVICES (32), MAX_TOKENS_PER_DEV (8),
 *         MAX_SCHED (8), MAX_ARGS (số từ tối đa trong một dòng lệnh, vd. 16).
 *   [ ] Thời gian: SCAN_WAIT_MS (2000), REQ_TIMEOUT_MS (3000),
 *         TOKEN_TTL_SEC (1800), LOCK_FAILS (5), LOCK_SEC (60), TICK_MS (1000).
 *   [ ] Tên lệnh dạng chuỗi hằng: "SCAN", "DEVICE", "HELLO", "CONNECT", "INFO",
 *         "POWER", "SET", "MODE", "ACT", "SCHED", "CHPASS", "LOG",
 *         "DISCONNECT", "BYE".
 *   [ ] Mã phản hồi (enum hoặc #define): 200, 400, 401, 403, 404, 405, 422, 423, 500,
 *         kèm chuỗi tên tương ứng (OK, BAD_REQUEST, AUTH_FAILED, ...).
 *   [ ] enum kiểu thiết bị: ENV_SENSOR, EGG_COUNTER, FEEDER, WATERER, FAN,
 *         HEATER, MISTER (+ một giá trị "không hợp lệ").
 *         Ghi chú bên cạnh tiền tố ID: ENV-, EGG-, FED-, WAT-, FAN-, HEA-, MIS-.
 *   [ ] enum power (OFF/ON) và mode (AUTO/MANUAL).
 *   [ ] Chuỗi kết thúc phản hồi nhiều dòng: "." (dùng cho lệnh LOG).
 *   [ ] Phiên bản giao thức, vd. "device_sim/1.0".
 *
 *   Hàm đổi enum <-> chuỗi ("FAN" <-> DEV_FAN) KHÔNG đặt ở đây mà đặt
 *   ở common/kv.c (Minh) để cả 2 chương trình dùng chung.
 *
 * XONG KHI
 *   - Mọi giá trị khớp với PROTOCOL.md.
 *   - grep cả project không còn số cổng / mã lỗi / tên lệnh viết cứng ở chỗ khác.
 * =============================================================================
 */
