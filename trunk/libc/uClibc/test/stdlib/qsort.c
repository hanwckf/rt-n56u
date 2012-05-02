#include <stdio.h>
#include <dirent.h>
#include <stdlib.h>
#include <unistd.h>

int select_files(const struct dirent *dirbuf)
{
    if (dirbuf->d_name[0] == '.')
	return 0;
    else         
	return 1;
}


int main(void)
{
    struct dirent **array;
    struct dirent *dirbuf;

    int i, numdir;        

    chdir("/");
    numdir = scandir(".", &array, select_files, NULL);
    printf("\nGot %d entries from scandir().\n", numdir);              
    for (i = 0; i < numdir; ++i) {      
	dirbuf = array[i];
	printf("[%d] %s\n", i, dirbuf->d_name);
	free(array[i]);
    }                  
    free(array);
    numdir = scandir(".", &array, select_files, alphasort);
    printf("\nGot %d entries from scandir() using alphasort().\n", numdir);                   
    for (i = 0; i < numdir; ++i) {      
	dirbuf = array[i];
	printf("[%d] %s\n", i, dirbuf->d_name);
    }                                          
    printf("\nCalling qsort()\n");                   
    qsort(array, numdir, sizeof(struct dirent *), alphasort);
    for (i = 0; i < numdir; ++i) {                           
	dirbuf = array[i];
	printf("[%d] %s\n", i, dirbuf->d_name);
	free(array[i]);
    }
    free(array);
    return(0);
}

