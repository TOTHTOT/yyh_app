/*
 * @Description: linux 共享内存 读取
 * @Author: TOTHTOT
 * @Date: 2023-02-10 16:34:34
 * @LastEditTime: 2023-02-12 12:47:48
 * @LastEditors: TOTHTOT
 * @FilePath: \yyh_app\linux_c\8_shm\shm_read.c
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#define SHM_KEY 0x123456

struct shm_test
{
    char name;
    char age;
    char six;
    char country;
};

int main(void)
{
    int ret = 0;
    int shm_id; // 共享内存的 id
    struct shm_test *p_shm_st;

    // 0666 设置读写属性, IPC_CREAT 如果不存在就创建
    shm_id = shmget(SHM_KEY, sizeof(struct shm_test), 0666 | IPC_CREAT);
    if (shm_id < 0)
    {
        perror("shmget() err");
        exit(-1);
    }

    // 连接共享内存地址
    p_shm_st = shmat(shm_id, 0, 0);
    if (p_shm_st == (void *)-1)
    {
        perror("shmat() err");
        exit(-1);
    }
    printf("share memory adder:0x%x\n", (int)p_shm_st);

    getchar();

    // 移除共享内存
    shmctl(shm_id, IPC_RMID, 0);

    return 0;
}