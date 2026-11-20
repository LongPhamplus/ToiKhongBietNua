/*
 * device/discovery.h: trả lời gói quét UDP (Minh, tuần 2). TIÊU CHÍ 5.
 *
 * Giao diện này do Long định nghĩa sẵn ở tuần 1 (bàn giao H5) để main.c gọi đúng chỗ.
 * Tuần 2 Minh chỉ thay phần thân trong discovery.c, KHÔNG phải sửa main.c.
 */
#ifndef DISCOVERY_H
#define DISCOVERY_H

/* Mở socket UDP nghe gói SCAN. Trả fd, hoặc -1 nếu chưa hỗ trợ / lỗi. */
int  discovery_open(int udp_port);

/* main.c gọi khi select() báo fd sẵn sàng: đọc MỘT datagram rồi trả lời. */
void discovery_handle(int fd);

void discovery_close(int fd);

#endif /* DISCOVERY_H */
