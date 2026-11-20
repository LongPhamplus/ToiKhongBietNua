# TUẦN 1: KẾ HOẠCH CODE (T6 20/11 → T2 30/11/2026)

> Tài liệu này chỉ nói về **việc code**: cần code phần nào, **phần đó để làm gì**, làm đến đâu thì dừng, và **output** phải thấy được.
> Code thật nằm trong thư mục `RealApp/` của từng người. Các file hướng dẫn trong repo này chỉ để đọc.
> Hướng dẫn chi tiết từng hàm vẫn nằm trong khối chú thích của từng file tương ứng (`common/*.c`, `device/*.c`, `controller/*.c`).

---

## 0. Mục tiêu tuần và thứ tự phụ thuộc

### 0.1. Hết tuần phải có

> **`device_sim`** nạp 7 thiết bị từ file cấu hình, phục vụ **nhiều client TCP cùng lúc**, tách lệnh **đúng theo dòng** dù dữ liệu đến bị cắt vụn hay bị dồn, và trả lời `HELLO` / `INFO` / `BYE`.
> **`controller`** có App ID cố định, quản lý được chuồng nuôi (có lưu file), mở kết nối TCP và gửi/nhận lệnh có timeout.
> Hai chương trình **nói chuyện được với nhau**.

### 0.2. Tuần này CHƯA làm (để tuần 2, 3)

| Chưa làm | Lý do để sau |
|---|---|
| Token, mật khẩu, `CONNECT` | Phase 2. Cần server ổn định trước rồi mới thêm lớp xác thực |
| Quét UDP (`SCAN`) | Phase 2. Cắm vào vòng `select()`, nên vòng lặp phải chạy tốt trước |
| Mô phỏng nhiệt độ/độ ẩm, quạt, đèn, lịch cho ăn | Phase 3 |
| `SET`, `POWER`, `MODE`, `ACT`, `LOG` | Phase 3, 4 |

### 0.3. Sơ đồ phụ thuộc (ai cần gì của ai)

```
protocol.h ─┬─► mọi file
device.h ───┤
            │
util.c (Long) ─────► device.c (Long), controller/main.c (Minh)
net_io.c (Long) ───► server.c (Long), conn.c (Minh)
kv.c (Minh) ───────► device.c (Long), handlers.c (Long), coop.c (Minh)
ui.c (Minh) ───────► main.c, coop.c (Minh)
device.c + dlog.c ─► main.c ─► server.c ─► handlers.c   (Long)
```

**Hai lần bàn giao quan trọng của tuần:**
- 🔁 **H2**: Long đưa `util` + `net_io` cho Minh, **trước T3 24/11**
- 🔁 **H3**: Minh đưa `kv` cho Long, **trước T3 24/11**

### 0.4. Lịch tổng

| Ngày | Long | Minh |
|---|---|---|
| T6 20/11 | **Bước 0**: `protocol.h`, `device.h`, `Makefile` | Duyệt `protocol.h`, `device.h` |
| T7 21/11 | L1 `util.c` | M1 `kv.c` (phần 1) |
| CN 22/11 | L2 `net_io.c` (phần 1) | M1 `kv.c` (phần 2) + test |
| T2 23/11 | L2 `net_io.c` (phần 2) + test → 🔁 H2 | M2 `ui.c` (phần 1) → 🔁 H3 |
| T3 24/11 | L3 `device.c` (mặc định, ID) | M2 `ui.c` (phần 2) |
| T4 25/11 | L3 `device.c` (nạp/lưu) + `coop_A.conf` | M3 `controller/main.c` (App ID, menu) |
| T5 26/11 | L4 `dlog.c` | M4 `coop.c` (dữ liệu) |
| T6 27/11 | L5 `device/main.c` (khởi tạo, listener) | M4 `coop.c` (lưu file) |
| T7 28/11 | L5 `device/main.c` (vòng `select`) | M4 `coop.c` (menu) |
| CN 29/11 | L6 `server.c` + `handlers.c` | M5 `conn.c` |
| T2 30/11 | ★ **Tích hợp** | ★ **Tích hợp** |

---

## BƯỚC 0: Phần dùng chung (T6 20/11, Long viết, Minh duyệt)

Đây là **hợp đồng bằng code**. Có nó trước thì hai người code song song mà không lệch nhau.

### 0-A. `common/protocol.h`

**Mục đích:** nơi duy nhất chứa mọi con số và chuỗi của giao thức. Nếu cổng `7000` hay mã `403` được viết cứng ở 5 chỗ, sửa 1 chỗ quên 4 chỗ là hai chương trình không hiểu nhau. Đặt hết ở đây thì cả `device_sim` lẫn `controller` cùng include, luôn khớp.

**Cần code:**

| Nhóm | Nội dung | Dùng để |
|---|---|---|
| Cổng | `DEFAULT_UDP_PORT 9999`, `DEFAULT_TCP_PORT 7000` | Server bind, client kết nối |
| Giới hạn độ dài | `MAX_LINE 1024`, `MAX_ID`, `MAX_NAME`, `MAX_PASS`, `MIN_PASS`, `TOKEN_HEX_LEN 32`, `APP_ID_HEX_LEN 8` | Khai báo mảng `char`, chống tràn bộ đệm |
| Giới hạn số lượng | `MAX_CLIENTS 64`, `MAX_DEVICES 32`, `MAX_TOKENS_PER_DEV 8`, `MAX_SCHED 8`, `MAX_ARGS 16` | Khai báo mảng tĩnh |
| Thời gian | `TICK_MS 1000`, `REQ_TIMEOUT_MS 3000`, `SCAN_WAIT_MS 2000`, `TOKEN_TTL_SEC`, `LOCK_FAILS`, `LOCK_SEC` | Timeout của `select`, khoá đăng nhập |
| Tên lệnh | `CMD_HELLO "HELLO"`, `CMD_INFO`, `CMD_CONNECT`, … | So sánh chuỗi lệnh ở cả 2 phía |
| Mã phản hồi | `RC_OK 200`, `RC_BAD_REQUEST 400`, … `RC_SERVER_ERROR 500` | Server trả, client hiểu |
| Kiểu thiết bị | `enum dev_type { DEV_ENV_SENSOR, DEV_EGG_COUNTER, DEV_FEEDER, DEV_WATERER, DEV_FAN, DEV_HEATER, DEV_MISTER, DEV_INVALID }` | Phân nhánh xử lý theo loại |
| Trạng thái | `enum power { PWR_OFF, PWR_ON }`, `enum mode { MODE_AUTO, MODE_MANUAL }` | Trường của thiết bị |

