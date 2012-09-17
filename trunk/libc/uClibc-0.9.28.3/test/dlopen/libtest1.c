#include <stdio.h>

extern int libtest2_func(const char *s);


void __attribute__((constructor)) libtest1_ctor(void)
{
    printf("libtest1: constructor!\n");
}

void __attribute__((destructor)) libtest1_dtor(void)
{
    printf("libtest1: destructor!\n");
}

void __attribute__((weak)) function1(void)
{
    printf("libtest1: I am weak function1!\n");
}

void function2(void)
{
    printf("libtest1: I am function2!\n");
}


int dltest(const char *s)
{
    printf( "libtest1: function1 = %p\n"
	    "libtest1: function2 = %p\n",
	    function1, function2);
    function1();
    function2();
    return(libtest2_func(s));
}


