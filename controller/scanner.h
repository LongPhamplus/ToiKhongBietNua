/*
 * =============================================================================
 * FILE        : controller/scanner.h
 * PHỤ TRÁCH   : Minh
 * PHASE       : 2 (01/12 – 05/12)
 * TIÊU CHÍ    : 5. Quét thiết bị (2 điểm, phía controller)
 * =============================================================================
 *
 * CẦN KHAI BÁO
 *   [ ] struct scan_result: id, type, ip, port, name, power.
 *   [ ] scan_run(target_ip_or_broadcast, app_id, results[], max, wait_ms):
 *       trả số thiết bị tìm thấy (đã loại trùng), hoặc -1 nếu lỗi.
 *   [ ] scan_menu(app_id): màn hình quét, hiển thị và thêm vào chuồng.
 * =============================================================================
 */