**Không đặt ở đây:** hàm, biến toàn cục.
**Output:** một file `.c` chỉ có `#include "protocol.h"` biên dịch không lỗi. Chạy `grep -rn "7000\|1024" --include=*.c` không ra kết quả nào ngoài comment.

### 0-B. `device/device.h`

**Mục đích:** mô tả **một thiết bị trông như thế nào trong bộ nhớ**. Mọi file phía server (`device.c`, `handlers.c`, `auth.c`, và các file `dev_feed.c`, `dev_mister.c` của Minh ở tuần 3) đều đọc/ghi các struct này. Đặt tên trường sai rồi đổi về sau sẽ phải sửa rất nhiều file, nên **phải chốt ngay từ tuần 1**.

**Cần code:**

| Struct | Các trường | Ghi chú |
|---|---|---|
| `env_params` | `interval`, `temp`, `humidity`, `last_read` | Cảm biến môi trường |
| `egg_params` | `capacity`, `rate`, `count` (double), `total_collected`, `full` | `count` dùng double để cộng dồn phần lẻ |
| `feed_params` | `amount`, `stock`, `times[MAX_SCHED]`, `n_times`, `last_run_day[MAX_SCHED]`, `today_total`, `last_run`, `running` | **Dùng chung cho FEEDER và WATERER** (W/Vw và stock/tank) |
| `fan_params` | `Tmax`, `Tp1`, `running`, `run_minutes` | |
| `heater_params` | `Tmin`, `Tp2`, `running`, `run_minutes` | |
| `mister_params` | `Hmin`, `Hp`, `Vh`, `running`, `water_used` | |
| `token_entry` | `token[]`, `app_id[]`, `created`, `last_used` | Tuần 1 chỉ khai báo, tuần 2 mới dùng |
| `device` | `id`, `type`, `name`, `password`, `power`, `mode`, `tokens[]`, `n_tokens`, `fail_count`, `lock_until`, `FILE *log`, `union { … } p` | `union` vì mỗi thiết bị chỉ thuộc 1 loại, nên tiết kiệm bộ nhớ và gọn |

Thêm một struct thông tin của tiến trình: `tcp_port`, `udp_port`, `speed`, `conf_name` (vd. `"coop_A"`).

**Khai báo hàm** (cài đặt ở L3): `devices_load`, `devices_save`, `device_find`, `device_count`, `device_at`, `device_find_first`, `device_set_defaults`.

**Output:** Minh đọc và xác nhận "đủ trường cho máy ăn/uống, phun sương, đếm trứng".

### 0-C. `Makefile`

**Mục đích:** gõ `make` một lần là build cả 2 chương trình. Chỉ dịch lại file đã sửa. Sửa `.h` thì tự dịch lại file phụ thuộc. Không có nó thì mỗi lần build phải gõ lệnh `gcc` dài với hàng chục file.

**Cần code:**

| Đích | Làm gì |
|---|---|
| `all` (mặc định) | Build `bin/device_sim` (`common/*.c` + `device/*.c`) và `bin/controller` (`common/*.c` + `controller/*.c`) |
| `device`, `controller` | Build riêng từng chương trình |
| `test` | Build và chạy `tests/test_*.c` |
| `clean` | Xoá `bin/`, `build/` |
| `reset-data` | Xoá `data/*`, `logs/*` để thử lại "lần chạy đầu tiên" |
| `sanitize` | Build với `-fsanitize=address` để bắt lỗi bộ nhớ khi phát triển |

Cờ biên dịch: `-std=gnu11 -Wall -Wextra -g -Icommon -Idevice -MMD -MP`.
**Output:** với 2 `main.c` tạm chỉ in một dòng, `make` tạo ra 2 file trong `bin/` và không có cảnh báo.

---

## PHẦN CỦA LONG: chương trình `device_sim` (server)

### L1. `common/util.c`: tiện ích chung (T7 21/11)

**Mục đích:** gom các thao tác nhỏ mà **cả hai chương trình** đều cần vào một chỗ, để không ai phải viết lại và không ai viết sai mỗi người một kiểu.

| Hàm | Mục đích | Ai dùng |
|---|---|---|
| `random_hex(out, nbytes)` | Sinh chuỗi hex ngẫu nhiên từ `/dev/urandom`. `/dev/urandom` là nguồn ngẫu nhiên an toàn mật mã, khác `rand()` vốn đoán được nếu biết seed | Sinh ID thiết bị (tuần 1), App ID (Minh, tuần 1), token (tuần 2) |
| `now_ms()` | Thời gian tính bằng mili giây (`CLOCK_MONOTONIC`) | Tính timeout `select`, nhịp tick 1 giây |
| `now_str(buf, sz)` | Chuỗi `"YYYY-MM-DD HH:MM:SS"` | Ghi log |
| `trim(s)` | Bỏ dấu cách, `\r`, `\n` ở hai đầu | Xử lý dòng đọc từ file/stdin/mạng |
| `safe_copy(dst, sz, src)` | Chép chuỗi có giới hạn, luôn kết thúc bằng `\0` | Thay `strcpy` (cấm dùng) |
| `ensure_dir(path)` | Tạo thư mục nếu chưa có | Tạo `data/`, `logs/` khi khởi động |
| `die(msg)` | `perror` + `exit(1)` | Lỗi nghiêm trọng lúc khởi động (bind thất bại, …) |

**Làm đến đâu:** chỉ các hàm trên.
**Output:** `tests/test_util.c` chạy bằng `make test`:
```
[PASS] random_hex: 1000 giá trị không trùng, đúng 2*n ký tự, chỉ gồm 0-9a-f
[PASS] trim("  abc \r\n") == "abc"
[PASS] safe_copy cắt đúng khi src dài hơn dst
[PASS] ensure_dir tạo được thư mục, gọi lại lần 2 không lỗi
```

---

