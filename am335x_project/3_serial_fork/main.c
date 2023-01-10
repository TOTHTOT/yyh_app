#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include "uart.h"
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>

#define UART_DEV_NAME "/dev/ttyO1"
#define UART_DMA_NAME "/dev/ray_uart"

int main(int argc, char **argv)
{
    pid_t chird_read_pid;
    long looptimes = 100;
    long ture_times = 0;
    long child_receive_times = 0;
    char child_receive_buf[128] = {0};
    char father_send_buf[128] = "123456";
    int uart_fd, uart_dma_fd;
    int err;
    int str_len;

    // 打开串口
    uart_fd = uart_init(uart_fd, UART_DEV_NAME);
    if (uart_fd < 0)
    {
        perror("uart_fd failed");
        return -1;
    }
    uart_dma_fd = uart_init(uart_dma_fd, UART_DMA_NAME);
    if (uart_fd < 0)
    {
        perror("uart_dma_fd failed");
        return -1;
    }

    // 设置串口波特率以及停止符等
    err = uart_set(uart_fd, B3000000, 0, 8, 1, 'n');
    if (err == -1)
    {
        perror("uart_set failed");
        return -1;
    }
#if 1
    err = uart_set(uart_dma_fd, B3000000, 0, 8, 1, 'n');
    if (err == -1)
    {
        perror("uart_dma_fd failed");
        return -1;
    }
#endif
    printf("open uart success version:1.3\r\n uart_fd:%d\tuart_dma_fd:%d\r\n", uart_fd, uart_dma_fd);

    chird_read_pid = fork();
    if (chird_read_pid < 0)
    {
        perror("chird_read_pid error");
        return -1;
    }

    while (1)
    {
        if (chird_read_pid == 0)
        {
            while (looptimes > 0)
            {
                str_len = uart_receive(uart_dma_fd, child_receive_buf, sizeof(child_receive_buf));
                if (strlen(child_receive_buf) != 0)
                {
                    child_receive_times++;
                    looptimes--;
                    printf("receive_times:%ld \t str_len:%d \t child receive:%s\r\n", child_receive_times, str_len, child_receive_buf);
                    if (strcmp(child_receive_buf, father_send_buf) == 0)
                    {
                        ture_times++;
                        // printf("child receive:%s\treve_time:%ld\r\n", child_receive_buf, child_receive_times);
                    }
                    else
                    {
                        // printf("child receive:%s\r\n", child_receive_buf);
                    }
                    memset(child_receive_buf, 0, sizeof(child_receive_buf));
                    if (strstr(child_receive_buf, "exit") != NULL)
                    {
                        break;
                    }
                }
                else
                {
                    // looptimes--;
                    // printf("ret:%d\n", str_len);
                }
            }
            printf("receive_times: %ld\tture_times:%ld\t%f%\r\n", child_receive_times, ture_times, 100.0 * (ture_times * 1.0 / child_receive_times));
            exit(0);
        }
        else
        {
            while (looptimes--)
            {
                uart_send(uart_fd, father_send_buf, strlen(father_send_buf));
                usleep(5000);
            }
            uart_send(uart_fd, "exit\n", 5); // 发送结束
            printf("end\r\n");
            wait(NULL);
            return 0;
        }
    }
}
