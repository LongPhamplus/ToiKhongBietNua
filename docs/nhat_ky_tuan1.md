# NHẬT KÝ CODE TUẦN 1

> Ghi lại **từng bước đã code** trong tuần 1, theo đúng thứ tự phát triển. Mỗi bước tương ứng **một commit**.
> Mục đích: Long và Minh đọc từng bước để **tự code lại** trong `RealApp/` của mình. Đọc phần "Tại sao" trước, sau đó tự viết, rồi mới so với code mẫu trong repo.
> Kế hoạch gốc: `docs/tuan1_code.md`. Môi trường build: WSL Debian, gcc 14, GNU make 4.4.

## Mục lục

| Bước | Người | Nội dung | File |
|---|---|---|---|
| 1 | Long | Phần dùng chung: hằng số, mô hình thiết bị, Makefile | `common/protocol.h`, `device/device.h`, `Makefile`, `.gitignore`, `.gitattributes` |
| 2 | Long | Tiện ích chung + framework test | `common/util.h/.c`, `tests/test.h`, `tests/test_util.c` |
| 3 | Long | **Xử lý truyền dòng** (tiêu chí 1) | `common/net_io.h/.c`, `tests/test_netio.c` |
| 4 | Minh | Tách lệnh, đọc/ghi key=value | `common/kv.h/.c`, `tests/test_kv.c` |
| 5 | Long | **Khởi tạo thông số** (tiêu chí 4): nạp/lưu thiết bị, sinh ID | `device/device.c`, `config/coop_A.conf`, `config/coop_B.conf` |
| 6 | Long | Ghi log từng thiết bị (tiêu chí 12, phần nền) | `device/dlog.h/.c` |

---

## Bước 1: Phần dùng chung (Long viết, Minh duyệt)

**Commit:** `Bước 1: protocol.h, device.h, Makefile`

### Đã làm
- `common/protocol.h`: cổng, giới hạn độ dài/số lượng, thời gian, tên lệnh, mã phản hồi, `enum` kiểu thiết bị / nguồn / chế độ.
- `device/device.h`: 6 struct thông số theo loại, `token_entry_t`, `device_t` (dùng `union`), `sim_config_t`, cùng prototype các hàm sẽ cài đặt ở bước 5.
- `Makefile`: build 2 chương trình, `make test`, `clean`, `reset-data`, `sanitize`, `run-*`.
- `.gitignore`: bỏ qua `bin/`, `build/`, dữ liệu sinh ra trong `data/` và `logs/`.
- `.gitattributes`: ép xuống dòng kiểu LF.
- `device/main.c` và `controller/main.c` tạm in một dòng để kiểm tra build.

### Tại sao
- **`protocol.h` có trước mọi thứ**: đây là "hợp đồng" giữa 2 người. Cả hai cùng include nên không thể lệch cổng hay mã lỗi.
- **`dev_mode_t` chứ không phải `mode_t`**: `mode_t` đã có trong `<sys/types.h>` (kiểu quyền truy cập file). Trùng tên sẽ gây lỗi biên dịch rất khó hiểu.
- **`device.h` chốt ngay tuần 1** dù tuần 3 mới dùng hết: Minh viết `dev_feed.c`, `dev_mister.c` truy cập trực tiếp các trường. Đổi tên trường về sau sẽ phải sửa nhiều file của cả 2 người.
- **`feed_params_t` dùng chung cho FEEDER và WATERER**: logic giống hệt (lượng mỗi lần, bồn chứa, lịch), chỉ khác tên key (`W`/`Vw`, `stock`/`tank`). Một struct thì viết một lần code lịch.
- **`egg.count` là `double`**: mỗi tick chỉ đẻ thêm 0.2 quả. Dùng `int` thì cộng 0.2 sẽ luôn mất phần lẻ, số trứng không bao giờ tăng.
- **Makefile dịch từng `.o` riêng + `-MMD -MP`**: sửa 1 file chỉ dịch lại file đó; sửa `.h` thì tự dịch lại các file include nó.
- **`.gitattributes` ép LF**: máy Windows đang để `core.autocrlf=true`, khi checkout sẽ đổi sang CRLF. Lúc đó `make` báo lỗi `missing separator` vì Makefile có ký tự `\r`.

