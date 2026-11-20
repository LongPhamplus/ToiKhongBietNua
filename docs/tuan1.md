# TUẦN 1 (20/11 – 30/11/2026): Phase 0 + Phase 1

> Mục tiêu tuần: **chốt thiết kế** (Phase 0) và dựng **xương sống** của cả 2 chương trình (Phase 1).
> Cuối tuần controller của Minh phải gửi được `HELLO` / `INFO` tới `device_sim` thật của Long.
> Chi tiết từng file xem khối chú thích trong `App/<thư mục>/<file>`, kế hoạch tổng ở `App/KE_HOACH.md`.
> Đánh dấu `[x]` khi xong.

---

## PHASE 0: Chuẩn bị & chốt thiết kế (T6 20/11 → T2 23/11)

### T6 20/11: Môi trường

**Long**
- [ ] Cài WSL Ubuntu + `build-essential gdb make netcat-openbsd wireshark valgrind`
- [ ] Tạo repo Git cho project, đẩy lên GitHub, mời Minh vào repo

**Minh**
- [ ] Cài môi trường giống Long
- [ ] Clone repo, chạy thử `IT4062/2-Chap2/tcp_server.c` + `tcp_client.c` để kiểm tra máy

### T7 21/11: Đọc code mẫu IT4062

**Long**
- [ ] `3-Chap3/L5-IOMultiPlexing/select_server.c`
- [ ] `3-Chap3/L1-NonBlocking/non_blocking_server.c`
- [ ] `4-Chap4/http_server.c` (vòng `recv` đến khi gặp `\r\n\r\n`)
- [ ] `1-Chap1/lab07_file_stdio.c`, `lab08_lowlevel_io.c`
- [ ] Ghi lại các lỗi trong code mẫu (xem KE_HOACH.md mục 1)

**Minh**
- [ ] `2-Chap2/udp_sender.c`, `udp_receiver.c`
- [ ] `3-Chap3/L5-IOMultiPlexing/select_client.c`
- [ ] `2-Chap2/music_client.c`, `domain2ip.c`
- [ ] `1-Chap1/lab05_function_pointer.c`, `lab09_string.c`
- [ ] Ghi lại các lỗi trong code mẫu

### CN 22/11: Viết nháp thiết kế

**Long**
- [ ] Viết nháp vào `PROTOCOL.md`: các lệnh TCP, xác thực/token, bảng thông số
- [ ] Viết nháp tên các trường của `struct device` (dạng chú thích trong `device.h`)

**Minh**
- [ ] Viết nháp vào `PROTOCOL.md`: phần quét UDP (`SCAN` / `DEVICE`)
- [ ] Viết nháp định dạng `data/coops.txt`
- [ ] Phác thảo menu controller bằng text

### T2 23/11 ★ HỌP CHỐT THIẾT KẾ (≈ 2 giờ, cả hai)

- [ ] `PROTOCOL.md` được chốt từng lệnh, cả hai đánh dấu "Đã chốt"
- [ ] Danh sách hằng số cho `common/protocol.h` (cổng, độ dài, mã lỗi, tên lệnh, kiểu thiết bị)
- [ ] **Tên trường của `struct device`.** Phase 3 Minh sẽ truy cập trực tiếp các trường này
- [ ] **Chữ ký các hàm dùng làm điểm gắn giữa hai người:**
  - [ ] Bảng hàm theo loại thiết bị trong `handlers.h`: `info` / `set` / `act` / `sched`
  - [ ] Bảng màn hình trong `ui_device.h`: `show_info` / `edit_params` / `direct_control`
  - [ ] `conn_cmd(...)` trong `conn.h`
  - [ ] `discovery_open` / `discovery_handle`
  - [ ] API của `env.h`
- [ ] Trả lời 2 câu hỏi còn mở:
  - [ ] Quạt và phun sương đọc số đo từ **cảm biến** hay từ **mô hình môi trường**?
  - [ ] Máy ăn/uống ở chế độ MANUAL có chạy theo lịch không? (đề xuất: **không**)
- [ ] `Makefile` (Long) + `.gitignore` (Minh) hoạt động, commit lên nhánh `main`
- [ ] Quy ước Git: nhánh `long/...`, `minh/...`, gộp qua Pull Request, người kia review
- [ ] Chốt lịch họp ngắn 15 phút: **T4 25/11, T6 27/11, T2 30/11**

---

## PHASE 1: Nền tảng (T3 24/11 → T2 30/11)

| Ngày | Long (server) | Minh (client) |
|---|---|---|
| **T3 24/11** | `common/util.c` | `common/kv.c` (phần 1) |
| **T4 25/11** | `common/net_io.c` | `common/kv.c` (phần 2) + bắt đầu `controller/ui.c` |
| **T5 26/11** 🔁 | **Bàn giao H2** (`net_io` + `util`) → `device/device.c` | **Bàn giao H3** (`kv`) → hoàn thiện `ui.c` |
| **T6 27/11** | `device.c` (nạp/lưu) + `config/coop_A.conf` | `controller/main.c` + `coop.c` (phần 1) |
| **T7 28/11** | `device/dlog.c` + `device/main.c` (vòng `select`) | `coop.c` (lưu file + menu) |
| **CN 29/11** | `device/server.c` + `handlers.c` (HELLO/BYE/INFO) + `discovery.h` giả | `controller/conn.c` (khung), thử với `nc -l` |
| **T2 30/11** ★ | **Tích hợp + bàn giao H4, H5** | **Tích hợp** |

