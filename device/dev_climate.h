/*
 * =============================================================================
 * FILE        : device/dev_climate.h
 * PHỤ TRÁCH   : Long
 * PHASE       : 3 (08/12 – 14/12)
 * TIÊU CHÍ    : 9b quạt (1đ), 9c đèn sưởi (1đ), 10 (phần quạt/đèn), 11 (ENV_SENSOR, FAN, HEATER)
 * =============================================================================
 *
 * CẦN KHAI BÁO
 *   [ ] Bộ hàm theo loại (đúng chữ ký đã định nghĩa trong handlers.h):
 *         env_sensor_info / env_sensor_set
 *         fan_info / fan_set / fan_act
 *         heater_info / heater_set / heater_act
 *   [ ] climate_register(): đăng ký 3 bộ hàm vào bảng của handlers.c.
 *   [ ] climate_tick(): main.c gọi mỗi tick (đo cảm biến, quạt, đèn theo ngưỡng).
 * =============================================================================
 */