### L2. `common/net_io.c`: xử lý truyền dòng (CN 22/11 → T2 23/11). TIÊU CHÍ 1 (1 điểm)

**Mục đích:** đây là phần **quan trọng nhất về mặt lý thuyết** của tuần.
TCP là **luồng byte**, không giữ ranh giới thông điệp:
- Client gửi `"INFO FAN-1\n"` một lần, nhưng server có thể nhận thành `"INF"` rồi `"O FAN-1\n"` (bị **cắt vụn**).
- Client gửi 3 lệnh liên tiếp, server có thể nhận cả 3 trong **một** lần `recv` (bị **dồn**).

Code mẫu trong IT4062 (`tcp_server.c`, `select_server.c`) chỉ gọi `recv` một lần rồi coi như đã nhận đủ một thông điệp. Cách đó **sai** khi mạng chậm hoặc tải cao. Module này giải quyết bằng một **bộ đệm dòng**: tích luỹ byte, chỉ trả ra khi đã có một dòng hoàn chỉnh (kết thúc bằng `\n`), phần dư giữ lại cho lần sau.

`send()` cũng có thể gửi **thiếu** byte, nên cần `send_all` lặp đến khi gửi hết.

**Cần code:**

| Thành phần | Mục đích |
|---|---|
| `struct linebuf { char data[2*MAX_LINE]; size_t len; int discarding; }` | Mỗi kết nối có một bộ đệm riêng. `discarding` đánh dấu đang bỏ qua một dòng quá dài |
| `linebuf_init(lb)` | Đặt bộ đệm rỗng |
| `linebuf_fill(lb, fd)` | Gọi `recv` **một lần**, nối byte vào cuối bộ đệm. Trả về `>0` / `0` (bên kia đóng) / `-1` (lỗi). Server gọi hàm này **sau khi `select` báo fd sẵn sàng**, nên không bị chặn |
| `linebuf_next(lb, out, sz)` | Lấy **một** dòng hoàn chỉnh ra `out` (đã bỏ `\n`, `\r`). Trả `1` (có dòng) / `0` (chưa đủ) / `-1` (dòng quá dài, đã bỏ). Người gọi phải **lặp** đến khi nhận `0`, vì một lần `recv` có thể chứa nhiều dòng. Dùng `memchr` tìm `\n` (dữ liệu có thể chứa byte 0) và `memmove` dồn phần dư về đầu |
| `read_line_blocking(fd, lb, out, sz, timeout_ms)` | Dành cho **client**: chờ đến khi có đủ một dòng hoặc hết giờ. Bên trong lặp `linebuf_next`, chưa có dòng thì `select` với thời gian còn lại rồi `linebuf_fill`. **Đặt lại `struct timeval` mỗi vòng**. Trả `1` / `0` / `-1` / `-2` (hết giờ) |
| `send_all(fd, buf, len)` | Lặp `send` đến khi gửi hết. `EINTR` thì thử lại. Dùng cờ `MSG_NOSIGNAL` |
| `send_line(fd, s)` | Gửi chuỗi kèm `\n` |
| `send_linef(fd, fmt, ...)` | Như `printf` nhưng gửi lên socket (`vsnprintf` giới hạn `MAX_LINE`) |

**Làm đến đâu:** đủ cho cả server (`linebuf_fill` + `linebuf_next`) và client (`read_line_blocking`).
**Output:** `tests/test_netio.c` dùng `socketpair(AF_UNIX, SOCK_STREAM, …)` để giả hai đầu kết nối trong một chương trình:
```
[PASS] "HEL" + "LO\n" (2 lần gửi)        -> 1 dòng "HELLO"
[PASS] "A\nB\nC\n" (1 lần gửi)            -> 3 dòng A, B, C
[PASS] "A\r\n"                            -> "A"
[PASS] 5000 byte không có \n + "HELLO\n"  -> 1 lần -1, sau đó "HELLO"
[PASS] gửi "AB" rồi đóng                  -> linebuf_fill trả 0
[PASS] read_line_blocking không có dữ liệu, timeout 500ms -> -2 sau ~500ms
[PASS] send_all 100 KB                    -> bên kia nhận đủ 100 KB
```
🔁 **Bàn giao H2**: gộp vào `main`, nhắn Minh.

---

### L3. `device/device.c` + `config/coop_A.conf`: khởi tạo thông số (T3 24/11 → T4 25/11). TIÊU CHÍ 4 (1 điểm)

**Mục đích:** đề bài yêu cầu "mỗi thiết bị có một ID duy nhất" và chấm điểm "khởi tạo các thông số cho thiết bị". Module này:
1. **Tạo** danh sách thiết bị lúc khởi động từ file cấu hình con người viết được.
2. **Điền giá trị mặc định** hợp lý cho thông số bị thiếu (Tmax=32, W=5kg, …), để thiết bị chạy được ngay.
3. **Sinh ID duy nhất** một lần, rồi **lưu lại**, để lần sau ID không đổi. Controller đã lưu ID vào chuồng, nên ID mà đổi thì mất liên kết.
4. **Lưu mọi thay đổi** (tuần 2–3: đổi mật khẩu, SET thông số) để khởi động lại không mất.
5. **Kiểm tra ràng buộc** (Tp1 < Tmax, …). Cấu hình sai thì không chạy, tránh thiết bị hoạt động vô lý.

**Cần code:**

| Hàm / phần | Mục đích |
|---|---|
| Mảng `static struct device devices[MAX_DEVICES]` + biến đếm | Kho thiết bị của tiến trình, chỉ truy cập qua hàm |
| `device_set_defaults(dev, type)` | Điền mặc định theo bảng 4.1 của KE_HOACH: power=ON, mode=AUTO, thông số theo loại |
| `gen_unique_id(type, out)` | Tiền tố theo loại (`FAN-`) + `random_hex(2)` viết hoa, lặp đến khi không trùng |
| `parse_device_line(line, dev)` | Đọc dòng `device type=FAN name=Quat_1 password=123456 Tmax=32` bằng `kv.c`. Key nào có thì ghi đè lên mặc định |
| `validate_device(dev, err, sz)` | Kiểm tra khoảng giá trị và ràng buộc. Trả thông báo lỗi cụ thể |
| `devices_load(config_path)` | Có `data/devices_<tên>.conf` thì nạp từ đó. Chưa có thì đọc config gốc, điền mặc định, sinh ID, rồi `devices_save()` ngay. Lỗi thì in `file:dòng: lý do` rồi trả `-1` |
| `devices_save()` | Ghi toàn bộ ra **file tạm** rồi `rename()` đè file thật. Lý do: bị tắt giữa chừng thì file cũ vẫn nguyên, không bị hỏng một nửa |
| `device_find(id)`, `device_count()`, `device_at(i)`, `device_find_first(type)` | Truy cập cho các module khác |
| `config/coop_A.conf` | 3 dòng thông số tiến trình + 7 dòng thiết bị. **Cố ý bỏ trống vài thông số** để chứng minh có giá trị mặc định khi demo |

