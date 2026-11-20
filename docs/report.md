# Khung báo cáo bài tập giữa kỳ IT4062

> Phụ trách: Long + Minh · Viết ở Phase 5 (19/12 – 20/12). Ghi chú ý tưởng thì có thể ghi dần từ Phase 1.
> Định dạng nộp cuối (PDF / Word) theo yêu cầu của thầy.

## Việc cần làm
- [ ] Điền từng mục dưới đây. Người viết chính ghi trong ngoặc.
- [ ] Mọi hình vẽ để trong `docs/img/`.
- [ ] Mỗi tiêu chí chấm điểm phải được nhắc đến ít nhất 1 lần, kèm bằng chứng (ảnh, log).
- [ ] Hai người đọc chéo toàn bộ báo cáo trước khi nộp.

## Mục lục dự kiến

1. **Giới thiệu đề tài**: bài toán, mục tiêu, phạm vi *(Minh)*
2. **Phân tích yêu cầu**: 2 chương trình, 7 loại thiết bị, bảng thang điểm, cách nhóm đáp ứng từng tiêu chí *(Minh)*
3. **Kiến trúc hệ thống**: sơ đồ tổng thể, lý do chọn UDP cho quét và TCP cho điều khiển, một tiến trình mô phỏng một chuồng *(Long)*
4. **Thiết kế giao thức**: tóm tắt PROTOCOL.md, bảng lệnh, mã lỗi, sơ đồ tuần tự *(Long + Minh)*
5. **Chương trình mô phỏng thiết bị** *(Long)*
   - 5.1 Cơ chế vào/ra: vòng `select()`, so sánh với fork/pthread
   - 5.2 Xử lý truyền dòng: bộ đệm dòng, `send_all`
   - 5.3 Xác thực: mật khẩu, token, khoá khi sai nhiều, đổi mật khẩu
   - 5.4 Khởi tạo & lưu thông số
   - 5.5 Mô hình môi trường, điều khiển trễ (sơ đồ trạng thái quạt/đèn)
   - 5.6 Ghi log
   - 5.7 Máy ăn/uống theo lịch, phun sương, đếm trứng *(Minh viết mục này)*
   - 5.8 Trả lời gói quét (discovery) *(Minh viết mục này)*
6. **Chương trình điều khiển** *(Minh)*
   - 6.1 App ID
   - 6.2 Quét thiết bị (broadcast, timeout, loại trùng)
   - 6.3 Quản lý kết nối & phiên
   - 6.4 Quản lý chuồng nuôi
   - 6.5 Giao diện: menu, các màn hình *(phần quạt/đèn/cảm biến do Long viết)*
7. **Kiểm thử**: tóm tắt `docs/test_cases.md`, ảnh Wireshark *(Long + Minh)*
8. **Hạn chế & hướng phát triển**: mật khẩu truyền dạng rõ (đề xuất TLS / challenge–response), lưu mật khẩu dạng rõ (đề xuất hash + salt), broadcast không qua router, giới hạn FD_SETSIZE *(Long + Minh)*
9. **Phân công công việc**: bảng mục 5.2 của KE_HOACH.md + tự đánh giá *(Long + Minh)*
10. **Hướng dẫn build & chạy** *(Long + Minh)*
11. **Tài liệu tham khảo**: lấy từ IT4062/README.md mục 8
