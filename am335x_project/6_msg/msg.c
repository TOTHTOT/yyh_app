/*
 * @Description: 进程间通信之消息队列
 * @Author: TOTHTOT
 * @Date: 2023-01-13 09:10:01
 * @LastEditTime: 2023-01-13 10:57:50
 * @LastEditors: TOTHTOT
 * @FilePath: \am335x_project\6_msg\msg.c
 */
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/stat.h>
#include <sys/msg.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <sys/wait.h>

// 消息类型定义
#define MESSAGE_TYPE_NORMAL 1

#define MSG_KEY 0x123456

struct msg_st
{
    long int message_type; // 消息类型
    int msg_num;           // 发送的数据, 发送消息的累积次数
    char str[100];         // 发送的数据
    void *p_v_data;        // 发送的数据
};

int main(void)
{
    pid_t msg_get_pid;                                          // 子进程接收消息队列, 父进程发送消息队列
    int msg_id;                                                 // 消息队列id
    int ret;                                                    // 接收各种返回数据
    struct msg_st my_msg;                                       // 消息队列发送的数据
    int msg_st_size = sizeof(struct msg_st) - sizeof(long int); // 消息队列的大小

    do
    {
        msg_id = msgget(MSG_KEY, IPC_CREAT | 0666);
        if (msg_id < 0)
        {
            if (errno == EEXIST)
            {
                fprintf(stderr, "msg exist, line:%d\n", __LINE__);
                msgctl(msg_id, IPC_RMID, 0);
            }
            else
            {
                perror("msgget() err");
                exit(-1);
            }
        }
        else
            break;
    } while (1);

    msg_get_pid = fork();
    if (msg_get_pid < 0)
    {
        perror("fork() err");
        exit(-1);
    }
    while (1)
    {
        if (msg_get_pid == 0) // 子进程
        {
            ret = msgrcv(msg_id, (void *)&my_msg, msg_st_size, MESSAGE_TYPE_NORMAL, 0);
            if (ret < 0)
            {
                perror("msgrcv() err");
                exit(-1);
            }
            if (strcmp(my_msg.str, "exit") == 0)
            {
                printf("chiled [%d] exit\n", getpid());
                exit(0);
            }
            printf("chiled [%d] receive:%s\n", getpid(), my_msg.str);
        }
        else // 父进程
        {
            // 对发送的数处理一下
            my_msg.message_type = MESSAGE_TYPE_NORMAL;
            my_msg.msg_num++;
            printf("Please input what you want to send?\n");
            scanf("%s", my_msg.str);
            // 发送队列
            ret = msgsnd(msg_id, (void *)&my_msg, msg_st_size, 0);
            if (ret < 0)
            {
                perror("msgsend() err");
                exit(-1);
            }
            printf("father [%d] send:%s\n", getpid(), my_msg.str);
            if (strcmp(my_msg.str, "exit") == 0)
            {
                printf("father [%d] exit\n", getpid());
                break;
            }
        }
    }
    waitpid(msg_get_pid, NULL, 0);
    msgctl(msg_id, IPC_RMID, 0);
    printf("end\n");
    return 0;
}