### Kiểm tra
```
$ make clean && make
... (không có warning)
$ ./bin/device_sim
device_sim/1.0 (khung): chua co gi
$ ./bin/controller
controller (khung): chua co gi
```
Lần build đầu có warning `"/*" within comment` vì chú thích hướng dẫn trong `device/device.c` có chuỗi `config/*.conf`. Đã sửa. **Bài học:** không viết `/*` bên trong chú thích `/* ... */`.

### Tự code lại
1. Viết `protocol.h` theo bảng ở `docs/tuan1_code.md` mục 0-A. Kiểm tra bằng một file `.c` rỗng chỉ include nó.
2. Viết `device.h`, đối chiếu tên trường với người kia.
3. Viết Makefile, thử `touch common/protocol.h && make`: phải thấy mọi file được dịch lại.

---

## Bước 2: `common/util` + framework test (Long)

**Commit:** `Bước 2: util.c (random_hex, now_ms, trim, ...) + test`

### Đã làm
| Hàm | Dùng để |
|---|---|
| `random_hex(out, n)` | Sinh ID thiết bị (bước 5), App ID (bước 10), token (tuần 2) |
| `now_ms()` | Tính timeout `select`, nhịp tick |
| `now_str()`, `today_str()` | Thời gian trong log, ngày tạo chuồng |
| `trim()`, `safe_copy()`, `str_upper()` | Xử lý chuỗi an toàn |
| `ensure_dir()` | Tạo `data/`, `logs/` khi khởi động |
| `die()` | Thoát khi lỗi khởi động |

Thêm `tests/test.h`: hai macro `CHECK` và `TEST_DONE` để viết test ngắn gọn, `make test` dừng ngay khi có `[FAIL]`.

### Tại sao
- **`/dev/urandom` thay vì `rand()`**: `rand()` chỉ là công thức toán. Biết seed (thường là `time(NULL)`) thì đoán được mọi giá trị, tức là đoán được token. `/dev/urandom` do kernel lấy từ nhiễu phần cứng.
- **Lặp `read()`**: `read` được phép trả về ít byte hơn yêu cầu. Đây là cùng một bài học với `recv` ở bước 3.
- **`CLOCK_MONOTONIC`**: `time()` nhảy lùi nếu người dùng chỉnh đồng hồ, lúc đó timeout tính sai. Đồng hồ monotonic chỉ tăng.
- **`localtime_r`** (có hậu tố `_r`): `localtime` trả con trỏ tới vùng nhớ tĩnh dùng chung, không an toàn. Bản `_r` ghi vào biến của mình.
- **`trim` dùng `memmove`**: vùng nguồn và đích chồng lên nhau. `memcpy` với vùng chồng nhau là hành vi không xác định.
- **`isspace((unsigned char)c)`**: ký tự UTF-8 tiếng Việt có byte > 127. Kiểu `char` có dấu sẽ thành số âm, truyền số âm vào `isspace` là lỗi.

### Kiểm tra
```
$ make test
== bin/test_util
[PASS] random_hex: dung 8 ky tu, chi gom 0-9a-f
[PASS] random_hex: 1000 gia tri khong trung
[PASS] random_hex(16) -> token 32 ky tu
[PASS] trim("  abc \r\n") == "abc"
[PASS] trim chuoi toan khoang trang -> rong
[PASS] safe_copy cat dung khi src dai hon dst
[PASS] str_upper
[PASS] ensure_dir tao duoc thu muc
[PASS] ensure_dir goi lan 2 khong loi
[PASS] now_ms do duoc khoang 50ms
```
Phát sinh: make tự xoá `build/tests/test_util.o` (file trung gian). Đã thêm `.SECONDARY:` vào Makefile để giữ lại.

### Tự code lại
Viết test **trước** (chỉ cần prototype trong `util.h`), chạy thấy FAIL, rồi mới cài đặt hàm cho đến khi PASS.

---

## Bước 3: `common/net_io`, xử lý truyền dòng (Long). TIÊU CHÍ 1

**Commit:** `Bước 3: net_io.c - bộ đệm dòng, send_all (tiêu chí 1)`

### Đã làm
- `linebuf_t` gồm bộ đệm `2*MAX_LINE` byte, độ dài hiện có, cờ `discarding`.
- `linebuf_fill`: gọi `recv` **một lần**, nối byte vào cuối bộ đệm.
- `linebuf_next`: tìm `\n`, trả **một** dòng, dồn phần dư về đầu. Dòng quá dài thì báo lỗi một lần rồi bỏ phần còn lại.
- `read_line_blocking`: dành cho client, chờ một dòng có timeout (dùng `select`).
- `send_all`, `send_line`, `send_linef`.

