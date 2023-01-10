/*
 * @Description: socket 编程之 server.c
 * @Author: TOTHTOT
 * @Date: 2023-01-06 10:57:42
 * @LastEditTime: 2023-01-10 13:49:39
 * @LastEditors: TOTHTOT
 * @FilePath: \am335x_project\5_socket\2_udp\server.c
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
#include <errno.h>
#include <sys/epoll.h>

#define DEBUG_FLAG 1     // 该宏用于控制是否输出 debug 信息
#define SERVE_PORT 8888  // 端口
#define SERVER_MAXNUM 10 // 最大连接 client 数

#if DEBUG_FLAG == 1
#include <stdarg.h>
#include <sys/syslog.h>
#define DEBUG_PRINT(fmt, ...)                                                                \
    do                                                                                       \
    {                                                                                        \
        printf("DEBUG: %s,%s,%d: " fmt "", __FILE__, __FUNCTION__, __LINE__, ##__VA_ARGS__); \
                                                                                             \
    } while (0)
#else
#define DEBUG_PRINT(fmt, ...) \
    do                        \
    {                         \
    } while (0)
#endif

pthread_t tid_server_link, tid_server_send, tid_client_connect; // 线程

// server 连接状态
enum server_link_state_em
{
    STATE_UNLINKED,   // 未连接
    STATE_UPDATALINK, // 更新连接状态, 当连接的 client 增加或者减少时进入此状态
    STATE_LINKED      // 已连接
};

// 用于给线程传参
struct thr_connect_data_st
{
    // int sfd_client;
    int my_client_num;                   // 当前连接的 client 编号
    int sfd_client[SERVER_MAXNUM];       // client 的文件描述符, 可能会发生读写冲突
    struct client_num_st *manage_client; // 该指针指向 main 里的 client
};

/* 该结构体用于管理 client 编号以及 server 连接状态
    一般使用 ret_available_client_num() 设置一个编号
    使用 repay_client_num() 归还一个编号
*/
struct client_num_st
{
    int sfd_server;                               // 服务器的 sfd
    int epfd;                                     // epoll 描述符
    struct epoll_event ep_event;                  // epfd 的事假绑定
    struct thr_connect_data_st *thr_connect_data; // 该指针指向 thr_server_link 线程的 thr_connect_data
    int current_num;                              // 已连接的编号数量的+1, 实际连接数是-1, 理论上应该小于 SERVER_MAXNUM
    enum server_link_state_em server_link_state;  // server 连接状态
    char client_num_flag[SERVER_MAXNUM];          // 此数组的每一位用于表示每个 client 编号的状态, 等于0 未被使用, 等于1 正在被使用,其它值未定义, 能够管理最大 SERVER_MAXNUM 个
};

/**
 * @name: client_num_to_sfd
 * @msg: 将 client 编号转换成 sfd
 * @param {client_num_st} client_num    管理 client 的结构体
 * @param {uint32_t} num    传入的编号
 * @return {*}  >= 0        成功,   返回sfd
 *              == -EINVAL  失败,   参数非法
 */
int client_num_to_sfd(const struct client_num_st client_num, const uint32_t num)
{
    int i = 0;

    if (num < 0 || num > SERVER_MAXNUM)
    {
        return -EINVAL;
    }
    DEBUG_PRINT("client_num_to_sfd:%d\n", client_num.thr_connect_data->sfd_client[num]);
    return client_num.thr_connect_data->sfd_client[num];
}

/**
 * @name: ret_available_client_num
 * @msg: 返回一个可用的 client 编号, 编号最大值 SERVER_MAXNUM
 * @param {client_num_st} *clitnt_num   结构体参数用于管理编号
 * @return {*} >= 成功
 *              == -1 失败, 表示没有剩余空位给新的 client
 */
int ret_available_client_num(struct client_num_st *client_num)
{
    // int available_client_num = 0;
    int i;
    if (client_num->current_num <= SERVER_MAXNUM)
    {
        // 寻找没被使用的编号
        for (i = 0; i < SERVER_MAXNUM; i++)
        {
            if (client_num->client_num_flag[i] == 0x30) // 找到了
            {
                printf("find\n");
                client_num->client_num_flag[i] = 1;
                client_num->current_num++;
                DEBUG_PRINT("current_num:%d\n", client_num->current_num);
                return i;
            }
        }
    }

    fprintf(stderr, "ERR: Client connection is full!, i = %d\n", i);
    return -1;
}

