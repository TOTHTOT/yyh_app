/*
 * @Description: socket 编程之 client.c
 * @Author: TOTHTOT
 * @Date: 2023-01-06 15:11:22
 * @LastEditTime: 2023-01-06 16:27:08
 * @LastEditors: TOTHTOT
 * @FilePath: \am335x_project\5_socket\1_tcp\client.c
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

int main(int argc, char **argv)
{
    struct sockaddr_in socket_client_addr;
    u_char send_buf[1000];
    int send_data_len = 0;

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