/*
 * =============================================================================
 * FILE        : common/net_io.c
 * PHỤ TRÁCH   : Long
 * PHASE       : 1 (24/11 – 26/11)
 * TIÊU CHÍ    : 1. Xử lý truyền dòng (1 điểm)
 * =============================================================================
 *
 * MỤC ĐÍCH
 *   Cài đặt các hàm khai báo trong net_io.h. Đây là phần thầy chấm "xử lý truyền
 *   dòng": chương trình phải đúng dù dữ liệu TCP đến bị CẮT VỤN hoặc bị DỒN.
 *
 * VIỆC CẦN LÀM
 *   [ ] linebuf_fill:
 *         - recv vào phần trống còn lại của bộ đệm (sizeof - len).
 *         - Nếu bộ đệm đã đầy mà vẫn chưa có '\n': bật cờ "bỏ qua", xoá bộ đệm.
 *           Từ đó mọi byte đều bị vứt cho đến '\n' kế tiếp, rồi báo -1 MỘT lần
 *           để server trả "400 LINE_TOO_LONG".
 *         - recv trả về -1 với errno = EINTR: thử lại; EAGAIN: coi như 0 byte mới.
 *   [ ] linebuf_next:
 *         - Tìm '\n' trong [0, len) (dùng memchr).
 *         - Không có thì trả về 0.
 *         - Có thì chép phần trước nó ra out (cắt nếu dài hơn outsz), bỏ '\r' cuối,
 *           thêm '\0'. Dồn phần còn lại về đầu bộ đệm bằng memmove, giảm len.
 *         - Chú ý: dữ liệu có thể chứa byte 0 (rác nhị phân), nên KHÔNG dùng
 *           strlen/strchr trên bộ đệm thô, phải dùng len và memchr.
 *   [ ] read_line_blocking: vòng lặp như mô tả trong .h. Tính thời gian còn lại
 *       và ĐẶT LẠI struct timeval MỖI VÒNG trước khi gọi select (Linux sửa nó).
 *   [ ] send_all: lặp send(fd, buf+sent, len-sent, MSG_NOSIGNAL nếu có);
 *       EINTR thì thử lại; lỗi khác thì trả -1.
 *   [ ] send_line / send_linef: dùng vsnprintf có giới hạn, sau đó send_all.
 *
 * LỖI CẦN TRÁNH (thầy hay hỏi)
 *   - Gọi recv một lần rồi coi như đủ thông điệp (lỗi của IT4062/2-Chap2/tcp_server.c).
 *   - Gọi send một lần rồi coi như đã gửi hết.
 *   - Quên '\0' khi in bộ đệm, dẫn tới in ra rác.
 *
 * KIỂM THỬ (tests/README.md mục "Truyền dòng")
 *   [ ] Gửi "HEL", chờ 1s, rồi gửi "LO\n": nhận đúng 1 dòng "HELLO".
 *   [ ] Gửi "A\nB\nC\n" trong 1 lần: nhận đúng 3 dòng.
 *   [ ] Gửi "A\r\n": nhận "A" (không có \r).
 *   [ ] Gửi 5000 byte không có '\n', rồi "HELLO\n": báo lỗi 1 lần, sau đó HELLO vẫn chạy.
 *   [ ] Kết nối bị đóng giữa chừng một dòng: trả về 0, không crash.
 * =============================================================================
 */
