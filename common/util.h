/*
 * =============================================================================
 * FILE        : common/util.h
 * PHỤ TRÁCH   : Long        · NGƯỜI DÙNG: cả hai
 * PHASE       : 1 (bàn giao kèm H2, hạn 26/11)
 * TIÊU CHÍ    : hỗ trợ 3 (token), 4 (ID thiết bị), App ID của controller, 12 (thời gian log)
 * =============================================================================
 *
 * CẦN KHAI BÁO
 *   [ ] random_hex(out, nbytes): sinh 2*nbytes ký tự hex ngẫu nhiên + '\0'.
 *       Dùng cho: token (16 byte = 32 hex), App ID (4 byte = 8 hex),
 *       hậu tố ID thiết bị (2 byte = 4 hex).
 *   [ ] now_ms(): thời gian hiện tại tính bằng mili giây (cho timeout, đo tick).
 *   [ ] now_str(buf, sz): "YYYY-MM-DD HH:MM:SS" theo giờ địa phương (cho log).
 *   [ ] trim(str): bỏ dấu cách / \r / \n ở hai đầu, sửa trực tiếp.
 *   [ ] ensure_dir(path): tạo thư mục nếu chưa có (cho data/, logs/).
 *   [ ] safe_copy(dst, dstsz, src): chép có giới hạn, luôn kết thúc bằng '\0'.
 *   [ ] die(msg): perror + exit, chỉ dùng lúc khởi động (bind lỗi, ...).
 * =============================================================================
 */
