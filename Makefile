# =============================================================================
# Makefile: build device_sim và controller
# Người phụ trách: Long  ·  Phase 0 (khung), cập nhật mỗi khi thêm file .c
# Tham khảo: IT4062/1-Chap1/lab00_build/Makefile, IT4062/1-Chap1/Makefile
# =============================================================================
#
# YÊU CẦU (chưa có code, chỉ là danh sách việc cần làm):
#
# [ ] Biến CC (mặc định gcc) và CFLAGS = -std=gnu11 -Wall -Wextra -g -Icommon
# [ ] Ba nhóm nguồn:
#       COMMON_SRC = common/*.c                      (link vào cả 2 chương trình)
#       DEVICE_SRC = device/*.c      -> bin/device_sim
#       CTRL_SRC   = controller/*.c  -> bin/controller
# [ ] Đích mặc định `all`: tạo thư mục bin/ rồi build cả 2 chương trình
# [ ] Biên dịch từng .c thành .o riêng (trong build/) để chỉ dịch lại file đã sửa
# [ ] Sinh phụ thuộc header tự động (cờ -MMD -MP) để sửa .h thì dịch lại đúng file
# [ ] Đích `device` và `controller` để build riêng từng chương trình
# [ ] Đích `run-device`  : chạy bin/device_sim -c config/coop_A.conf
# [ ] Đích `run-device2` : chạy bin/device_sim -c config/coop_B.conf
# [ ] Đích `run-ctrl`    : chạy bin/controller
# [ ] Đích `sanitize`    : build lại với -fsanitize=address (dùng khi phát triển)
# [ ] Đích `valgrind`    : chạy device_sim dưới valgrind --leak-check=full (Phase 4)
# [ ] Đích `clean`       : xoá bin/ và build/ (KHÔNG xoá data/ và logs/)
# [ ] Đích `reset-data`  : xoá nội dung data/ và logs/ (giữ README.md) để thử lại "lần chạy đầu"
# [ ] Khai báo .PHONY cho các đích không phải file
# [ ] Nếu sau này dùng pthread thì thêm -pthread vào LDFLAGS
#
# XONG KHI:
#   - Gõ `make` trong thư mục App/ ra 2 file trong bin/, không có cảnh báo
#   - `touch common/protocol.h && make` dịch lại cả hai chương trình
#   - `make clean && make` vẫn chạy được
