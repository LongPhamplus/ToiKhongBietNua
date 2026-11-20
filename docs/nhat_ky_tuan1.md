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
