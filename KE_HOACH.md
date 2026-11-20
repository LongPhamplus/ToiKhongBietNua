# KẾ HOẠCH BÀI TẬP GIỮA KỲ — IT4062
# Ứng dụng mô phỏng điều khiển hệ thống nuôi gà lấy trứng tự động

> Nhóm: **Long** & **Minh** · Thời gian: **20/11/2026 → 20/12/2026** (4 tuần + 2 ngày dự phòng)
> Ngôn ngữ: C (gnu11) · Môi trường: Linux / WSL Ubuntu · Biên dịch: `gcc -Wall -Wextra -g`
>
> Tài liệu này **chỉ là kế hoạch** — chưa có dòng code nào. Mọi quyết định thiết kế bên dưới là đề xuất; nếu cả hai thống nhất thay đổi thì sửa lại tài liệu này trước, rồi mới code.

---

## Mục lục

0. [Tóm tắt đề bài & thang điểm](#0-tóm-tắt-đề-bài--thang-điểm)
1. [Kiến thức cần nắm & ánh xạ với thư mục IT4062](#1-kiến-thức-cần-nắm--ánh-xạ-với-thư-mục-it4062)
2. [Kiến trúc tổng thể](#2-kiến-trúc-tổng-thể)
3. [Đặc tả giao thức (hợp đồng giữa 2 người)](#3-đặc-tả-giao-thức-hợp-đồng-giữa-2-người)
4. [Mô hình dữ liệu thiết bị & thông số mặc định](#4-mô-hình-dữ-liệu-thiết-bị--thông-số-mặc-định)
5. [Phân công tổng quát Long / Minh](#5-phân-công-tổng-quát-long--minh)
6. [Lộ trình theo Phase](#6-lộ-trình-theo-phase)
7. [Bảng đối chiếu thang điểm → Phase → Người → Cách kiểm chứng](#7-bảng-đối-chiếu-thang-điểm)
8. [Quy trình làm việc chung (Git, review, họp)](#8-quy-trình-làm-việc-chung)
9. [Kịch bản demo & kiểm thử cuối kỳ](#9-kịch-bản-demo--kiểm-thử-cuối-kỳ)
10. [Rủi ro & phương án dự phòng](#10-rủi-ro--phương-án-dự-phòng)

---

## 0. Tóm tắt đề bài & thang điểm

Hai chương trình:

| Chương trình | Vai trò mạng | Chức năng chính |
|---|---|---|
| **`device_sim`** — mô phỏng cảm biến & thiết bị | **Server** (UDP để được quét + TCP để được điều khiển) | Mỗi thiết bị có ID duy nhất; trả lời gói quét; xác thực mật khẩu → cấp **token** gắn với **App ID**; thực hiện lệnh bật/tắt, đặt thông số; tự vận hành (quạt/đèn/phun sương theo ngưỡng, máy ăn/uống theo lịch); ghi log |
| **`controller`** — chương trình điều khiển của người dùng | **Client** | Sinh **App ID** ngẫu nhiên khi cài đặt; quản lý chuồng nuôi; quét thiết bị; thêm thiết bị vào chuồng; kết nối (gửi mật khẩu + App ID); đổi mật khẩu; bật/tắt; đặt thông số; điều khiển trực tiếp; xem thông tin & log |

Thang điểm (tổng **20 điểm**):

| # | Tiêu chí | Điểm |
|---|---|:---:|
| 1 | Xử lý truyền dòng | 1 |
| 2 | Cài đặt cơ chế vào/ra socket trên server | 2 |
| 3 | Xác thực truy cập điều khiển | 1 |
| 4 | Khởi tạo các thông số cho thiết bị | 1 |
| 5 | Quét thiết bị | 2 |
| 6 | Kết nối thiết bị | 2 |
| 7 | Đổi mật khẩu | 1 |
| 8 | Điều khiển bật/tắt | 1 |
| 9 | Thiết lập thông số cho mỗi bộ phận (máy ăn uống, quạt, đèn sưởi, phun sương) | 4 |
| 10 | Điều khiển trực tiếp (ăn, uống, quạt, đèn, phun sương) bên cạnh lập lịch | 1 |
| 11 | Lấy thông tin, thông số mỗi loại thiết bị (7 loại) | 2 |
| 12 | Ghi log hoạt động trên mỗi thiết bị | 1 |
| 13 | Quản lý thông tin chuồng nuôi | 1 |

---

## 1. Kiến thức cần nắm & ánh xạ với thư mục IT4062

Cả hai người **đều phải đọc hết** bảng này trong Phase 0 — khi vấn đáp, thầy có thể hỏi bất kỳ ai về bất kỳ phần nào.

| Kiến thức | Dùng vào đâu trong bài | Đọc/chạy lại file nào trong `IT4062/` |
|---|---|---|
| Biên dịch nhiều file, `Makefile`, header guard | Tách project thành `common/`, `device/`, `controller/` | `1-Chap1/lab00_build/` (main.c + mathutil.c/h + Makefile) |
| Cấp phát động `malloc/realloc/free` | Danh sách thiết bị, danh sách chuồng, bộ đệm nhận | `1-Chap1/lab03_dynamic_array.c` |
| Con trỏ hàm, **bảng điều phối lệnh** (dispatch table) | Ánh xạ `"POWER"`, `"SET"`, `"INFO"`… → hàm xử lý (cả 2 phía) | `1-Chap1/lab05_function_pointer.c` |
| `stdin/stdout/stderr`, `errno`, `perror` | Báo lỗi socket rõ ràng | `1-Chap1/lab06_io_basic.c` |
| File `FILE*`: `fopen/fprintf/fgets` | **Log** từng thiết bị, file cấu hình, lưu dữ liệu chuồng | `1-Chap1/lab07_file_stdio.c` |
| `open/read/write` mức thấp | Đọc `/dev/urandom` để sinh token/App ID | `1-Chap1/lab08_lowlevel_io.c` |
| Xử lý xâu: `strtok_r`, `snprintf`, `strncpy` an toàn | Phân tích lệnh `KEY=VALUE`, tránh tràn bộ đệm | `1-Chap1/lab09_string.c` |
| `memset/memcpy/memmove` | Dịch bộ đệm nhận sau khi cắt 1 dòng (xử lý truyền dòng) | `1-Chap1/lab10_memory.c` |
| `stat`, thư mục | Tạo thư mục `logs/`, `data/` nếu chưa có | `1-Chap1/lab11_filesystem.c` |
| Socket TCP: `socket/bind/listen/accept/connect/send/recv` | Kênh điều khiển controller ↔ thiết bị | `2-Chap2/tcp_server.c`, `2-Chap2/tcp_client.c` |
| Socket UDP: `sendto/recvfrom` | **Quét thiết bị** (discovery) | `2-Chap2/udp_sender.c`, `2-Chap2/udp_receiver.c` |
| Giao thức lệnh dạng text, dấu kết thúc (`END_OF_LIST`) | Ý tưởng cho giao thức tự định nghĩa | `2-Chap2/music_server.c`, `music_client.c` |
| `getaddrinfo` / phân giải tên | Cho phép nhập IP hoặc hostname của máy chạy `device_sim` | `2-Chap2/domain2ip.c` |
| `SO_REUSEADDR`, non-blocking (`ioctl FIONBIO`) | Khởi động lại server không bị "Address already in use" | `3-Chap3/L1-NonBlocking/non_blocking_server.c` |
| Đa tiến trình `fork`, `SIGCHLD` | Biết để so sánh / trả lời vấn đáp (không dùng chính) | `3-Chap3/L3-MultiProcessing/*` |
| Đa luồng `pthread`, `mutex` | Phương án dự phòng cho server; biết để so sánh | `3-Chap3/L4-MultiThreading/pthread_server.c`, `pthread_chatroom.c` |
| **`select()` — I/O multiplexing** | **Cơ chế vào/ra chính của server** (UDP + TCP listener + nhiều client + timer mô phỏng) và của bộ quét phía client (timeout chờ phản hồi) | `3-Chap3/L5-IOMultiPlexing/select_server.c`, `select_client.c` |
| Nhận đến khi đủ dấu phân cách (`\r\n\r\n`), ghép bộ đệm | **Xử lý truyền dòng** (TCP là luồng byte, không có ranh giới thông điệp) | `4-Chap4/http_server.c` (vòng `recv` tới khi gặp `\r\n\r\n`) |
| Broadcast UDP (`SO_BROADCAST`) | Quét toàn mạng LAN | Buổi 12 (Multicast/Broadcast) + UNP Vol.1 ch.20 trong `0-Docs/` |
| Thiết kế giao thức | Phần 3 của tài liệu này | Buổi 8 + `0-Docs/IT4062_..._LectureNotes_1.pdf` |

**Lỗi trong code mẫu cần biết để KHÔNG lặp lại** (thầy rất hay hỏi):

- `select_server.c`: client ngắt kết nối nhưng **không bị xoá khỏi `client_list`** và `max_fd` không được tính lại → phải xoá phần tử & tính lại `max_fd` mỗi vòng.
- `select_client.c`: `timeout` chỉ đặt 1 lần ngoài vòng lặp — trên Linux `select()` **sửa đổi** `timeval`, phải đặt lại mỗi vòng.
- `pthread_chatroom.c`: truyền `&client_socket` (biến cục bộ của vòng lặp) vào thread → race condition; phải `malloc` một ô riêng cho mỗi thread.
- `tcp_server.c` / nhiều file: gọi `recv` **một lần** và coi như nhận đủ thông điệp → sai với TCP; đây chính là lý do có tiêu chí "xử lý truyền dòng".
- `send()` có thể gửi thiếu byte → cần hàm `send_all()` lặp đến khi gửi hết.
- `telnet_server.c` / `music_server.c`: dùng `system()` với dữ liệu từ mạng → lỗ hổng chèn lệnh; bài của mình **không dùng `system()`**.

---

## 2. Kiến trúc tổng thể

### 2.1. Sơ đồ

```
            ┌──────────────── controller (Minh chủ trì) ────────────────┐
            │ App ID (sinh 1 lần, lưu data/app.conf)                    │
            │ Quản lý chuồng (data/coops.txt)                           │
            │ Menu CLI ──► Bảng điều phối lệnh ──► Connection Manager   │
            └───────┬──────────────────────────────────────┬────────────┘
       UDP broadcast│ "SCAN"                     TCP (1 kết nối/thiết bị)
       port 9999    │                            "CONNECT/INFO/SET/..."
                    ▼                                      ▼
            ┌──────────────── device_sim (Long chủ trì) ────────────────┐
            │  vòng lặp select():                                       │
            │   ├─ UDP socket  :9999  (trả lời SCAN)   ← module Minh    │
            │   ├─ TCP listener:7000  (accept)                          │
            │   ├─ N client TCP (bộ đệm dòng riêng từng client)         │
            │   ├─ stdin (lệnh quản trị: status, quit)                  │
            │   └─ timeout 1s ──► sim_tick(): mô phỏng môi trường,      │
            │                     ngưỡng, lịch cho ăn, đếm trứng        │
            │  Danh sách thiết bị: ENV_SENSOR, EGG_COUNTER, FEEDER,     │
            │   WATERER, FAN, HEATER, MISTER (mỗi cái: ID, mật khẩu,    │
            │   thông số, bảng token, file log riêng)                   │
            └───────────────────────────────────────────────────────────┘
```

### 2.2. Các quyết định thiết kế chính (và lý do — dùng để trả lời vấn đáp)

| Quyết định | Lựa chọn | Lý do |
|---|---|---|
| Một tiến trình `device_sim` mô phỏng bao nhiêu thiết bị? | **Một tiến trình = một "cụm thiết bị" của 1 chuồng** (nạp từ file cấu hình, mặc định 7 thiết bị, mỗi cái có ID riêng). Muốn mô phỏng nhiều chuồng → chạy nhiều tiến trình với file cấu hình + cổng TCP khác nhau | Các thiết bị cùng chuồng **dùng chung một mô hình môi trường** (quạt bật thì nhiệt độ cảm biến giảm), nên đặt chung tiến trình là tự nhiên nhất. Mỗi thiết bị vẫn có ID, mật khẩu, token, log độc lập đúng đề bài |
| Quét thiết bị | **UDP**, cổng ứng dụng cố định **9999**, controller gửi **broadcast** (`255.255.255.255` hoặc IP cụ thể) | UDP không cần kết nối, hỗ trợ broadcast → phát hiện thiết bị chưa biết địa chỉ. Đúng câu "lắng nghe các thông điệp quét trên một cổng ứng dụng" |
| Điều khiển | **TCP**, mỗi tiến trình `device_sim` một cổng (mặc định 7000, cổng được báo trong phản hồi quét) | Lệnh điều khiển cần tin cậy, có thứ tự |
| Cơ chế vào/ra server | **`select()` đơn luồng** | Một vòng lặp quản lý được UDP + TCP listener + nhiều client + stdin + **bộ đếm thời gian mô phỏng** (qua timeout). Không có dữ liệu dùng chung giữa các luồng → **không cần mutex**, tránh race. (Dự phòng: pthread + mutex nếu cần) |
| Định dạng thông điệp | **Text, mỗi thông điệp 1 dòng kết thúc `\n`**, trường cách nhau bằng dấu cách, tham số dạng `key=value` | Dễ đọc, dễ debug bằng `nc` / Wireshark, dễ tách dòng (truyền dòng) |
| Thời gian mô phỏng | Đồng hồ mô phỏng **tăng tốc**: mặc định 1 giây thực = 1 phút mô phỏng (tham số `--speed`) | Để thấy được lịch cho ăn 06:00, 11:00, 17:00 trong vài phút demo |
| Lưu trạng thái | Thông số & mật khẩu thiết bị lưu ra `data/devices_<tên>.conf` mỗi khi thay đổi; dữ liệu chuồng lưu ở `data/coops.txt` phía controller | Khởi động lại không mất cấu hình (ID ổn định, đổi mật khẩu có hiệu lực lâu dài) |

### 2.3. Cấu trúc thư mục dự kiến (sẽ tạo ở Phase 0, chưa tạo bây giờ)

```
App/
├── KE_HOACH.md            ← tài liệu này
├── PROTOCOL.md            ← tách phần 3 ra khi đã chốt (Phase 0)
├── Makefile               ← build cả 2 chương trình vào bin/
├── common/                ← DÙNG CHUNG — sửa phải báo người kia
│   ├── protocol.h         ← hằng số: cổng, mã lỗi, tên lệnh, kiểu thiết bị, độ dài tối đa
│   ├── net_io.h / .c      ← send_all(), linebuf_*() (xử lý truyền dòng)      [Long]
│   ├── kv.h / .c          ← tách "key=value", tách token theo dấu cách         [Minh]
│   └── util.h / .c        ← random_hex() từ /dev/urandom, thời gian, trim      [Long]
├── device/                ← chương trình device_sim
│   ├── main.c             ← đọc tham số, vòng select()                        [Long]
│   ├── server.c/.h        ← quản lý client TCP, gọi dispatcher                [Long]
│   ├── auth.c/.h          ← mật khẩu, token, kiểm tra quyền                   [Long]
│   ├── device.c/.h        ← struct thiết bị, nạp/lưu cấu hình, mặc định       [Long]
│   ├── dlog.c/.h          ← ghi log từng thiết bị                             [Long]
│   ├── discovery.c/.h     ← socket UDP 9999, trả lời SCAN                      [Minh]
│   ├── env.c/.h           ← mô hình môi trường nhiệt độ/độ ẩm                 [Long]
│   ├── dev_climate.c      ← xử lý FAN, HEATER, ENV_SENSOR                     [Long]
│   ├── dev_feed.c         ← xử lý FEEDER, WATERER (lịch), EGG_COUNTER         [Minh]
│   ├── dev_mister.c       ← xử lý MISTER                                      [Minh]
│   └── handlers.c         ← bảng điều phối lệnh phía thiết bị                 [Long]
├── controller/            ← chương trình controller
│   ├── main.c             ← khởi tạo App ID, vòng menu                        [Minh]
│   ├── ui.c/.h            ← menu, nhập liệu an toàn, in bảng                  [Minh]
│   ├── coop.c/.h          ← quản lý chuồng nuôi + lưu file                    [Minh]
│   ├── scanner.c/.h       ← gửi SCAN broadcast, gom phản hồi với select()    [Minh]
│   ├── conn.c/.h          ← kết nối TCP, giữ token, gửi lệnh/nhận phản hồi   [Minh]
│   ├── ui_device.c/.h     ← màn hình chung: bật/tắt, mode, đổi mk, log, tổng quan [Minh]
│   ├── ui_climate.c       ← màn hình quạt/đèn/cảm biến                        [Long]
│   └── ui_feed.c          ← màn hình ăn/uống/phun sương/đếm trứng             [Minh]
├── config/
│   └── coop_A.conf        ← cấu hình mẫu 7 thiết bị cho device_sim
├── data/                  ← (tự sinh) app.conf, coops.txt, devices_*.conf
├── logs/                  ← (tự sinh) <device_id>.log
└── docs/
    ├── report.md          ← báo cáo
    └── test_cases.md      ← kết quả kiểm thử
```

> Ghi chú cách chia file: mỗi file có **đúng một chủ** → hầu như không bao giờ xung đột Git. Thư mục `common/` là "hợp đồng" — ai sửa phải nhắn người kia.

---

## 3. Đặc tả giao thức (hợp đồng giữa 2 người)

**Đây là phần quan trọng nhất của Phase 0.** Khi giao thức đã chốt, Long và Minh có thể code song song mà không cần chờ nhau: Long kiểm thử server bằng `nc`, Minh kiểm thử client bằng `nc -l` giả làm thiết bị.

### 3.1. Quy tắc chung

- Mã hoá: ASCII/UTF-8, **mỗi thông điệp là một dòng kết thúc bằng `\n`** (chấp nhận và bỏ qua `\r` đứng trước để dùng được `telnet`/`nc`).
- Độ dài tối đa một dòng: **1024 byte** (`MAX_LINE`). Dòng dài hơn → server trả `400 LINE_TOO_LONG`, bỏ phần dư đến `\n` tiếp theo.
- Trường cách nhau bằng **một dấu cách**; tham số dạng `key=value`; giá trị không chứa dấu cách (tên có dấu cách thì thay bằng `_`).
- Mỗi yêu cầu TCP nhận **đúng một dòng phản hồi**, bắt đầu bằng mã số:

| Mã | Ý nghĩa |
|---|---|
| `200 OK ...` | Thành công (kèm dữ liệu `key=value`) |
| `400 BAD_REQUEST <lý do>` | Sai cú pháp / thiếu tham số / dòng quá dài |
| `401 AUTH_FAILED` | Sai mật khẩu |
| `403 INVALID_TOKEN` | Token không tồn tại / không khớp thiết bị / không khớp App ID |
| `404 NO_SUCH_DEVICE` | ID thiết bị không có trong tiến trình này |
| `405 NOT_SUPPORTED` | Lệnh không áp dụng cho loại thiết bị này (vd. `SET Tmax` cho máy đếm trứng) |
| `409 POWER_OFF` | Gửi `ACT` khi thiết bị đang tắt nguồn |
| `422 INVALID_PARAM <key>` | Giá trị ngoài khoảng hoặc vi phạm ràng buộc (vd. Tp1 ≥ Tmax) |
| `423 LOCKED <giây>` | Sai mật khẩu quá 5 lần liên tiếp → khoá tạm 60 giây (điểm cộng an toàn) |
| `500 SERVER_ERROR` | Lỗi nội bộ |

### 3.2. Quét thiết bị — UDP cổng 9999

| Chiều | Thông điệp |
|---|---|
| Controller → broadcast | `SCAN <app_id>` |
| Device_sim → controller (mỗi thiết bị **một datagram**) | `DEVICE id=<device_id> type=<TYPE> port=<tcp_port> name=<tên>` |

- Controller lấy **IP** từ địa chỉ nguồn của `recvfrom`, không cần đưa IP vào gói.
- Controller chờ phản hồi trong **2 giây** bằng `select()` có timeout (đặt lại timeout mỗi vòng), rồi in danh sách, đánh dấu thiết bị nào đã có trong chuồng.
- Thông điệp UDP không phải `SCAN ...` → bỏ qua, ghi log cấp DEBUG.

### 3.3. Lệnh TCP (cổng do `port=` trong phản hồi quét)

| Lệnh (controller → thiết bị) | Phản hồi thành công | Tiêu chí điểm |
|---|---|:---:|
| `HELLO` | `200 OK devices=<số> server=device_sim/1.0` | kiểm tra kết nối |
| `CONNECT <device_id> <app_id> <password>` | `200 OK token=<32 hex> type=<TYPE> name=<tên>` | 3, 6 |
| `INFO <device_id> <token>` | `200 OK type=FAN power=ON mode=AUTO running=1 Tmax=32.0 Tp1=28.0 temp=30.4 ...` | 11 |
| `POWER <device_id> <token> ON\|OFF` | `200 OK power=ON` | 8 |
| `SET <device_id> <token> k1=v1 [k2=v2 ...]` | `200 OK` + thông số mới | 9 |
| `MODE <device_id> <token> AUTO\|MANUAL` | `200 OK mode=MANUAL` | 10 |
| `ACT <device_id> <token> <hành_động> [tham_số]` | `200 OK ...` | 10 |
| `SCHED <device_id> <token> LIST\|ADD hh:mm\|DEL hh:mm` | `200 OK times=06:00,11:00,17:00` | 9 |
| `CHPASS <device_id> <token> <mk_cũ> <mk_mới>` | `200 OK` (mọi token khác của thiết bị bị huỷ) | 7 |
| `LOG <device_id> <token> [n]` | `200 OK lines=<k>` theo sau là k dòng log, kết thúc bằng dòng `.` | 12 (xem từ xa) |
| `DISCONNECT <device_id> <token>` | `200 OK` (huỷ token) | 6 |
| `BYE` | `200 OK` rồi server đóng socket | — |

> Thứ tự tham số luôn là **`LỆNH <device_id> <token> ...`** để server dùng chung một hàm kiểm tra quyền trước khi gọi hàm xử lý riêng.

**Hành động `ACT` theo loại thiết bị (điều khiển trực tiếp):**

| Loại | Hành động | Ý nghĩa |
|---|---|---|
| FEEDER | `FEED_NOW [kg=<số>]` | Cho ăn ngay (mặc định W kg), không chờ lịch |
| WATERER | `WATER_NOW [l=<số>]` | Cấp nước ngay (mặc định Vw lít) |
| FAN | `RUN` / `STOP` | Bật/tắt quạt ngay (tự chuyển sang MANUAL) |
| HEATER | `RUN` / `STOP` | Bật/tắt đèn sưởi ngay |
| MISTER | `RUN` / `STOP` | Bật/tắt phun sương ngay |
| EGG_COUNTER | `COLLECT` | Thu trứng: số trứng trong máng về 0, cộng vào tổng đã thu |

**Phân biệt 3 khái niệm (rất dễ bị hỏi):**
- `POWER OFF` = **ngắt nguồn thiết bị**: không chạy gì, kể cả lịch/ngưỡng; `INFO` vẫn trả lời được (giả lập bộ điều khiển còn điện chờ).
- `MODE AUTO` = thiết bị tự chạy theo **ngưỡng** (quạt/đèn/phun sương) hoặc **lịch** (ăn/uống).
- `MODE MANUAL` + `ACT RUN/STOP` = người dùng **điều khiển trực tiếp**, bỏ qua ngưỡng. Với FEEDER/WATERER, `FEED_NOW/WATER_NOW` chạy được cả ở AUTO (cho ăn thêm ngoài lịch).

### 3.4. Xác thực & token

- Mật khẩu: 4–32 ký tự, không dấu cách. Mặc định lấy từ file cấu hình (vd. `123456`).
- Khi `CONNECT` đúng mật khẩu: sinh **token 32 ký tự hex** từ 16 byte `/dev/urandom`, lưu bảng `{token, app_id, thời điểm tạo, lần dùng cuối}` **trong từng thiết bị** (tối đa 8 token/thiết bị; đầy thì thay token dùng lâu nhất).
- Mọi lệnh sau `CONNECT` phải có token; server kiểm tra: token tồn tại **và** thuộc đúng `device_id` **và** (tuỳ chọn) đến từ đúng App ID đã đăng ký.
- `CHPASS` thành công → huỷ mọi token khác của thiết bị đó (các app khác phải kết nối lại).
- Token hết hạn sau 30 phút không dùng (tham số cấu hình).
- Sai mật khẩu 5 lần liên tiếp (theo thiết bị) → `423 LOCKED 60`.

### 3.5. Ví dụ phiên làm việc hoàn chỉnh

```
C→S (UDP)  SCAN 7f3a9c21
S→C (UDP)  DEVICE id=FAN-8A21 type=FAN port=7000 name=Quat_1
S→C (UDP)  DEVICE id=FED-11C0 type=FEEDER port=7000 name=May_an_1
...
C→S (TCP)  CONNECT FAN-8A21 7f3a9c21 123456
S→C        200 OK token=4b1f...e9 type=FAN name=Quat_1
C→S        SET FAN-8A21 4b1f...e9 Tmax=31 Tp1=27
S→C        200 OK Tmax=31.0 Tp1=27.0
C→S        SET FAN-8A21 4b1f...e9 Tp1=35
S→C        422 INVALID_PARAM Tp1 (phai nho hon Tmax=31.0)
C→S        INFO FAN-8A21 4b1f...e9
S→C        200 OK type=FAN power=ON mode=AUTO running=1 Tmax=31.0 Tp1=27.0 temp=31.6
```

---

## 4. Mô hình dữ liệu thiết bị & thông số mặc định

### 4.1. Bảy loại thiết bị

| Loại (`TYPE`) | Tiền tố ID | Thông số đặt được (`SET`) | Mặc định | Ràng buộc | Thông tin trả về (`INFO`) | Người phụ trách |
|---|---|---|---|---|---|---|
| `ENV_SENSOR` | `ENV-` | `interval` (phút mô phỏng giữa 2 lần đo) | 1 | 1–60 | `temp`, `humidity`, `last_read` | **Long** |
| `EGG_COUNTER` | `EGG-` | `capacity` (sức chứa máng), `rate` (trứng/giờ) | 300, 12 | 1–10000 | `count`, `total_collected`, `full` (0/1) | **Minh** |
| `FEEDER` | `FED-` | `W` (kg/lần), lịch `SCHED`, `stock` (kg trong bồn) | 5.0 kg; 06:00, 11:00, 17:00; 100 kg | 0.1 ≤ W ≤ 50; tối đa 8 mốc | `W`, `times`, `stock`, `last_feed`, `fed_today` | **Minh** |
| `WATERER` | `WAT-` | `Vw` (lít/lần), lịch `SCHED`, `tank` (lít) | 10 l; 06:00, 11:00, 17:00; 500 l | 0.1 ≤ Vw ≤ 200 | `Vw`, `times`, `tank`, `last_water`, `water_today` | **Minh** |
| `FAN` | `FAN-` | `Tmax`, `Tp1` | 32.0, 28.0 | 10 ≤ Tp1 < Tmax ≤ 45 | `running`, `Tmax`, `Tp1`, `temp`, `run_minutes` | **Long** |
| `HEATER` | `HEA-` | `Tmin`, `Tp2` | 18.0, 22.0 | 5 ≤ Tmin < Tp2 ≤ 35 | `running`, `Tmin`, `Tp2`, `temp`, `run_minutes` | **Long** |
| `MISTER` | `MIS-` | `Hmin`, `Hp`, `Vh` (lít/giờ) | 55, 70, 2.0 | 10 ≤ Hmin < Hp ≤ 95; 0.1 ≤ Vh ≤ 20 | `running`, `Hmin`, `Hp`, `Vh`, `humidity`, `water_used` | **Minh** |

Trường chung mọi thiết bị: `id`, `type`, `name`, `password`, `power` (ON/OFF), `mode` (AUTO/MANUAL — với cảm biến và máy đếm trứng luôn AUTO), `tokens[]`, `fail_count`, `lock_until`, `log_path`.

**Ràng buộc chéo** (kiểm tra khi SET quạt/đèn trong cùng tiến trình): `Tp2 < Tp1` — tránh quạt và đèn sưởi bật cùng lúc "đánh nhau". Long kiểm tra ràng buộc này vì cả FAN và HEATER đều thuộc Long.

### 4.2. Logic tự động (chạy trong `sim_tick()` mỗi giây thực)

```
Quạt   (AUTO, power ON):  nếu temp > Tmax  → running=1 ;  nếu temp ≤ Tp1 → running=0
Đèn    (AUTO, power ON):  nếu temp < Tmin  → running=1 ;  nếu temp ≥ Tp2 → running=0
Phun   (AUTO, power ON):  nếu hum  < Hmin  → running=1 ;  nếu hum  ≥ Hp  → running=0
                         khi running: water_used += Vh × (phút mô phỏng / 60)
Ăn/uống (power ON, AUTO): khi đồng hồ mô phỏng chạm một mốc trong times → nhả W kg / Vw lít
                         (mỗi mốc chỉ chạy 1 lần/ngày; bồn không đủ → nhả phần còn lại, log WARN)
Đếm trứng:                count += rate/60 mỗi phút mô phỏng (cộng dồn phần lẻ), không vượt capacity
```

Đây là **điều khiển trễ (hysteresis)**: hai ngưỡng khác nhau để thiết bị không bật/tắt liên tục quanh một giá trị.

### 4.3. Mô hình môi trường (Long — `env.c`)

- Nhiệt độ ngoài trời dao động hình sin theo giờ mô phỏng (vd. 24 ± 8 °C, đỉnh lúc 14:00) + nhiễu nhỏ.
- Nhiệt độ chuồng tiến dần về nhiệt độ ngoài; quạt chạy: −0.3 °C/phút; đèn sưởi chạy: +0.4 °C/phút.
- Độ ẩm tự giảm dần (vd. −0.2 %/phút, nhanh hơn khi quạt chạy); phun sương chạy: + (Vh × hệ số) %/phút.
- API cho người khác dùng: `env_temp()`, `env_humidity()`, `env_add_humidity(delta)`, `env_add_temp(delta)`. **Minh dùng `env_humidity()` / `env_add_humidity()` cho phun sương** → điểm phụ thuộc chéo đã được định nghĩa rõ.
- Lệnh quản trị trên stdin của `device_sim` để demo nhanh: `settemp 35`, `sethum 40`, `speed 60`, `status`.

### 4.4. File cấu hình `device_sim` (định dạng dự kiến)

```
# config/coop_A.conf
tcp_port=7000
udp_port=9999
speed=60
device type=ENV_SENSOR  name=Cam_bien_1  password=123456
device type=EGG_COUNTER name=Dem_trung_1 password=123456
device type=FEEDER      name=May_an_1    password=123456 W=5 times=06:00,11:00,17:00
device type=WATERER     name=May_uong_1  password=123456 Vw=10
device type=FAN         name=Quat_1      password=123456 Tmax=32 Tp1=28
device type=HEATER      name=Den_1       password=123456 Tmin=18 Tp2=22
device type=MISTER      name=Phun_1      password=123456 Hmin=55 Hp=70 Vh=2
```

- Lần chạy đầu: dòng nào thiếu thông số → dùng mặc định ở bảng 4.1; **sinh ID ngẫu nhiên duy nhất** (tiền tố + 4 hex, kiểm tra trùng) rồi ghi trạng thái ra `data/devices_coop_A.conf`.
- Lần chạy sau: nạp từ `data/devices_coop_A.conf` → ID, mật khẩu, thông số giữ nguyên. → **Tiêu chí 4 (khởi tạo thông số)**.

### 4.5. Dữ liệu phía controller

- `data/app.conf`: `app_id=<8 hex>` — sinh **một lần** khi chạy lần đầu ("mỗi ứng dụng cài đặt"), các lần sau đọc lại.
- `data/coops.txt`: danh sách chuồng và thiết bị trong chuồng:
  ```
  coop id=1 name=Chuong_A location=Khu_1 capacity=500 chickens=420 created=2026-11-25
  dev  coop=1 id=FAN-8A21 type=FAN ip=192.168.1.10 port=7000 name=Quat_1
  ```
- Token **không** lưu xuống file (chỉ trong bộ nhớ) → mỗi lần mở app phải kết nối lại bằng mật khẩu (an toàn hơn; giải thích được khi vấn đáp).

### 4.6. Định dạng log thiết bị (`logs/<device_id>.log`)

```
2026-12-10 14:03:21 | sim 06:00 | INFO  | app=7f3a9c21 | CONNECT ok
2026-12-10 14:03:40 | sim 06:19 | INFO  | app=7f3a9c21 | SET Tmax 32.0->31.0
2026-12-10 14:05:02 | sim 07:41 | INFO  | auto         | FAN running 0->1 (temp=31.6 > Tmax=31.0)
2026-12-10 14:06:10 | sim 08:49 | WARN  | app=99aa0012 | CONNECT fail (wrong password, 3/5)
```
Ghi: kết nối/ngắt, sai mật khẩu, đổi mật khẩu (**không ghi mật khẩu**), mọi SET/POWER/MODE/ACT (giá trị cũ → mới), mọi lần tự bật/tắt, mỗi lần cho ăn/uống, thu trứng, máng đầy. Dùng `fflush` sau mỗi dòng để log không mất khi Ctrl+C.

---

## 5. Phân công tổng quát Long / Minh

### 5.1. Nguyên tắc chia

1. **Chia theo tầng cho phần lõi**: Long chủ trì **server** (`device_sim`), Minh chủ trì **client** (`controller`). Mỗi người hiểu sâu một đầu socket.
2. **Chia theo chiều dọc cho phần tính năng thiết bị**: mỗi người nhận **trọn vẹn** một nhóm thiết bị, làm **cả phía server lẫn màn hình phía controller** → ai cũng phải code cả hai chương trình, không ai chỉ biết một nửa.
3. **Đan xen phụ thuộc có chủ đích**: Minh viết module **quét** gắn vào server của Long; Long viết các màn hình khí hậu gắn vào menu của Minh; Minh dùng API môi trường của Long. → "phục vụ công việc cho nhau".
4. **Giao thức là hợp đồng**: chốt ở Phase 0, sau đó cả hai code song song, kiểm thử độc lập bằng `nc`.

### 5.2. Bảng phân công theo điểm

| Tiêu chí (điểm) | **Long** | **Minh** |
|---|---|---|
| 1. Xử lý truyền dòng (1) | ✅ `net_io.c`: `send_all`, bộ đệm dòng (dùng cho cả 2 chương trình) | Dùng lại ở `conn.c`; viết test gửi phân mảnh |
| 2. Vào/ra socket server (2) | ✅ Vòng `select()`, quản lý nhiều client, timer mô phỏng | — |
| 3. Xác thực truy cập (1) | ✅ Mật khẩu, token, kiểm tra quyền mọi lệnh, khoá khi sai nhiều | — |
| 4. Khởi tạo thông số (1) | ✅ Nạp/lưu cấu hình, sinh ID, mặc định, kiểm tra ràng buộc | — |
| 5. Quét thiết bị (2) | Cung cấp chỗ gắn UDP fd vào `select()` | ✅ `discovery.c` (phía server) + `scanner.c` (phía client) + UI thêm vào chuồng |
| 6. Kết nối thiết bị (2) | Xử lý `CONNECT/DISCONNECT` phía server (thuộc tiêu chí 3) | ✅ `conn.c`: quản lý kết nối nhiều thiết bị, giữ token, App ID, xử lý mất kết nối |
| 7. Đổi mật khẩu (1) | Handler `CHPASS` + huỷ token cũ + lưu file | ✅ UI đổi mật khẩu (nhập ẩn, nhập lại để xác nhận) |
| 8. Bật/tắt (1) | ✅ Handler `POWER` chung cho mọi thiết bị | ✅ UI bật/tắt chung |
| 9. Thiết lập thông số (4) | ✅ **Quạt (1) + Đèn sưởi (1)**: cả server + UI | ✅ **Máy ăn uống (1) + Phun sương (1)**: cả server + UI, gồm lịch |
| 10. Điều khiển trực tiếp (1) | ✅ Quạt, đèn: `MODE`, `ACT RUN/STOP` | ✅ Ăn, uống, phun sương: `FEED_NOW`, `WATER_NOW`, `RUN/STOP` |
| 11. Lấy thông tin (2) | ✅ ENV_SENSOR, FAN, HEATER | ✅ EGG_COUNTER, FEEDER, WATERER, MISTER |
| 12. Ghi log (1) | ✅ `dlog.c` + lệnh `LOG` phía server | ✅ UI xem log từ xa |
| 13. Quản lý chuồng nuôi (1) | — | ✅ `coop.c`: thêm/sửa/xoá/liệt kê chuồng, thiết bị trong chuồng, lưu file |
| **Ước lượng khối lượng** | **≈ 10 điểm** | **≈ 10 điểm** |
| Mô phỏng môi trường (không chấm riêng, nhưng cần để demo) | ✅ `env.c` | Dùng API |
| Báo cáo | Phần server, giao thức, I/O | Phần client, quét, kết nối, chuồng |

### 5.3. Các điểm "giao hàng" giữa hai người (handoff)

| Mã | Người giao → nhận | Nội dung | Hạn |
|---|---|---|---|
| H1 | Cả hai | `common/protocol.h` + `PROTOCOL.md` chốt | 23/11 |
| H2 | Long → Minh | `net_io.c` (send_all + line buffer) đã có test | 26/11 |
| H3 | Minh → Long | `kv.c` (tách key=value / tách từ) đã có test | 26/11 |
| H4 | Long → Minh | Server chạy được: `HELLO`, `CONNECT`, `INFO` tối thiểu cho mọi loại (dữ liệu giả) | 30/11 |
| H5 | Long → Minh | Giao diện gắn module vào vòng `select()`: `int discovery_open(port)`, `void discovery_handle(fd, devices)` | 30/11 |
| H6 | Minh → Long | `conn_request(dev, "LỆNH ...", resp, size)` + khung menu cho phép Long thêm màn hình `ui_climate.c` | 02/12 |
| H7 | Long → Minh | `env.h`: `env_humidity()`, `env_add_humidity()`; hook `dev_tick()` trong `sim_tick()` | 05/12 |
| H8 | Minh → Long | `discovery.c` chạy được trong `device_sim` | 05/12 |

---

## 6. Lộ trình theo Phase

```
Tuần 1 (20–30/11)  Phase 0 Chuẩn bị & chốt giao thức  ─► Phase 1 Nền tảng
Tuần 2 (01–07/12)  Phase 2 Quét – Kết nối – Xác thực – Đổi mật khẩu   ★ Mốc M2: end-to-end
Tuần 3 (08–14/12)  Phase 3 Tính năng thiết bị (thông số, điều khiển, thông tin)
Tuần 4 (15–18/12)  Phase 4 Tích hợp, log, độ bền, kiểm thử
       (19–20/12)  Phase 5 Báo cáo, demo, tập vấn đáp (+ dự phòng)
```

---

### PHASE 0 — Chuẩn bị & chốt thiết kế (20/11 – 23/11)

**Mục tiêu:** cả hai cùng hiểu bài toán, có môi trường build giống nhau, **chốt giao thức** để từ Phase 1 làm song song được.

| Việc | Long | Minh |
|---|---|---|
| Môi trường | Cài WSL Ubuntu, gcc, gdb, make, netcat, Wireshark; chạy được `2-Chap2/tcp_*`, `3-Chap3/L5-IOMultiPlexing/select_*` | Như Long |
| Ôn kiến thức | Đọc & chạy: `select_server.c`, `non_blocking_server.c`, `http_server.c` (vòng recv đến `\r\n\r\n`), `lab07`, `lab08` | Đọc & chạy: `udp_sender/receiver.c`, `select_client.c`, `music_client.c`, `domain2ip.c`, `lab05`, `lab09` |
| Thiết kế | Viết nháp mục 3.3–3.4 (lệnh TCP, xác thực), mục 4.2–4.4 | Viết nháp mục 3.2 (quét), mục 4.5 (dữ liệu chuồng), luồng menu controller |
| Chung | Họp 1 buổi: rà từng dòng phần 3 & 4, sửa tài liệu này, tạo `PROTOCOL.md`, `common/protocol.h` (chỉ hằng số), `Makefile`, cây thư mục, repo Git | Như Long |

**Yêu cầu đầu ra (Definition of Done):**
- [ ] Repo Git có cấu trúc thư mục như mục 2.3, `make` chạy được (dù các `main` còn rỗng).
- [ ] `PROTOCOL.md` được cả hai xác nhận — **từ đây đổi giao thức phải hỏi người kia**.
- [ ] Mỗi người tự giải thích được cho người kia: TCP là luồng byte nghĩa là gì; vì sao chọn `select()`; vì sao quét dùng UDP.
- [ ] Phác thảo menu controller (Minh vẽ bằng text, Long góp ý).

**Phác thảo menu (để thống nhất):**
```
=== CHICKEN FARM CONTROLLER ===  App ID: 7f3a9c21
1. Quản lý chuồng nuôi        (thêm / sửa / xoá / xem)
2. Quét thiết bị              (→ chọn thiết bị → thêm vào chuồng)
3. Vào một chuồng             (→ danh sách thiết bị trong chuồng)
     a. Kết nối thiết bị (nhập mật khẩu)
     b. Xem thông tin / thông số
     c. Bật / tắt
     d. Thiết lập thông số     (màn hình riêng theo loại)
     e. Điều khiển trực tiếp   (màn hình riêng theo loại)
     f. Đổi mật khẩu
     g. Xem log thiết bị
     h. Ngắt kết nối
4. Tổng quan chuồng           (INFO tất cả thiết bị đã kết nối trong 1 bảng)
0. Thoát
```

---

### PHASE 1 — Nền tảng (24/11 – 30/11)

**Mục tiêu:** có "xương sống" của cả 2 chương trình. Server nhận được nhiều client, tách dòng đúng; controller có menu, quản lý chuồng, gửi/nhận được lệnh.

#### Long — server core

| # | Việc | Yêu cầu chi tiết |
|---|---|---|
| L1.1 | `common/net_io.c` — **xử lý truyền dòng** (tiêu chí 1) | `send_all(fd, buf, len)` lặp đến khi gửi hết, xử lý `EINTR`. Cấu trúc **bộ đệm dòng** riêng mỗi kết nối: nối dữ liệu `recv` vào cuối; tìm `\n`; trả ra **từng dòng hoàn chỉnh**; phần dư giữ lại (dùng `memmove`); một lần `recv` có thể chứa **0, 1 hoặc nhiều dòng**; dòng > 1024 byte → báo lỗi & bỏ. Một hàm đọc dòng **chặn** (cho client) và một hàm **không chặn** (gọi sau khi `select` báo sẵn sàng, cho server) |
| L1.2 | `common/util.c` | `random_hex(buf, nbytes)` đọc `/dev/urandom` (dự phòng `rand()` seed bằng thời gian + pid); hàm thời gian thực/định dạng; `trim()` bỏ `\r\n` |
| L1.3 | `device/main.c` + `server.c` — **vòng `select()`** (tiêu chí 2) | Đọc tham số dòng lệnh (`-c config`, `-p port`); `SO_REUSEADDR`; mảng client (fd + bộ đệm dòng + địa chỉ IP) tối đa 64; mỗi vòng: `FD_ZERO`, thêm listener + mọi client + stdin (+ UDP fd ở Phase 2), tính `max_fd`, **timeout 1 s đặt lại mỗi vòng**; accept → thêm client; recv = 0/−1 → **đóng và xoá khỏi mảng**; hết timeout → gọi `sim_tick()`; bỏ qua `SIGPIPE` (`signal(SIGPIPE, SIG_IGN)`) để client ngắt đột ngột không giết server |
| L1.4 | `device/device.c` — **khởi tạo thông số** (tiêu chí 4) | `struct device` chung + `union` thông số theo loại; nạp file cấu hình 4.4; điền mặc định 4.1; sinh ID duy nhất; ghi/nạp `data/devices_*.conf`; hàm `device_find(id)` |
| L1.5 | `device/handlers.c` — bảng điều phối | Mảng `{ "HELLO", handler_hello }, { "CONNECT", ... }` (giống `lab05`); dòng không khớp → `400 BAD_REQUEST unknown command`. Phase 1 chỉ cần `HELLO`, `BYE`, `INFO` trả dữ liệu thật từ struct (chưa xác thực) |
| L1.6 | `device/dlog.c` — **log** (tiêu chí 12, phần nền) | `dlog(dev, level, app_id, fmt, ...)` dùng `vfprintf`; mỗi thiết bị một file; tạo thư mục `logs/` nếu chưa có |

**Kiểm thử của Long (không cần chờ Minh):**
- `nc localhost 7000` gõ `HELLO` → nhận `200 OK ...`.
- Mở 3 terminal `nc` cùng lúc → cả 3 được phục vụ; tắt 1 cái → server không treo, không crash.
- `printf 'HEL' | nc ...; sleep 1; printf 'LO\nINFO ...\nINFO ...\n'` → server xử lý đúng 3 lệnh (kiểm chứng truyền dòng).
- Gửi dòng 5000 byte → `400 LINE_TOO_LONG`, kết nối vẫn dùng tiếp được.
- Chạy lại `device_sim` → ID thiết bị giống lần trước.

#### Minh — controller core

| # | Việc | Yêu cầu chi tiết |
|---|---|---|
| M1.1 | `common/kv.c` | Tách một dòng thành mảng từ (dùng `strtok_r`, như `lab09`); tra `key=value` → chuỗi/số thực/số nguyên, có kiểm tra lỗi (`strtod` + kiểm tra ký tự thừa); hàm ghép `key=value` an toàn bằng `snprintf` |
| M1.2 | `controller/main.c` — **App ID** | Lần đầu: sinh 8 hex ngẫu nhiên (dùng `random_hex` của Long — tạm tự viết nếu H2 chưa xong), lưu `data/app.conf`; các lần sau đọc lại; in App ID ở đầu menu |
| M1.3 | `controller/ui.c` | Đọc dòng bằng `fgets` + `trim` (không dùng `scanf("%s")` trần); hàm nhập số có khoảng kiểm tra; nhập mật khẩu ẩn ký tự (`termios` tắt echo) ; hàm in bảng căn cột |
| M1.4 | `controller/coop.c` — **quản lý chuồng** (tiêu chí 13) | Danh sách động (`realloc`) các chuồng: `id, name, location, capacity, chickens, created` + danh sách thiết bị thuộc chuồng (`id, type, ip, port, name`). Chức năng: thêm/sửa/xoá/liệt kê chuồng; xoá thiết bị khỏi chuồng; một thiết bị không được nằm ở 2 chuồng; lưu/nạp `data/coops.txt` sau mỗi thay đổi |
| M1.5 | `controller/conn.c` — khung kết nối | `conn_open(ip, port)` (hỗ trợ hostname qua `getaddrinfo` như `domain2ip.c`); `conn_request(dev, cmd, resp)`: gửi bằng `send_all`, đọc **đúng 1 dòng** phản hồi bằng bộ đệm dòng; timeout đọc 3 s (`select` hoặc `SO_RCVTIMEO`); bảng phiên `{device_id, fd, token, connected}` |

**Kiểm thử của Minh (không cần chờ Long):**
- `nc -l 7000` giả làm thiết bị: controller gửi lệnh → Minh gõ tay `200 OK token=abc` → controller hiển thị đúng.
- Giả lập phản hồi chậm (không gõ gì) → controller báo "hết thời gian chờ", không treo menu.
- Thêm 3 chuồng, thoát, mở lại → dữ liệu còn nguyên. Xoá chuồng có thiết bị → hỏi xác nhận.
- Xoá `data/app.conf` → App ID mới được sinh; không xoá → App ID giữ nguyên.

**Mốc cuối Phase 1 (30/11):** controller của Minh gửi `HELLO`/`INFO` tới server thật của Long và in kết quả. *(Chưa có xác thực.)*

---

### PHASE 2 — Quét, kết nối, xác thực, đổi mật khẩu (01/12 – 07/12)

**Mục tiêu:** luồng nghiệp vụ chính chạy trọn: **quét → thêm vào chuồng → kết nối bằng mật khẩu → nhận token → đổi mật khẩu**. Đây là 8/20 điểm (tiêu chí 3, 5, 6, 7) — ưu tiên cao nhất.

#### Long — xác thực phía thiết bị

| # | Việc | Yêu cầu chi tiết |
|---|---|---|
| L2.1 | `auth.c` — `CONNECT` (tiêu chí 3) | So mật khẩu; đúng → sinh token, lưu `{token, app_id, created, last_used}` vào thiết bị; sai → tăng `fail_count`, đủ 5 → khoá 60 s; trả mã lỗi theo 3.1; log mọi lần thử |
| L2.2 | Kiểm tra quyền chung | Hàm `authorize(line) → device*` dùng cho **mọi lệnh trừ** `HELLO/CONNECT/BYE`: kiểm tra số tham số, thiết bị tồn tại (404), token hợp lệ & khớp thiết bị (403), cập nhật `last_used`, huỷ token hết hạn |
| L2.3 | `CHPASS` (tiêu chí 7, phía server) | Kiểm tra token + mật khẩu cũ + độ dài mật khẩu mới; lưu file ngay; huỷ các token khác; log "password changed by app=..." (không ghi mật khẩu) |
| L2.4 | `DISCONNECT` | Huỷ token đó; log |
| L2.5 | Gắn UDP fd của Minh vào vòng `select()` (H5) | Thêm fd vào `read_fds`, khi sẵn sàng gọi `discovery_handle()` |

#### Minh — quét & kết nối

| # | Việc | Yêu cầu chi tiết |
|---|---|---|
| M2.1 | `device/discovery.c` — phía thiết bị (tiêu chí 5) | Mở UDP, `SO_REUSEADDR` (+ `SO_REUSEPORT` nếu có) để **nhiều tiến trình `device_sim` cùng nhận được broadcast trên 1 máy**; bind `INADDR_ANY:9999`; nhận `SCAN <app_id>` → gửi lại **mỗi thiết bị một datagram** `DEVICE ...` về đúng địa chỉ nguồn; log "scanned by app=..." vào log từng thiết bị |
| M2.2 | `controller/scanner.c` — phía client (tiêu chí 5) | UDP socket, bật `SO_BROADCAST`; người dùng chọn: broadcast `255.255.255.255` hoặc nhập IP cụ thể; gửi `SCAN <app_id>`; vòng `select()` gom phản hồi trong 2 s (**tính thời gian còn lại, đặt lại timeout mỗi vòng**); bỏ trùng theo `id`; hiển thị bảng `STT | Loại | ID | IP:Port | Tên | Đã thuộc chuồng?` |
| M2.3 | Thêm thiết bị vào chuồng (UI) | Sau khi quét: chọn STT → chọn chuồng → lưu vào `coops.txt` |
| M2.4 | `conn.c` — **kết nối** (tiêu chí 6) | Chọn thiết bị trong chuồng → mở TCP tới `ip:port` (tái dùng kết nối nếu đã mở tới cùng `ip:port`) → gửi `CONNECT <id> <app_id> <mật khẩu người dùng nhập>` → lưu token; hiển thị trạng thái kết nối của từng thiết bị (●/○); phát hiện server đóng kết nối (recv = 0) → đánh dấu mất kết nối, đề nghị kết nối lại; xử lý mã 401/423 thành thông báo tiếng Việt dễ hiểu |
| M2.5 | UI **đổi mật khẩu** (tiêu chí 7) | Yêu cầu đã kết nối; nhập mật khẩu cũ, mới, nhập lại mới (ẩn ký tự); kiểm tra khớp & độ dài phía client trước khi gửi `CHPASS` |

**Kiểm thử tích hợp (cả hai cùng ngồi, 07/12) — ★ MỐC M2:**
- [ ] Chạy 2 tiến trình `device_sim` (coop_A cổng 7000, coop_B cổng 7001) → quét 1 lần thấy **14 thiết bị**.
- [ ] Thêm thiết bị vào 2 chuồng khác nhau; thử thêm 1 thiết bị vào 2 chuồng → bị từ chối.
- [ ] Kết nối sai mật khẩu → "Sai mật khẩu"; sai 5 lần → "Thiết bị tạm khoá 60 giây".
- [ ] Kết nối đúng → nhận token; `INFO` với token sai (gõ tay qua `nc`) → `403`.
- [ ] Hai controller (2 App ID khác nhau) cùng kết nối 1 thiết bị → cả hai đều có token riêng.
- [ ] Controller A đổi mật khẩu → token của controller B bị huỷ (B nhận `403` → UI báo "cần kết nối lại").
- [ ] Khởi động lại `device_sim` → mật khẩu mới vẫn còn hiệu lực.
- [ ] Wireshark bắt được gói `SCAN` UDP và phiên TCP — chụp màn hình cho báo cáo.

---

### PHASE 3 — Tính năng từng thiết bị (08/12 – 14/12)

**Mục tiêu:** hoàn thành tiêu chí 8, 9, 10, 11 (8 điểm). Mỗi người làm **trọn vẹn** nhóm thiết bị của mình: handler server + logic tự động + màn hình controller.

#### Phần chung làm trước (08–09/12)

| # | Ai | Việc |
|---|---|---|
| P3.0a | Long | Handler `POWER` (tiêu chí 8) và `MODE` chung cho mọi loại; `INFO` gọi hàm `info_<loại>()` qua bảng con trỏ hàm theo `type`; `SET` gọi `set_<loại>()` — **khung để Minh cắm hàm của mình vào** |
| P3.0b | Minh | UI bật/tắt chung (tiêu chí 8), UI chọn chế độ AUTO/MANUAL; khung màn hình "thiết lập thông số" & "điều khiển trực tiếp" gọi hàm theo loại thiết bị — **khung để Long cắm `ui_climate.c` vào** |

#### Long — nhóm khí hậu: ENV_SENSOR, FAN, HEATER

| # | Việc | Yêu cầu chi tiết |
|---|---|---|
| L3.1 | `env.c` | Mô hình 4.3; lệnh quản trị stdin `settemp`, `sethum`, `speed`, `status` (bảng tất cả thiết bị + nhiệt độ/độ ẩm) |
| L3.2 | ENV_SENSOR | Đo theo `interval`, lưu `temp/humidity/last_read`; `INFO`; `SET interval` |
| L3.3 | FAN — thông số (tiêu chí 9, 1đ) | `SET Tmax= Tp1=` (đặt được 1 hoặc cả 2 trong 1 lệnh, **kiểm tra ràng buộc sau khi áp dụng cả hai**, sai → không đổi gì), logic trễ AUTO, đếm `run_minutes`, log mỗi lần tự bật/tắt kèm nhiệt độ |
| L3.4 | HEATER — thông số (tiêu chí 9, 1đ) | Tương tự với `Tmin`, `Tp2`; ràng buộc chéo `Tp2 < Tp1` của quạt cùng tiến trình |
| L3.5 | Điều khiển trực tiếp FAN/HEATER (tiêu chí 10) | `ACT RUN/STOP` → tự chuyển MANUAL, log; `MODE AUTO` → quay lại theo ngưỡng ở tick tiếp theo |
| L3.6 | `controller/ui_climate.c` | Màn hình xem/đặt thông số quạt, đèn (hiện giá trị hiện tại trước khi nhập, Enter = giữ nguyên); màn hình điều khiển trực tiếp; màn hình xem cảm biến (tiêu chí 11) |

#### Minh — nhóm chăn nuôi: FEEDER, WATERER, MISTER, EGG_COUNTER

| # | Việc | Yêu cầu chi tiết |
|---|---|---|
| M3.1 | FEEDER & WATERER — thông số & lịch (tiêu chí 9, 1đ) | Dùng chung một module lịch: mảng tối đa 8 mốc `hh:mm` đã sắp xếp, không trùng; `SCHED LIST/ADD/DEL`; `SET W=` / `SET Vw=` / `SET stock=` / `SET tank=`; trong `dev_tick()`: khi đồng hồ mô phỏng **vượt qua** mốc (so phút trước & phút hiện tại để không bỏ lỡ khi tốc độ cao) → nhả W/Vw, trừ bồn, cộng `fed_today`; sang ngày mới reset bộ đếm ngày; bồn cạn → log WARN |
| M3.2 | FEEDER & WATERER — trực tiếp (tiêu chí 10) | `ACT FEED_NOW [kg=]`, `ACT WATER_NOW [l=]` |
| M3.3 | MISTER — thông số (tiêu chí 9, 1đ) | `SET Hmin= Hp= Vh=` + ràng buộc; logic trễ theo `env_humidity()`; khi chạy gọi `env_add_humidity()` và cộng `water_used` |
| M3.4 | MISTER — trực tiếp (tiêu chí 10) | `ACT RUN/STOP`, `MODE` |
| M3.5 | EGG_COUNTER | Tăng `count` theo `rate`, chặn ở `capacity`, log khi đầy; `ACT COLLECT`; `INFO` (tiêu chí 11) |
| M3.6 | `controller/ui_feed.c` | Màn hình lịch (liệt kê/thêm/xoá mốc, kiểm tra định dạng `hh:mm` phía client), đặt W/Vw, phun sương, thu trứng, cho ăn/uống ngay; màn hình `INFO` cho 4 loại |
| M3.7 | Màn hình **Tổng quan chuồng** | Gửi `INFO` tới mọi thiết bị đã kết nối trong chuồng, in 1 bảng: nhiệt độ, độ ẩm, quạt/đèn/phun (bật/tắt), số trứng, thức ăn/nước còn |

**Kiểm thử cuối Phase 3 (14/12):**
- [ ] Mỗi loại trong 7 loại: `INFO` trả đủ trường ở bảng 4.1 và controller hiển thị đúng.
- [ ] `SET` sai ràng buộc cho cả 4 bộ phận → `422`, giá trị cũ giữ nguyên.
- [ ] Gõ `settemp 35` ở `device_sim` → quạt tự bật; nhiệt độ giảm về ≤ Tp1 → quạt tự tắt (xem log).
- [ ] `settemp 10` → đèn sưởi tự bật/tắt tương tự. `sethum 30` → phun sương tự bật, độ ẩm tăng, chạm Hp → tắt.
- [ ] Đặt lịch cho ăn sau 2 phút mô phỏng → đúng giờ thì `fed_today` tăng W, `stock` giảm W.
- [ ] `FEED_NOW` ngoài lịch, `ACT RUN` quạt khi nhiệt độ thấp → chạy được; `MODE AUTO` → trở về tự động.
- [ ] `POWER OFF` quạt rồi `settemp 40` → quạt **không** bật.

---

### PHASE 4 — Tích hợp, log, độ bền, kiểm thử (15/12 – 18/12)

**Mục tiêu:** không còn lỗi treo/crash, log đầy đủ, mọi tiêu chí đều có bằng chứng.

| # | Long | Minh |
|---|---|---|
| 4.1 | Lệnh `LOG <id> <token> [n]`: đọc n dòng cuối file log, gửi nhiều dòng + dòng `.` kết thúc (lại là xử lý truyền dòng) | UI xem log: đọc nhiều dòng đến khi gặp `.` |
| 4.2 | Rà soát log: mọi hành động ở mục 4.6 đều được ghi | Rà soát: mọi lỗi mạng ở controller có thông báo tiếng Việt, không lộ `errno` thô |
| 4.3 | Chạy server dưới `valgrind --leak-check=full` trong một phiên demo đầy đủ → 0 rò rỉ | Như Long với controller |
| 4.4 | **Kiểm thử phá hoại server**: gửi rác nhị phân, dòng rỗng, thiếu tham số, 50 kết nối cùng lúc, client ngắt giữa chừng, `kill -9` controller | **Kiểm thử phá hoại client**: tắt server khi đang kết nối, server phản hồi chậm, phản hồi sai định dạng, nhập chữ vào ô số |
| 4.5 | **Chéo**: Long kiểm thử toàn bộ chức năng phía Minh | **Chéo**: Minh kiểm thử toàn bộ chức năng phía Long |
| 4.6 | Biên dịch `-Wall -Wextra` không còn cảnh báo; thống nhất định dạng code | Như Long |
| 4.7 | Viết `docs/test_cases.md` phần server | Viết `docs/test_cases.md` phần client |

**Yêu cầu đầu ra:** tất cả ô ở bảng mục 7 được đánh dấu "Đã kiểm chứng" kèm ảnh chụp / đoạn log.

---

### PHASE 5 — Báo cáo, demo, tập vấn đáp (19/12 – 20/12)

| Việc | Long | Minh |
|---|---|---|
| Báo cáo (`docs/report.md` → PDF/Word theo yêu cầu thầy) | Kiến trúc server, giao thức, cơ chế `select()`, truyền dòng, xác thực/token, mô phỏng môi trường, log | Kiến trúc client, quét UDP/broadcast, quản lý kết nối, App ID, quản lý chuồng, lịch cho ăn, phun sương |
| Hình minh hoạ | Sơ đồ vòng `select()`, sơ đồ trạng thái quạt/đèn (hysteresis) | Sơ đồ tuần tự quét → kết nối → điều khiển; ảnh Wireshark |
| Demo | Điều khiển terminal `device_sim` (settemp, status) | Điều khiển controller |
| **Tập vấn đáp chéo** | Giải thích được phần của Minh: vì sao broadcast, vì sao đặt lại timeout, token lưu ở đâu | Giải thích được phần của Long: bộ đệm dòng, vì sao `select` không cần mutex, `SIGPIPE`, `SO_REUSEADDR` |
| `README.md` cho `App/` | Cách build, chạy `device_sim` | Cách chạy `controller`, kịch bản demo |

**Câu hỏi vấn đáp nên chuẩn bị (cả hai đều trả lời được):**
1. TCP là "luồng byte" nghĩa là gì? Nếu không có bộ đệm dòng thì lỗi gì xảy ra? Demo bằng gửi phân mảnh.
2. So sánh `select()` với `fork()` và `pthread` cho server này. Giới hạn `FD_SETSIZE`?
3. Vì sao quét dùng UDP còn điều khiển dùng TCP? Broadcast hoạt động thế nào, có qua router được không?
4. Token khác mật khẩu ở chỗ nào? Vì sao cần gắn token với App ID? Điểm yếu: truyền mật khẩu dạng rõ — cải tiến bằng TLS (OpenSSL) hoặc challenge–response.
5. Vì sao cần hai ngưỡng (Tmax/Tp1) thay vì một?
6. Điều gì xảy ra khi client ngắt kết nối đột ngột khi server đang `send`? (`SIGPIPE`, `EPIPE`)
7. Hai tiến trình cùng bind UDP 9999 được không? (`SO_REUSEADDR`/`SO_REUSEPORT`)

---

## 7. Bảng đối chiếu thang điểm

| # | Tiêu chí | Điểm | Phase | Chủ trì | Cách kiểm chứng khi demo | ☐ |
|---|---|:---:|:---:|---|---|:---:|
| 1 | Xử lý truyền dòng | 1 | 1 | Long | Gửi 1 lệnh tách thành 2 lần `send` + gửi 3 lệnh trong 1 lần → xử lý đúng; dòng quá dài bị từ chối; lệnh `LOG` nhiều dòng | ☐ |
| 2 | Vào/ra socket server | 2 | 1 | Long | 3 controller + 2 `nc` cùng lúc; ngắt 1 cái, các cái khác vẫn chạy; mô phỏng vẫn chạy khi không có client | ☐ |
| 3 | Xác thực truy cập | 1 | 2 | Long | Lệnh không token / token sai → 403; sai mật khẩu → 401; 5 lần → 423 | ☐ |
| 4 | Khởi tạo thông số | 1 | 1 | Long | Lần đầu sinh ID + mặc định; lần sau nạp lại đúng | ☐ |
| 5 | Quét thiết bị | 2 | 2 | Minh | Quét thấy thiết bị của 2 tiến trình; hiển thị loại + ID; thêm vào chuồng | ☐ |
| 6 | Kết nối thiết bị | 2 | 2 | Minh | Kết nối bằng mật khẩu + App ID; nhận token; mất kết nối được phát hiện | ☐ |
| 7 | Đổi mật khẩu | 1 | 2 | Minh (UI) + Long (server) | Đổi → app khác bị huỷ token → mật khẩu cũ không dùng được; còn hiệu lực sau khởi động lại | ☐ |
| 8 | Bật/tắt | 1 | 3 | Long + Minh | `POWER OFF` → thiết bị không tự chạy dù vượt ngưỡng | ☐ |
| 9a | Thông số máy ăn uống (W, Vw, lịch) | 1 | 3 | Minh | Đặt lịch, đến giờ thì cho ăn/uống đúng lượng | ☐ |
| 9b | Thông số quạt (Tmax, Tp1) | 1 | 3 | Long | `settemp` vượt Tmax → bật; ≤ Tp1 → tắt | ☐ |
| 9c | Thông số đèn sưởi (Tmin, Tp2) | 1 | 3 | Long | `settemp` dưới Tmin → bật; ≥ Tp2 → tắt | ☐ |
| 9d | Thông số phun sương (Hmin, Hp, Vh) | 1 | 3 | Minh | `sethum` dưới Hmin → bật, nước tiêu thụ theo Vh; ≥ Hp → tắt | ☐ |
| 10 | Điều khiển trực tiếp | 1 | 3 | Long + Minh | `FEED_NOW`, `WATER_NOW`, `RUN/STOP` quạt/đèn/phun ngoài ngưỡng/lịch | ☐ |
| 11 | Lấy thông tin 7 loại | 2 | 3 | Long (3 loại) + Minh (4 loại) | `INFO` cả 7 loại + màn hình tổng quan chuồng | ☐ |
| 12 | Ghi log | 1 | 1, 4 | Long (+ Minh UI) | Mở `logs/<id>.log`; xem log từ xa trong controller | ☐ |
| 13 | Quản lý chuồng | 1 | 1 | Minh | Thêm/sửa/xoá chuồng; thiết bị theo chuồng; lưu file | ☐ |

---

## 8. Quy trình làm việc chung

- **Git**: nhánh `main` luôn build được. Mỗi việc một nhánh `long/<việc>` hoặc `minh/<việc>`; gộp qua Pull Request, **người kia review** trước khi gộp (cách tốt nhất để hiểu code của nhau cho buổi vấn đáp).
- **Sửa `common/`** hoặc `PROTOCOL.md`: phải nhắn người kia trước, ghi vào mục "Nhật ký thay đổi giao thức" bên dưới.
- **Họp ngắn**: thứ 2, thứ 4, thứ 6 — 15 phút: hôm qua làm gì, hôm nay làm gì, đang bị chặn bởi gì.
- **Họp tích hợp**: cuối mỗi Phase (30/11, 07/12, 14/12, 18/12) — ngồi cùng nhau chạy hết danh sách kiểm thử của Phase đó.
- **Quy ước code**: tiếng Anh cho tên biến/hàm, chú thích tiếng Việt; mọi lời gọi hệ thống phải kiểm tra giá trị trả về; không dùng `gets`, `strcpy` không giới hạn, `system()`.
- **Khi bị chặn**: không ngồi chờ — dùng `nc` để giả đầu bên kia, hoặc viết hàm giả (stub) trả dữ liệu cố định.

### Nhật ký thay đổi giao thức

| Ngày | Người | Thay đổi | Người kia đã đồng ý |
|---|---|---|---|
| | | | |

---

## 9. Kịch bản demo & kiểm thử cuối kỳ (≈ 10 phút)

1. `make` → build sạch, không cảnh báo.
2. Terminal 1: `./bin/device_sim -c config/coop_A.conf` · Terminal 2: `./bin/device_sim -c config/coop_B.conf -p 7001`.
3. Terminal 3: `./bin/controller` → hiện App ID. Tạo chuồng "Chuong_A", "Chuong_B".
4. Quét → 14 thiết bị → thêm vào 2 chuồng. *(Tiêu chí 5, 13)*
5. Kết nối quạt với mật khẩu sai → báo lỗi; đúng → có token. *(3, 6)*
6. Kết nối hết thiết bị Chuong_A → màn hình tổng quan. *(11)*
7. Đặt quạt Tmax=30, Tp1=27 → Terminal 1 gõ `settemp 33` → quạt tự bật → nhiệt độ giảm → tự tắt. Tương tự đèn sưởi, phun sương. *(9)*
8. Thêm mốc cho ăn sau 2 phút mô phỏng → chờ → máy ăn chạy. Bấm "cho ăn ngay". Thu trứng. *(9, 10)*
9. `POWER OFF` quạt → `settemp 40` → quạt không chạy. *(8)*
10. Terminal 4: controller thứ hai (App ID khác) kết nối cùng quạt → controller 1 đổi mật khẩu → controller 2 bị yêu cầu kết nối lại. *(7)*
11. Xem log của quạt trong controller; mở file `logs/FAN-xxxx.log`. *(12)*
12. Demo truyền dòng: `nc` gửi lệnh phân mảnh / nhiều lệnh một lần. *(1, 2)*
13. Tắt & mở lại `device_sim` → ID, mật khẩu mới, thông số vẫn giữ. *(4)*

---

## 10. Rủi ro & phương án dự phòng

| Rủi ro | Dấu hiệu | Phương án |
|---|---|---|
| Broadcast UDP không tới được trên WSL / máy ảo | Quét không thấy gì dù `device_sim` đang chạy | Cho người dùng nhập IP cụ thể (`127.0.0.1` hoặc IP máy) — đã có trong M2.2. Dự phòng 2: mỗi `device_sim` dùng cổng UDP khác (9999, 10000, …) và controller quét một dải cổng |
| Nhiều `device_sim` không cùng bind được cổng 9999 | `bind: Address already in use` | Đặt `SO_REUSEADDR` **và** `SO_REUSEPORT` trước `bind`; nếu vẫn lỗi → phương án dải cổng ở trên |
| Một người chậm tiến độ | Mốc cuối Phase trễ > 2 ngày | Người kia chuyển sang làm phần kiểm thử/báo cáo của người chậm; cắt phần mở rộng (khoá 423, token hết hạn, màn hình tổng quan) trước — **không** cắt tiêu chí chấm điểm |
| Giao thức phải đổi giữa chừng | Phát hiện thiếu trường | Chỉ **thêm** trường `key=value` mới (bên nhận bỏ qua key lạ) → không phá code cũ; ghi nhật ký thay đổi |
| `select()` khó gỡ lỗi | Server treo / bỏ sót client | Log DEBUG mỗi vòng (fd sẵn sàng); kiểm tra `max_fd` và việc xoá client; phương án cuối: chuyển sang 1 luồng/client + mutex cho mảng thiết bị (`pthread_server.c`) |
| Lỗi bộ nhớ | Crash ngẫu nhiên | Build `-fsanitize=address` khi phát triển, `valgrind` ở Phase 4 |
| Trùng thời gian thi các môn khác | — | Phase 5 có 2 ngày đệm; Phase 2 là ưu tiên số 1 vì chiếm 8/20 điểm |

---

### Tóm tắt một dòng cho mỗi người

- **Long**: "Tôi làm cho thiết bị **sống**: nhận nhiều kết nối, hiểu đúng từng dòng lệnh, chỉ nghe lời người có token, tự điều chỉnh khí hậu chuồng và ghi lại mọi thứ."
- **Minh**: "Tôi làm cho người dùng **tìm thấy và điều khiển** được thiết bị: quét mạng, tổ chức theo chuồng, kết nối an toàn, và lo toàn bộ chuyện ăn, uống, phun sương, thu trứng."
