/*
 * @Description: buzzer 的app
 * @Author: TOTHTOT
 * @Date: 2023-03-01 14:23:57
 * @LastEditTime: 2023-08-19 20:10:54
 * @LastEditors: TOTHTOT
 * @FilePath: \yyh_app\linux_driver\4_gpio_beep\buzzer_app.c
 */
#include "stdio.h"
#include "unistd.h"
#include "sys/types.h"
#include "sys/stat.h"
#include "fcntl.h"
#include "stdlib.h"
#include "string.h"
#include <signal.h>
#include <stdint.h>

#define BUZZEROFF 0
#define BUZZERON 1

typedef struct
{
    int buzzer_fd;              // buzzer驱动的文件描述符
    uint8_t buzzer_state;       // buzzer的状态
    uint8_t buzzer_read_state;  // 读取来的buzzer状态
    int8_t *buzzer_module_path; // buzzer 驱动地址
} buzzer_app_info_t;

buzzer_app_info_t g_buzzer_app_st = {0};

/**
 * @name: sig_handle
 * @msg: 信号处理函数
 * @param {int} sig 信号类型
 * @return {无}
 * @author: TOTHTOT
 * @date: 2023年8月19日
 */
void sig_handle(int sig)
{
    int32_t retvalue = 0;

    switch (sig)
    {
    case SIGINT:
        retvalue = close(g_buzzer_app_st.buzzer_fd); /* 关闭文件 */
        if (retvalue < 0)
        {
            printf("file %s close faibuzzer!\r\n", g_buzzer_app_st.buzzer_module_path);
        }
        exit(0);
        break;

    default:
        break;
    }
}

/**
 * @name: main
 * @msg: main主程序
 * @param {int} argc argv数组元素个数
 * @param {char} *argv  具体参数
 * @return {0} 成功
 * @return {1} 失败
 * @author: TOTHTOT
 * @date: 2023年8月19日
 */
int main(int argc, char *argv[])
{
    int retvalue;

    if (argc != 2)
    {
        printf("Error Usage!\r\n");
        return -1;
    }
    g_buzzer_app_st.buzzer_state = BUZZEROFF;

    signal(SIGINT, sig_handle);

    g_buzzer_app_st.buzzer_module_path = argv[1];

    /* 打开buzzer驱动 */
    g_buzzer_app_st.buzzer_fd = open(g_buzzer_app_st.buzzer_module_path, O_RDWR);
    if (g_buzzer_app_st.buzzer_fd < 0)
    {
        printf("file %s open faibuzzer!\r\n", argv[1]);
        return -1;
    }

    while (1)
    {
        /* 向/dev/buzzer文件写入数据 */
        g_buzzer_app_st.buzzer_state = !g_buzzer_app_st.buzzer_state;
        retvalue = write(g_buzzer_app_st.buzzer_fd, &g_buzzer_app_st.buzzer_state, sizeof(g_buzzer_app_st.buzzer_state));
        read(g_buzzer_app_st.buzzer_fd, &g_buzzer_app_st.buzzer_read_state, sizeof(g_buzzer_app_st.buzzer_read_state));
        // printf("buzzer state %s\n", g_buzzer_app_st.buzzer_read_state == BUZZERON ? "on" : "off");
        usleep(500000);
    }

    return 0;
}
