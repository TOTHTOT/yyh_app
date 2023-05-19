/*
 * @Description: 共享内存写入数据, 使用信号量实现共享内存同步
 * @Author: TOTHTOT
 * @Date: 2023-05-18 12:47:41
 * @LastEditTime: 2023-05-19 09:55:11
 * @LastEditors: TOTHTOT
 * @FilePath: \yyh_app\linux_c\8_shm\shm_write.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <sys/stat.h>
#include <signal.h>
#include <semaphore.h>
#include <fcntl.h>
#include <pthread.h>

#define SHM_SEM_NAME_WRITE "sem_write" // 有名写信号量名字
#define SHM_SEM_NAME_READ "sem_read"   // 有名读信号量名字
#define SHM_KEY 0x00123456             // 共享内存键值
int g_shm_id = 0;
sem_t *g_semr_p = NULL, *g_semw_p = NULL;

/* 共享内存的结构体 */
struct shm_test
{
    char name[20];
    int age;
    char sex[20];
    char country[20];
};

/**
 * @name: sig_fun
 * @msg: 信号chulihasnhu
 * @param {int} signum
 * @return {*}
 * @author: TOTHTOT
 * @date:
 */
void sig_fun(int signum)
{
    switch (signum)
    {
    case SIGINT:
        shmctl(g_shm_id, IPC_RMID, NULL);

        sem_close(g_semr_p);            // 关闭有名信号量
        sem_unlink(SHM_SEM_NAME_READ);  // 从系统中删除信号量
        sem_close(g_semw_p);            // 关闭有名信号量
        sem_unlink(SHM_SEM_NAME_WRITE); // 从系统中删除信号量

        printf("shm_write exit\n");
        exit(0);
        break;
    default:
        printf("unknow signum\n");
        break;
    }
}

int main(void)
{
    int ret = 0;
    int sem_w_value = 110;
    struct shm_test *shm_test_st_p;

    /* 使用信号 ctrl+c 关闭程序并删除共享内存 */
    signal(SIGINT, sig_fun);

    /* 有名信号量 */
    g_semr_p = sem_open(SHM_SEM_NAME_READ, O_CREAT | O_RDWR, 0666, 0); // 创建一个有名信号量, 初始值为0
    if (g_semr_p == SEM_FAILED)
    {
        fprintf(stderr, "sem_open(SHM_SEM_NAME_READ) error\n");
        exit(-1);
    }
    g_semw_p = sem_open(SHM_SEM_NAME_WRITE, O_CREAT | O_RDWR, 0666, 1); // 创建一个有名信号量, 初始值为1
    if (g_semw_p == SEM_FAILED)
    {
        fprintf(stderr, "sem_open(SHM_SEM_NAME_WRITE) error\n");
        exit(-1);
    }
    if (sem_getvalue(g_semw_p, &sem_w_value) == -1)
    {
        perror("sem_getvalue() error\n");
    }
    printf("1 sem_w_value = %d\n", sem_w_value);

    // 获取一个共享内存, 以读写方式打开, 如果不存在就创建一个
    g_shm_id = shmget(SHM_KEY, sizeof(struct shm_test), IPC_CREAT | S_IRUSR | S_IWUSR);
    if (g_shm_id == -1) // 出错流程
    {
        perror("shmget() error");
        exit(-1);
    }

    // 连接一个共享内存
    shm_test_st_p = shmat(g_shm_id, NULL, 0);
    if (shm_test_st_p == (void *)-1)
    {
        perror("shmat() error");
        exit(-2);
    }
    printf("share memory adder = %p\n", shm_test_st_p);

    // 赋值共享内存
    while (1)
    {
        if (sem_getvalue(g_semw_p, &sem_w_value) == -1)
        {
            perror("sem_getvalue() error\n");
        }
        printf("sem_w_value = %d\n", sem_w_value);
        sem_wait(g_semw_p); // 写信号量-1, 读进程里面+1
        printf("input name:\n");
        scanf("%s", shm_test_st_p->name);
        printf("input age:\n");
        scanf("%d", &shm_test_st_p->age);
        printf("input sex:\n");
        scanf("%s", shm_test_st_p->sex);
        sem_post(g_semr_p); // 写进程里读信号+1, 读进程里-1
    }
}