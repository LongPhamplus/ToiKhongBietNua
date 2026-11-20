/*
 * tests/test_util.c: kiểm thử common/util.c   (chạy: make test)
 */
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include "test.h"
#include "util.h"

static int is_hex(const char *s)
{
    for (; *s; s++)
        if (!((*s >= '0' && *s <= '9') || (*s >= 'a' && *s <= 'f')))
            return 0;
    return 1;
}

int main(void)
{
    /* random_hex: 1000 giá trị 4 byte (8 hex) đều đúng định dạng và không trùng nhau */
    enum { N = 1000 };
    static char vals[N][9];
    int ok_fmt = 1, dup = 0;
    for (int i = 0; i < N; i++) {
        random_hex(vals[i], 4);
        if (strlen(vals[i]) != 8 || !is_hex(vals[i]))
            ok_fmt = 0;
        for (int j = 0; j < i && !dup; j++)
            if (strcmp(vals[i], vals[j]) == 0)
                dup = 1;
    }
    CHECK(ok_fmt, "random_hex: dung 8 ky tu, chi gom 0-9a-f");
    CHECK(!dup, "random_hex: 1000 gia tri khong trung");

    char tok[33];
    random_hex(tok, 16);
    CHECK(strlen(tok) == 32, "random_hex(16) -> token 32 ky tu");

    char s1[] = "  abc \r\n";
    CHECK(strcmp(trim(s1), "abc") == 0, "trim(\"  abc \\r\\n\") == \"abc\"");
    char s2[] = "   ";
    CHECK(strcmp(trim(s2), "") == 0, "trim chuoi toan khoang trang -> rong");

    char small[5];
    safe_copy(small, sizeof small, "abcdefgh");
    CHECK(strcmp(small, "abcd") == 0, "safe_copy cat dung khi src dai hon dst");

    char up[] = "fan-8a21";
    str_upper(up);
    CHECK(strcmp(up, "FAN-8A21") == 0, "str_upper");

    const char *dir = "build/test_tmp_dir";
    rmdir(dir);
    CHECK(ensure_dir(dir) == 0, "ensure_dir tao duoc thu muc");
    CHECK(ensure_dir(dir) == 0, "ensure_dir goi lan 2 khong loi");
    rmdir(dir);

    long long t0 = now_ms();
    usleep(50 * 1000);
    long long dt = now_ms() - t0;
    CHECK(dt >= 45 && dt < 500, "now_ms do duoc khoang 50ms");

    return TEST_DONE();
}
