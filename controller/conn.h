/*
 * =============================================================================
 * FILE        : controller/conn.h
 * PHỤ TRÁCH   : Minh        · NGƯỜI DÙNG: ui_device.c, ui_feed.c (Minh), ui_climate.c (Long)
 * PHASE       : 1 (khung), 2 (CONNECT), bàn giao H6 cho Long hạn 02/12
 * TIÊU CHÍ    : 6. Kết nối thiết bị (2 điểm)
 * =============================================================================
 *
 * CẦN KHAI BÁO
 *   [ ] struct session: device_id, ip, port, fd (có thể dùng chung với phiên khác
 *       cùng ip:port), struct linebuf, token, connected (0/1), thời điểm kết nối.
 *   [ ] conn_init(app_id), conn_shutdown()
 *   [ ] conn_connect(coop_device, password, resp, sz): mở TCP nếu cần, gửi
 *       CONNECT, lưu token. Trả mã phản hồi (200/401/423/...) hoặc âm nếu lỗi mạng.
 *   [ ] conn_disconnect(device_id)
 *   [ ] conn_is_connected(device_id)
 *   [ ] conn_cmd(device_id, resp, sz, fmt, ...): HÀM CHÍNH MÀ MỌI MÀN HÌNH DÙNG.
 *       Người gọi chỉ viết phần sau token, ví dụ:
 *           conn_cmd(id, resp, sz, "SET Tmax=%.1f Tp1=%.1f", a, b)
 *       Hàm tự chèn thành "SET <id> <token> Tmax=.. Tp1=..". Trả mã phản hồi.
 *   [ ] conn_cmd_multiline(device_id, callback_mỗi_dòng, fmt, ...): cho lệnh LOG.
 *   [ ] conn_list(): cho màn hình trạng thái.
 * =============================================================================
 */
