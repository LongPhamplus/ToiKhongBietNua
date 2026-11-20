# App: Mô phỏng điều khiển hệ thống nuôi gà lấy trứng

> Kế hoạch tổng thể nằm ở [`KE_HOACH.md`](KE_HOACH.md). File này là **bản đồ thư mục**.
> Mỗi file `.c/.h` hiện chỉ chứa **khối chú thích hướng dẫn**, chưa có code.
> Khi bắt đầu code: giữ lại khối chú thích đầu file (rút gọn còn vài dòng). Làm xong mục nào thì đổi `[ ]` thành `[x]`.

## Cây thư mục

```
App/
├── KE_HOACH.md          Kế hoạch, phân công, phase (đọc trước tiên)
├── README.md            File này: bản đồ thư mục, cách build/chạy (cập nhật ở Phase 5)
├── PROTOCOL.md          Đặc tả giao thức CHÍNH THỨC (chốt ở Phase 0)        [Long + Minh]
├── Makefile             Build 2 chương trình vào bin/                         [Long]
├── .gitignore           Bỏ qua bin/, build/, data/, logs/                     [Minh]
│
├── common/              Dùng chung cho cả 2 chương trình. Sửa phải báo người kia
│   ├── protocol.h       Hằng số giao thức                                     [Long + Minh]
│   ├── net_io.h/.c      send_all + bộ đệm dòng (XỬ LÝ TRUYỀN DÒNG)            [Long]
│   ├── kv.h/.c          Tách từ, đọc/ghi key=value                            [Minh]
│   └── util.h/.c        Sinh chuỗi hex ngẫu nhiên, thời gian, trim, mkdir     [Long]
│
├── device/              Chương trình device_sim (SERVER)
│   ├── main.c           Đọc tham số, khởi tạo, vòng select()                  [Long]
│   ├── server.h/.c      Quản lý client TCP, nhận dòng, gọi dispatcher         [Long]
│   ├── handlers.h/.c    Bảng điều phối lệnh TCP                               [Long]
│   ├── auth.h/.c        Mật khẩu, token, kiểm tra quyền, CHPASS               [Long]
│   ├── device.h/.c      struct thiết bị, nạp/lưu cấu hình, mặc định, ID       [Long]
│   ├── dlog.h/.c        Log từng thiết bị                                     [Long]
│   ├── env.h/.c         Mô hình môi trường + đồng hồ mô phỏng                 [Long]
│   ├── dev_climate.h/.c ENV_SENSOR, FAN, HEATER                               [Long]
│   ├── discovery.h/.c   UDP 9999 trả lời SCAN                                 [Minh]
│   ├── dev_feed.h/.c    FEEDER, WATERER (lịch), EGG_COUNTER                   [Minh]
│   └── dev_mister.h/.c  MISTER                                                [Minh]
│
├── controller/          Chương trình controller (CLIENT)
│   ├── main.c           App ID, vòng menu chính                               [Minh]
│   ├── ui.h/.c          Nhập liệu an toàn, menu, in bảng                      [Minh]
│   ├── coop.h/.c        Quản lý chuồng nuôi + lưu file                        [Minh]
│   ├── scanner.h/.c     Gửi SCAN, gom phản hồi                                [Minh]
│   ├── conn.h/.c        Kết nối TCP, token, gửi lệnh/nhận phản hồi            [Minh]
│   ├── ui_device.h/.c   Màn hình chung: bật/tắt, mode, đổi mk, log, tổng quan [Minh]
│   ├── ui_climate.h/.c  Màn hình cảm biến, quạt, đèn                          [Long]
│   └── ui_feed.h/.c     Màn hình ăn, uống, phun sương, đếm trứng              [Minh]
│
├── config/              Cấu hình mẫu cho device_sim
│   ├── coop_A.conf      7 thiết bị, TCP 7000                                  [Long]
│   └── coop_B.conf      7 thiết bị, TCP 7001                                  [Long]
│
├── data/                (tự sinh khi chạy) app.conf, coops.txt, devices_*.conf
├── logs/                (tự sinh khi chạy) <device_id>.log
├── tests/
│   └── README.md        Kịch bản kiểm thử bằng nc / thủ công                  [Long + Minh]
└── docs/
    ├── report.md        Khung báo cáo                                         [Long + Minh]
    └── test_cases.md    Bảng kiểm thử + kết quả                               [Long + Minh]
```

> So với KE_HOACH.md mục 2.3 có thêm `controller/ui_device.h/.c` (Minh), chứa các màn hình dùng chung cho mọi loại thiết bị. File này giúp `ui_climate` (Long) và `ui_feed` (Minh) chỉ lo phần riêng của từng loại.

## Thứ tự làm (theo phase)

| Phase | Long làm các file | Minh làm các file |
|---|---|---|
| 0 (20–23/11) | `PROTOCOL.md`, `common/protocol.h`, `Makefile` | `PROTOCOL.md`, `common/protocol.h`, `.gitignore`, phác thảo menu |
| 1 (24–30/11) | `net_io`, `util`, `device/main.c`, `server`, `handlers` (HELLO/BYE/INFO), `device`, `dlog`, `config/*.conf` | `kv`, `controller/main.c`, `ui`, `coop`, `conn` (khung) |
| 2 (01–07/12) | `auth`, gắn discovery vào `device/main.c` | `discovery`, `scanner`, `conn` (CONNECT), `ui_device` (đổi mật khẩu) |
| 3 (08–14/12) | `env`, `dev_climate`, `handlers` (POWER/MODE/SET/ACT), `ui_climate` | `dev_feed`, `dev_mister`, `ui_feed`, `ui_device` (bật/tắt, mode, tổng quan) |
| 4 (15–18/12) | `handlers` (LOG), `tests/`, `docs/test_cases.md` | `ui_device` (xem log), `tests/`, `docs/test_cases.md` |
| 5 (19–20/12) | `docs/report.md`, `README.md` | `docs/report.md`, `README.md` |

## Sơ đồ phụ thuộc giữa các module

```
common/protocol.h ◄── mọi file
common/net_io  ◄── device/server, controller/conn
common/kv      ◄── device/handlers, device/device, controller/conn, controller/coop, ui_*
common/util    ◄── device/auth (token), device/device (ID), controller/main (App ID), dlog

device/main ─► server ─► handlers ─► auth ─► device
                              │                 ▲
                              ├─► dev_climate ──┤──► env, dlog
                              ├─► dev_feed ─────┤──► env (đồng hồ), dlog
                              └─► dev_mister ───┘──► env (độ ẩm), dlog
device/main ─► discovery ─► device (danh sách), dlog

controller/main ─► ui, coop, scanner, conn
                   ui_device ─► conn, ui ─► gọi ui_climate / ui_feed theo loại thiết bị
```

## Quy ước chung cho mọi file

- Đầu mỗi file giữ khối chú thích: tên file, người phụ trách, mục đích (tối đa 5 dòng).
- File `.h` phải có header guard và chỉ chứa khai báo (struct, prototype, hằng số), **không** định nghĩa biến toàn cục.
- Biến toàn cục chỉ được khai báo `static` trong file `.c` và truy cập qua hàm.
- Mọi lời gọi hệ thống (socket, bind, recv, fopen, ...) phải kiểm tra giá trị trả về.
- Không dùng: `gets`, `strcpy`/`strcat`/`sprintf` không giới hạn độ dài, `system()`.
- Biên dịch với `-Wall -Wextra` không được còn cảnh báo nào.
- Tên hàm/biến tiếng Anh, chú thích tiếng Việt.
