/*
 * @Description: 串口驱动
 * @Author: TOTHTOT
 * @Date: 2023-01-04 17:35:26
 * @LastEditTime: 2023-01-05 14:53:07
 * @LastEditors: TOTHTOT
 * @FilePath: \am335x_project\Serial_project_Fork\uart.c
 */
/*
 *                                                     __----~~~~~~~~~~~------___
 *                                    .  .   ~~//====......          __--~ ~~
 *                    -.            \_|//     |||\\  ~~~~~~::::... /~
 *                 ___-==_       _-~o~  \/    |||  \\            _/~~-
 *         __---~~~.==~||\=_    -_--~/_-~|-   |\\   \\        _/~
 *     _-~~     .=~    |  \\-_    '-~7  /-   /  ||    \      /
 *   .~       .~       |   \\ -_    /  /-   /   ||      \   /
 *  /  ____  /         |     \\ ~-_/  /|- _/   .||       \ /
 *  |~~    ~~|--~~~~--_ \     ~==-/   | \~--===~~        .\
 *           '         ~-|      /|    |-~\~~       __--~~
 *                       |-~~-_/ |    |   ~\_   _-~            /\
 *                            /  \     \__   \/~                \__
 *                        _--~ _/ | .-~~____--~-/                  ~~==.
 *                       ((->/~   '.|||' -_|    ~~-/ ,              . _||
 *                                  -_     ~\      ~~---l__i__i__i--~~_/
 *                                  _-~-__   ~)  \--______________--~~
 *                                //.-~~~-~_--~- |-------~~~~~~~~
 *                                       //.-~~~--\
 *                       ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *
 *                               神兽保佑            永无BUG
 */

#include "uart.h"
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>

// 打开串口
int uart_init(int fd, char *path)
{
    // O_RDWR: 可读可写；O_NOCTTY：不当做终端
    fd = open(path, O_RDWR | O_NOCTTY | O_NONBLOCK);
    if (fd < 0)
    {
        perror("fd open failed");
        return -1;
    }
    // 串口驱动没有实现阻塞方式!!!!!
#if USD_BLOCK
    // 串口恢复阻塞
    if (fcntl(fd, F_SETFL, 0) < 0)
    {
        perror("fcntl failed");
        return -1;
    }
#endif

    if (isatty(STDIN_FILENO) == 0)
    {
        perror("fd is not terminal");
        return -1;
    }
    else
    {
        printf("fd open:%d\r\n", fd);
    }
    return fd;
}

// 串口关闭
void uart_close(int fd)
{
    close(fd);
}

// 设置串口
/**
 * @name: uart_set
 * @msg: 串口设置参数设置
 * @param {int} fd  文件描述符
 * @param {int} u_speed 串口波特率
 * @param {int} flow_ctrl   流控制
 * @param {int} databits    数据位
 * @param {int} stopbits    停止位
 * @param {int} parity      校验位
 * @return {*}  == 0,   成功
 *              == -1,  失败
 */
