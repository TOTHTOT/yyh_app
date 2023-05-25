/*
 * @Description:
 * @Author: TOTHTOT
 * @Date: 2023-05-22 09:17:34
 * @LastEditTime: 2023-05-22 14:58:15
 * @LastEditors: TOTHTOT
 * @FilePath: \yyh_app\linux_c\10_shm_posix\posix_shm_read.c
 */
#include <sys/mman.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <signal.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <semaphore.h>
#include <fcntl.h>

#define SHM_NAME "my_shm"
#define SEM_NAME_W "my_sem_w"
#define SEM_NAME_R "my_sem_r"

/* 共享内存的结构体 */
typedef struct
{
    char name[20];
    int age;
    char sex[20];
    char country[20];
} shm_test;

// 全局变量
int g_shm_id = 0;
sem_t *g_semr_ctrl_p = NULL;
sem_t *g_semw_ctrl_p = NULL;

void sig_fun(int signuim)
{
    switch (signuim)
    {
    case SIGINT:
        shm_unlink(SHM_NAME); // 删除posix共享内存
        sem_close(g_semr_ctrl_p);
        sem_close(g_semw_ctrl_p);
        sem_unlink(SEM_NAME_W);
        sem_unlink(SEM_NAME_R);
        printf("shm write exit\n");
        exit(0);
        break;
    default:
        break;
    }
}

/**
 * @name: my_shm_init
 * @msg: 共享内存初始化
 * @param {int} *shm_id 返回的shm_id
 * @param {shm_test} **mmp_ret 返回的共享内存地址
 * @param {char} *name 共享内存名字
 * @return {*} == 0成功, 其他失败
 * @author: TOTHTOT
 * @date: 2023年5月22日13:01:26
 */

static int8_t my_shm_init(int *shm_id, shm_test **mmp_ret, const char *name)
{
    // 打开共享内存
    *shm_id = shm_open(name, O_CREAT | O_RDWR, 0666);
    if (*shm_id == -1)
    {
        perror("shm open error:");
        shm_unlink(name);
        return -1;
    }
    // 设置共享内存大小
    ftruncate(*shm_id, sizeof(shm_test));

    // 连接共享内存
    *mmp_ret = mmap(NULL, sizeof(shm_test), PROT_READ | PROT_WRITE, MAP_SHARED, *shm_id, 0);
    if (*mmp_ret == MAP_FAILED) // 连接错误流程
    {
        perror("mmap() error:");
        shm_unlink(name);
        return -2;
    }

    return 0;
}

/**
 * @name: my_sem_init
 * @msg: 初始化信号量
 * @param {sem_t} *
 * @param {char} *name
 * @return {*}
 * @author: TOTHTOT
 * @date: 2023年5月22日13:41:35
 */
static int8_t my_sem_init(sem_t **sem_ret, const char *name, int sem_value)
{
    *sem_ret = sem_open(name, O_CREAT | O_RDWR, 0666, sem_value);
    if (*sem_ret == SEM_FAILED)
    {
        return -1;
    }
    return 0;
}

int main(void)
{
    shm_test *shm_p;
    int ret = 0;
    int semw_ctrl_value = 0;

    signal(SIGINT, sig_fun);

    // 初始化共享内存
    ret = my_shm_init(&g_shm_id, &shm_p, SHM_NAME);
    if (ret != 0)
    {
        fprintf(stderr, "my_shm_init() error = %d\n", ret);
    }
    // 初始化信号量
    ret = my_sem_init(&g_semw_ctrl_p, SEM_NAME_W, 1);
    ret = my_sem_init(&g_semr_ctrl_p, SEM_NAME_R, 0);

    printf("shm_p = %p\n", shm_p);
    while (1)
    {
#if 0
        // sem_wait(g_semr_ctrl_p);
        printf("name:%s, age:%d, sex:%s\n", shm_p->name, shm_p->age, shm_p->sex);

        // 获取一下信号量计数值, 当等于0时才+1一次, 等于0说明已经写过一次-
        sem_getvalue(g_semw_ctrl_p, &semw_ctrl_value);
        if (semw_ctrl_value == 0)
            sem_post(g_semw_ctrl_p);
#else
        sem_getvalue(g_semw_ctrl_p, &semw_ctrl_value);  // 获取信号量状态, 等于1说明写入完成
        if (semw_ctrl_value == 1)
        {
            printf("age:%d\n", shm_p->age);
        }
        else
        {
            printf("writing...\n");
        }
#endif
        usleep(100000);
    }

    return 0;
}