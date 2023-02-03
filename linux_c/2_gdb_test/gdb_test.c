#include <stdio.h>

void f()
{
	printf("f is called\n");
}

int i = 0;

int main()
{
	f();
	i = 4;
	printf("hello world\n");
	return 0;
}
