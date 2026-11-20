/*
 * =============================================================================
 * FILE        : device/handlers.c
 * PHỤ TRÁCH   : Long
 * PHASE       : 1 → 4 (làm dần)
 * TIÊU CHÍ    : 3, 6, 7, 8 (POWER), 9 (SET/SCHED), 10 (MODE/ACT), 11 (INFO), 12 (LOG)
 * =============================================================================
 *
 * MỤC ĐÍCH
 *   Bảng điều phối lệnh: tên lệnh ánh xạ tới hàm xử lý (giống IT4062/1-Chap1/lab05_function_pointer.c).
 *   Các handler ở đây chỉ làm phần CHUNG (kiểm tra tham số, quyền, ghi log),
 *   phần riêng theo loại thiết bị thì gọi qua bảng hàm theo loại.
 *
 * VIỆC CẦN LÀM
 *   [ ] handlers_dispatch: chép line ra bản sao, split_words (kv.c), tra bảng
 *       {tên, hàm, cần_token?}. Không thấy thì "400 BAD_REQUEST unknown_command".
 *   [ ] Lệnh cần token: gọi auth_authorize(argv[1], argv[2], &dev) (auth.c).
 *       Nếu lỗi thì trả ngay 404/403. Từ argv[3] trở đi là tham số riêng.
 *
 *   PHASE 1
 *   [ ] HELLO : "200 OK devices=<n> server=device_sim/1.0"
 *   [ ] BYE   : "200 OK" + đánh dấu client cần đóng.
 *   [ ] INFO  : (tạm chưa kiểm tra token) "200 OK id= type= name= power= mode="
 *               + phần riêng do hàm info theo loại thêm vào.
 *
 *   PHASE 2 (cùng auth.c)
 *   [ ] CONNECT <id> <app_id> <pass>: gọi auth_connect() rồi trả token.
 *   [ ] CHPASS  <id> <token> <old> <new>: gọi auth_change_password().
 *   [ ] DISCONNECT <id> <token>: auth_revoke().
 *   [ ] Bật kiểm tra token cho INFO.
 *
 *   PHASE 3
 *   [ ] POWER <id> <token> ON|OFF: (tiêu chí 8) đổi dev->power; nếu OFF thì tắt
 *       mọi trạng thái running; log "POWER ON->OFF by app=...".
 *       Sai giá trị thì 400.
 *   [ ] MODE <id> <token> AUTO|MANUAL: (tiêu chí 10) ENV_SENSOR/EGG_COUNTER
 *       trả 405. Log thay đổi.
 *   [ ] SET <id> <token> k=v ...: (tiêu chí 9) gọi hàm set theo loại; sau khi
 *       thành công thì devices_save() và log từng key "cũ -> mới".
 *       Ngoài ra luôn cho phép SET name=<tên> cho mọi loại (xử lý ngay tại đây).
 *   [ ] ACT <id> <token> <action> ...: (tiêu chí 10) nếu power OFF thì
 *       trả "409 POWER_OFF". Mã này chưa có trong KE_HOACH, cần bổ sung vào PROTOCOL.md.
 *       Còn lại gọi hàm act theo loại.
 *   [ ] SCHED <id> <token> LIST|ADD|DEL ...: gọi hàm sched theo loại (Minh viết).
 *
 *   PHASE 4
 *   [ ] LOG <id> <token> [n]: n mặc định 20, tối đa 200. Đọc n dòng cuối của
 *       logs/<id>.log (dlog_tail trong dlog.c), trả "200 OK lines=k" rồi
 *       k dòng rồi ".". Nếu một dòng log đúng bằng "." thì đổi thành ".."
 *       (ghi chú trong PROTOCOL.md).
 *
 * QUY ƯỚC PHẢN HỒI
 *   - Luôn đúng MỘT dòng (trừ LOG), không quá MAX_LINE.
 *   - Lỗi tham số số học thì "422 INVALID_PARAM <key> <lý do ngắn không dấu cách>".
 *
 * KIỂM THỬ: dùng nc gõ tay từng lệnh trong PROTOCOL.md, cả trường hợp đúng và sai.
 * =============================================================================
 */
