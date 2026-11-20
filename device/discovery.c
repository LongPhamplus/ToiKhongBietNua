/*
 * =============================================================================
 * FILE        : device/discovery.c
 * PHỤ TRÁCH   : Minh
 * PHASE       : 2 (01/12 – 05/12)
 * TIÊU CHÍ    : 5. Quét thiết bị (2 điểm)
 * =============================================================================
 *
 * ĐỀ BÀI: "Lắng nghe các thông điệp quét trên một cổng ứng dụng nào đó."
 *
 * VIỆC CẦN LÀM
 *   [ ] discovery_open:
 *         socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)
 *         setsockopt SO_REUSEADDR = 1 và (nếu hệ thống có) SO_REUSEPORT = 1.
 *         Cả hai phải đặt TRƯỚC bind. Nhờ vậy coop_A và coop_B chạy trên cùng
 *         một máy đều bind được cổng 9999 và cùng nhận gói broadcast.
 *         bind INADDR_ANY:udp_port. Lỗi thì in rõ ràng, trả -1.
 *         Tham khảo IT4062/2-Chap2/udp_receiver.c
 *   [ ] discovery_handle:
 *         recvfrom vào bộ đệm, đặt '\0', trim.
 *         Tách từ (kv.c). Không phải "SCAN <app_id>" thì bỏ qua (log DEBUG), KHÔNG trả lời.
 *         Hợp lệ thì với MỖI thiết bị (device_count / device_at):
 *             "DEVICE id=<id> type=<TYPE> port=<tcp_port> name=<tên>"
 *             sendto về đúng địa chỉ nguồn (struct sockaddr_in lấy từ recvfrom).
 *             dlog(dev, INFO, app_id, "scanned from <ip>").
 *         Mỗi thiết bị MỘT datagram (đơn giản, không lo vượt kích thước gói).
 *   [ ] Không trả mật khẩu hay token trong phản hồi quét.
 *   [ ] Có trả lời cả khi thiết bị đang POWER OFF không? Nên CÓ (bộ điều khiển
 *       của thiết bị vẫn có điện), thêm key "power=OFF". Bổ sung vào PROTOCOL.md.
 *
 * TRẢ LỜI VẤN ĐÁP
 *   - Vì sao UDP? Không cần kết nối và gửi broadcast được, nên tìm được thiết bị
 *     mà không cần biết trước IP.
 *   - Broadcast không đi qua router. Muốn quét khác mạng thì phải nhập IP cụ thể.
 *   - UDP có thể mất gói. Controller có thể gửi SCAN 2 lần và loại trùng theo ID.
 *
 * KIỂM THỬ
 *   [ ] echo "SCAN 1234abcd" | nc -u -w1 127.0.0.1 9999  → nhận 7 dòng DEVICE.
 *   [ ] echo "HELLO" | nc -u -w1 127.0.0.1 9999          → không nhận gì.
 *   [ ] Chạy coop_A và coop_B cùng lúc → cả hai bind được 9999.
 *   [ ] Trong log từng thiết bị có dòng "scanned".
 * =============================================================================
 */