### Tại sao (phần thầy hay hỏi)
```
Client gửi:   "INFO FAN-1\n"  "INFO HEA-2\n"
Server có thể nhận:
  recv #1 → "INFO FA"                    → linebuf_next trả 0 (chưa đủ)
  recv #2 → "N-1\nINFO HEA-2\n"          → linebuf_next trả "INFO FAN-1", rồi "INFO HEA-2", rồi 0
```
- **Server dùng `fill` + `next` tách riêng**: sau khi `select()` báo socket có dữ liệu, chỉ được gọi `recv` **một** lần (gọi lần nữa có thể bị chặn và treo cả server). Sau đó gọi `next` lặp đến khi hết dòng.
- **Client dùng `read_line_blocking`**: client gửi một lệnh rồi chờ đúng một dòng phản hồi. Phải có timeout để thiết bị treo không kéo theo cả menu treo.
- **`memchr` thay `strchr`**: dữ liệu từ mạng có thể chứa byte `0`. `strchr` dừng ở byte đó, nên phải tìm theo độ dài `len`.
- **Cờ `discarding`**: khi một dòng vượt `MAX_LINE`, ta báo lỗi **một lần** rồi vứt các byte còn lại đến `\n`. Không có cờ này thì phần đuôi của dòng dài bị hiểu nhầm thành một lệnh mới.
- **Đặt lại `timeval` mỗi vòng**: Linux sửa `timeval` sau mỗi lần `select`. Code mẫu `select_client.c` chỉ đặt một lần nên từ vòng 2 timeout gần như bằng 0.
- **`send_all` lặp**: `send` có thể chỉ gửi một phần (bộ đệm gửi của kernel đầy).
- **`MSG_NOSIGNAL`**: bên kia đã đóng mà mình vẫn `send` thì kernel gửi `SIGPIPE`. Mặc định tín hiệu này **giết tiến trình**. Có cờ thì `send` chỉ trả lỗi `EPIPE`.
- **`send_line` ghép `\n` vào cùng bộ đệm**: tránh gửi 2 gói nhỏ liên tiếp, vì thuật toán Nagle có thể làm gói thứ 2 trễ khoảng 40ms.

### Đính chính kế hoạch
`tuan1_code.md` ghi test "5000 byte + `HELLO\n` → lỗi 1 lần, sau đó đọc được HELLO". Viết như vậy là **sai**: không có `\n` giữa hai phần thì `HELLO` chỉ là phần đuôi của dòng dài, bị vứt cùng dòng đó. Test đúng là `5000 byte + "\n" + "HELLO\n"`.

### Kiểm tra
```
== bin/test_netio
[PASS] "HEL" + "LO\n" (2 lan gui) -> 1 dong HELLO
[PASS] "A\nB\nC\n" (1 lan gui) -> 3 dong A, B, C
[PASS] het dong -> tra 0
[PASS] "A\r\n" -> "A"
[PASS] 5000 byte + "\nHELLO\n" -> loi 1 lan, sau do doc duoc HELLO
[PASS] gui "AB" roi dong -> chua co dong, fill tra 0
[PASS] read_line_blocking khong co du lieu -> -2 sau ~300ms
[PASS] send_all 100 KB -> ben kia nhan du 100 KB
[PASS] send_line tu choi dong >= MAX_LINE
```
Test dùng `socketpair()` để có 2 đầu socket nối nhau ngay trong một chương trình, giả lập được việc cắt vụn và dồn dòng mà không cần mạng. Test `send_all` dùng `fork()`: con gửi 100 KB, cha đọc và đếm (tham khảo `IT4062/3-Chap3/L3-MultiProcessing`).

### Tự code lại
Vẽ ra giấy bộ đệm `[I N F O _ F A]` → `[I N F O _ F A N - 1 \n I N ...]`, làm tay từng bước `memchr` / `memcpy` / `memmove`, rồi mới code `linebuf_next`.

---

## Bước 4: `common/kv`, tách lệnh và đọc key=value (Minh)

**Commit:** `Bước 4: kv.c - split_words, kv_get_*, parse_hhmm + test`