**Làm đến đâu:** chưa lưu token, chưa có trạng thái chạy tức thời (`running`, `temp`). Các giá trị này không cần lưu.
**Output:**
```
$ make reset-data && ./bin/device_sim -c config/coop_A.conf      (lần 1)
→ sinh data/devices_coop_A.conf:
  device id=ENV-3F10 type=ENV_SENSOR name=Cam_bien_1 password=123456 power=ON mode=AUTO interval=1
  device id=FAN-8A21 type=FAN name=Quat_1 password=123456 power=ON mode=AUTO Tmax=32.0 Tp1=28.0
  ... (7 dòng)

$ ./bin/device_sim -c config/coop_A.conf                         (lần 2)
→ ID in ra giống hệt lần 1

Sửa coop_A.conf: Tp1=40
→ config/coop_A.conf:9: Tp1=40.0 phai nho hon Tmax=32.0   (thoát với mã 1)
```

---

### L4. `device/dlog.c`: nền tảng ghi log (T5 26/11). TIÊU CHÍ 12 (1 điểm, phần nền)

**Mục đích:** đề bài chấm "ghi log hoạt động **trên mỗi thiết bị**", nghĩa là mỗi thiết bị có nhật ký riêng: ai kết nối, ai đổi gì, thiết bị tự bật/tắt lúc nào. Log còn là **công cụ debug** và **bằng chứng khi demo**. Làm nền ngay tuần 1 để từ đây mọi module mới đều ghi log luôn, không phải chèn thêm về sau.

**Cần code:**

| Hàm | Mục đích |
|---|---|
| `dlog_open_all()` | Mở `logs/<id>.log` chế độ `"a"` (ghi nối) cho từng thiết bị, lưu `FILE*` vào `dev->log` |
| `dlog(dev, level, app_id, fmt, ...)` | Ghi 1 dòng `thời gian | sim hh:mm | MỨC | app=xxx hoặc auto | nội dung`. Dùng `va_list` + `vfprintf`. **`fflush` ngay** để Ctrl+C không mất log. Mức ≥ INFO thì in thêm ra stdout để terminal server "kể chuyện" khi demo |
| `dlog_close_all()` | Đóng file khi thoát |
| `dlog_set_min_level(level)` | Ẩn DEBUG khi demo |

**Làm đến đâu:** giờ mô phỏng tạm ghi `--:--` (tuần 3 mới có `env.c`). Chưa làm `dlog_tail` (tuần 4).
**Output:** khởi động xong có 7 file trong `logs/`, mỗi file có dòng
```
2026-11-26 20:10:01 | sim --:-- | INFO  | auto | device_sim started (coop_A, tcp 7000)
```

---

### L5. `device/main.c`: vòng `select()` (T6 27/11 → T7 28/11). TIÊU CHÍ 2 (2 điểm, cùng L6)

**Mục đích:** đây là **trái tim của server** và là phần được 2 điểm "cơ chế vào/ra socket trên server". Server phải **cùng lúc**:
- chấp nhận kết nối mới,
- nhận lệnh từ nhiều client,
- (tuần 2) nhận gói quét UDP,
- nhận lệnh quản trị từ bàn phím để demo,
- **và** chạy mô phỏng đều đặn mỗi giây dù không ai gửi gì.

`select()` cho phép **một luồng duy nhất** chờ trên tất cả nguồn đó. Timeout của nó đóng vai bộ đếm thời gian cho mô phỏng. Vì chỉ có một luồng nên không có hai đoạn code cùng sửa một thiết bị, **không cần mutex**, không có race condition. Đây là lý do chọn `select` thay vì `fork`/`pthread` (ghi vào báo cáo).

**Cần code:**

| Phần | Mục đích |
|---|---|
| Đọc tham số `-c <config> -p <port> -s <speed> -h` | Chạy nhiều tiến trình (coop_A cổng 7000, coop_B cổng 7001) mà không phải sửa code |
| `signal(SIGPIPE, SIG_IGN)` | Mặc định, ghi vào socket mà client đã đóng sẽ **giết tiến trình**. Bỏ qua tín hiệu này thì `send` chỉ trả lỗi |
| Bắt `SIGINT` (Ctrl+C) và đặt cờ thoát | Thoát sạch: lưu dữ liệu, đóng log, đóng socket |
| `ensure_dir`, `devices_load`, `dlog_open_all` | Khởi tạo theo đúng thứ tự |
| Tạo listener: `socket`, `setsockopt(SO_REUSEADDR)`, `bind`, `listen` | `SO_REUSEADDR` để tắt rồi bật lại ngay không bị "Address already in use" (cổng còn ở trạng thái TIME_WAIT) |
| In banner (cổng + bảng 7 thiết bị) | Để người demo biết ID mà dùng |
| **Vòng lặp chính** | Mỗi vòng: `FD_ZERO`, `FD_SET` listener + stdin + mọi client (`server_fill_fdset`), **tính lại `max_fd`**, tính thời gian đến tick kế tiếp và **đặt lại `timeval`**, gọi `select`. `EINTR` thì quay vòng. Fd nào sẵn sàng thì gọi hàm tương ứng. Đến giờ tick thì gọi `sim_tick()` |
| `sim_tick()` | Tuần 1 chỉ tăng bộ đếm tick. Tuần 3 sẽ gọi `env_tick`, `climate_tick`, `feed_tick`, `mister_tick` (chỗ cắm hàm cho Minh) |
| `admin_command()` (stdin) | `status` in số tick + bảng thiết bị, `clients` in danh sách kết nối, `quit` thoát. Tuần 3 thêm `settemp`, `sethum`, `speed` |
| Dọn dẹp khi thoát | `devices_save`, `server_shutdown`, `dlog_close_all`, đóng listener |

