/*
 * @Description: socket 编程之 client.c
 * @Author: TOTHTOT
 * @Date: 2023-01-06 15:11:22
 * @LastEditTime: 2023-01-12 17:02:10
 * @LastEditors: TOTHTOT
 * @FilePath: \am335x_project\5_socket\2_tcp_V2.0\client.c
 */ 
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>

#define SERVE_PORT 8888

int sfd_client;
// 用于给线程传参
struct thr_data_st
{
    int sfd_server;
    int client_num;
};
/**
 * @name: sig_handle
 * @msg: 信号处理函数
 * @param {int} sig
 * @return {*}
 */
static void sig_handle(int sig)
{
    switch (sig)
    {
    case SIGINT: // 接收到中断信号, 结束
        printf("exit\n");
        close(sfd_client); // 关闭sfd
        exit(0);
        break;
    default:
        break;
    }
}

/**
 * @name: thr_receive_from_serve
 * @msg: 该线程用于处理接收到的数据
 * @param {void} *ptr
 * @return {*}
 */
static void *thr_receive_from_serve(void *ptr)
{
    u_char buf[1000];
    int recv_len; // recv 返回值
    struct thr_data_st thr_data = *(struct thr_data_st *)ptr;

    while (1)
    {
        recv_len = recv(thr_data.sfd_server, buf, 999, 0);
        if (recv_len <= 0)
        {
            fprintf(stderr, "recv() err\n");
            break;
        }
        printf("Receive Len:%d Msg:%s\n", recv_len, buf);
        if (strcmp("exit", buf) == 0) // 匹配到 exit指令就退出
        {
            printf("thr_receive_from_serve exit\n");
            break;
        }
        memset(buf, 0, sizeof(buf));
    }

    pthread_exit(ptr);
}

int main(int argc, char **argv)
{
    struct thr_data_st *p_thr_data;        // 传递给线程的数据
    struct sockaddr_in socket_client_addr; // socket 的相关配置
    u_char send_buf[1000];                 // 发送字符串缓存
    int send_data_len = 0;                 // 发送字符长度
    pthread_t tid;                         // 线程

    // 判断参数是否合法
    if (argc != 2)
    {
        printf("Usage:\n%s <server_ip>\n", argv[0]);
        exit(-1);
    }

    // 申请 socket
    sfd_client = socket(AF_INET, SOCK_STREAM, 0);
    if (sfd_client == -1)
    {
        perror("socket() err");
        exit(-1);
    }

    // 注册信号, 收到 SIGINT 就关闭 socket 结束进程
    if (signal(SIGINT, sig_handle) == SIG_ERR)
    {
        perror("signal() err");
        exit(-1);
    }

    socket_client_addr.sin_family = AF_INET;
    socket_client_addr.sin_port = htons(SERVE_PORT);
    if (inet_aton(argv[1], &socket_client_addr.sin_addr) == 0) // 字符串转ip
    {
        perror("inet_aton() err");
        exit(-1);
    }
    memset(socket_client_addr.sin_zero, 0, 8);

    if (connect(sfd_client, (const struct sockaddr *)&socket_client_addr, sizeof(socket_client_addr)) < 0)
    {
        perror("connect() err");
        exit(-1);
    }

    // 连接成功后创建一个线程用于接收 serve 发来的数据
    p_thr_data = malloc(sizeof(struct thr_data_st));
    if (p_thr_data == NULL)
    {
        fprintf(stderr, "malloc() err, len[%d]\n", __LINE__);
        exit(-1);
    }
    p_thr_data->sfd_server = sfd_client;
    if (pthread_create(&tid, NULL, thr_receive_from_serve, (void *)p_thr_data) < 0)
    {
        perror("pthread_create() err");
        exit(-1);
    }

    // 主进程在收到数据后发送出去
    while (1)
    {
        if (fgets(send_buf, 999, stdin))
        {
            send_data_len = send(sfd_client, send_buf, strlen(send_buf), 0);
            if (send_data_len < 0)
            {
                fprintf(stderr, "send() err, len:%d\n", send_data_len);
                close(sfd_client);
                exit(-1);
            }
        }
    }

    return 0;
}