### Đã làm
| Hàm | Dùng để | Ai dùng |
|---|---|---|
| `split_words` | Tách `"INFO FAN-1"` thành `["INFO","FAN-1"]` | `handlers.c`, `device.c`, `coop.c` |
| `kv_find`, `kv_get_str/double/int` | Đọc `Tmax=32` trong lệnh hoặc file | Cả hai |
| `parse_double_strict`, `parse_int_strict` | Đổi chuỗi sang số, từ chối rác | Cả hai (kể cả ô nhập trong `ui.c`) |
| `parse_hhmm`, `format_hhmm` | Giờ lịch cho ăn/uống | Tuần 3 |
| `kv_append` | Ghép phản hồi `INFO` (server), ghép lệnh `SET` (client) | Cả hai |
| `dev_type_*` | Đổi `"FAN"` ↔ `DEV_FAN` ↔ `"FAN-"` ↔ `"Quat dieu hoa"` | Cả hai |
| `resp_code`, `rc_name` | `"403 ..."` → `403`, `403` → `"INVALID_TOKEN"` | Client đọc / server ghi |

### Tại sao
- **`strtok_r` thay vì `strtok`**: `strtok` nhớ vị trí bằng biến tĩnh ẩn. Nếu đang tách một dòng mà gọi hàm khác cũng dùng `strtok`, vị trí bị ghi đè. Bản `_r` giữ vị trí trong biến `save` của mình (xem `IT4062/1-Chap1/lab09_string.c`).
- **`split_words` sửa trực tiếp `line`**: không cấp phát thêm bộ nhớ, `argv[i]` trỏ vào chính `line`. Người gọi phải dùng **bản sao** nếu còn cần dòng gốc.
- **`kv_find` kiểm tra ký tự `=` ngay sau key**: tìm `"Tp1"` mà chỉ so tiền tố thì sẽ khớp nhầm `"Tp10=5"`.
- **Mã trả về 3 mức (`0` / `-1` / `-2`)**: người gọi phân biệt được "không có key" (dùng giá trị mặc định) với "có key nhưng giá trị sai" (báo lỗi `422`).
- **`*_strict`**: `atof("28abc")` trả 28 mà không báo lỗi, `atof("abc")` trả 0. Kiểm tra `end` của `strtod` mới biết chuỗi có đúng là số hay không. Từ chối thêm `nan`/`inf` vì thông số nhiệt độ không thể là vô cùng.
- **`kv_append` hoàn tác khi thiếu chỗ**: phản hồi bị cắt nửa chừng (`Tmax=3`) còn tệ hơn không có. Hoàn tác thì người gọi biết là lỗi.
- **Một bảng `DEV_TABLE` duy nhất**: thêm loại thiết bị mới chỉ cần sửa một chỗ.

### Kiểm tra
```
== bin/test_kv
[PASS] split_words("SET FAN-1 tok Tmax=31 Tp1=27") -> 5 tu
[PASS] kv_get_double Tmax -> 31.0
[PASS] kv_get_double Tp1 -> 27.0
[PASS] key khong co -> -1
[PASS] split_words bo qua dau cach thua -> 3 tu
[PASS] split_words qua max tu -> -1
[PASS] kv_find("Tp1") khong khop nham "Tp10=5"
[PASS] kv_get_double("Tmax=abc") -> -2
[PASS] kv_get_str qua dai -> -2
[PASS] parse_double_strict("28.5")
[PASS] parse_double_strict tu choi "", "abc", "28,5", "28.5x", "nan"
[PASS] parse_int_strict tu choi so tran
[PASS] parse_hhmm("06:00") = 360
[PASS] parse_hhmm("23:59") = 1439
[PASS] parse_hhmm tu choi 24:00, 6:00, 06-00, 06:60
[PASS] format_hhmm(390) = "06:30"
[PASS] kv_append -> "200 OK Tmax=32.0"
[PASS] kv_append vuot kich thuoc -> -1, buf giu nguyen
[PASS] dev_type_from_str("FAN")
[PASS] dev_type_from_str("XYZ") -> DEV_INVALID
[PASS] dev_type_prefix(FEEDER) = "FED-"
[PASS] resp_code("200 OK x") = 200
[PASS] resp_code("403 INVALID_TOKEN") = 403
[PASS] resp_code dong sai -> -1
```
🔁 **Bàn giao H3**: Long dùng `kv.c` ngay ở bước 5 (`device.c`).

