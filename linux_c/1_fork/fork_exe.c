#include <unistd.h>
#include "stdlib.h"
#include "stdio.h"


int main(void)
{
	pid_t pid;
	printf("Begin\n");
	fflush(NULL);
	
	pid = fork();
	if(pid < 0)
	{
		perror("fork err");
		return -1;
	}
	
	if(pid == 0)
	{
		if(execl("/bin/date", "date", "+%s", NULL))
		{
			perror("fork err"); exit(-1);
		}
	}
	
	wait(NULL);
	
	printf("End\n");
	return 0;
}
