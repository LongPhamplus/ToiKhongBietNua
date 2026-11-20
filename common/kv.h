/*
 * =============================================================================
 * FILE        : common/kv.h
 * PHỤ TRÁCH   : Minh        · NGƯỜI DÙNG: cả hai (handlers, device, conn, coop, ui_*)
 * PHASE       : 1 (bàn giao H3 cho Long hạn 26/11)
 * TIÊU CHÍ    : hỗ trợ 9, 11 (đọc/ghi thông số), 13 (file chuồng)
 * =============================================================================
 *
 * MỤC ĐÍCH
 *   Phân tích và tạo các dòng dạng:   LỆNH arg1 arg2 key1=val1 key2=val2
 *   Dùng chung cho thông điệp mạng VÀ cho file cấu hình/dữ liệu (cùng định dạng).
 *
 * CẦN KHAI BÁO
 *   [ ] split_words(line, argv[], max): tách tại dấu cách (bỏ dấu cách thừa),
 *       SỬA TRỰC TIẾP trên line, trả về số từ. Vượt max thì trả -1.
 *   [ ] kv_find(argc, argv, key): tìm từ có dạng "key=...", trả con trỏ tới
 *       phần giá trị hoặc NULL.
 *   [ ] kv_get_str(argc, argv, key, out, outsz), trả 0 / -1
 *   [ ] kv_get_double(argc, argv, key, &out), trả 0 / -1 (không có) / -2 (không phải số)
 *   [ ] kv_get_int(...): tương tự
 *   [ ] kv_split_pair(word, &key, &val): tách "a=b" tại dấu '=' đầu tiên
 *   [ ] kv_append(buf, bufsz, key, fmt, ...): nối " key=value" vào buf an toàn,
 *       dùng để server xây phản hồi INFO và controller xây lệnh SET.
 *   [ ] parse_double_strict(str, &out) và parse_int_strict: dùng strtod/strtol
 *       và báo lỗi nếu còn ký tự thừa ("12abc") hoặc chuỗi rỗng.
 *   [ ] parse_hhmm(str, &minute_of_day) / format_hhmm(minute, buf):
 *       "06:30" <-> 390. Dùng cho lịch máy ăn/uống (cả 2 phía).
 *   [ ] dev_type_from_str / dev_type_to_str / dev_type_prefix (ENV-, FAN-...)
 *   [ ] resp_code(line): đọc mã số ở đầu dòng phản hồi ("200 OK ..." thì 200)
 *
 * THAM KHẢO
 *   IT4062/1-Chap1/lab09_string.c: strtok_r, sao chép an toàn
 * =============================================================================
 */
