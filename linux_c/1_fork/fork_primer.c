#include <unistd.h>
#include "stdlib.h"
#include "stdio.h"

#define LEFT 30000000
#define RIGHT 30000200


int main(void)
{
	int i, j, mark;
	pid_t pid;
	
	for(i = LEFT;i <= RIGHT; i++)
	{
		pid = fork();
		if(pid < 0)
		{
			perror("fork err");
			exit(-1);
		}
		if(pid == 0)
		{
			mark = 1;
			for (j = 2; j < i/2; j++)
			{
				if(i%j==0)
				{
					mark = 0;
					break;
				}
			}
			if(mark == 1)
			printf("%d is a primer\n", i);
			exit(0);
		}
	}
	sleep(1000);
	return 0;
}