/**
 * @name:repay_client_num
 * @msg:将所占用的 client 编号归还
 * @param {client_num_st} *clitnt_num   管理编号的结构体
 * @param {int} num 要归还的编号
 * @return {*}  == 0    成功
 *              == -1   失败, 参数非法
 *              == -2   失败, 该编号已被归还
 */
int repay_client_num(struct client_num_st *client_num, int num)
{
    int epoll_ret;

    if (num > SERVER_MAXNUM) // 参数非法
    {
        return -1;
    }

    // 传入的编号已经是0了, 如果没有就会导致编号总数错误的减1
    if (client_num->client_num_flag[num] == 0)
    {
        return -2;
    }

    if ((epoll_ret = epoll_ctl(client_num->epfd, EPOLL_CTL_DEL, client_num_to_sfd(client_num, num), client_num->ep_event)) < 0)
    {
        fprintf(stderr, "epoll_ctl() err:%s, line:%d\n", strerror(epoll_ret), __LINE__);
        exit(-1);
    }
    // 正常流程
    client_num->current_num--;            // 总使用数减一
    client_num->client_num_flag[num] = 0; // 对应位置0

    return 0;
}
/**
 * @name: thr_connect
 * @msg: 每当有一个客户端接入就创建一个线程,
 *       线程中负责接收数据
 * @param {void} *ptr
 * @return {*}
 */
static void *thr_connect(void *ptr)
{
    struct thr_connect_data_st *p_data = (struct thr_connect_data_st *)ptr;
    u_char recv_buf[1000];
    int recv_len;
    // 将线程与主进程分离出来, 在 pthread_exit 后自动回收资源
    // pthread_detach(pthread_self());
    while (1)
    {
        // 有连接了才执行
        if (p_data->manage_client->server_link_state == STATE_LINKED)
        {
            epoll_wait(p_data->manage_client->epfd,&p_data->manage_client->ep_event, SERVER_MAXNUM, -1);
            if()
            // 主机接数据
            recv_len = recv(p_data->sfd_client[data.my_client_num], recv_buf, 999, 0);
            if (recv_len <= 0) // == -1 出错, == 0 客户端掉线
            {
                fprintf(stderr, "Client Num:[%d] recv() error code :%d\n", p_data->my_client_num, recv_len);
                break;
            }
            else // 正确
            {
                recv_buf[recv_len] = '\0'; // 添加结束符
                printf("Client Num:[%d] Receive Msg:%s\n", p_data->my_client_num, recv_buf);
            }
        }
    }
    // 一个 client 结束连接了, 要归还 client_num
    repay_client_num(p_data->manage_client, p_data->my_client_num);
    pthread_exit(NULL);
}

/**
 * @name: thr_server_link
 * @msg: 用于处理 client 连接, 每当有一个 client 连接
 *       就分配一个 client_num, 当  client_num 等于
 *       SERVER_MAXNUM 就停止连接处理
 * @param {void} *ptr
 * @return {*}
 */
static void *thr_server_link(void *ptr)
{
    int addr_len;                                                     // socket_client_addr 的长度, 由于需要一个指针所以要一个变量
    struct client_num_st *p_thr_client = (struct client_num_st *)ptr; // 负责传递数据给线程
    struct thr_connect_data_st thr_connect_data;                      // 传递给 thr_connect 线程的数据
    struct sockaddr_in socket_client_addr[SERVER_MAXNUM];             // 客户端地址
    int available_client_num;                                         // client 编号 通过 ret_available_client_num() 获得
    pthread_t tid[SERVER_MAXNUM];                                     // 线程

    while (1)
    {
        available_client_num = ret_available_client_num(p_thr_client); // 寻找可用编号, 如果没有就表示已连接client已达最大值
        if (available_client_num >= 0)
        {
            addr_len = sizeof(socket_client_addr[0]);
            if ((thr_connect_data.sfd_client[available_client_num] = accept(p_thr_client->sfd_server, (struct sockaddr *)&socket_client_addr[available_client_num], &addr_len)) != -1)
            {
                // 此处应该为 epoll 增加一个监听
                p_thr_client->ep_event.events = EPOLLIN;
                p_thr_client->ep_event.data.fd = thr_connect_data.sfd_client[available_client_num];
                p_thr_client->ep_event.data.u32 = available_client_num;
                if (epoll_ctl(p_thr_client->epfd, EPOLL_CTL_MOD, thr_connect_data.sfd_client[available_client_num], p_thr_client->ep_event) < 0)
                {
                    perror("epoll_ctl() err");
                    exit(-1);
                }
            }
        }
        else
        {
            fprintf(stderr, "ret_available_client_num() err: no space\n");
            // break;
        }
    }
    pthread_exit(ptr);
}