### Việc chi tiết: LONG

**T3 24/11: `common/util.c`**
- [ ] `random_hex(out, nbytes)` đọc `/dev/urandom` (dự phòng `rand()`)
- [ ] `now_ms()`, `now_str()`
- [ ] `trim()`, `safe_copy()`, `ensure_dir()`, `die()`
- [ ] Thử: gọi 1000 lần `random_hex`, không có giá trị trùng, chỉ gồm `0-9a-f`

**T4 25/11: `common/net_io.c` (tiêu chí 1: truyền dòng)**
- [ ] `send_all`, `send_line`, `send_linef`
- [ ] `struct linebuf`, `linebuf_init`, `linebuf_fill`, `linebuf_next` (dùng `memchr` + `memmove`)
- [ ] `read_line_blocking` (đặt lại `timeval` mỗi vòng)
- [ ] Chương trình thử riêng: gửi phân mảnh, 3 dòng trong 1 lần, `\r\n`, dòng > 1024 byte, đóng kết nối giữa chừng

**T5 26/11: bàn giao + `device/device.c`**
- [ ] 🔁 **H2**: gộp `net_io` + `util` vào `main`, báo Minh
- [ ] Định nghĩa `struct device` + các struct thông số theo loại (`device.h`)
- [ ] `device_set_defaults()` đúng bảng 4.1 trong KE_HOACH
- [ ] Sinh ID `<tiền tố>-<4 hex>` không trùng

**T6 27/11: `device.c` (tiêu chí 4: khởi tạo thông số)**
- [ ] Nạp `config/coop_A.conf` (dùng `kv.c` của Minh)
- [ ] Lưu `data/devices_coop_A.conf` (ghi vào file tạm rồi `rename`)
- [ ] Lần sau nạp lại từ `data/`
- [ ] Kiểm tra ràng buộc (Tp1 < Tmax, ...); sai thì báo lỗi rồi thoát
- [ ] Viết `config/coop_A.conf` (7 thiết bị)

**T7 28/11: `device/dlog.c` + `device/main.c`**
- [ ] `dlog(dev, level, app_id, fmt, ...)`, mỗi thiết bị 1 file, gọi `fflush` sau mỗi dòng
- [ ] `main.c`: đọc tham số `-c -p -u -s`, bỏ qua `SIGPIPE`, xử lý `SIGINT`
- [ ] TCP listener + `SO_REUSEADDR`
- [ ] Vòng `select()`: listener + stdin + client, timeout 1s gọi `sim_tick()` (tạm để rỗng)
- [ ] Lệnh stdin: `status`, `clients`, `quit`

**CN 29/11: `device/server.c` + `handlers.c` (tiêu chí 2: vào/ra server)**
- [ ] Mảng client, mỗi client có `linebuf` riêng
- [ ] `server_accept`, `server_handle_readable` (lặp `linebuf_next`), `server_close_client`
- [ ] Xoá client khỏi mảng khi ngắt kết nối, tính lại `max_fd`
- [ ] `handlers.c`: bảng điều phối lệnh + `HELLO`, `BYE`, `INFO` (chưa kiểm tra token)
- [ ] `discovery.h` + hàm giả `discovery_open/handle` để Minh thay bằng bản thật ở Phase 2 (**H5**)

### Việc chi tiết: MINH

**T3 24/11: `common/kv.c` (phần 1)**
- [ ] `split_words` (dùng `strtok_r`)
- [ ] `kv_find`, `kv_split_pair` (`Tp1` không được khớp nhầm `Tp10`)
- [ ] `kv_get_str`, `kv_get_double`, `kv_get_int`
- [ ] `parse_double_strict`, `parse_int_strict`

**T4 25/11: `kv.c` (phần 2) + `controller/ui.c`**
- [ ] `parse_hhmm`, `format_hhmm`
- [ ] `kv_append` (báo lỗi nếu vượt `MAX_LINE`)
- [ ] `dev_type_from_str`, `dev_type_to_str`, `dev_type_prefix`, `resp_code`
- [ ] Chương trình thử riêng cho `kv.c` (các trường hợp trong khối chú thích của `kv.c`)
- [ ] `ui_read_line`, `ui_read_int`, `ui_read_double`

**T5 26/11: bàn giao + hoàn thiện `ui.c`**
- [ ] 🔁 **H3**: gộp `kv` vào `main`, báo Long
- [ ] Thay hàm tạm `random_hex` / `send_all` bằng bản của Long (nếu đã viết tạm)
- [ ] `ui_read_double_keep`, `ui_read_hhmm`, `ui_read_password` (tắt echo bằng termios)
- [ ] `ui_menu`, hàm in bảng, `ui_ok/err/info`, `ui_explain_code`, `ui_pause`

