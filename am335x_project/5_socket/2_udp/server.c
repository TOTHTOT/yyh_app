/*
 * @Description: socket 编程之 server.c
 * @Author: TOTHTOT
 * @Date: 2023-01-06 10:57:42
 * @LastEditTime: 2023-01-12 16:03:55
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
#include <sys/sem.h>
#include <sys/ipc.h>
#include <semaphore.h>

static void *thr_server_send(void *ptr);
static void *thr_connect(void *ptr);
static void *thr_server_link(void *ptr);

#define DEBUG_FLAG 1    // 该宏用于控制是否输出 debug 信息
#define SERVE_PORT 8888 // 端口
#define SERVER_MAXNUM 3 // 最大连接 client 数

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

// server 连接状态
enum server_link_state_em
{
    STATE_UNLINKED,             // 没有client连接
    STATE_JUST_ONE_CLIENT_LINK, // 只有1个 client 连接时, 作为和没有 client 连接的过渡态
                                // 只要client 有空位, thr_server_link 线程就会占用一个编号,
                                // 使得 current_num 始终 >= 1, 这就导致 repay_client_num() 函数
                                // 无法准确切换到到 STATE_UNLINKED, 所以需要 STATE_JUST_ONE_CLIENT_LINK
                                // 来保证当所有 client 都断连后再进入 STATE_UNLINKED
    STATE_LINKED                // 有连接
};

// client 编号占用情况
enum client_num_state_em
{
    NUM_UNLOCK,           // 此编号未被占用
    NUM_LOCK_BUT_NO_LINK, // 此编号已被占用, 但是未被连接
    NUM_LOCK_LINK         // 此编号已被使用, 且 client 以连接
};

// 用于给线程传参
struct thr_connect_data_st
{
    // int sfd_client;
    int my_client_num;                   // 当前连接的 client 编号
    int sfd_client[SERVER_MAXNUM];       // client 的文件描述符, 可能会发生读写冲突
    struct client_num_st *manage_client; // 该指针指向 main 里的 client
};

// 此结构体用于 epoll 事件数据
struct ep_event_data_st
{
    int fd;
    int my_client_num;
};

/* 该结构体用于管理 client 编号以及 server 连接状态
    一般使用 ret_available_client_num() 设置一个编号
    使用 repay_client_num() 归还一个编号
*/
struct client_num_st
{
    pthread_rwlock_t client_num_rw_lock;                            // 读写锁用于保护
    pthread_t tid_server_link, tid_server_send, tid_client_connect; // 线程
    int sfd_server;                                                 // 服务器的 sfd
    int epfd;                                                       // epoll 描述符
    struct epoll_event ep_event[SERVER_MAXNUM];                     // epfd 的事假绑定, 每个 ep_event 都要有自己的 ep_event_data
    struct ep_event_data_st ep_event_data[SERVER_MAXNUM];           // 此结构体用于传输 epoll 的数据
    struct thr_connect_data_st *thr_connect_data;                   // 该指针指向 thr_server_link 线程的 thr_connect_data
    sem_t my_sem;                                                   // 信号量
    int current_num;                                                // 使用信号量, 正在准备连接编号数量, 实际连接数是-1, 理论上应该小于 SERVER_MAXNUM
    int real_num;                                                   // 真实的 client 连接数, 在 set_client_num_link() 里加1 在 repay_client_num() 函数里减1
    enum server_link_state_em server_link_state;                    // server 连接状态
    enum client_num_state_em client_num_flag[SERVER_MAXNUM];        // 此数组的每一位用于表示每个 client 编号的状态, 等于0 未被使用, 等于1 正在被使用,其它值未定义, 能够管理最大 SERVER_MAXNUM 个
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
 *       在连接设备满了后会阻塞, 直到有 client 断开连接, 成功归还编号后再回解除
 * @param {client_num_st} *clitnt_num   结构体参数用于管理编号
 * @return {*} >= 成功
 *              == -1 失败, 表示没有剩余空位给新的 client
 */
int ret_available_client_num(struct client_num_st *client_num)
{
    // int available_client_num = 0;
    int i;

    // 信号量减 1
    sem_wait(&client_num->my_sem);
    // 必须在sem_wait 后面, 有信号量的情况下执行 不然会死锁
    pthread_rwlock_wrlock(&client_num->client_num_rw_lock);
    if (client_num->current_num - 1 < SERVER_MAXNUM)
    {
        // 寻找没被使用的编号
        for (i = 0; i < SERVER_MAXNUM; i++)
        {
            if (client_num->client_num_flag[i] == NUM_UNLOCK) // 找到了
            {
                // printf("find\n");
                client_num->client_num_flag[i] = NUM_LOCK_BUT_NO_LINK;
                client_num->current_num++;
                DEBUG_PRINT("current_num:%d, return:%d\n", client_num->current_num, i);
                pthread_rwlock_unlock(&client_num->client_num_rw_lock);
                return i;
            }
        }
    }
    // 解锁
    pthread_rwlock_unlock(&client_num->client_num_rw_lock);
    fprintf(stderr, "ERR: Client connection is full!, i = %d, line:%d\n", i, __LINE__);
    return -1;
}

/**
 * @name: set_client_num_link
 * @msg: 在 accept 成功后将 NUM_LOCK_BUT_NO_LINK, 设置为 NUM_LOCK_LINK, 并且 real_client 加1
 * @param {client_num_st} *client_num   管理的编号
 * @param {int} num 要设置的位置
 * @return {*}  == 0        成功
 *              == -EINVAL  失败, 参数非法
 */
int set_client_num_link(struct client_num_st *client_num, int num)
{
    // 错误判断
    if (client_num == NULL || num > SERVER_MAXNUM || num < 0)
    {
        return -EINVAL;
    }

    client_num->client_num_flag[num] = NUM_LOCK_LINK;
    client_num->real_num++;
    DEBUG_PRINT("Set %d LINK, real_num:%d\n", num, client_num->real_num);
    return 0;
}

/**
 * @name: print_available_client_num
 * @msg: 输出所有已连接的 client
 * @param {client_num_st} client_num
 * @return {*}
 */
void print_available_client_num(const struct client_num_st client_num)
{
    int i;

    printf("\nList All Available Client Number:");
    for (i = 0; i < SERVER_MAXNUM; i++)
    {
        if (client_num.client_num_flag[i] == NUM_LOCK_LINK)
        {
            printf("%d ", i);
        }
    }
    printf("\nSelect a available number in the list.\n");
}

/**
 * @name:repay_client_num
 * @msg:将所占用的 client 编号归还
 * @param {client_num_st} *clitnt_num   管理编号的结构体
 * @param {int} num 要归还的编号
 * @return {*}  == 0        成功
 *              == -EINVAL  失败, 参数非法
 *              == -2   失败, 该编号已被归还
 */
int repay_client_num(struct client_num_st *client_num, int num)
{
    int epoll_ret;
    int sfd = client_num_to_sfd(*client_num, num);

    if (num > SERVER_MAXNUM) // 参数非法
    {
        return -EINVAL;
    }

    // 传入的编号已经是0了, 如果没有就会导致编号总数错误的减1
    if (client_num->client_num_flag[num] == NUM_UNLOCK)
    {
        return -2;
    }

    if ((epoll_ret = epoll_ctl(client_num->epfd, EPOLL_CTL_DEL, sfd, &client_num->ep_event[num])) < 0)
    {
        fprintf(stderr, "epoll_ctl() err:%s, line:%d\n", strerror(epoll_ret), __LINE__);
        exit(-1);
    }

    close(sfd);
    // 正常流程
    client_num->current_num--;                     // 总使用数减一
    client_num->client_num_flag[num] = NUM_UNLOCK; // 对应位置0
    client_num->real_num--;                        // 实际 client 连接数

    if (client_num->real_num == 0) // 减成 0 了就说明现在没有 client 连接, 那就进入 STATE_UNLINKED
    {
        // client_num->current_num = 0;
        client_num->server_link_state = STATE_UNLINKED;
        DEBUG_PRINT("server_link_state:STATE_UNLINKED\n");
    }
    DEBUG_PRINT("current_num:%d real_num:%d\n", client_num->current_num, client_num->real_num);
    // 信号量加 1
    sem_post(&client_num->my_sem);
    return 0;
}


/**
 * @name: judge_client_num_is_available
 * @msg: 判断传入的编号是有有client连接
 * @param {client_num_st} client_num 管理 client_num 的结构体
 * @param {int} num client 编号
 * @return {*}  == 0    存在
 *              == -1   不存在
 */
int judge_client_num_is_available(const struct client_num_st client_num, int num)
{
    int i;

    if(client_num.client_num_flag[num] == NUM_LOCK_LINK)
        return 0;
    else
        return -1;
    
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
    int epoll_ret; // epoll_wait 的返回值, 小于SERVER_MAXNUM
    int i = 0;
    struct ep_event_data_st *p_ep_event_data;
    int ret;

    // 将线程与主进程分离出来, 在 pthread_exit 后自动回收资源
    // pthread_detach(pthread_self());
    while (1)
    {
        // 有连接了才执行
        if (p_data->manage_client->server_link_state == STATE_LINKED)
        {
            epoll_ret = epoll_wait(p_data->manage_client->epfd, &p_data->manage_client->ep_event[0], SERVER_MAXNUM, -1);
            if (epoll_ret > 0)
            {
                // DEBUG_PRINT("wait end,epoll_ret:%d\n", epoll_ret);
                // 遍历监听事件中符合条件的 fd
                for (i = 0; i < epoll_ret; i++)
                {
                    if (p_data->manage_client->ep_event[i].events & EPOLLIN) // 如果可读
                    {
                        // 从epoll_wait 返回的数据中获取数据
                        p_ep_event_data = (struct ep_event_data_st *)p_data->manage_client->ep_event[i].data.ptr;
                        // DEBUG_PRINT("i:%d, sfd:%d, num:%d\n", i, p_ep_event_data->fd, p_ep_event_data->my_client_num);
                        // 主机接数据
                        recv_len = recv(p_ep_event_data->fd, recv_buf, 999, 0);
                        if (recv_len <= 0) // == -1 出错, == 0 客户端掉线
                        {
                            if (recv_len < 0)
                                fprintf(stderr, "Client Num:[%d] recv() error code :%d\n", p_ep_event_data->my_client_num, recv_len);
                            else
                                fprintf(stderr, "Client Num:[%d] Exit code :%d\n", p_ep_event_data->my_client_num, recv_len);

                            // 这里涉及到修改数据, 要锁住写
                            pthread_rwlock_wrlock(&p_data->manage_client->client_num_rw_lock);
                            // 一个 client 结束连接了, 要归还 client_num
                            repay_client_num(p_data->manage_client, p_ep_event_data->my_client_num);
                            // 写入完成, 解锁
                            pthread_rwlock_unlock(&p_data->manage_client->client_num_rw_lock);
                            // DEBUG_PRINT("SFD:%d event:%d\n", p_ep_event_data->fd, p_data->manage_client->ep_event[i].events);

                            // client 断开连接也要取消一次, 切换状态了要把 阻塞的 thr_connect 线程杀死然后重启
                            if (pthread_cancel(p_data->manage_client->tid_server_send) < 0)
                            {
                                perror("pthread_cancel() err");
                            }

                            // 回收线程资源
                            ret = pthread_join(p_data->manage_client->tid_server_send, NULL);
                            if (ret < 0)
                            {
                                fprintf(stderr, "pthread_join() fail:%s\n", strerror(ret));
                            }

                            if (pthread_create(&p_data->manage_client->tid_server_send, NULL, thr_server_send, (void *)p_data->manage_client) < 0)
                            {
                                perror("pthread_create() err");
                            }
                            DEBUG_PRINT("Restart thr_connect thread.\n");
                        }
                        else // 正确
                        {
                            recv_buf[recv_len] = '\0'; // 添加结束符
                            printf("Client Num:[%d] Client sfd:%d Receive Msg:%s\n", p_ep_event_data->my_client_num, p_ep_event_data->fd, recv_buf);
                        }
                    }
                }
            }
            else
            {
                fprintf(stderr, "epoll_wait() err, line:%d, function:%s\n", __LINE__, __FUNCTION__);
                // DEBUG_PRINT("SFD:%d event:%d\n", p_ep_event_data->fd, p_data->manage_client->ep_event.events);
                // pthread_exit(NULL);
            }
        }
    }
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
    // struct thr_connect_data_st thr_connect_data;                      // 传递给 thr_connect 线程的数据
    struct sockaddr_in socket_client_addr[SERVER_MAXNUM]; // 客户端地址
    int available_client_num;                             // client 编号 通过 ret_available_client_num() 获得
    pthread_t tid[SERVER_MAXNUM];                         // 线程
    struct epoll_event ev;                                // epoll 的监听事件
    int ret;                                              // 返回值

    while (1)
    {
        // 这里分配编号, 如果在 accept 期间有个 client 断开连接, 这个归还的编号只会在下个 client 接入时使用
        available_client_num = ret_available_client_num(p_thr_client); // 寻找可用编号, 如果没有就表示已连接client已达最大值
        if (available_client_num >= 0)
        {
            addr_len = sizeof(socket_client_addr[0]);
            if ((p_thr_client->thr_connect_data->sfd_client[available_client_num] = accept(p_thr_client->sfd_server, (struct sockaddr *)&socket_client_addr[available_client_num], &addr_len)) != -1)
            {
                // 此处应该为 epoll 增加一个监听, socket 关闭时也会触发 EPOLLIN 事件
                p_thr_client->ep_event_data[available_client_num].fd = p_thr_client->thr_connect_data->sfd_client[available_client_num];
                p_thr_client->ep_event_data[available_client_num].my_client_num = available_client_num;
                // 设置监听事件
                ev.events = EPOLLIN | EPOLLHUP;
                ev.data.ptr = (void *)&p_thr_client->ep_event_data[available_client_num];
                if (epoll_ctl(p_thr_client->epfd, EPOLL_CTL_ADD, p_thr_client->thr_connect_data->sfd_client[available_client_num], &ev) < 0)
                {
                    perror("epoll_ctl() err");
                    exit(-1);
                }
                p_thr_client->server_link_state = STATE_LINKED; // 有一个 client 连接进来就进入 STATE_LINKED 态
                printf("Client num:%d sfd:%d addr:%s\n", available_client_num, p_thr_client->thr_connect_data->sfd_client[available_client_num], inet_ntoa(socket_client_addr[available_client_num].sin_addr));
                set_client_num_link(p_thr_client, available_client_num);
                // 切换状态了要把 阻塞的 thr_connect 线程杀死然后重启
                if (pthread_cancel(p_thr_client->tid_server_send) < 0)
                {
                    perror("pthread_cancel() err");
                }

                // 回收线程资源
                ret = pthread_join(p_thr_client->tid_server_send, NULL);
                if (ret < 0)
                {
                    fprintf(stderr, "pthread_join() fail:%s\n", strerror(ret));
                }

                if (pthread_create(&p_thr_client->tid_server_send, NULL, thr_server_send, ptr) < 0)
                {
                    perror("pthread_create() err");
                }
                DEBUG_PRINT("Restart thr_connect thread.\n");
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
    int send_len;
    // 发送数据的长度
    if (p_thr_client == NULL)
    {
        fprintf(stderr, "thr_server_send's argument is null!\n");
        pthread_exit(NULL);
    }
    while (1)
    {
        if (p_thr_client->server_link_state == STATE_LINKED) // 已连接
        {
            print_available_client_num(*p_thr_client);
            scanf("%d", &input_client_num);
            if (judge_client_num_is_available(*p_thr_client, input_client_num) != 0)
            {
                printf("\nOut of maximum number! Please reenter.\n");
                continue;
            }
            printf("What are you want to send?\n");
            scanf("%s", send_buf);
            send_len = send(client_num_to_sfd(*p_thr_client, input_client_num), send_buf, strlen(send_buf), 0);
            if (send_len < 0)
            {
                fprintf(stderr, "send() err, len:%d\n", send_len);
                close(p_thr_client->sfd_server);
                exit(-1);
            }
        }
        else if (p_thr_client->server_link_state == STATE_UNLINKED) // 未连接
        {
            printf("Waiting for client ask...\n");
            usleep(1500000);
        }
    }
    pthread_exit(NULL);
}

int main(int argc, char **argv)
{
    struct sockaddr_in socket_server_addr; // 服务器地址
    struct client_num_st client_num;       // 管理 client_num 编号的结构体

    // 初始化 client_num 数据
    client_num.server_link_state = STATE_UNLINKED;
    client_num.real_num = 0;
    // 初始化 读写锁
    if (pthread_rwlock_init(&client_num.client_num_rw_lock, NULL) < 0)
    {
        perror("pthread_rwlock_init() err");
        exit(-1);
    }

    // 申请一个 socket 描述符
    client_num.sfd_server = socket(AF_INET, SOCK_STREAM, 0);
    if (client_num.sfd_server == -1)
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
    if (bind(client_num.sfd_server, (const struct sockaddr *)&socket_server_addr, sizeof(socket_server_addr)) < 0)
    {
        perror("bind() err");
        exit(-1);
    }

    // 监听
    if (listen(client_num.sfd_server, SERVER_MAXNUM) < 0)
    {
        perror("listen() err");
        exit(-1);
    }

    // 创建一个epoll, thr_connect 线程使用
    client_num.epfd = epoll_create(SERVER_MAXNUM);
    if (client_num.epfd < 0)
    {
        perror("epoll_create() err");
        exit(-1);
    }
#if 0
    // 注册信号, 收到 SIGINT 就关闭 socket 结束进程
    if (signal(SIGINT, sig_handle) == SIG_ERR)
    {
        perror("signal() err");
        exit(-1);
    }
#endif

    // 初始化信号量
    if (sem_init(&client_num.my_sem, 0, SERVER_MAXNUM) < 0)
    {
        perror("sem_init() err");
        exit(-1);
    }

    // 初始化数据
    memset(client_num.client_num_flag, NUM_UNLOCK, sizeof(client_num.client_num_flag));
    client_num.current_num = 0;
    // 该线程用于处理 client_num 的连接请求
    if (pthread_create(&client_num.tid_server_link, NULL, thr_server_link, (void *)&client_num))
    {
        perror("pthread_create() err\n");
        exit(-1);
    }

    // 该线程用于接收来自终端的数据, 然后根据编号发给对应的连接设备
    if (pthread_create(&client_num.tid_server_send, NULL, thr_server_send, (void *)&client_num) < 0)
    {
        perror("pthread_create() err\n");
        exit(-1);
    }

    // 为 thr_connect 线程准备参数
    client_num.thr_connect_data = malloc(sizeof(*client_num.thr_connect_data));
    if (client_num.thr_connect_data == NULL)
    {
        fprintf(stderr, "malloc() err: no space!\n");
        exit(-1);
    }
    client_num.thr_connect_data->manage_client = &client_num; // 结构体互引用, thr_connect 线程需要用到 epfd和ep_event 所以要把这个传给它
    // 此线程用于接收 client_num 发送的数据
    if (pthread_create(&client_num.tid_client_connect, NULL, thr_connect, (void *)client_num.thr_connect_data) < 0)
    {
        perror("pthread_create() err");
        exit(-1);
    }

    while (1)
    {
        // pthread_join()
    }
    // 创建的线程还没回收
    close(client_num.sfd_server); // 关闭服务器
    close(client_num.epfd);       // 关闭 epoll
    return 0;
}