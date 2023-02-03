/*
 * @Description: linux 环境变量
 * @Author: TOTHTOT
 * @Date: 2023-02-03 14:24:14
 * @LastEditTime: 2023-02-03 14:30:20
 * @LastEditors: TOTHTOT
 * @FilePath: \yyh_app\linux_c\6_environ\environ.c
 */

#include "stdlib.h"
#include "stdio.h"

extern char **environ;

int main(void)
{
    for (int i = 0; environ[i] != NULL; i++)
    {
        printf("[%d]:%s\n", i, environ[i]);
    }
    return 0;
}