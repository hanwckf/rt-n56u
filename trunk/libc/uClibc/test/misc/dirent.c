#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <dirent.h>

int main(int argc, char **argv)
{

    DIR *dirh;
    struct dirent *dirp;
    static char mydir[20] = "/tmp";

    if ((dirh = opendir(mydir)) == NULL) {
	perror("opendir");
	return 1;
    }

    for (dirp = readdir(dirh); dirp != NULL; dirp = readdir(dirh)) {
	printf("Got dir entry: %s\n",dirp->d_name);
    }

    closedir(dirh);
    return 0;
}

