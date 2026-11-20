/*
 * =============================================================================
 * FILE        : controller/scanner.c
 * PHỤ TRÁCH   : Minh
 * PHASE       : 2 (01/12 – 05/12)
 * TIÊU CHÍ    : 5. Quét thiết bị (2 điểm)
 * =============================================================================
 *
 * ĐỀ BÀI: "Khi thực hiện lệnh quét, chương trình quét các thiết bị và hiển thị loại
 *          thiết bị và ID cho người dùng. Người dùng thêm thiết bị mới này vào danh
 *          sách thiết bị trong chuồng nuôi."
 *
 * VIỆC CẦN LÀM: scan_run
 *   [ ] socket UDP; setsockopt SO_BROADCAST = 1 (thiếu thì sendto tới
 *       255.255.255.255 bị lỗi EACCES).
 *   [ ] Địa chỉ đích: "255.255.255.255" (broadcast) hoặc IP người dùng nhập,
 *       cổng UDP_PORT (9999). Hỗ trợ hostname qua getaddrinfo
 *       (tham khảo IT4062/2-Chap2/domain2ip.c).
 *   [ ] sendto "SCAN <app_id>". Gửi 2 lần cách nhau ~200ms để giảm rủi ro mất gói UDP.
 *   [ ] Vòng gom phản hồi đến khi hết wait_ms:
 *         còn_lại = hạn - now_ms(); <= 0 thì dừng.
 *         ĐẶT LẠI struct timeval từ còn_lại MỖI VÒNG (lỗi của
 *         IT4062/3-Chap3/L5-IOMultiPlexing/select_client.c là đặt 1 lần).
 *         select(fd+1, ...) rồi recvfrom. Lấy IP nguồn bằng inet_ntop.
 *         Dòng bắt đầu bằng "DEVICE " thì đọc id/type/port/name/power (kv.c).
 *         Bỏ qua gói sai định dạng. Loại trùng theo id.
 *   [ ] Đóng socket, trả số kết quả.
 *
 * VIỆC CẦN LÀM: scan_menu
 *   [ ] Hỏi: 1. Quét toàn mạng (broadcast)   2. Quét một địa chỉ IP cụ thể.
 *   [ ] In "Đang quét..." rồi bảng:
 *         STT | Loại | ID | IP:Port | Tên | Trạng thái
 *       Trạng thái: "Mới" hoặc "Đã thuộc <tên chuồng>" (coop_find_device).
 *       Loại hiển thị tiếng Việt: FAN = "Quạt điều hoà", FEEDER = "Máy cho ăn", ...
 *   [ ] Nếu thiết bị đã thuộc chuồng mà IP/port đổi thì coop_update_device_addr.
 *   [ ] Cho chọn một hoặc nhiều STT (vd. "1,3,5" hoặc "all") rồi chọn chuồng rồi
 *       coop_add_device rồi coop_save. Chưa có chuồng nào thì đề nghị tạo ngay.
 *   [ ] Không tìm thấy gì thì gợi ý: kiểm tra device_sim đã chạy chưa, thử quét theo
 *       IP cụ thể (127.0.0.1), kiểm tra tường lửa.
 *
 * KIỂM THỬ
 *   [ ] 1 device_sim, broadcast: 7 thiết bị.
 *   [ ] 2 device_sim (coop_A, coop_B), broadcast: 14 thiết bị, port 7000 và 7001.
 *   [ ] Quét IP 127.0.0.1: thấy tất cả.
 *   [ ] Không có device_sim nào: sau 2 giây báo "Không tìm thấy", không treo.
 *   [ ] Quét 2 lần liên tiếp: không bị trùng, thiết bị đã thêm hiện "Đã thuộc ...".
 *   [ ] Wireshark: chụp gói SCAN và các gói DEVICE cho báo cáo.
 * =============================================================================
 */