/**
 * @name: thr_server_send
 * @msg: 该线程用于发送数据到 client
 * @param {void} *ptr 线程参数
 * @return {*}
 */
static void *thr_server_send(void *ptr)
{

    struct client_num_st *p_thr_client = (struct client_num_st *)ptr; // 负责传递数据给线程
    uint32_t input_client_num;                                        // 服务器要给哪个 client 发数据
    char send_buf[1000];                                              // 服务器要发送的数据
    int send_len;                                                     // 发送数据的长度

    while (1)
    {
        if (p_thr_client->server_link_state == STATE_LINKED) // 已连接
        {
            printf("Which client? Please input %d ~ %d.\n", 0, p_thr_client.current_num - 1);
            scanf("%d", &input_client_num);
            if (input_client_num > p_thr_client.current_num - 1)
            {
                printf("Out of maximum number! Please reenter.\n");
                continue;
            }
            printf("What are you want to send?\n");
            scanf("%s", send_buf);
            send_len = send(client_num_to_sfd(*p_thr_client, input_client_num), send_buf, strlen(send_buf), 0);
            if (send_len < 0)
            {
                fprintf(stderr, "send() err, len:%d\n", send_len);
                close(client.sfd_server);
                exit(-1);
            }
        }
        else if (p_thr_client->server_link_state == STATE_UPDATALINK) // 因为scanf 等函数会阻塞, 然而当 client 连接发生变化时不会及时输出当前的连接数
        {
        }
        else if (p_thr_client->server_link_state == STATE_UNLINKED) // 未连接
        {
            printf("Waiting for client ask...\n");
            usleep(1500000);
        }
    }
    pthread_exit(ptr);
}

int main(int argc, char **argv)
{
    // pthread_t tid[SERVER_MAXNUM];          // 线程, 每当有客户端连接进来就创建一个线程
    struct sockaddr_in socket_server_addr; // 服务器地址
    struct client_num_st client;           // 管理 client 编号的结构体

    // 申请一个 socket 描述符
    client.sfd_server = socket(AF_INET, SOCK_STREAM, 0);
    if (client.sfd_server == -1)
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
    if (bind(client.sfd_server, (const struct sockaddr *)&socket_server_addr, sizeof(socket_server_addr)) < 0)
    {
        perror("bind() err");
        exit(-1);
    }

    // 监听
    if (listen(client.sfd_server, SERVER_MAXNUM) < 0)
    {
        perror("listen() err");
        exit(-1);
    }

    // 创建一个epoll, thr_connect 线程使用
    client.epfd = epoll_create(SERVER_MAXNUM);
    if (client.epfd < 0)
    {
        perror("epoll_create() err");
        exit(-1);
    }

    memset(client.client_num_flag, '0', sizeof(client.client_num_flag));
    client.current_num = 0;
    // 该线程用于处理 client 的连接请求
    if (pthread_create(&tid_server_link, NULL, thr_server_link, (void *)&client))
    {
        perror("pthread_create() err\n");
        exit(-1);
    }

    // 该线程用于接收来自 shell 的数据, 然后根据编号发给对应的连接设备
    if (pthread_create(&tid_server_send, NULL, thr_server_send, (void *)&client) < 0)
    {
        perror("pthread_create() err\n");
        exit(-1);
    }

    // 每当有连接成功就创建一个线程
    printf("Client num:%d sfd:%d addr:%s\n", available_client_num, thr_connect_data.sfd_client[available_client_num], inet_ntoa(socket_client_addr[available_client_num].sin_addr));

    /*     // thr_connect_data.sfd_client = thr_connect_data.sfd_client[available_client_num];
        thr_connect_data.my_client_num = available_client_num;
        thr_connect_data.manage_client = p_thr_client;
        p_thr_client->thr_connect_data = &thr_connect_data; */
    client.thr_connect_data = malloc(sizeof(*client.thr_connect_data));
    if (client.thr_connect_data == NULL)
    {
        fprintf(stderr, "malloc() err: no space!\n");
        exit(-1);
    }
    if (pthread_create(&tid_client_connect, NULL, thr_connect, (void *)client.thr_connect_data) < 0)
    {
        perror("pthread_create() err");
        exit(-1);
    }
    while (1)
    {
        // pthread_join()
    }

    return 0;
}