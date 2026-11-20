/*
 * =============================================================================
 * FILE        : controller/coop.c
 * PHỤ TRÁCH   : Minh
 * PHASE       : 1 (24/11 – 30/11)
 * TIÊU CHÍ    : 13. Quản lý thông tin chuồng nuôi (1 điểm)
 * =============================================================================
 *
 * VIỆC CẦN LÀM
 *   [ ] Mảng động static các coop (malloc/realloc, nhân đôi khi đầy).
 *       Mỗi coop có mảng động thiết bị riêng. Nhớ free khi xoá.
 *       Tham khảo IT4062/1-Chap1/lab03_dynamic_array.c.
 *   [ ] coop_load / coop_save: định dạng data/coops.txt (KE_HOACH.md mục 4.5):
 *         coop id=1 name=Chuong_A location=Khu_1 capacity=500 chickens=420 created=2026-11-25
 *         dev  coop=1 id=FAN-8A21 type=FAN ip=192.168.1.10 port=7000 name=Quat_1
 *       Đọc bằng fgets + split_words + kv_get_*. Dòng hỏng thì bỏ qua và cảnh báo
 *       (không crash). Ghi qua file tạm + rename.
 *       Tên có dấu cách thì đổi thành '_' khi lưu (hoặc cấm nhập dấu cách).
 *   [ ] Kiểm tra dữ liệu: tên không rỗng và không trùng; capacity > 0;
 *       0 <= chickens <= capacity.
 *   [ ] coop_remove: nếu chuồng còn thiết bị thì hỏi xác nhận. Thiết bị nào đang
 *       kết nối thì ngắt (conn_disconnect) trước khi xoá.
 *   [ ] coop_menu:
 *         1. Danh sách chuồng (bảng: ID | Tên | Vị trí | Gà/Sức chứa | Số thiết bị | Ngày tạo)
 *         2. Thêm chuồng
 *         3. Sửa thông tin chuồng (Enter giữ giá trị cũ)
 *         4. Xoá chuồng
 *         5. Xem thiết bị trong chuồng (ID | Loại | Tên | IP:Port | Kết nối ●/○)
 *         6. Xoá thiết bị khỏi chuồng
 *         0. Quay lại
 *   [ ] (Tuỳ chọn, điểm cộng) Lưu số trứng thu được theo ngày cho từng chuồng,
 *       lấy từ EGG_COUNTER khi COLLECT. Như vậy "thông tin chuồng nuôi" có thêm thống kê.
 *
 * KIỂM THỬ
 *   [ ] Thêm 3 chuồng, sửa 1, xoá 1, thoát, mở lại: dữ liệu đúng.
 *   [ ] Thêm chuồng trùng tên / chickens > capacity: bị từ chối.
 *   [ ] Sửa tay coops.txt cho hỏng 1 dòng: app vẫn chạy, cảnh báo dòng hỏng.
 *   [ ] Một thiết bị không thể nằm ở 2 chuồng.
 * =============================================================================
 */
