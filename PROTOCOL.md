# PROTOCOL.md: Đặc tả giao thức chính thức

> **Người phụ trách:** Long + Minh (cùng viết, cùng duyệt) · **Phase 0, hạn 23/11**
> Tiêu chí liên quan: 1 (truyền dòng), 3 (xác thực), 5 (quét), 6 (kết nối) và mọi lệnh điều khiển.
> Sau khi chốt, đây là **hợp đồng**: Long code server và Minh code client hoàn toàn dựa trên file này.

## Việc cần làm

- [ ] Chép mục **3** và **4.1** của `KE_HOACH.md` sang đây làm bản gốc.
- [ ] Rà từng lệnh và chốt **chính xác**: tên lệnh, thứ tự tham số, tên key (phân biệt hoa thường), đơn vị, số chữ số thập phân khi in.
- [ ] Với mỗi lệnh ghi đủ 4 mục: *cú pháp yêu cầu*, *phản hồi thành công*, *các mã lỗi có thể trả về*, *ví dụ*.
- [ ] Với mỗi loại thiết bị: bảng key được `SET`, key trả về trong `INFO`, các `ACT` hợp lệ, khoảng giá trị và ràng buộc.
- [ ] Ghi rõ quy tắc dòng: kết thúc `\n` (bỏ qua `\r`), `MAX_LINE = 1024`; key lạ trong phản hồi thì bên nhận bỏ qua.
- [ ] Ghi rõ phản hồi nhiều dòng của `LOG`: dòng đầu `200 OK lines=k`, tiếp theo k dòng, kết thúc bằng một dòng chỉ có `.`.
- [ ] Vẽ 2 sơ đồ tuần tự dạng text: (1) quét rồi thêm vào chuồng; (2) kết nối, SET, INFO, CHPASS.
- [ ] Cả hai đánh dấu "Đã chốt" ở cuối file. Từ đó mọi thay đổi phải ghi vào bảng nhật ký.

## Khung nội dung (điền vào)

### 1. Quy tắc chung

### 2. Mã phản hồi

| Mã | Tên | Khi nào |
|---|---|---|

### 3. Quét thiết bị (UDP, cổng 9999)

### 4. Lệnh TCP

#### 4.x `<TÊN LỆNH>`
- Yêu cầu:
- Thành công:
- Lỗi:
- Ví dụ:

### 5. Thông số theo loại thiết bị

| Loại | Key SET | Mặc định | Ràng buộc | Key INFO | ACT |
|---|---|---|---|---|---|

### 6. Sơ đồ tuần tự

### 7. Nhật ký thay đổi

| Ngày | Người | Thay đổi | Người kia đồng ý |
|---|---|---|---|

**Đã chốt:** Long ☐ · Minh ☐
