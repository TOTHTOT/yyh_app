#include <signal.h>
#include <stdio.h>
#include <unistd.h>

void sig_handler(int sig)
{
	write(1, "!", 1);
}

int main(void)
{
	int i;
	// signal(SIGINT, SIG_IGN);	// 忽略掉ctrl+c
	signal(SIGINT, sig_handler);	// 信号会打断阻塞的系统调用！！！！ 一直按着 ctrl+c 会把 sleep 打断
	
	for(i = 0; i < 10; i++)
	{
		write(1, "*", 1);
		sleep(1);	// sleep阻塞了 write（这是系统调用）
	}
	
	return 0;
}
