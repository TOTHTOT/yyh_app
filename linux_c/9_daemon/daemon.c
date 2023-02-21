/*
 * @Description: linux 之守护进程
 * @Author: TOTHTOT
 * @Date: 2023-02-16 09:10:22
 * @LastEditTime: 2023-02-16 11:24:26
 * @LastEditors: TOTHTOT
 * @FilePath: \yyh_app\linux_c\9_daemon\daemon.c
 */
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdlib.h>


int main(void)
{
    pid_t pid;

    pid = fork();
    if(pid < 0)
    {
        perror("fork() err");
        exit(-1);
    }

    while (1)
    {
        if(pid == 0)    // 子进程
        {
            printf("[chiled]:sid = %d\n", setsid());
            sleep(10);
            printf("daemon end\n");
            exit(0);
        }
        else    // 父进程
        {
            printf("[father]:pid = %d\n", getpid());
            exit(0);
        }

    }
    

    return 0;
}