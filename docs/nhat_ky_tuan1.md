# NHẬT KÝ CODE TUẦN 1

> Ghi lại **từng bước đã code** trong tuần 1, theo đúng thứ tự phát triển. Mỗi bước tương ứng **một commit**.
> Mục đích: Long và Minh đọc từng bước để **tự code lại** trong `RealApp/` của mình. Đọc phần "Tại sao" trước, sau đó tự viết, rồi mới so với code mẫu trong repo.
> Kế hoạch gốc: `docs/tuan1_code.md`. Môi trường build: WSL Debian, gcc 14, GNU make 4.4.

## Mục lục

| Bước | Người | Nội dung | File |
|---|---|---|---|
| 1 | Long | Phần dùng chung: hằng số, mô hình thiết bị, Makefile | `common/protocol.h`, `device/device.h`, `Makefile`, `.gitignore`, `.gitattributes` |
| 2 | Long | Tiện ích chung + framework test | `common/util.h/.c`, `tests/test.h`, `tests/test_util.c` |

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
