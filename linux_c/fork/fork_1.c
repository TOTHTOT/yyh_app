#include <unistd.h>
#include "stdlib.h"
#include "stdio.h"

int main(void)
{
	pid_t pid;
	
	printf("begin\n");
	
	fflush(NULL);	// 刷新流
	
	pid = fork();
	if(pid < 0)
	{
		perror("pid err");
		return -1;
	}
	
	if (pid == 0)
	{
		printf("child\n");
	}
	else
	{
		printf("father\n");
	}
	printf("end\n");
	
	return 0;
}