**Lỗi trong code mẫu phải tránh** (`IT4062/3-Chap3/L5-IOMultiPlexing/select_server.c`): không xoá client đã ngắt khỏi danh sách, không tính lại `max_fd`, `timeval` không đặt lại mỗi vòng.

**Làm đến đâu:** chưa có UDP (tuần 2), chưa có `env` (tuần 3).
**Output:**
```
$ ./bin/device_sim -c config/coop_A.conf
device_sim/1.0  config=coop_A  TCP=7000  devices=7
  ID        TYPE         NAME
  ENV-3F10  ENV_SENSOR   Cam_bien_1
  EGG-11A0  EGG_COUNTER  Dem_trung_1
  ...
> status
tick=42  clients=0
  (bảng 7 thiết bị: ID, loại, power, mode)
> quit
Saving data... bye.
```
Ctrl+C rồi chạy lại ngay thì không có lỗi bind.

---

### L6. `device/server.c` + `device/handlers.c`: phục vụ client, điều phối lệnh (CN 29/11). TIÊU CHÍ 2 (tiếp)

**Mục đích:**
- `server.c` lo phần **kết nối**: ai đang kết nối, bộ đệm của từng người, đọc byte, cắt dòng, gửi phản hồi, dọn dẹp khi ngắt. Mỗi client có `linebuf` **riêng** vì hai client có thể cùng gửi dở một dòng. Dùng chung một bộ đệm thì byte của hai người trộn vào nhau.
- `handlers.c` lo phần **ý nghĩa**: dòng `"INFO FAN-8A21"` nghĩa là gì và trả lời gì. Dùng **bảng điều phối** (mảng `{tên lệnh, con trỏ hàm}`, giống `IT4062/1-Chap1/lab05_function_pointer.c`), nên thêm lệnh mới chỉ là thêm một dòng vào bảng, không phải viết thêm chuỗi `if/else` dài.

Tách hai file để `server.c` không cần biết gì về thiết bị, còn `handlers.c` không cần biết gì về socket.

**Cần code: `server.c`**

| Hàm | Mục đích |
|---|---|
| `struct client { fd; char ip[]; port; struct linebuf lb; time_t since; int n_cmds; int close_after; }` | Trạng thái một kết nối |
| `server_init()` | Đặt mảng client rỗng |
| `server_accept(listen_fd)` | `accept`, lấy IP:port bằng `inet_ntop`, thêm vào mảng. Đầy thì gửi `500 SERVER_FULL` rồi đóng. In `[+] client 127.0.0.1:51234 (fd=5)` |
| `server_fill_fdset(&set, &max_fd)` | `main.c` gọi để thêm fd client vào `select` |
| `server_handle_readable(&set)` | Với mỗi client sẵn sàng: `linebuf_fill`. Nhận `0`/lỗi thì đóng. Ngược lại **lặp** `linebuf_next`: dòng quá dài thì gửi `400 BAD_REQUEST LINE_TOO_LONG`, dòng rỗng thì bỏ qua, dòng thường thì `handlers_dispatch` rồi `send_line` phản hồi. Cuối cùng, nếu `close_after` (BYE) thì đóng |
| `server_close_client(i)` | `close(fd)`, xoá khỏi mảng (đánh dấu `fd=-1` rồi dọn sau vòng duyệt, để không làm lệch chỉ số khi đang duyệt). In `[-] …` |
| `server_print_clients()` | Cho lệnh `clients` |
| `server_shutdown()` | Đóng hết |

**Cần code: `handlers.c`**

| Phần | Mục đích |
|---|---|
| Kiểu `handler_fn(client*, argc, argv, resp, sz)` | Mọi hàm xử lý lệnh có cùng chữ ký, nên đặt chung được vào một bảng |
| Bảng `{ "HELLO", h_hello, 0 }, { "INFO", h_info, 0 }, { "BYE", h_bye, 0 }` | Cột thứ 3 là "cần token". Tuần 1 đặt 0 hết, tuần 2 bật lên |
| `handlers_dispatch(c, line, resp, sz)` | Chép dòng ra bản sao, `split_words` (kv.c của Minh), tra bảng. Không thấy thì `400 BAD_REQUEST unknown_command` |
| `h_hello` | `200 OK devices=7 server=device_sim/1.0`. Dùng để client kiểm tra "đúng là device_sim" |
| `h_info` | `device_find(argv[1])`, không thấy thì `404 NO_SUCH_DEVICE`. Thấy thì `200 OK id= type= name= power= mode=` + thông số riêng theo loại (tạm viết `switch` theo `type`, tuần 3 chuyển sang bảng hàm theo loại) |
| `h_bye` | `200 OK` và đặt `close_after = 1` |

**Cần code: `discovery.h` + hàm giả**
- `discovery_open()` trả `-1`, `discovery_handle()` để rỗng. **Mục đích:** `main.c` gọi sẵn theo đúng giao diện. Tuần 2 Minh chỉ cần thay ruột hàm, không phải sửa `main.c` (**H5**).

**Output:**
```
$ nc localhost 7000
HELLO
200 OK devices=7 server=device_sim/1.0
INFO FAN-8A21
200 OK id=FAN-8A21 type=FAN name=Quat_1 power=ON mode=AUTO Tmax=32.0 Tp1=28.0
INFO XYZ
404 NO_SUCH_DEVICE
ABC
400 BAD_REQUEST unknown_command
BYE
200 OK
(server đóng kết nối)
```
Terminal `device_sim`:
```
[+] client 127.0.0.1:51234 (fd=5)
[5] HELLO
[5] INFO FAN-8A21
[-] client 127.0.0.1:51234 (fd=5) closed
```

---

## PHẦN CỦA MINH: chương trình `controller` (client) + thư viện chung `kv`

### M1. `common/kv.c`: tách lệnh và đọc key=value (T7 21/11 → CN 22/11)

