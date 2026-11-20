#!/usr/bin/env python3
"""
tests/fake_device.py: GIẢ LÀM THIẾT BỊ để thử controller khi chưa có device_sim (thay cho nc -l).
Nghe 127.0.0.1:7100, mỗi kết nối chạy một kịch bản:
  1) trả "200 OK hi" cho lệnh đầu, lệnh thứ hai im lặng 4 giây  -> controller báo "khong phan hoi"
  2) nhận lệnh rồi đóng ngay                                    -> "mat ket noi"
  3) trả "xin chao" (không có mã)                              -> "phan hoi sai dinh dang"
Dùng: python3 tests/fake_device.py   rồi trong controller chọn 5, Port 7100
"""

import socket, time
srv = socket.socket(); srv.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
srv.bind(("127.0.0.1", 7100)); srv.listen()
# Kết nối 1: trả lời 1 lệnh, lệnh 2 im lặng (để test timeout)
c, _ = srv.accept(); f = c.makefile("rb")
f.readline(); c.sendall(b"200 OK hi\n")
f.readline(); time.sleep(4); c.close()
# Kết nối 2: đóng ngay sau khi nhận lệnh (để test mất kết nối)
c, _ = srv.accept(); c.makefile("rb").readline(); c.close()
# Kết nối 3: trả rác
c, _ = srv.accept(); c.makefile("rb").readline(); c.sendall(b"xin chao\n"); time.sleep(0.5); c.close()
