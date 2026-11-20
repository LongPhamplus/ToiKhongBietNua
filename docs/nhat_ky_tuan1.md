# NHẬT KÝ CODE TUẦN 1

> Ghi lại **từng bước đã code** trong tuần 1, theo đúng thứ tự phát triển. Mỗi bước tương ứng **một commit**.
> Mục đích: Long và Minh đọc từng bước để **tự code lại** trong `RealApp/` của mình. Đọc phần "Tại sao" trước, sau đó tự viết, rồi mới so với code mẫu trong repo.
> Kế hoạch gốc: `docs/tuan1_code.md`. Môi trường build: WSL Debian, gcc 14, GNU make 4.4.

## Mục lục

| Bước | Người | Nội dung | File |
|---|---|---|---|
| 1 | Long | Phần dùng chung: hằng số, mô hình thiết bị, Makefile | `common/protocol.h`, `device/device.h`, `Makefile`, `.gitignore`, `.gitattributes` |

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
