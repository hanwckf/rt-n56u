#include <syscall.h>
#include <unistd.h>

int setpgrp(void)
{
	return setpgid(0,0);
}
