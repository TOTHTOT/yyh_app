#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include "uart.h"
#include <string.h>
#include <pthread.h>
#include <signal.h>
#include <stdlib.h>

#define UART_DEV_NAME  "/dev/ttyO1"		
#define UART_DMA_NAME  "/dev/ray_uart"


static pthread_mutex_t mut_uart = PTHREAD_MUTEX_INITIALIZER;
int uart_fd, uart_dma_fd;
int uart_state = 0; // 互斥量锁住这个变量， 主进程与串口接收线程通过判断这个值来实现“阻塞”

struct uart_rev_st
{
    int fd;
};


// 线程函数， 用于接收串口数据
static void *thr_uart_reveive(void *s)
{
    struct uart_rev_st reveive = *(struct uart_rev_st *)s;
    char uart_buf[128] = {0};
    char len = 0;

    while(1)
    {
	pthread_mutex_lock(&mut_uart);
	printf("1 lock uart_state:%d\n", uart_state);
	while(uart_state == 0)	// 此处循环是为了保证，出让调度器后，主进程没有马上执行获取锁而导致线程获取了自己释放的锁
	{
		printf("1 unlock\n");
	    pthread_mutex_unlock(&mut_uart);
	    sched_yield();  // 出让调度器保证不是解开了自己释放的锁！！！
	    pthread_mutex_lock(&mut_uart);
	    printf("2 lock\n");
	    
	}
	if(uart_state == -1)
	{
	    pthread_mutex_unlock(&mut_uart);
	    printf("2 unclock\n");
	    break;
	}
	len = uart_receive(uart_dma_fd, uart_buf, sizeof(uart_buf));
	uart_state = 0;	// 置0 接收完成

	printf("len:%d thr receive:%s\n", strlen(uart_buf),  uart_buf);
	memset(uart_buf, 0, strlen(uart_buf));
	pthread_mutex_unlock(&mut_uart);
	printf("3 unclock\n");
	sched_yield();
    }
    pthread_exit(NULL);
}

int main(int argc, char **argv)
{
    long looptimes = 100;
    long ture_times = 0;
    long thr_receive_times = 0;
    char send_buf[128] = "123456789";
    int err;
    int str_len;
    pthread_t tid;
    struct uart_rev_st *uar_fd;


    // 打开串口
    uart_fd = uart_init(uart_fd, UART_DEV_NAME);

    if(uart_fd < 0)
    {
	perror("uart_fd failed");
	return -1;
    }
    uart_dma_fd = uart_init(uart_dma_fd, UART_DMA_NAME);
    if(uart_fd < 0)
    {
	perror("uart_dma_fd failed");
	return -1;
    }

    // 设置串口波特率以及停止符等
    err = uart_set(uart_fd, B3000000, 0, 8, 1, 'n');
    if(err == -1)
    {
	perror("uart_set failed");
	return -1;
    }
#if 1
    err = uart_set(uart_dma_fd, B3000000, 0, 8, 1, 'n');
    if(err == -1)	
    {
	perror("uart_dma_fd failed");
	return -1;
    }
#endif

    printf("open uart success version:1.2\r\n uart_fd:%d\tuart_dma_fd:%d\r\n", uart_fd, uart_dma_fd);
    
    // 创建一个线程
    uar_fd = (struct uart_rev_st *)malloc(sizeof(struct uart_rev_st));
    uar_fd->fd = dup(uart_dma_fd);  // 复制描述符， 传递给线程
    err = pthread_create(&tid, NULL, thr_uart_reveive, uar_fd);
    if(err)
    {
	fprintf(stderr, "pthread_create():%s\n", strerror(err));
	exit(-1);
    }
    
    
    while(looptimes>0)
    {
	pthread_mutex_lock(&mut_uart);	// 发送时锁住， 发送完成解锁
	printf("3 lock uart_state:%d\n", uart_state);

	while(uart_state == 1)	// 等于1说明进程解锁了，但是锁被自己接收了
	{
	    // 自己抢了自己的锁那就再锁上
	    pthread_mutex_unlock(&mut_uart);  // 锁主
	    	printf("4 unlock\n");
	    pthread_mutex_lock(&mut_uart);	// 解锁
	    	printf("4 lock\n");
	    sched_yield();  // 切换调度， 不让自己抢到
	}

	uart_send(uart_dma_fd, send_buf, strlen(send_buf));
	looptimes--;
	uart_state = 1;	// 置1 发送完成
	pthread_mutex_unlock(&mut_uart);
	printf("5 unclock\n");
	sched_yield();
    }
    
    printf("send finish\n");
    pthread_mutex_lock(&mut_uart);
    while(uart_state == 1)  //确保最后一次发送能被线程接收到
    {
	pthread_mutex_unlock(&mut_uart);
	sched_yield();
	pthread_mutex_lock(&mut_uart);
	
    }
    uart_state = -1;	// 等于-1 所有数据发送完成
    pthread_mutex_unlock(&mut_uart);	// 解锁

    pthread_join(tid, NULL);	// 等待线程接收到-1后结束自己
    pthread_mutex_destroy(&mut_uart);	//销毁互斥量
    return 0;
}


