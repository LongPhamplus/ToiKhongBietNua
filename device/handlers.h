/*
 * =============================================================================
 * FILE        : device/handlers.h
 * PHỤ TRÁCH   : Long
 * PHASE       : 1 (khung), 2 (CONNECT/CHPASS/DISCONNECT), 3 (POWER/MODE/SET/ACT/SCHED), 4 (LOG)
 * TIÊU CHÍ    : 3, 6, 7, 8, 9, 10, 11, 12
 * =============================================================================
 *
 * CẦN KHAI BÁO
 *   [ ] Kiểu con trỏ hàm xử lý lệnh:
 *         (client*, argc, argv[], resp, respsz) trả về int
 *   [ ] handlers_dispatch(client*, line, resp, respsz): điểm vào duy nhất mà server.c gọi.
 *
 *   [ ] BẢNG HÀM THEO LOẠI THIẾT BỊ (bàn giao P3.0a cho Minh, hạn 09/12).
 *       Mỗi loại thiết bị đăng ký một bộ hàm:
 *         info(dev, resp, sz)            : ghi các key=value của INFO
 *         set(dev, argc, argv, resp, sz) : áp dụng SET, trả mã 200/405/422
 *         act(dev, action, argc, argv, resp, sz) : ACT, trả 200/405/422
 *         sched(dev, argc, argv, resp, sz) : chỉ FEEDER/WATERER, loại khác là NULL
 *       Long đăng ký ENV_SENSOR/FAN/HEATER (dev_climate.h).
 *       Minh đăng ký FEEDER/WATERER/EGG_COUNTER (dev_feed.h), MISTER (dev_mister.h).
 *       Hàm NULL nghĩa là lệnh đó không hỗ trợ, trả 405 NOT_SUPPORTED.
 * =============================================================================
 */
