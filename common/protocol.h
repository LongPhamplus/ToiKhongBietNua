/*
 * common/protocol.h: hằng số của giao thức (dùng chung cho device_sim và controller)
 * Phụ trách: Long + Minh (sửa phải báo người kia)
 *
 * TẠI SAO CÓ FILE NÀY
 *   Mọi con số / chuỗi mà HAI chương trình phải hiểu giống nhau (cổng, độ dài tối đa,
 *   mã lỗi, tên lệnh, kiểu thiết bị) chỉ được định nghĩa ở đây. Nếu viết cứng "7000"
 *   ở nhiều chỗ, sửa một chỗ mà quên chỗ khác thì hai bên sẽ "lệch" nhau.
 *   File này chỉ chứa #define và enum: không có hàm, không có biến.
 */
#ifndef PROTOCOL_H
#define PROTOCOL_H

/* ---- Phiên bản & cổng mặc định ---------------------------------------- */
#define PROTO_VERSION      "device_sim/1.0"
#define DEFAULT_UDP_PORT   9999   /* cổng nghe gói quét SCAN (tuần 2)           */
#define DEFAULT_TCP_PORT   7000   /* cổng điều khiển                            */

/* ---- Giới hạn độ dài (đều tính cả '\0' trừ khi ghi khác) ----------------- */
#define MAX_LINE        1024   /* một dòng giao thức tối đa 1023 ký tự + '\n'   */
#define MAX_ID            16   /* "FAN-8A21"                                     */
#define MAX_NAME          32   /* tên thiết bị / chuồng                          */
#define MAX_PASS          32   /* độ dài tối đa mật khẩu (không tính '\0')       */
#define MIN_PASS           4
#define TOKEN_HEX_LEN     32   /* token = 16 byte ngẫu nhiên = 32 ký tự hex      */
#define APP_ID_HEX_LEN     8   /* App ID = 4 byte ngẫu nhiên = 8 ký tự hex       */

/* ---- Giới hạn số lượng (dùng để khai báo mảng tĩnh) ---------------------- */
#define MAX_CLIENTS       64   /* số kết nối TCP đồng thời của một device_sim   */
#define MAX_DEVICES       32   /* số thiết bị trong một device_sim              */
#define MAX_TOKENS_PER_DEV 8
#define MAX_SCHED          8   /* số mốc lịch cho ăn/uống                        */
#define MAX_ARGS          32   /* số từ tối đa trong một dòng lệnh               */

/* ---- Thời gian ---------------------------------------------------------- */
#define TICK_MS         1000   /* nhịp mô phỏng: 1 giây thực                     */
#define REQ_TIMEOUT_MS  3000   /* client chờ phản hồi tối đa 3 giây              */
#define SCAN_WAIT_MS    2000   /* thời gian gom phản hồi quét                    */
#define TOKEN_TTL_SEC   1800   /* token hết hạn sau 30 phút không dùng           */
#define LOCK_FAILS         5   /* sai mật khẩu 5 lần liên tiếp → khoá            */
#define LOCK_SEC          60

/* ---- Tên lệnh ----------------------------------------------------------- */
#define CMD_SCAN       "SCAN"
#define CMD_DEVICE     "DEVICE"
#define CMD_HELLO      "HELLO"
#define CMD_CONNECT    "CONNECT"
#define CMD_INFO       "INFO"
#define CMD_POWER      "POWER"
#define CMD_SET        "SET"
#define CMD_MODE       "MODE"
#define CMD_ACT        "ACT"
#define CMD_SCHED      "SCHED"
#define CMD_CHPASS     "CHPASS"
#define CMD_LOG        "LOG"
#define CMD_DISCONNECT "DISCONNECT"
#define CMD_BYE        "BYE"

/* Phản hồi nhiều dòng (lệnh LOG) kết thúc bằng một dòng chỉ có dấu chấm */
#define MULTILINE_END  "."

/* ---- Mã phản hồi (số đứng đầu mỗi dòng phản hồi TCP) -------------------- */
#define RC_OK            200
#define RC_BAD_REQUEST   400
#define RC_AUTH_FAILED   401
#define RC_INVALID_TOKEN 403
#define RC_NO_SUCH_DEVICE 404
#define RC_NOT_SUPPORTED 405
#define RC_POWER_OFF     409
#define RC_INVALID_PARAM 422
#define RC_LOCKED        423
#define RC_SERVER_ERROR  500

/* ---- Kiểu thiết bị ------------------------------------------------------ */
typedef enum {
    DEV_ENV_SENSOR = 0,  /* tiền tố ID: ENV- */
    DEV_EGG_COUNTER,     /* EGG- */
    DEV_FEEDER,          /* FED- */
    DEV_WATERER,         /* WAT- */
    DEV_FAN,             /* FAN- */
    DEV_HEATER,          /* HEA- */
    DEV_MISTER,          /* MIS- */
    DEV_TYPE_COUNT,
    DEV_INVALID = -1
} dev_type_t;

typedef enum { PWR_OFF = 0, PWR_ON = 1 } power_t;

/* Không đặt tên mode_t vì trùng kiểu mode_t của <sys/types.h> */
typedef enum { MODE_AUTO = 0, MODE_MANUAL = 1 } dev_mode_t;

#endif /* PROTOCOL_H */
