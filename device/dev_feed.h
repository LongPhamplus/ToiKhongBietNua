/*
 * =============================================================================
 * FILE        : device/dev_feed.h
 * PHỤ TRÁCH   : Minh
 * PHASE       : 3 (08/12 – 14/12)
 * TIÊU CHÍ    : 9a máy ăn uống (1đ), 10 (ăn/uống), 11 (FEEDER, WATERER, EGG_COUNTER)
 * =============================================================================
 *
 * CẦN KHAI BÁO (chữ ký theo handlers.h của Long)
 *   [ ] feeder_info / feeder_set / feeder_act / feeder_sched
 *   [ ] waterer_info / waterer_set / waterer_act / waterer_sched
 *       (hai bộ này dùng chung code bên trong; chỉ khác tên key W/Vw, stock/tank, kg/l)
 *   [ ] egg_info / egg_set / egg_act
 *   [ ] feed_register(): đăng ký 3 bộ hàm vào bảng của handlers.c.
 *   [ ] feed_tick(): main.c gọi mỗi tick.
 * =============================================================================
 */
