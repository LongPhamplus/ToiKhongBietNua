/*
 * device/discovery.c: trả lời gói quét UDP   (Minh, TUẦN 2) · TIÊU CHÍ 5
 *
 * >>> TUẦN 1: BẢN GIẢ (stub) do Long viết để main.c gọi sẵn đúng giao diện. <<<
 *     discovery_open() trả -1, nên main.c bỏ qua UDP. Mọi thứ khác chạy bình thường.
 *
 * VIỆC MINH LÀM Ở TUẦN 2 (thay thân các hàm dưới, không sửa main.c):
 *   [ ] discovery_open: socket(AF_INET, SOCK_DGRAM); setsockopt SO_REUSEADDR = 1
 *       và SO_REUSEPORT = 1 (TRƯỚC bind, để coop_A và coop_B cùng bind được 9999);
 *       bind INADDR_ANY:udp_port. Tham khảo IT4062/2-Chap2/udp_receiver.c
 *   [ ] discovery_handle: recvfrom; đúng "SCAN <app_id>" thì với MỖI thiết bị
 *       sendto "DEVICE id=.. type=.. port=<tcp_port> name=.. power=.." về địa chỉ nguồn;
 *       dlog(dev, DLOG_INFO, app_id, "scanned from <ip>"). Gói sai định dạng thì bỏ qua.
 *   [ ] Kiểm thử: echo "SCAN 1234abcd" | nc -u -w1 127.0.0.1 9999 phải nhận 7 dòng DEVICE.
 */
#include "discovery.h"

#include <unistd.h>

int discovery_open(int udp_port)
{
    (void)udp_port;
    return -1;              /* tuần 1: chưa hỗ trợ quét */
}

void discovery_handle(int fd)
{
    (void)fd;
}

void discovery_close(int fd)
{
    if (fd >= 0)
        close(fd);
}
