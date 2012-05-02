#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <dlfcn.h>

extern int dltest(const char *s);

int main(int argc, char **argv)
{
	dltest("hello world!");
	return EXIT_SUCCESS;
}

