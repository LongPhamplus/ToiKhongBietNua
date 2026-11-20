/*
 * =============================================================================
 * FILE        : device/dlog.h
 * PHỤ TRÁCH   : Long        · NGƯỜI DÙNG: mọi file trong device/ (cả của Minh)
 * PHASE       : 1
 * TIÊU CHÍ    : 12. Ghi log hoạt động trên mỗi thiết bị (1 điểm)
 * =============================================================================
 *
 * CẦN KHAI BÁO
 *   [ ] enum mức log: DEBUG, INFO, WARN, ERROR.
 *   [ ] dlog_open_all() / dlog_close_all(): mở/đóng file logs/<id>.log ở chế độ "a".
 *   [ ] dlog(dev, level, app_id_or_NULL, fmt, ...): ghi 1 dòng.
 *       app_id == NULL thì nguồn ghi là "auto" (do mô phỏng tự sinh).
 *   [ ] dlog_tail(dev, n, callback hoặc mảng đầu ra): đọc n dòng cuối, cho lệnh LOG (Phase 4).
 *   [ ] dlog_set_min_level(level): ẩn DEBUG khi demo.
 * =============================================================================
 */
