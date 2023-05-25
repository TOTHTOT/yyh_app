/*
 * @Description: linux 共享内存 读取
 * @Author: TOTHTOT
 * @Date: 2023-02-10 16:34:34
 * @LastEditTime: 2023-05-24 14:16:50
 * @LastEditors: TOTHTOT
 * @FilePath: \yyh_app\linux_c\8_shm\shm_read.c
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <signal.h>
#include <semaphore.h>
#include <fcntl.h>

#define SHM_SEM_NAME_WRITE "sem_write" // 有名写信号量名字
#define SHM_SEM_NAME_READ "sem_read"   // 有名读信号量名字
#define SHM_KEY 0x123456
int g_shm_id; // 共享内存的 id
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
 * @msg: 信号处理函数
 * @param {int} signum 信号
 * @return {*} 无
 * @author: TOTHTOT
 * @date:
 */
void sig_fun(int signum)
{
    switch (signum)
    {
    case SIGINT:
        printf("shm_read exit\n");
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
    struct shm_test *shm_st_p;

    /* 使用信号 ctrl+c 关闭程序并删除共享内存 */
    signal(SIGINT, sig_fun);

    /* 有名信号量 */
    g_semr_p = sem_open(SHM_SEM_NAME_READ, O_CREAT | O_RDWR, 0777, 0);
    if (g_semr_p == SEM_FAILED) // 创建一个有名信号量
    {
        fprintf(stderr, "sem_open(SHM_SEM_NAME_READ) error\n");
        exit(-1);
    }
    g_semw_p = sem_open(SHM_SEM_NAME_WRITE, O_CREAT | O_RDWR, 0777, 1);
    if (g_semw_p == SEM_FAILED) // 创建一个有名信号量
    {
        fprintf(stderr, "sem_open(SHM_SEM_NAME_WRITE) error\n");
        exit(-1);
    }

    // 0666 设置读写属性, IPC_CREAT 如果不存在就创建
    g_shm_id = shmget(SHM_KEY, sizeof(struct shm_test), 0666 | IPC_CREAT);
    if (g_shm_id < 0)
    {
        perror("shmget() err");
        exit(-1);
    }

    // 连接共享内存地址
    shm_st_p = shmat(g_shm_id, 0, 0);
    if (shm_st_p == (void *)-1)
    {
        perror("shmat() err");
        exit(-1);
    }
    printf("share memory adder:0x%p\n", shm_st_p);
    while (1)
    {
        sem_wait(g_semr_p); // 读进程的读信号量-1
        printf("name:%s, age:%d, sex:%s\n", shm_st_p->name, shm_st_p->age, shm_st_p->sex);
        // usleep(1000000);    // 每秒读一次共享内存
        sem_post(g_semw_p); // 读进程读完写信号量+1
    }

    // getchar();

    // 移除共享内存
    // shmctl(g_shm_id, IPC_RMID, 0);

    return 0;
}