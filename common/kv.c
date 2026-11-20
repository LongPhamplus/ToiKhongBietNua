/*
 * =============================================================================
 * FILE        : common/kv.c
 * PHỤ TRÁCH   : Minh
 * PHASE       : 1 (24/11 – 26/11)
 * TIÊU CHÍ    : hỗ trợ 9, 11, 13
 * =============================================================================
 *
 * VIỆC CẦN LÀM
 *   [ ] split_words: dùng strtok_r với dấu phân cách " \t" (không dùng strtok vì
 *       không an toàn khi lồng nhau). Dừng và báo lỗi khi vượt max.
 *   [ ] kv_find: so sánh tiền tố "key=" bằng strncmp với độ dài key. Lưu ý
 *       "Tp1" không được khớp nhầm "Tp10=...", nên phải kiểm tra ký tự ngay sau
 *       key là '='.
 *   [ ] parse_double_strict: strtod + kiểm tra endptr trỏ tới '\0' + errno ERANGE.
 *       Chấp nhận "28", "28.5"; từ chối "", "abc", "28,5", "28.5x".
 *   [ ] parse_hhmm: đúng định dạng 2 chữ số ':' 2 chữ số, 00 <= hh <= 23,
 *       00 <= mm <= 59. Kết quả là số phút trong ngày (0..1439).
 *   [ ] kv_append: snprintf vào phần còn trống; nếu bị cắt thì trả -1 để người gọi
 *       biết phản hồi quá dài (không được vượt MAX_LINE).
 *   [ ] Bảng chuyển đổi kiểu thiết bị: một mảng hằng {enum, "FAN", "FAN-"}
 *       và tra cứu tuyến tính.
 *
 * KIỂM THỬ (tạm viết một main thử riêng, không đưa vào bin/)
 *   [ ] "SET FAN-1 tok Tmax=31 Tp1=27" tách ra 5 từ, đọc được Tmax=31.0, Tp1=27.0.
 *   [ ] "  nhiều   dấu   cách " tách ra 3 từ.
 *   [ ] kv_get_double với "Tmax=abc" trả -2.
 *   [ ] parse_hhmm: "06:00" = 360; "24:00", "6:00", "06-00" bị từ chối.
 *   [ ] dev_type_from_str("XYZ") trả giá trị không hợp lệ.
 * =============================================================================
 */
