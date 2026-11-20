# data/: dữ liệu sinh ra khi chạy (KHÔNG đưa lên Git, trừ file này)

| File | Ai ghi | Nội dung | Khi nào tạo/ghi |
|---|---|---|---|
| `app.conf` | controller (`controller/main.c`, Minh) | `app_id=<8 hex>` | Lần chạy controller đầu tiên |
| `coops.txt` | controller (`controller/coop.c`, Minh) | Danh sách chuồng + thiết bị thuộc chuồng (định dạng ở KE_HOACH.md mục 4.5) | Sau mỗi lần thêm/sửa/xoá |
| `devices_<tên cấu hình>.conf` | device_sim (`device/device.c`, Long) | Trạng thái đầy đủ của từng thiết bị: ID, mật khẩu, thông số, lịch | Lần chạy đầu + sau mỗi SET / CHPASS / SCHED |

- Muốn thử lại "lần chạy đầu tiên" thì dùng `make reset-data`.
- Không lưu token vào đây. Token chỉ nằm trong bộ nhớ (xem KE_HOACH.md mục 4.5).
