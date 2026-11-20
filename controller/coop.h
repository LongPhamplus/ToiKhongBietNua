/*
 * controller/coop.h: quản lý chuồng nuôi (Minh). TIÊU CHÍ 13.
 *
 * Chuồng là cách NGƯỜI DÙNG tổ chức thiết bị (chuồng A có quạt nào, máy ăn nào).
 * Thông tin chuồng thuộc phía người dùng nên lưu ở controller (<data>/coops.txt),
 * không lưu ở thiết bị.
 */
#ifndef COOP_H
#define COOP_H

#include <stddef.h>
#include "protocol.h"

/* Thông tin để TÌM LẠI và KẾT NỐI thiết bị. Trạng thái kết nối nằm ở conn.c. */
typedef struct {
    char       id[MAX_ID];
    dev_type_t type;
    char       ip[64];
    int        port;
    char       name[MAX_NAME];
} coop_device_t;

typedef struct {
    int            id;
    char           name[MAX_NAME];
    char           location[MAX_NAME];
    int            capacity;     /* số gà tối đa */
    int            chickens;     /* số gà hiện có */
    char           created[11];  /* YYYY-MM-DD */
    coop_device_t *devs;         /* mảng động */
    int            n_dev, cap_dev;
} coop_t;

int     coop_load(const char *data_dir);   /* trả 0 / -1 */
int     coop_save(void);
void    coop_free_all(void);

int     coop_count(void);
coop_t *coop_at(int i);
coop_t *coop_find(int id);

/* Trả id chuồng mới, hoặc -1 kèm err */
int     coop_add(const char *name, const char *location, int capacity, int chickens,
                 char *err, size_t errsz);
int     coop_edit(int id, const char *name, const char *location, int capacity, int chickens,
                  char *err, size_t errsz);
int     coop_remove(int id);

/* Một thiết bị chỉ được thuộc MỘT chuồng. Trả 0, hoặc -1 kèm err. */
int     coop_add_device(coop_t *c, const coop_device_t *d, char *err, size_t errsz);
int     coop_remove_device(coop_t *c, const char *device_id);

/* Tìm thiết bị thuộc chuồng nào (tuần 2: đánh dấu "đã thêm" khi quét). */
coop_t *coop_find_device(const char *device_id, coop_device_t **out);

void    coop_menu(void);

#endif /* COOP_H */
