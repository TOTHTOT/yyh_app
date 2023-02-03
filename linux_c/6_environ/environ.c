/*
 * @Description: linux 环境变量
 * @Author: TOTHTOT
 * @Date: 2023-02-03 14:24:14
 * @LastEditTime: 2023-02-03 14:38:12
 * @LastEditors: TOTHTOT
 * @FilePath: \yyh_app\linux_c\6_environ\environ.c
 */

#include "stdlib.h"
#include "stdio.h"

extern char **environ;

int main(void)
{
    // 使用 environ 输出所有的环境变量
    for (int i = 0; environ[i] != NULL; i++)
    {
        // printf("[%d]:%s\n", i, environ[i]);
    }

    // 使用 setenv 设置环境变量, 改变或者添加
    setenv("my_env", "my name yyh", 1);
    // 使用 getenv 输出环境变量
    puts(getenv("my_env"));
    getchar();
    // unsetenv 删除一个环境变量
    unsetenv("my_env");
    return 0;
}