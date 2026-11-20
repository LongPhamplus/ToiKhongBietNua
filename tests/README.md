# tests/: Kịch bản kiểm thử

> Phụ trách: **Long** (mục A, B, C, E) + **Minh** (mục D, F, G) · Viết dần từ Phase 1, chạy đầy đủ ở Phase 4.
> Mỗi kịch bản khi chạy xong thì ghi kết quả (Đạt/Không đạt + ảnh/log) vào `docs/test_cases.md`.
> Có thể viết thêm các file `.sh` nhỏ trong thư mục này để tự động hoá. Mỗi file ghi rõ mục đích ở đầu.

## Việc cần làm

- [ ] Với mỗi mục dưới đây, viết **lệnh cụ thể** (nc / printf / thao tác menu) và **kết quả mong đợi**.
- [ ] Đánh mã kịch bản (A1, A2, ...) để báo cáo tham chiếu được.
- [ ] Gợi ý công cụ: `nc` (TCP), `nc -u` (UDP), `printf '...' | nc`, `ss -tlnp` / `ss -ulnp` (xem cổng), Wireshark (lọc `udp.port==9999 || tcp.port==7000`), `valgrind`.

## A. Truyền dòng (tiêu chí 1): Long
- A1 Một lệnh bị chia làm 2 lần gửi cách nhau 1 giây
- A2 Ba lệnh trong một lần gửi
- A3 Kết thúc bằng `\r\n` (telnet)
- A4 Dòng dài hơn 1024 byte, sau đó gửi một lệnh bình thường
- A5 Gửi rác nhị phân (`head -c 500 /dev/urandom | nc ...`)
- A6 Phản hồi nhiều dòng của `LOG` được controller đọc đúng

## B. Vào/ra server (tiêu chí 2): Long
- B1 5 client đồng thời (3 nc + 2 controller)
- B2 Một client ngắt đột ngột (kill -9), các client khác vẫn chạy
- B3 50 kết nối liên tiếp mở/đóng: không rò fd (`ls /proc/<pid>/fd | wc -l` không tăng mãi)
- B4 Khởi động lại ngay: không lỗi "Address already in use"
- B5 Không có client: mô phỏng vẫn chạy (`status`)

## C. Xác thực & khởi tạo (tiêu chí 3, 4): Long
- C1 Đúng mật khẩu thì được token; sai thì 401; sai 5 lần thì 423
- C2 Lệnh không token / token sai / token của thiết bị khác thì 403
- C3 Lần chạy đầu sinh ID + mặc định; lần sau giữ nguyên
- C4 File cấu hình sai ràng buộc: báo lỗi rồi thoát

## D. Quét & kết nối (tiêu chí 5, 6): Minh
- D1 Quét broadcast với 1 và 2 tiến trình device_sim
- D2 Quét theo IP cụ thể
- D3 Không có thiết bị: báo sau 2 giây, không treo
- D4 Thêm vào chuồng; thêm trùng chuồng khác bị từ chối
- D5 Kết nối, tắt device_sim, thao tác tiếp: báo mất kết nối
- D6 Thiết bị không phản hồi (nc -l không gõ gì): báo timeout

## E. Thông số & điều khiển khí hậu (tiêu chí 8, 9b, 9c, 10, 11): Long
- E1 → E6: theo mục KIỂM THỬ trong `device/dev_climate.c`

## F. Thông số & điều khiển chăn nuôi (tiêu chí 9a, 9d, 10, 11): Minh
- F1 → F8: theo mục KIỂM THỬ trong `device/dev_feed.c` và `device/dev_mister.c`

## G. Đổi mật khẩu, chuồng, log (tiêu chí 7, 12, 13): Minh (+ Long kiểm log)
- G1 Hai controller (`-d data`, `-d data2`): A đổi mật khẩu, B bị 403
- G2 Mật khẩu mới còn hiệu lực sau khi khởi động lại device_sim
- G3 CRUD chuồng + khởi động lại controller
- G4 Mỗi file log có đủ loại sự kiện (danh sách trong `device/dlog.c`), không chứa mật khẩu

## H. Kiểm thử chéo (Phase 4)
- [ ] Long chạy toàn bộ mục D, F, G
- [ ] Minh chạy toàn bộ mục A, B, C, E
- [ ] Ghi lỗi tìm được thành issue / ghi chú, người phụ trách sửa
