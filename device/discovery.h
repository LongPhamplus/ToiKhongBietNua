/*
 * =============================================================================
 * FILE        : device/discovery.h
 * PHỤ TRÁCH   : Minh        · NGƯỜI GẮN VÀO main.c: Long
 * PHASE       : 2 (bàn giao H8 cho Long hạn 05/12)
 * TIÊU CHÍ    : 5. Quét thiết bị (2 điểm, phía thiết bị)
 * =============================================================================
 *
 * CẦN KHAI BÁO (giao diện đã hứa ở H5, không đổi)
 *   [ ] discovery_open(udp_port): trả fd UDP đã bind, hoặc -1.
 *   [ ] discovery_handle(fd): main.c gọi khi select() báo fd sẵn sàng đọc.
 *       Đọc MỘT datagram rồi trả lời. Không được chặn.
 *   [ ] discovery_close(fd)
 * =============================================================================
 */
