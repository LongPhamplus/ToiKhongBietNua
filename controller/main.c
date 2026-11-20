/*
 * controller/main.c: chương trình controller (Minh)
 * Bước 9: chương trình thử các hàm của ui.c. App ID + menu thật làm ở bước 10.
 */
#include <stdio.h>
#include "ui.h"

int main(void)
{
    int n;
    double d;
    char pass[40];

    int r = ui_read_int("Nhap so 1-10: ", 1, 10, &n);
    printf("-> r=%d n=%d\n", r, r == 0 ? n : -1);
    r = ui_read_double_keep("Tmax", 32.0, 10, 45, &d);
    printf("-> r=%d d=%.1f\n", r, d);
    r = ui_read_password("Mat khau: ", pass, sizeof pass);
    printf("-> r=%d pass_len=%d\n", r, r == 0 ? (int)__builtin_strlen(pass) : -1);
    int c = ui_menu("Menu thu", (const char *[]){ "Mot", "Hai" }, 2, NULL);
    printf("-> chon=%d\n", c);
    ui_ok("xong");
    ui_err("%s", ui_explain_code(403));
    return 0;
}
