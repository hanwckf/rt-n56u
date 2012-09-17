#include <dirent.h>
#include <string.h>
#include "dirstream.h"

int alphasort(const void * a, const void * b)
{
    return strcmp ((*(const struct dirent **) a)->d_name,
	    (*(const struct dirent **) b)->d_name);
}

