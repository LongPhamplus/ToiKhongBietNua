/*
 * device/main.c: chương trình device_sim (Long)
 * Bước 1: tạm in một dòng để kiểm tra Makefile build được. Vòng select() làm ở bước 7.
 */
#include <stdio.h>
#include "protocol.h"
#include "device.h"

int main(void)
{
    printf("%s (khung): chua co gi\n", PROTO_VERSION);
    return 0;
}