**Mục đích:** mọi thông điệp của giao thức và mọi file dữ liệu đều có dạng `LỆNH arg1 arg2 key1=val1 key2=val2`. Module này là **bộ phân tích cú pháp dùng chung** cho cả hai chương trình:
- Long dùng để đọc file cấu hình thiết bị và tách lệnh client gửi lên.
- Minh dùng để đọc phản hồi `INFO`, đọc/ghi `coops.txt`, ghép lệnh `SET`.

Viết chung một lần, kiểm thử kỹ, thì hai phía **hiểu cú pháp giống hệt nhau**. Các hàm `*_strict` chặn dữ liệu rác (`"28abc"`, `""`) ngay từ đầu, không để lọt vào thông số thiết bị.

**Cần code:**

| Hàm | Mục đích |
|---|---|
| `split_words(line, argv[], max)` | Tách dòng thành mảng từ bằng `strtok_r` (an toàn, khác `strtok`). Sửa trực tiếp trên `line`. Quá `max` từ thì trả `-1` |
| `kv_find(argc, argv, key)` | Tìm từ `"key=..."`, trả con trỏ tới giá trị. **Chú ý:** `Tp1` không được khớp nhầm `Tp10=…`, phải kiểm tra ký tự ngay sau key là `=` |
| `kv_split_pair(word, &k, &v)` | Tách `"a=b"` tại dấu `=` đầu tiên |
| `kv_get_str / kv_get_double / kv_get_int` | Lấy giá trị theo kiểu. Trả `0` (được) / `-1` (không có) / `-2` (sai kiểu) để người gọi báo lỗi chính xác |
| `parse_double_strict / parse_int_strict` | `strtod`/`strtol` + kiểm tra không còn ký tự thừa, không rỗng, không tràn số |
| `parse_hhmm / format_hhmm` | `"06:30"` ↔ `390` (phút trong ngày). Dùng cho lịch máy ăn/uống ở tuần 3, nhưng viết luôn bây giờ vì thuộc nhóm phân tích chuỗi |
| `kv_append(buf, sz, key, fmt, ...)` | Nối `" key=value"` vào cuối bộ đệm an toàn. Server dùng để xây phản hồi `INFO`, client dùng để xây lệnh `SET`. Vượt giới hạn thì trả `-1` |
| `dev_type_from_str / dev_type_to_str / dev_type_prefix` | `"FAN"` ↔ `DEV_FAN` ↔ `"FAN-"`. Bảng hằng tra tuyến tính |
| `resp_code(line)` | `"200 OK ..."` → `200`. Client dùng để biết lệnh thành công hay không |

**Output:** `tests/test_kv.c` chạy bằng `make test`:
```
[PASS] split_words("SET FAN-1 tok Tmax=31 Tp1=27") -> 5 từ
[PASS] split_words với nhiều dấu cách thừa
[PASS] kv_get_double Tmax=31 -> 31.0 ; Tp1 -> 27.0
[PASS] kv_find("Tp1") không khớp "Tp10=5"
[PASS] kv_get_double("Tmax=abc") -> -2 ; key không có -> -1
[PASS] parse_double_strict từ chối "", "abc", "28,5", "28.5x"
[PASS] parse_hhmm "06:00"=360 ; từ chối "24:00", "6:00", "06-00"
[PASS] kv_append báo -1 khi vượt kích thước
[PASS] dev_type_from_str("XYZ") -> DEV_INVALID
[PASS] resp_code("403 INVALID_TOKEN") = 403
```
🔁 **Bàn giao H3**: gộp vào `main`, nhắn Long (Long cần để làm L3 từ T3 24/11).

---

### M2. `controller/ui.c`: bộ công cụ nhập/xuất (T2 23/11 → T3 24/11)

**Mục đích:** controller là chương trình **tương tác với người dùng**, và người dùng sẽ nhập sai: gõ chữ vào ô số, Enter rỗng, Ctrl+D. Nếu mỗi màn hình tự `scanf` thì vừa lặp code, vừa dễ treo hoặc tràn bộ đệm (`scanf("%d")` để lại `\n` trong stdin và lặp vô hạn khi gặp chữ). Module này cung cấp **một bộ hàm nhập/xuất an toàn và đồng nhất**, nên mọi màn hình sau này (kể cả `ui_climate.c` của Long ở tuần 3) chỉ việc gọi.

**Cần code:**

| Hàm | Mục đích |
|---|---|
| `ui_read_line(prompt, buf, sz)` | `fgets` + `trim`. Dòng dài hơn bộ đệm thì đọc bỏ phần thừa. Ctrl+D thì trả `-1` |
| `ui_read_int / ui_read_double(prompt, min, max, &out)` | Hỏi lại đến khi hợp lệ. Enter rỗng thì huỷ |
| `ui_read_double_keep(prompt, cur, min, max, &out)` | Hiện giá trị hiện tại, Enter thì **giữ nguyên**. Tuần 3 dùng cho mọi màn hình chỉnh thông số |
| `ui_read_hhmm(prompt, &minute)` | Nhập giờ cho lịch |
| `ui_read_password(prompt, buf, sz)` | Tắt hiển thị ký tự bằng `termios`, khôi phục kể cả khi lỗi. Từ chối mật khẩu có dấu cách vì giao thức tách theo dấu cách |
| `ui_confirm(prompt)` | Hỏi `y/n` trước các thao tác xoá |
| `ui_menu(title, items, n)` | In menu đánh số, trả lựa chọn (`0` là quay lại) |
| Hàm in bảng | Căn cột cho danh sách chuồng, thiết bị |
| `ui_ok / ui_err / ui_info` | In có tiền tố `[OK]` `[LỖI]` `[i]`, có thể tô màu ANSI |
| `ui_explain_code(code)` | Mã `401/403/404/...` thành câu tiếng Việt dễ hiểu. Người dùng không cần biết "403" là gì |
| `ui_pause()` | "Nhấn Enter để tiếp tục" |

**Output (tự thử):**
- Ô số nhập `abc`, `99` (ngoài khoảng), `12x` thì hỏi lại. Enter rỗng thì huỷ.
- Ctrl+D ở bất kỳ ô nào thì quay về menu trước, không lặp vô hạn.
- Nhập mật khẩu không hiện chữ. Sau đó terminal hiện chữ lại bình thường.

---

### M3. `controller/main.c`: App ID và menu chính (T4 25/11)

