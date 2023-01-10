#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>

static void *func(void *p)
{
	puts("Thread is working");
	
	pthread_exit(NULL);	//线程结束
}

int main(void)
{
	pthread_t tid;
	int err;
	
	puts("begin\n");
	
	err = pthread_create(&tid, NULL, func, NULL);
	if(err != 0)
	{
		fprintf(stderr, "pthread_create%d\n", strerror(err));
		exit(-1);
	}
	
	pthread_join(tid, NULL);
	puts("end\n");
	return 0;
}