---

## Bước 5: `device.c` + file cấu hình, khởi tạo thông số (Long). TIÊU CHÍ 4

**Commit:** `Bước 5: device.c - nạp config, mặc định, sinh ID, lưu data/ (tiêu chí 4)`

### Đã làm
| Hàm | Mục đích |
|---|---|
| `device_set_defaults` | Điền mặc định theo bảng 4.1 (Tmax=32, W=5kg, lịch 06:00/11:00/17:00, …) |
| `device_set_param` | Đặt **một** thông số theo tên key. Dùng lại cho lệnh `SET` ở tuần 3 |
| `device_validate` + `validate_cross` | Kiểm tra khoảng giá trị, `Tp1 < Tmax`, `Tmin < Tp2`, `Hmin < Hp`, và ràng buộc chéo `Tp2(đèn) < Tp1(quạt)` |
| `device_params_to_kv` | Thông số thành `" Tmax=32.0 Tp1=28.0"`. Dùng chung cho lưu file và lệnh `INFO` |
| `parse_device_words` | Đọc dòng `device type=... name=... key=...` |
| `gen_unique_id` | `FAN-8A21` = tiền tố + 4 hex ngẫu nhiên, lặp đến khi không trùng |
| `devices_load` | Lần đầu: config, rồi mặc định, rồi sinh ID, rồi lưu. Lần sau: nạp từ `data/` |
| `devices_save` | Ghi vào file tạm rồi `rename()` |

`device/main.c` tạm thời: nạp thiết bị, in bảng rồi thoát.

### Tại sao
- **Tách config gốc và file trạng thái**: `config/coop_A.conf` do người viết (không có ID). `data/devices_coop_A.conf` do chương trình sinh (có ID, mật khẩu đã đổi, thông số đã SET). Nhờ vậy ID **ổn định** qua các lần chạy. Controller đã lưu ID vào chuồng, ID mà đổi thì mất liên kết.
- **Điền mặc định trước, ghi đè sau**: dòng config thiếu key nào thì dùng mặc định của key đó. Demo được "khởi tạo thông số" bằng cách cố ý bỏ trống vài key.
- **`device_set_param` dùng chung cho config và `SET`**: một chỗ kiểm tra kiểu dữ liệu, tuần 3 không phải viết lại.
- **Báo lỗi `file:dòng: lý do` rồi thoát**: chạy với cấu hình vô lý (quạt tắt ở 40°C trong khi bật ở 32°C) còn tệ hơn không chạy.
- **Ràng buộc chéo**: nếu đèn sưởi tắt ở 30°C mà quạt đã bật từ 28°C thì trong khoảng 28–30°C cả hai cùng chạy.
- **`rename()` là thao tác nguyên tử**: nếu bị tắt khi đang ghi thì file tạm hỏng, còn file thật vẫn là bản cũ nguyên vẹn.

### Lỗi gặp phải (bài học)
Lần chạy đầu **treo vô hạn**. Nguyên nhân: `gen_unique_id` ghi ID ứng viên **thẳng vào `d->id`**, rồi gọi `device_find(d->id)` để kiểm tra trùng. Hàm này tìm thấy **chính thiết bị `d`**, nên ID nào cũng bị coi là trùng.
**Sửa:** sinh vào biến tạm `cand`, kiểm tra xong mới chép vào `d->id`.

