#include <unistd.h>
#include "stdlib.h"
#include "stdio.h"

int main()
{
	// system 是对fork exec wait 的封装
	system("date +%s");
	
	return 0;
}
