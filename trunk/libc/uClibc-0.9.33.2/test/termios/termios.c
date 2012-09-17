#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>
#include <asm/ioctls.h>
#include <sys/ioctl.h>

int main(int argc,char *argv[])
{
	struct termios t;
	int ret;

	printf("TCGETS = 0x%08x\n",TCGETS);
	printf("sizeof(struct termios) = %ld\n",(long)sizeof(struct termios));

	ret = ioctl(fileno(stdout),TCGETS,&t);

	if(ret<0){
		perror("ioctl");
	}else{
		printf("ioctl returned %d\n",ret);
	}

	return 0;
}
