/*
 * @Description: linux POSIX 信号量, 使用线程+信号量模拟 两个办事窗口有五个人来办理业务
 * @Author: TOTHTOT
 * @Date: 2023-02-10 13:56:31
 * @LastEditTime: 2023-02-10 16:39:52
 * @LastEditors: TOTHTOT
 * @FilePath: \yyh_app\linux_c\7_sem\sem.c
 */
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <semaphore.h>
#include <pthread.h>

#define TOTAL_WINDOW 2
#define TOTAL_MAN 5

sem_t g_mysem_st = {0};
int g_man_num = TOTAL_MAN;

struct window_state
{
    int window_num;           // 窗口编号
    int cumulative_reception; // 累积接待人数
};

/**
 * @name: pthread_window
 * @msg: 办理窗口线程
 * @param {void} *v_ptr
 * @return {*}
 * @author: TOTHTOT
 * @date:
 */
void *pthread_window(void *v_ptr)
{
    struct window_state win_num = *(struct window_state *)v_ptr; // 获取传递进来的参数

    while (1)
    {
        sem_wait(&g_mysem_st);
        if (--g_man_num < 0)
        {
            sem_wait(&g_mysem_st);
            break;
        }
        // sleep(2);
        printf("[%d]:deal with a man.\n", win_num.window_num);
        win_num.cumulative_reception++;
        if (g_man_num == 0)
        {
        }
        sem_post(&g_mysem_st);
    }

    printf("[%d]:total deal with %d man.\n", win_num.window_num, win_num.cumulative_reception);
    pthread_exit(NULL);
}

int main(void)
{
    pthread_t tid[TOTAL_WINDOW];
    int ret = 0;
    int i = 0;
    struct window_state window_st[TOTAL_WINDOW] = {0};

    for (i = 0; i < TOTAL_WINDOW; i++)
    {
        window_st[i].cumulative_reception = 0;
        window_st[i].window_num = i;
    }

    ret = sem_init(&g_mysem_st, 1, TOTAL_MAN); // 初始化信号量
    if (ret < 0)
    {
        perror("sem_init() err");
    }

    for (i = 0; i < TOTAL_WINDOW; i++)
    {
        ret = pthread_create(&tid[i], NULL, pthread_window, (void *)&window_st[i]);
        if (ret < 0)
        {
            perror("pthread_creat() err");
        }
    }

    // 回收线程
    for (i = 0; i < TOTAL_WINDOW; i++)
    {
        pthread_join(tid[i], NULL);
        // printf("waitting thread exit\n");
    }

    sem_destroy(&g_mysem_st); // 销毁信号量

    return 0;
}
