#include <stdio.h>
#include <pthread.h>


extern int __pthread_mutex_init (void);

void __attribute__((constructor)) libtest2_ctor(void)
{
    printf("libtest2: constructor!\n");
}

void __attribute__((destructor)) libtest2_dtor(void)
{
    printf("libtest2: destructor!\n");
}

void function1(void)
{
    printf("libtest2: I am function1!\n");
}

void __attribute__((weak)) function2(void)
{
    printf("libtest2: I am weak function2!\n");
}


int libtest2_func(const char *s)
{
    printf( "libtest2: function1 = %p\n"
	    "libtest2: function2 = %p\n",
	    function1, function2);
    function1();
    function2();
    return 0;
}


