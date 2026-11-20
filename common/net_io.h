/*
 * =============================================================================
 * FILE        : common/net_io.h
 * PHỤ TRÁCH   : Long        · NGƯỜI DÙNG: Long (device/server.c), Minh (controller/conn.c)
 * PHASE       : 1 (bàn giao H2 cho Minh hạn 26/11)
 * TIÊU CHÍ    : 1. Xử lý truyền dòng (1 điểm)
 * =============================================================================
 *
 * MỤC ĐÍCH
 *   Khai báo giao diện cho tầng vào/ra TCP theo DÒNG. TCP là luồng byte, không
 *   giữ ranh giới thông điệp, nên phải tự cắt theo '\n'.
 *
 * CẦN KHAI BÁO
 *   [ ] struct linebuf: mảng char cỡ (2 * MAX_LINE), số byte đang có,
 *       cờ "đang bỏ qua dòng quá dài".
 *   [ ] linebuf_init(lb): đặt rỗng.
 *   [ ] linebuf_fill(lb, fd): gọi recv MỘT lần, nối vào cuối bộ đệm.
 *       Trả về: >0 số byte nhận, 0 nếu bên kia đóng kết nối, -1 nếu lỗi.
 *       (Server gọi sau khi select() báo fd sẵn sàng, nên không bị chặn.)
 *   [ ] linebuf_next(lb, out, outsz): lấy MỘT dòng hoàn chỉnh (bỏ '\n' và '\r')
 *       ra out. Trả về: 1 có dòng, 0 chưa đủ dòng, -1 dòng quá dài (đã bỏ).
 *       Phải gọi lặp đến khi trả về 0 vì một lần recv có thể chứa nhiều dòng.
 *   [ ] read_line_blocking(fd, lb, out, outsz, timeout_ms): dùng cho CLIENT:
 *       lặp {linebuf_next, nếu chưa có thì select có timeout rồi linebuf_fill}.
 *       Trả về 1 / 0 (đóng kết nối) / -1 (lỗi) / -2 (hết thời gian).
 *   [ ] send_all(fd, buf, len): gửi đến hết, trả về 0 / -1.
 *   [ ] send_line(fd, chuỗi): gửi chuỗi + '\n' bằng send_all.
 *   [ ] send_linef(fd, fmt, ...): như printf, rồi gửi bằng send_line (giới hạn MAX_LINE).
 *
 * THAM KHẢO
 *   IT4062/4-Chap4/http_server.c: vòng recv đến khi gặp "\r\n\r\n"
 *   IT4062/1-Chap1/lab10_memory.c: memmove
 * =============================================================================
 */
