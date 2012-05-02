/*****************************************************************************/

/*
 *	cpu.c -- simple CPU usage reporting tool.
 *
 *	(C) Copyright 2000, Greg Ungerer (gerg@snapgear.com)
 */

/*****************************************************************************/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <getopt.h>
#include <errno.h>
#include <signal.h>
#include <string.h>
#include <sys/types.h>
#include <sys/termios.h>
#include <sys/time.h>
#include <sys/swap.h>

/*****************************************************************************/

int main(int argc, char *argv[])
{
	int res;
	res = swapoff(argv[1]);
	printf("swapoff: result - %d\n", res);
	exit(0);
}

/*****************************************************************************/
