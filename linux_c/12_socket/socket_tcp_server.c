/*
 * @Description: socket 主机, 每当一个 client 接入就添加一个 epoll 监听
 * @Author: TOTHTOT
 * @Date: 2024-01-08 18:21:50
 * @LastEditTime: 2024-01-11 10:20:08
 * @LastEditors: TOTHTOT
 * @FilePath: \yyh_app\linux_c\12_socket\socket_tcp_server.c
 */
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

typedef struct socket_tcp_server
{
    int32_t socket_fd; // 监听的 socket 文件描述符
    uint16_t port;     // 使用的端口
    uint32_t max_client_num;

    bool server_run_flag;

    /* epoll 相关 */
    int32_t epoll_fd;
    struct epoll_event *events_p;

} socket_tcp_server_t;

socket_tcp_server_t g_socket_tcp_server_st = {0};

uint8_t socket_tcp_server_init(socket_tcp_server_t *socket_st_p, uint16_t port, uint32_t max_client)
{
    struct sockaddr_in serverAddress;
    struct epoll_event event;

    socket_st_p->server_run_flag = true;
    socket_st_p->port = port;
    socket_st_p->socket_fd = socket(AF_INET, SOCK_STREAM, 0);

    // 绑定地址和端口
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    serverAddress.sin_port = htons(port);

    if (bind(socket_st_p->socket_fd, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) == -1)
    {
        fprintf(stderr, "Error binding socket");
        close(socket_st_p->socket_fd);
        return 1;
    }

    socket_st_p->max_client_num = max_client;
    // 监听连接
    if (listen(socket_st_p->socket_fd, max_client) == -1)
    {
        fprintf(stderr, "Error listening for connections");
        close(socket_st_p->socket_fd);
        return 2;
    }

    /* 初始化epoll */
    socket_st_p->epoll_fd = epoll_create(1);
    if (socket_st_p->epoll_fd == -1)
    {
        fprintf(stderr, "epoll_create error");
        return 3;
    }

    // 将 server_socket 加入 epoll 实例中
    event.events = EPOLLIN;
    event.data.fd = socket_st_p->socket_fd;

    if (epoll_ctl(socket_st_p->epoll_fd, EPOLL_CTL_ADD, socket_st_p->socket_fd, &event) == -1)
    {
        fprintf(stderr, "Failed to add server_socket to epoll");
        close(socket_st_p->epoll_fd);
        close(socket_st_p->socket_fd);
        return 4;
    }

    // 根据最大连接 client 数分配空间
    socket_st_p->events_p = malloc(sizeof(struct epoll_event) * socket_st_p->max_client_num);
    return 0;
}

/**
 * @name: socket_tcp_server_exit
 * @msg: socket server 退出处理流程
 * @param {socket_tcp_server_t} *socket_st_p
 * @return == 0, 成功
 * @author: TOTHTOT
 * @Date: 2024-01-09 17:44:24
 */
uint8_t socket_tcp_server_exit(socket_tcp_server_t *socket_st_p)
{
    close(socket_st_p->epoll_fd);
    close(socket_st_p->socket_fd);

    free(socket_st_p->events_p);

    return 0;
}
/**
 * @name: check_arg
 * @msg: 校验输入参数是否合法, 合法就初始化 socket
 * @param {int} argc    参数数量
 * @param {char} *argv  参数内容
 * @param {socket_tcp_server_t} *socket_st_p    socket 结构体
 * @return == 0, 成功; == 1, 失败
 * @author: TOTHTOT
 * @Date: 2024-01-09 14:46:14
 */
uint8_t check_arg(int argc, char *argv[], socket_tcp_server_t *socket_st_p)
{
    uint16_t port = 0;
    uint32_t max_client = 0;

    if (argc != 3)
    {
        fprintf(stderr, "Usage:%s <port> <max_client>\n", argv[0]);
        return 1;
    }

    port = atoi(argv[1]);
    max_client = atoi(argv[2]);
    socket_tcp_server_init(socket_st_p, port, max_client);

    return 0;
}

/**
 * @name: sig_handle
 * @msg: 信号处理函数
 * @param {int} signo 信号
 * @return {void}
 * @author: TOTHTOT
 * @Date: 2024-01-09 14:54:52
 */
void sig_handle(int signo)
{
    if (signo == SIGINT)
    {
        printf("Received SIGINT\n");
        g_socket_tcp_server_st.server_run_flag = false; // 退出程序
    }
}

int main(int argc, char *argv[])
{
    if (check_arg(argc, argv, &g_socket_tcp_server_st) != 0)
    {
        fprintf(stderr, "check_arg error");
        return 1;
    }

    signal(SIGINT, sig_handle);

    printf("Server listening on port = %d max client number = %d...\n",
           g_socket_tcp_server_st.port, g_socket_tcp_server_st.max_client_num);

    while (g_socket_tcp_server_st.server_run_flag)
    {
        // 这里会阻塞等待
        int num_events = epoll_wait(g_socket_tcp_server_st.epoll_fd, g_socket_tcp_server_st.events_p,
                                    g_socket_tcp_server_st.max_client_num, -1);
        if (num_events == -1)
        {
            fprintf(stderr, "Failed to wait for events");
            break;
        }

        for (int i = 0; i < num_events; i++)
        {
            // 监听的文件描述符匹配
            if (g_socket_tcp_server_st.events_p[i].data.fd == g_socket_tcp_server_st.socket_fd)
            {
                int32_t temp_client_socket = 0;
                struct sockaddr_in temp_client_address = {0};
                struct epoll_event temp_event = {0};
                socklen_t temp_client_address_len = sizeof(temp_client_address);

                // 有新的连接
                temp_client_socket = accept(g_socket_tcp_server_st.socket_fd, (struct sockaddr *)&temp_client_address, &temp_client_address_len);
                if (temp_client_socket == -1)
                {
                    fprintf(stderr, "Error accepting connection");
                    continue;
                }

                printf("Client connected from %s\n", inet_ntoa(temp_client_address.sin_addr));

                // 将新的 temp_client_socket 加入 epoll 实例中
                temp_event.events = EPOLLIN;
                temp_event.data.fd = temp_client_socket;

                if (epoll_ctl(g_socket_tcp_server_st.epoll_fd, EPOLL_CTL_ADD, temp_client_socket, &temp_event) == -1)
                {
                    fprintf(stderr, "Failed to add temp_client_socket to epoll");
                    close(temp_client_socket);
                    continue;
                }
            }
            else
            {
                // 有数据可读
                // handle_client(events[i].data.fd);
            }
        }
    }

    socket_tcp_server_exit(&g_socket_tcp_server_st);

    printf("socket_tcp_server_exit\n");
    return 0;
}
