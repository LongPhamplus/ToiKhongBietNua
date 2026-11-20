/*
 * =============================================================================
 * FILE        : controller/coop.h
 * PHỤ TRÁCH   : Minh
 * PHASE       : 1 (24/11 – 30/11), mở rộng ở Phase 2 (thêm thiết bị sau khi quét)
 * TIÊU CHÍ    : 13. Quản lý thông tin chuồng nuôi (1 điểm)
 * =============================================================================
 *
 * CẦN KHAI BÁO
 *   [ ] struct coop_device: id, type, ip, port, name (thông tin lấy từ lúc quét).
 *       Trạng thái kết nối KHÔNG nằm ở đây mà ở conn.c.
 *   [ ] struct coop: id (số tăng dần), name, location, capacity (số gà tối đa),
 *       chickens (số gà hiện có), created (ngày), note,
 *       mảng động coop_device + số lượng.
 *   [ ] coop_load() / coop_save()
 *   [ ] coop_add(name, location, capacity, chickens), coop_edit(id, ...), coop_remove(id)
 *   [ ] coop_count(), coop_at(i), coop_find(id)
 *   [ ] coop_add_device(coop, coop_device): trả lỗi nếu thiết bị đã ở chuồng khác
 *   [ ] coop_remove_device(coop, device_id)
 *   [ ] coop_find_device(device_id, &coop_out): tìm thiết bị thuộc chuồng nào
 *       (scanner.c dùng để đánh dấu "đã thêm")
 *   [ ] coop_update_device_addr(device_id, ip, port): khi quét lại thấy IP đổi
 *   [ ] coop_menu(): màn hình quản lý chuồng
 * =============================================================================
 */
