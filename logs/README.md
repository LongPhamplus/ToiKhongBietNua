# logs/: log hoạt động từng thiết bị (KHÔNG đưa lên Git, trừ file này)

- Mỗi thiết bị một file `logs/<device_id>.log`, do `device/dlog.c` (Long) ghi.
- Định dạng một dòng: `thời gian thực | giờ mô phỏng | MỨC | nguồn (app=... hoặc auto) | nội dung`
- Những sự kiện bắt buộc phải ghi: xem KE_HOACH.md mục 4.6.
- Tiêu chí điểm 12 (1 điểm). Khi demo, mở file này song song với controller.
