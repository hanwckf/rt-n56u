#include <sys/io.h>

int main(void)
{
    ioperm(0x340,0x342,1);
    outb(0x340,0x0);
    exit(0);
}

