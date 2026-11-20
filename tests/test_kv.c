/*
 * tests/test_kv.c: kiểm thử common/kv.c
 */
#include <string.h>
#include "kv.h"
#include "test.h"

int main(void)
{
    char *argv[MAX_ARGS];
    double d;
    int i;

    char l1[] = "SET FAN-1 tok Tmax=31 Tp1=27";
    int n = split_words(l1, argv, MAX_ARGS);
    CHECK(n == 5 && strcmp(argv[0], "SET") == 0 && strcmp(argv[4], "Tp1=27") == 0,
          "split_words(\"SET FAN-1 tok Tmax=31 Tp1=27\") -> 5 tu");
    CHECK(kv_get_double(n, argv, "Tmax", &d) == 0 && d == 31.0, "kv_get_double Tmax -> 31.0");
    CHECK(kv_get_double(n, argv, "Tp1", &d) == 0 && d == 27.0, "kv_get_double Tp1 -> 27.0");
    CHECK(kv_get_double(n, argv, "Hmin", &d) == -1, "key khong co -> -1");

    char l2[] = "   nhieu    dau   cach  ";
    CHECK(split_words(l2, argv, MAX_ARGS) == 3, "split_words bo qua dau cach thua -> 3 tu");

    char l3[] = "a b c d";
    CHECK(split_words(l3, argv, 3) == -1, "split_words qua max tu -> -1");

    char l4[] = "X Tp10=5 Tp1=7";
    n = split_words(l4, argv, MAX_ARGS);
    CHECK(kv_get_int(n, argv, "Tp1", &i) == 0 && i == 7, "kv_find(\"Tp1\") khong khop nham \"Tp10=5\"");

    char l5[] = "X Tmax=abc";
    n = split_words(l5, argv, MAX_ARGS);
    CHECK(kv_get_double(n, argv, "Tmax", &d) == -2, "kv_get_double(\"Tmax=abc\") -> -2");

    char name[8];
    char l6[] = "X name=Quat_rat_dai";
    n = split_words(l6, argv, MAX_ARGS);
    CHECK(kv_get_str(n, argv, "name", name, sizeof name) == -2, "kv_get_str qua dai -> -2");

    CHECK(parse_double_strict("28.5", &d) == 0 && d == 28.5, "parse_double_strict(\"28.5\")");
    CHECK(parse_double_strict("", &d) == -1 && parse_double_strict("abc", &d) == -1 &&
          parse_double_strict("28,5", &d) == -1 && parse_double_strict("28.5x", &d) == -1 &&
          parse_double_strict("nan", &d) == -1,
          "parse_double_strict tu choi \"\", \"abc\", \"28,5\", \"28.5x\", \"nan\"");
    CHECK(parse_int_strict("99999999999", &i) == -1, "parse_int_strict tu choi so tran");

    int m;
    CHECK(parse_hhmm("06:00", &m) == 0 && m == 360, "parse_hhmm(\"06:00\") = 360");
    CHECK(parse_hhmm("23:59", &m) == 0 && m == 1439, "parse_hhmm(\"23:59\") = 1439");
    CHECK(parse_hhmm("24:00", &m) == -1 && parse_hhmm("6:00", &m) == -1 &&
          parse_hhmm("06-00", &m) == -1 && parse_hhmm("06:60", &m) == -1,
          "parse_hhmm tu choi 24:00, 6:00, 06-00, 06:60");
    char hh[8];
    format_hhmm(390, hh, sizeof hh);
    CHECK(strcmp(hh, "06:30") == 0, "format_hhmm(390) = \"06:30\"");

    char buf[32] = "200 OK";
    CHECK(kv_append(buf, sizeof buf, "Tmax", "%.1f", 32.0) == 0 &&
          strcmp(buf, "200 OK Tmax=32.0") == 0, "kv_append -> \"200 OK Tmax=32.0\"");
    CHECK(kv_append(buf, sizeof buf, "very_long_key", "%s", "very_long_value") == -1 &&
          strcmp(buf, "200 OK Tmax=32.0") == 0, "kv_append vuot kich thuoc -> -1, buf giu nguyen");

    CHECK(dev_type_from_str("FAN") == DEV_FAN, "dev_type_from_str(\"FAN\")");
    CHECK(dev_type_from_str("XYZ") == DEV_INVALID, "dev_type_from_str(\"XYZ\") -> DEV_INVALID");
    CHECK(strcmp(dev_type_prefix(DEV_FEEDER), "FED-") == 0, "dev_type_prefix(FEEDER) = \"FED-\"");

    CHECK(resp_code("200 OK x") == 200, "resp_code(\"200 OK x\") = 200");
    CHECK(resp_code("403 INVALID_TOKEN") == 403, "resp_code(\"403 INVALID_TOKEN\") = 403");
    CHECK(resp_code("OK") == -1 && resp_code("2000") == -1, "resp_code dong sai -> -1");

    return TEST_DONE();
}
