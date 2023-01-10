/*
 * @Description: socket 编程之 serve.c
 * @Author: TOTHTOT
 * @Date: 2023-01-06 10:57:42
 * @LastEditTime: 2023-01-06 16:41:51
 * @LastEditors: TOTHTOT
 * @FilePath: \am335x_project\5_socket\1_tcp\serve.c
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>

#define SERVE_PORT 8888 // 端口
#define SERVE_MAXNUM 10 // 最大连接 client 数

// 用于给线程传参
struct thr_data_st
{
    int sfd_client;
    int client_num;
};

/**
 * @name: thr_connect
 * @msg: 每当有一个客户端接入就创建一个线程,
 *       线程中负责接收数据
 * @param {void} *ptr
 * @return {*}
 */
static void *thr_connect(void *ptr)
{
    struct thr_data_st data = *(struct thr_data_st *)ptr;
    u_char recv_buf[1000];
    int recv_len;
    // 将线程与主进程分离出来, 在 pthread_exit 后自动回收资源
    // pthread_detach(pthread_self());
    while (1)
    {
        // 主机接数据
        recv_len = recv(data.sfd_client, recv_buf, 999, 0);
        if (recv_len <= 0) // == -1 出错, == 0 客户端掉线
        {
            fprintf(stderr, "Client Num:[%d] recv() error code :%d\n", data.client_num, recv_len);
            break;
        }
        else // 正确
        {
            recv_buf[recv_len] = '\0'; // 添加结束符
            printf("Client Num:[%d] Receive Msg:%s\n", data.client_num, recv_buf);
        }
    }
    pthread_exit(NULL);
}

int main(int argc, char **argv)
{
    int sfd, sfd_client[SERVE_MAXNUM];
    struct sockaddr_in socket_server_addr;               // 服务器地址
    struct sockaddr_in socket_client_addr[SERVE_MAXNUM]; // 客户端地址
    pthread_t tid[SERVE_MAXNUM];                         // 线程, 每当有客户端连接进来就创建一个线程
    int client_num = 0;                                  // 已经连接上的客户端数量, 最大 SERVE_MAXNUM 个
    int addr_len;                                        // socket_client_addr 的长度, 由于需要一个指针所以要一个变量
    struct thr_data_st *p_thr_data;                      // 负责传递数据给线程

    // 申请一个 socket 描述符
    sfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sfd == -1)
    {
        perror("socket() err");
        exit(-1);
    }

    // 对结构体初始化
    socket_server_addr.sin_family = AF_INET;
    socket_server_addr.sin_port = htons(SERVE_PORT);
    socket_server_addr.sin_addr.s_addr = INADDR_ANY;
    memset(socket_server_addr.sin_zero, 0, 8);

    // 绑定ip及端口
    if (bind(sfd, (const struct sockaddr *)&socket_server_addr, sizeof(socket_server_addr)) < 0)
    {
        perror("bind() err");
        exit(-1);
    }

    // 监听
    if (listen(sfd, SERVE_MAXNUM) < 0)
    {
        perror("listen() err");
        exit(-1);
    }

    while (1)
    {
        addr_len = sizeof(socket_client_addr[0]);
        if ((sfd_client[client_num] = accept(sfd, (struct sockaddr *)&socket_client_addr[client_num], &addr_len)) != -1)
        {
            // 每当有连接成功就创建一个线程
            printf("client addr:%s\n", inet_ntoa(socket_client_addr[client_num].sin_addr));
            p_thr_data = malloc(sizeof(*p_thr_data));

            if (p_thr_data == NULL)
            {
                fprintf(stderr, "malloc() err\n");
                exit(-1);
            }
            p_thr_data->sfd_client = sfd_client[client_num];
            p_thr_data->client_num = client_num;
            if (pthread_create(&tid[client_num], NULL, thr_connect, (void *)p_thr_data) < 0)
            {
                perror("pthread_create() err");
                exit(-1);
            }
            if (++client_num > SERVE_MAXNUM)
                client_num = 0;
        }
    }

    return 0;
}