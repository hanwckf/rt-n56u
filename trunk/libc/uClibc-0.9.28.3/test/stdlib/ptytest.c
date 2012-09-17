#define _XOPEN_SOURCE
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
    int fd;
    char *cp;

    fd=open("/dev/ptmx",O_NOCTTY|O_RDWR);
    cp=ptsname(fd);
    if (cp==NULL)
	return EXIT_FAILURE;
    printf("ptsname %s\n",cp);
    return EXIT_SUCCESS;
}

