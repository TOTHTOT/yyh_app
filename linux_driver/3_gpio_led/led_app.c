/*
 * @Description: led 的app
 * @Author: TOTHTOT
 * @Date: 2023-03-01 14:23:57
 * @LastEditTime: 2023-08-19 20:04:56
 * @LastEditors: TOTHTOT
 * @FilePath: \yyh_app\linux_driver\3_gpio_led\led_app.c
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

#define LEDOFF 0
#define LEDON 1

typedef struct
{
    int led_fd;              // led驱动的文件描述符
    uint8_t led_state;       // led的状态
    uint8_t led_read_state;  // 读取来的led状态
    int8_t *led_module_path; // led 驱动地址
} led_app_info_t;

led_app_info_t g_led_app_st = {0};

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
        retvalue = close(g_led_app_st.led_fd); /* 关闭文件 */
        if (retvalue < 0)
        {
            printf("file %s close failed!\r\n", g_led_app_st.led_module_path);
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
    g_led_app_st.led_state = LEDOFF;

    signal(SIGINT, sig_handle);

    g_led_app_st.led_module_path = argv[1];

    /* 打开led驱动 */
    g_led_app_st.led_fd = open(g_led_app_st.led_module_path, O_RDWR);
    if (g_led_app_st.led_fd < 0)
    {
        printf("file %s open failed!\r\n", argv[1]);
        return -1;
    }

    while (1)
    {
        /* 向/dev/led文件写入数据 */
        g_led_app_st.led_state = !g_led_app_st.led_state;
        retvalue = write(g_led_app_st.led_fd, &g_led_app_st.led_state, sizeof(g_led_app_st.led_state));
        read(g_led_app_st.led_fd, &g_led_app_st.led_read_state, sizeof(g_led_app_st.led_read_state));
        // printf("led state %s\n", g_led_app_st.led_read_state == LEDON ? "on" : "off");
        usleep(500000);
    }

    return 0;
}
