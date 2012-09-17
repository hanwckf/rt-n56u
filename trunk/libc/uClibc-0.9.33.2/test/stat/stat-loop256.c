#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
int main()
{
    struct stat statbuf;
    int ret = 0;
    char* loop255 = "/dev/loop255";
    char* loop256 = "/dev/loop256";
    mode_t mode = 0660;
    mknod(loop255, mode, 0x7ff);
    mknod(loop256, mode, 0x100700);
    ret = stat(loop255, &statbuf);
    if(ret < 0) {
	printf("stat: Cant stat %s\n",loop255);
	unlink(loop255);
	exit(1);
    }
    ret = stat(loop256, &statbuf);
    if(ret < 0) {
        printf("stat: Cant stat %s\n",loop256);
	unlink(loop255);
	unlink(loop256);
        exit(1);
    }

    unlink(loop255);
    unlink(loop256);
    exit(0);
}