### Kiểm tra
```
$ make reset-data && ./bin/device_sim                  (lần 1)
[i] Lan chay dau: sinh ID moi cho 7 thiet bi
device_sim/1.0  config=coop_A  TCP=7000  devices=7
  ENV-D7C3  ENV_SENSOR   Cam_bien_1   interval=1
  EGG-F88E  EGG_COUNTER  Dem_trung_1  capacity=300 rate=12.0
  FED-82F1  FEEDER       May_an_1     W=5.0 stock=100.0 times=06:00,11:00,17:00
  WAT-EEB3  WATERER      May_uong_1   Vw=10.0 tank=500.0 times=06:00,11:00,17:00
  FAN-2E08  FAN          Quat_1       Tmax=32.0 Tp1=28.0
  HEA-F7D2  HEATER       Den_1        Tmin=18.0 Tp2=22.0
  MIS-8642  MISTER       Phun_1       Hmin=55.0 Hp=70.0 Vh=2.0

$ ./bin/device_sim                                     (lần 2)
[i] Nap trang thai tu data/devices_coop_A.conf
  ENV-D7C3 ...                                         ← ID giống hệt lần 1

Cấu hình sai (mỗi dòng là một lần thử):
Loi cau hinh: /tmp/bad.conf:22: Tp1=40.0 phai nho hon Tmax=32.0
Loi cau hinh: Tp2 cua HEA-C4B6 (30.0) phai nho hon Tp1 cua FAN-D0A9 (28.0)
Loi cau hinh: /tmp/bad.conf:20: W='abc' khong phai so
Loi cau hinh: /tmp/bad.conf:24: thong so 'Tmax' khong ap dung cho MISTER
Loi cau hinh: /tmp/bad.conf:22: type='QUAT' khong hop le
(tất cả thoát với mã 1)
```
WATERER và HEATER không ghi thông số trong config, nên nhận mặc định (`Vw=10`, `Tmin=18`, `Tp2=22`). Đây là bằng chứng cho tiêu chí 4.

---

## Bước 6: `dlog`, ghi log từng thiết bị (Long). TIÊU CHÍ 12 (phần nền)

**Commit:** `Bước 6: dlog.c - log riêng từng thiết bị`

### Đã làm
- `dlog_open_all` / `dlog_close_all`: mở/đóng `logs/<id>.log` chế độ ghi nối.
- `dlog(dev, mức, app_id, fmt, ...)`: ghi một dòng cho một thiết bị. `app_id = NULL` nghĩa là nguồn "auto".
- `dlog_all(...)`: ghi cùng một dòng cho mọi thiết bị (khởi động / tắt).
- `dlog_set_min_level`, `dlog_set_echo`: ẩn DEBUG, bật/tắt in ra màn hình.
- `main.c` tạm thời: ghi thử vài dòng rồi thoát.

### Tại sao
- **Làm log ngay tuần 1** dù tiêu chí 12 chấm ở cuối: từ bước này, module nào viết thêm cũng gọi `dlog` luôn. Để cuối kỳ mới chèn log vào thì rất dễ sót sự kiện.
- **Một file cho mỗi thiết bị**: đề bài ghi "ghi log hoạt động **trên mỗi thiết bị**".
- **Chế độ `"a"` (append)**: giữ log của các lần chạy trước.
- **`fflush` sau mỗi dòng**: `fprintf` chỉ ghi vào bộ đệm của stdio. Nếu Ctrl+C hoặc crash trước khi bộ đệm đầy thì mất log, đúng lúc cần log nhất.
- **`vlog` nhận `va_list`**: `dlog` và `dlog_all` cùng dùng một hàm lõi. Chú ý `va_list` **chỉ duyệt được một lần**, nên trong vòng lặp của `dlog_all` phải `va_start` / `va_end` lại mỗi vòng.
- **`__attribute__((format(printf,...)))`**: gcc kiểm tra định dạng `%d` / `%s` của lời gọi `dlog` giống như với `printf`.
- **Cột "nguồn"** (`app=7f3a9c21` hoặc `auto`): biết **ai** gây ra sự kiện. Tuần 2 ghi App ID khi kết nối, đổi mật khẩu.

### Kiểm tra
```
$ ./bin/device_sim
...
[log FAN-2E08] INFO  thu ghi log rieng cho quat
$ ls logs
EGG-F88E.log  ENV-D7C3.log  FAN-2E08.log  FED-82F1.log  HEA-F7D2.log  MIS-8642.log  WAT-EEB3.log
$ cat logs/FAN-2E08.log
2026-11-20 14:06:19 | sim --:-- | INFO  | auto         | device_sim started (coop_A, tcp 7000)
2026-11-20 14:06:19 | sim --:-- | INFO  | auto         | thu ghi log rieng cho quat
2026-11-20 14:06:19 | sim --:-- | INFO  | auto         | device_sim stopped
```
Dòng DEBUG không xuất hiện (đã bị lọc). Chạy lần 2 thì file dài thêm (ghi nối).
Ghi chú: giờ trong log là **UTC** vì WSL chưa đặt múi giờ. Sửa bằng `sudo ln -sf /usr/share/zoneinfo/Asia/Ho_Chi_Minh /etc/localtime` hoặc chạy với `TZ=Asia/Ho_Chi_Minh`.
