/*
 * =============================================================================
 * FILE        : common/util.c
 * PHỤ TRÁCH   : Long
 * PHASE       : 1 (24/11 – 26/11)
 * =============================================================================
 *
 * VIỆC CẦN LÀM
 *   [ ] random_hex: open("/dev/urandom", O_RDONLY) rồi read đủ nbytes (lặp nếu
 *       đọc thiếu), close; đổi từng byte sang 2 ký tự hex.
 *       Nếu không mở được /dev/urandom (hiếm) thì dùng rand() đã srand bằng
 *       time ^ getpid MỘT lần duy nhất, và in cảnh báo ra stderr.
 *       Tham khảo IT4062/1-Chap1/lab08_lowlevel_io.c (open/read/close).
 *   [ ] now_ms: clock_gettime(CLOCK_MONOTONIC) cho khoảng thời gian;
 *       now_str: time + localtime_r + strftime.
 *   [ ] ensure_dir: stat(); nếu không tồn tại thì mkdir(path, 0755).
 *       Tham khảo IT4062/1-Chap1/lab11_filesystem.c.
 *   [ ] trim, safe_copy: không dùng strcpy.
 *
 * TRẢ LỜI VẤN ĐÁP
 *   Vì sao token lấy từ /dev/urandom chứ không dùng rand()?
 *   Vì rand() đoán được nếu biết seed (thời gian), còn /dev/urandom là nguồn
 *   ngẫu nhiên an toàn mật mã do kernel cung cấp.
 *
 * KIỂM THỬ
 *   [ ] Gọi random_hex 1000 lần: không có giá trị trùng, độ dài đúng, chỉ gồm 0-9a-f.
 * =============================================================================
 */
