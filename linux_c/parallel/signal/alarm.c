#include <signal.h>
#include <stdio.h>
#include <unistd.h>

char flag = 1;

static void sig_handler(int sig)
{
	if(sig == SIGALRM)
	{
		flag = 0;
	}
}

int main(void)
{
	long  l_i = 0;
	
	signal(SIGALRM, sig_handler);
	alarm(5);
	while(flag == 1)
	{
		l_i++;
	}
	printf("times:%ld\n", l_i);
	return 0;
}