**T6 27/11: `controller/main.c` + `coop.c` (phần 1)**
- [ ] App ID: đọc `data/app.conf`, chưa có thì sinh 8 hex rồi lưu
- [ ] Tham số `-d <thư mục data>` (để chạy 2 controller)
- [ ] Khung menu chính 1–4, 0
- [ ] `coop.c`: struct chuồng + mảng động thiết bị, `coop_add/edit/remove/find`

**T7 28/11: `coop.c` (tiêu chí 13: quản lý chuồng)**
- [ ] `coop_load` / `coop_save` (`data/coops.txt`, ghi vào file tạm rồi `rename`)
- [ ] Kiểm tra dữ liệu: tên không rỗng, không trùng; `0 ≤ chickens ≤ capacity`
- [ ] `coop_menu`: danh sách / thêm / sửa / xoá / xem thiết bị / xoá thiết bị
- [ ] `coop_add_device` từ chối thiết bị đã thuộc chuồng khác

**CN 29/11: `controller/conn.c` (khung)**
- [ ] `conn_open(ip, port)` dùng `getaddrinfo`
- [ ] Bảng phiên `{device_id, fd, linebuf, token, connected}`
- [ ] `conn_cmd(...)` gửi bằng `send_line`, đọc 1 dòng bằng `read_line_blocking` (timeout 3s)
- [ ] Phát hiện mất kết nối (recv = 0 / lỗi)
- [ ] Thử với `nc -l 7000` đóng vai thiết bị

---

## T2 30/11 ★ TÍCH HỢP & KIỂM TRA CUỐI TUẦN

### Long tự kiểm
- [ ] 3 cửa sổ `nc localhost 7000` cùng lúc đều nhận được `HELLO`; tắt 1 cửa sổ thì server vẫn chạy
- [ ] `(printf 'HEL'; sleep 1; printf 'LO\nINFO X\n') | nc localhost 7000` cho ra đúng 2 phản hồi
- [ ] Gửi dòng 5000 byte thì nhận `400 LINE_TOO_LONG`, sau đó kết nối vẫn dùng tiếp được
- [ ] Chạy lại `device_sim` thì ID không đổi; sau `make reset-data` thì ID mới
- [ ] Ctrl+C rồi chạy lại ngay, không gặp lỗi "Address already in use"
- [ ] Không client nào kết nối thì lệnh `status` vẫn chạy (vòng tick hoạt động)

### Minh tự kiểm
- [ ] Lần chạy đầu sinh App ID, các lần sau giữ nguyên; `-d data2` cho App ID khác
- [ ] Thêm / sửa / xoá chuồng, thoát rồi mở lại thì dữ liệu còn nguyên
- [ ] Nhập dữ liệu rác / Enter rỗng / Ctrl+D vào menu thì không crash, không lặp vô hạn
- [ ] `nc -l 7000` không gõ gì thì controller báo "Thiết bị không phản hồi" sau 3s
- [ ] Tắt `nc` giữa chừng thì controller báo "Mất kết nối"

### Kiểm chung
- [ ] Controller gửi `HELLO` và `INFO <id>` tới `device_sim` thật và in đúng kết quả
- [ ] 🔁 **H4** (server chạy được HELLO/CONNECT tối thiểu/INFO) và **H5** (giao diện discovery) đã bàn giao
- [ ] `make` với `-Wall -Wextra` không còn cảnh báo
- [ ] Toàn bộ code đã gộp vào `main` qua Pull Request, người kia đã review
- [ ] Ghi kết quả các kịch bản A1–A5, B1–B5, C3–C4, G3 vào `docs/test_cases.md`

---

## Lưu ý để hai người không phải chờ nhau

| Tình huống | Cách xử lý |
|---|---|
| Minh cần `random_hex` / `send_all` trước 26/11 | Tạm tự viết một bản đơn giản, đến 26/11 thì thay bằng bản của Long |
| Long cần `kv.c` cho `device.c` từ 26/11 | H3 đến hạn đúng hôm đó. Nếu Minh trễ, Long tạm tách chuỗi bằng `strtok_r` |
| Minh chưa có server thật để thử `conn.c` | Dùng `nc -l 7000` và gõ tay phản hồi |
| Long chưa có controller để thử server | Dùng `nc localhost 7000` gõ tay lệnh |
| Có người trễ quá 1 ngày | Báo ngay ở buổi họp ngắn. Ưu tiên giữ **`net_io` + vòng `select()`** vì toàn bộ Phase 2 dựa trên hai phần này |

## Ghi chú cuối tuần (điền vào 30/11)

- Đã xong:
- Chưa xong / chuyển sang tuần 2:
- Thay đổi giao thức (đã ghi vào `PROTOCOL.md` mục 7 chưa?):
- Vấn đề cần hỏi thầy:
