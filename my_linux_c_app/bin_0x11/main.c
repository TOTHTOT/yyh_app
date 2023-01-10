#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

int main(void)
{

    int i = 0, err = 0;
    int fd;
    char buf[100] = {0x11};

    fd = open("./bin_0x11.bin", O_RDWR);
    if(fd < 0)
    {
	perror("open err");
	exit(-1);
    }
    memset(buf, 0x11, 100);
    printf("open success, fd:%d\n", fd);
    while(i < 82)
    {
    	err = write(fd, buf, 100);
	// fprintf(fd, "%x", 0x11);
	if(err == -1)
	{
	    perror("write err");
	    exit(-1);
	}
        i++;
	if(i%2==0)
	{
	    printf("write times: %d\n", i);
	}
    }
    close(fd);
    return 0;
}