**Mục đích:**
- Đề bài: "*Mỗi ứng dụng cài đặt được khởi tạo một giá trị ID ngẫu nhiên*". App ID là **danh tính của controller**. Thiết bị gắn token với App ID này (tuần 2), nên App ID phải **sinh một lần và giữ cố định** (lưu `data/app.conf`). Xoá file thì coi như cài lại app.
- Menu chính là **khung** để các chức năng tuần sau cắm vào.
- Tham số `-d <thư mục data>` cho phép chạy **2 controller với 2 App ID khác nhau** trên cùng máy. Tuần 2 cần để demo "app A đổi mật khẩu thì app B bị đá ra".

**Cần code:**

| Phần | Mục đích |
|---|---|
| `signal(SIGPIPE, SIG_IGN)` | Thiết bị đóng kết nối thì app không chết |
| Đọc `-d` | Chọn thư mục dữ liệu |
| `load_or_create_app_id()` | Đọc `app_id=` trong `app.conf`, kiểm tra đúng 8 hex. Thiếu hoặc sai thì `random_hex(4)`, ghi file, báo "Đã khởi tạo App ID mới" |
| `coop_load()` (M4), `conn_init()` (M5) | Khởi tạo module |
| Vòng menu chính | 1. Quản lý chuồng, 2. Quét (tạm "Chưa hỗ trợ"), 3. Vào chuồng (tạm "Chưa hỗ trợ"), 4. Tổng quan (tạm), **9. [DEV] Gửi lệnh thô**, 0. Thoát |
| Mục 9 "Gửi lệnh thô" | Nhập IP, port, rồi gõ lệnh tự do và in nguyên phản hồi. **Mục đích:** công cụ tích hợp và debug của tuần 1, khi chưa có màn hình thật. Bỏ khỏi bản nộp cuối |
| Thoát | `conn_shutdown`, `coop_save` |

**Output:**
```
$ ./bin/controller
[i] Đã khởi tạo App ID mới: 7f3a9c21          (chỉ lần đầu)
=== CHICKEN FARM CONTROLLER ===  App ID: 7f3a9c21 | Chuồng: 0 | Đang kết nối: 0
1. Quản lý chuồng nuôi
2. Quét thiết bị
3. Vào một chuồng
4. Tổng quan chuồng
9. [DEV] Gửi lệnh thô
0. Thoát
```
- Chạy lại thì App ID giữ nguyên.
- `./bin/controller -d data2` cho App ID khác.

---

### M4. `controller/coop.c`: quản lý chuồng nuôi (T5 26/11 → T7 28/11). TIÊU CHÍ 13 (1 điểm)

**Mục đích:** đề bài: "*Quản lý các chuồng nuôi … Người dùng thêm thiết bị mới vào danh sách thiết bị trong chuồng nuôi*". Chuồng là **cách người dùng tổ chức thiết bị**: chuồng A có quạt nào, máy ăn nào. Thông tin chuồng (tên, vị trí, số gà, sức chứa) là dữ liệu **phía người dùng**, nên lưu ở controller, không ở thiết bị. Phải lưu file để mở app lần sau còn nguyên.

**Cần code:**

| Phần | Mục đích |
|---|---|
| `struct coop_device { id, type, ip, port, name }` | Thông tin để **tìm lại và kết nối** thiết bị. Trạng thái kết nối KHÔNG nằm ở đây (nằm ở `conn.c`) |
| `struct coop { id, name, location, capacity, chickens, created, devices*, n_dev, cap_dev }` | Một chuồng. Mảng thiết bị **cấp phát động** (`realloc` nhân đôi), tham khảo `lab03_dynamic_array.c` |
| Mảng động các chuồng (static) | Kho dữ liệu, chỉ truy cập qua hàm |
| `coop_add / coop_edit / coop_remove / coop_find / coop_count / coop_at` | CRUD chuồng. `coop_remove` phải `free` mảng thiết bị |
| `coop_add_device / coop_remove_device / coop_find_device` | Thêm/xoá thiết bị. **Một thiết bị không được ở 2 chuồng.** `coop_find_device` còn dùng ở tuần 2 để đánh dấu "đã thêm" khi quét |
| Kiểm tra dữ liệu | Tên không rỗng, không trùng, không có dấu cách (hoặc đổi thành `_`). `capacity > 0`. `0 ≤ chickens ≤ capacity` |
| `coop_load / coop_save` | Định dạng `data/coops.txt` (dòng `coop ...` và dòng `dev coop=...`), đọc bằng `kv.c`. Ghi file tạm rồi `rename`. **Dòng hỏng thì cảnh báo và bỏ qua**, không crash (người dùng có thể sửa tay file) |
| `coop_menu()` | 1. Danh sách, 2. Thêm, 3. Sửa (Enter giữ giá trị cũ), 4. Xoá (hỏi xác nhận nếu còn thiết bị), 5. Xem thiết bị, 6. Thêm thiết bị **nhập tay** (tạm thời, tuần 2 thay bằng thêm từ kết quả quét), 7. Xoá thiết bị, 0. Quay lại |

**Output:**
```
--- DANH SÁCH CHUỒNG ---
ID | Tên       | Vị trí | Gà/Sức chứa | Thiết bị | Ngày tạo
1  | Chuong_A  | Khu_1  | 420/500     | 2        | 2026-11-26
2  | Chuong_B  | Khu_2  | 0/300       | 0        | 2026-11-27
```
- Thoát rồi mở lại thì bảng y hệt.
- Thêm trùng tên, hoặc gà > sức chứa, thì `[LỖI] ...`.
- Thêm cùng một ID thiết bị vào chuồng thứ 2 thì bị từ chối.
- Sửa tay `coops.txt` cho hỏng 1 dòng thì có cảnh báo, các dòng khác vẫn nạp được.

---

### M5. `controller/conn.c`: khung kết nối TCP (CN 29/11). Chuẩn bị TIÊU CHÍ 6

**Mục đích:** đây là **cầu nối duy nhất** giữa giao diện và mạng. Mọi màn hình tuần 2–3 (của cả Minh và Long) chỉ gọi `conn_cmd(...)`, không màn hình nào tự mở socket. Lợi ích:
- Timeout, mất kết nối, đọc đúng một dòng phản hồi đều xử lý **một chỗ**.
- Tuần 2 thêm token chỉ phải sửa ở đây, mọi màn hình tự động có.
- Nếu không có timeout, thiết bị treo thì **cả menu treo theo**.

