/*
 * tests/test.h: "framework" kiểm thử tối giản dùng chung cho tests/test_*.c
 *
 * CHECK(điều_kiện, "mô tả") in [PASS]/[FAIL]. TEST_DONE() trả mã thoát
 * khác 0 nếu có ít nhất một [FAIL], nhờ đó `make test` dừng lại khi có lỗi.
 */
#ifndef TEST_H
#define TEST_H

#include <stdio.h>

static int g_test_fail = 0;

#define CHECK(cond, desc)                                                   \
    do {                                                                    \
        if (cond) {                                                         \
            printf("[PASS] %s\n", desc);                                    \
        } else {                                                            \
            printf("[FAIL] %s   (%s:%d)\n", desc, __FILE__, __LINE__);      \
            g_test_fail++;                                                  \
        }                                                                   \
    } while (0)

#define TEST_DONE() (g_test_fail ? (printf("==> %d test FAIL\n", g_test_fail), 1) : 0)

#endif
