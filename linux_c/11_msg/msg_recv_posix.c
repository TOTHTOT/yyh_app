#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>
#include <mqueue.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

#define MQ_NAME "/my_mq" // 消息队列名称
mqd_t g_mq_id = 0;      // 消息队列描述符

typedef struct
{
    int message_type;

    struct man_attr
    {
        char name[20];
        int age;
        char sex[10];
    } man_1_st;
} message_t;

/**
 * @name: sig_fun
 * @msg: 信号处理hanshu
 * @param {int} sig_num 信号
 * @return {*} 无
 * @author: TOTHTOT
 * @date: 2023-5-23 10:24:47
 */
void sig_fun(int sig_num)
{
    switch (sig_num)
    {
    case SIGINT:
        // mq_close(g_mq_id);  // 关闭消息队列
        // mq_unlink(MQ_NAME); // 从系统中删除消息队列
        printf("msg_recv_posix exit success\n");
        exit(0);
        break;
    default:
        break;
    }
}

void output_recv_data(message_t *msg,uint32_t mq_prio)
{
    printf("recv %d prio mq\n", mq_prio);
    printf("name:%s\tage:%d\tsex:%s\n", msg->man_1_st.name, msg->man_1_st.age, msg->man_1_st.sex);
}

int main(void)
{
    message_t message = {0};
    uint32_t msg_recv_prio = 0;
    struct mq_attr attr;
    
    signal(SIGINT, sig_fun); // 注册信号处理函数

    // 打开 消息队列
    g_mq_id = mq_open(MQ_NAME, O_CREAT | O_RDWR, 0666, NULL);
    if (g_mq_id == -1)
    {
        perror("mq_open() error");
        exit(-1);
    }
    printf("mq_id = %d\n", g_mq_id);

    mq_getattr(g_mq_id, &attr);

    while (1)
    {
        // 阻塞接收数据
        if (mq_receive(g_mq_id, (char *)&message, attr.mq_msgsize, &msg_recv_prio) == -1)
        {
            perror("mq_receive() error");
        }
        output_recv_data(&message, msg_recv_prio);
        usleep(100000);
    }
}