**Cần code:**

| Phần | Mục đích |
|---|---|
| `struct session { device_id, ip, port, fd, struct linebuf lb, token[], connected }` | Một phiên làm việc với thiết bị. Tuần 1 chưa dùng `token` |
| `conn_init(app_id)` / `conn_shutdown()` | Lưu App ID cho tuần 2. Đóng hết khi thoát (gửi `BYE`) |
| `conn_open(ip, port)` | `getaddrinfo` (nhập được cả hostname, tham khảo `domain2ip.c`), `socket`, `connect`. Lỗi thì trả mã âm kèm thông báo rõ (`Connection refused` nghĩa là "device_sim chưa chạy?") |
| `conn_raw(fd, lb, cmd, resp, sz)` | Gửi một dòng bằng `send_line`, đọc **đúng một dòng** bằng `read_line_blocking` với `REQ_TIMEOUT_MS`. Trả mã phản hồi (`resp_code`) hoặc mã âm (`-1` mất kết nối, `-2` hết giờ) |
| Phát hiện mất kết nối | Đọc trả `0`/lỗi hoặc gửi lỗi thì đóng fd, đánh dấu phiên mất kết nối |
| `conn_cmd(device_id, resp, sz, fmt, ...)` | Khai báo sẵn chữ ký (đã chốt ở Bước 0). Tuần 1 chỉ chuyển tiếp sang `conn_raw`, tuần 2 mới chèn `<id> <token>` |
| Gắn vào mục 9 của menu | Mở kết nối, lặp nhập lệnh, `conn_raw`, in phản hồi. Gõ `exit` thì thoát |

**Output (khi chưa có server của Long, dùng `nc -l 7000` đóng vai thiết bị):**

| Thao tác | Kết quả mong đợi |
|---|---|
| Controller gửi `HELLO`, bên `nc` gõ tay `200 OK hi` | Controller in `200 OK hi` |
| Bên `nc` không gõ gì | Sau 3 giây: `[LỖI] Thiết bị không phản hồi` |
| Ctrl+C bên `nc` rồi gửi lệnh tiếp | `[LỖI] Mất kết nối tới thiết bị` |
| Kết nối tới cổng không có ai nghe | `[LỖI] Không kết nối được 127.0.0.1:7000 (device_sim đã chạy chưa?)` |

---

## T2 30/11 ★ TÍCH HỢP (cả hai ngồi cùng nhau)

**Mục đích:** kiểm chứng hai nửa **thật sự khớp nhau** trước khi sang tuần 2. Lỗi tích hợp tìm ra bây giờ (lệch định dạng, quên `\n`, sai tên key) rẻ hơn nhiều so với tìm ra ở tuần 3.

### Kịch bản tích hợp

```
Terminal 1:  ./bin/device_sim -c config/coop_A.conf
Terminal 2:  ./bin/controller   → 9. [DEV] Gửi lệnh thô
             IP: 127.0.0.1   Port: 7000
             Lệnh> HELLO
             200 OK devices=7 server=device_sim/1.0
             Lệnh> INFO FAN-8A21
             200 OK id=FAN-8A21 type=FAN name=Quat_1 power=ON mode=AUTO Tmax=32.0 Tp1=28.0
             Lệnh> INFO XYZ
             [LỖI] Thiết bị không tồn tại trên máy này.  (404)
             Lệnh> exit
Terminal 3:  ./bin/controller -d data2   (cùng lúc, cũng gửi HELLO được)
```

### Checklist cuối tuần

**Build & test**
- [ ] `make clean && make` không có cảnh báo
- [ ] `make test`: mọi dòng của `test_util`, `test_netio`, `test_kv` đều `[PASS]`

**Server (Long)**
- [ ] 2 controller + 2 `nc` kết nối cùng lúc đều được phục vụ
- [ ] `kill -9` một controller thì server vẫn chạy, `clients` không còn client đó
- [ ] `(printf 'HEL'; sleep 1; printf 'LO\nINFO FAN-8A21\n') | nc localhost 7000` nhận đúng 2 phản hồi
- [ ] Gửi dòng 5000 byte thì nhận `400 ... LINE_TOO_LONG`, kết nối vẫn dùng tiếp được
- [ ] Ctrl+C rồi chạy lại ngay thì không có "Address already in use". ID không đổi
- [ ] `make reset-data` rồi chạy thì ID mới, file `data/` và `logs/` được tạo lại
- [ ] Không có client nào thì `status` vẫn thấy `tick` tăng

**Client (Minh)**
- [ ] App ID giữ nguyên qua các lần chạy. `-d data2` cho App ID khác
- [ ] Dữ liệu chuồng giữ nguyên sau khi khởi động lại
- [ ] Không có thao tác nhập liệu nào làm crash hoặc lặp vô hạn
- [ ] Timeout và mất kết nối được báo bằng tiếng Việt rõ ràng

**Quy trình**
- [ ] 🔁 H2, H3 đã bàn giao. H4 (server chạy được HELLO/INFO) và H5 (`discovery.h`) đã sẵn sàng cho tuần 2
- [ ] Code đã gộp vào `main` qua Pull Request, người kia đã review
- [ ] Ghi kết quả vào `docs/test_cases.md`

---

## Nếu bị trễ: thứ tự ưu tiên

| Mức | Phần | Lý do |
|---|---|---|
| 🔴 **Không được cắt** | `protocol.h`, `device.h`, `net_io`, `kv`, vòng `select` + `server.c`, `device.c` (nạp + ID), `conn_raw` | Toàn bộ tuần 2 dựa trên các phần này |
| 🟡 Lùi sang tuần 2 được | `coop_edit`, `ui_read_password`, lệnh `clients`, `dlog_set_min_level`, `test_util` | Không chặn công việc của người kia |
| 🟢 Lùi xa hơn được | Tô màu ANSI, hiển thị ngày tạo chuồng, tham số `-s` | Chỉ là phần trang trí |

Trễ quá 1 ngày thì báo ngay cho người kia. Người đang rảnh viết test hoặc giả lập đầu bên kia (`nc`) giúp người đang chậm.