int uart_set(int fd, int u_speed, int flow_ctrl, int databits, int stopbits, int parity)
{
    int i;
    int status;
    struct termios options;

    /*  tcgetattr(fd,&options)得到与fd指向对象的相关参数，并将它们保存于options,该函数还可以测试配置是否正确，
        该串口是否可用等。若调用成功，函数返回值为0，若调用失败，函数返回值为1.  */
    if (tcgetattr(fd, &options) != 0)
    {
        perror("set serial failed");
        return -1;
    }

    // 设置串口输入输出速率
    cfsetispeed(&options, u_speed);
    cfsetospeed(&options, u_speed);

    // 修改控制模式，保证程序不会占用串口
    options.c_cflag |= CLOCAL;
    // 修改控制模式，使得能够从串口中读取输入数据
    options.c_cflag |= CREAD;

    // 设置数据流控制
    switch (flow_ctrl)
    {

    case 0: // 不使用流控制
        options.c_cflag &= ~CRTSCTS;
        break;
    case 1: // 使用硬件流控制
        options.c_cflag |= CRTSCTS;
        break;
    case 2: // 使用软件流控制
        options.c_cflag |= IXON | IXOFF | IXANY;
        break;
    }

    // 设置数据位
    // 屏蔽其他标志位
    options.c_cflag &= ~CSIZE;
    switch (databits)
    {
    case 5:
        options.c_cflag |= CS5;
        break;
    case 6:
        options.c_cflag |= CS6;
        break;
    case 7:
        options.c_cflag |= CS7;
        break;
    case 8:
        options.c_cflag |= CS8;
        break;
    default:
        fprintf(stderr, "Unsupported data size\n");
        return -1;
    }
    // 设置校验位
    switch (parity)
    {
    case 'n':
    case 'N': // 无奇偶校验位。
        options.c_cflag &= ~PARENB;
        options.c_iflag &= ~INPCK;
        break;
    case 'o':
    case 'O': // 设置为奇校验
        options.c_cflag |= (PARODD | PARENB);
        options.c_iflag |= INPCK;
        break;
    case 'e':
    case 'E': // 设置为偶校验
        options.c_cflag |= PARENB;
        options.c_cflag &= ~PARODD;
        options.c_iflag |= INPCK;
        break;
    case 's':
    case 'S': // 设置为空格
        options.c_cflag &= ~PARENB;
        options.c_cflag &= ~CSTOPB;
        break;
    default:
        fprintf(stderr, "Unsupported parity\n");
        return -1;
    }
    // 设置停止位
    switch (stopbits)
    {
    case 1:
        options.c_cflag &= ~CSTOPB;
        break;
    case 2:
        options.c_cflag |= CSTOPB;
        break;
    default:
        fprintf(stderr, "Unsupported stop bits\n");
        return -1;
    }

    // 修改输出模式，原始数据输出
    options.c_oflag &= ~OPOST;

    options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
    // options.c_lflag &= ~(ISIG | ICANON);

    // 设置等待时间和最小接收字符
    options.c_cc[VTIME] = 0; /* 读取一个字符等待1*(1/10)s */
    options.c_cc[VMIN] = 1;  /* 读取字符的最少个数为 1 */

    // 如果发生数据溢出，接收数据，但是不再读取 刷新收到的数据但是不读
    tcflush(fd, TCIFLUSH);

    // 激活配置 (将修改后的termios数据设置到串口中）
    if (tcsetattr(fd, TCSANOW, &options) != 0)
    {
        perror("com set error!\n");
        return -1;
    }
    return 0;
}

/**
 * @name: uart_receive
 * @msg: 串口接收
 * @param {int} fd 文件描述符
 * @param {char} *rcv_buf   接收缓存
 * @param {int} data_len    接收数据长度
 * @return {*} >= 0     成功, 返回数据长度
 *             == -1    失败, 超时
 *             == -2    失败, select 出错
 */
int uart_receive(int fd, char *rcv_buf, int data_len)
{
#if 1
    int len, fs_sel;
    fd_set fs_read;

    struct timeval time;

    FD_ZERO(&fs_read);
    FD_SET(fd, &fs_read);

    time.tv_sec = 10;
    time.tv_usec = 0;

    // 使用select实现串口的多路通信
    fs_sel = select(fd + 1, &fs_read, NULL, NULL, NULL);
    // printf("fs_sel = %d\n", fs_sel);
    if (fs_sel) // 正常读取到数据
    {
        if (FD_ISSET(fd, &fs_read))
        {
            len = read(fd, rcv_buf, data_len);
        }
        else
        {
            len = -1;
        }
        return len;
    }
    else if (fs_sel == 0) // 超时
    {
        perror("select time out");
        return -1;
    }
    else // 出错
    {
        perror("select() err");
        return -2;
    }
#else
    int len;
    len = read(fd, rcv_buf, data_len);
    return len;
#endif
}

/**
 * @name: uart_send
 * @msg: 串口发送
 * @param {int} fd 文件描述符
 * @param {char} *send_buf  发送内容
 * @param {int} data_len    发送长度
 * @return {*}  >= 0,   成功
 *              == -1,  失败
 */
int uart_send(int fd, char *send_buf, int data_len)
{
    int len = 0;

    printf("len:%d uart_send data:%s\r\n", data_len, send_buf);
    len = write(fd, send_buf, data_len);
    if (len == data_len)
    {
        return len;
    }
    else
    {
        tcflush(fd, TCOFLUSH);
        return -1;
    }
}
