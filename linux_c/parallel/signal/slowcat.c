// 令牌桶

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>

#define CPS 10
#define BUFFSIZE CPS
#define BURST 100
#define ReadFileParh "/usr/lib/python2.7/lib2to3/Grammar.txt"

static volatile int token = 0;

static void sig_handler(int sig)
{
	
	switch(sig)
	{
		case SIGALRM:
			alarm(1);
			token++;
			if(token > BURST)
			{
				token = BURST;
			}
			break;
	}
}


int main(void)
{
	int len, ret, pos;
	int sfd, dfd =1;
	char read_buf[BUFFSIZE] = {0};
	
	signal(SIGALRM, sig_handler);
	alarm(1);
	
	if((sfd = open(ReadFileParh, O_RDONLY))<0)
	{
		perror("open failed");
		exit (-1);
	}
	
	
	while(1)
	{
		while(token <= 0)	// token 小于0 进入 pause 等待 alarm 发送信号
		{
			pause();
		}
		token--;
		while((len =read(sfd, read_buf, BUFFSIZE)) <= 0)	// 当 read 返回值小于0 判断是读到文件末尾还是读取错误
		{
			if(errno == EINTR)	// 读取到文件末尾
			{
				printf("end of file\n");
				continue;
			}
			else
			{
				perror("read error");
				break;
			}
		}
		while(len > 0)
		{
			write(1, read_buf, len);
			len = 0;
		}
		
	}
}

