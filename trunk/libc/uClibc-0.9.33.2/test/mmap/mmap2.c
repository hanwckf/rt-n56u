/* When trying to map /dev/mem with offset 0xFFFFF000 on the ARM platform, mmap
 * returns -EOVERFLOW.
 *
 * Since off_t is defined as a long int and the sign bit is set in the address,
 * the shift operation shifts in ones instead of zeroes
 * from the left. This results the offset sent to the kernel function becomes
 * 0xFFFFFFFF instead of 0x000FFFFF with MMAP2_PAGE_SHIFT set to 12.
 */

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/mman.h>

#define FATAL do { fprintf(stderr, "Error at line %d, file %s (%d) [%s]\n", \
  __LINE__, __FILE__, errno, strerror(errno)); exit(1); } while(0)

#define MAP_SIZE 4096UL
#define MAP_MASK (MAP_SIZE - 1)

int main(int argc, char **argv) {
    void* map_base = 0;
    int fd;
    off_t target = 0xfffff000;
    if((fd = open("/dev/mem", O_RDWR | O_SYNC)) == -1) {
        /* skip test for non-root users */
        if (errno == EACCES)
            return 0;
        FATAL;
    }
    printf("/dev/mem opened.\n");
    fflush(stdout);

   /* Map one page */
    map_base = mmap(0, MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED,
                        fd, target & ~MAP_MASK);
    if(map_base == (void *) -1) FATAL;
    printf("Memory mapped at address %p.\n", map_base);
    fflush(stdout);
    if(munmap(map_base, MAP_SIZE) == -1) FATAL;
    close(fd);
    return 0;
}
