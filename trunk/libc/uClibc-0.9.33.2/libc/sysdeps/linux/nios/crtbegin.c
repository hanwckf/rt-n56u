#include <errno.h>
#include <stdlib.h>
/*
static void (*__CTOR_LIST__[1]) __P((void))
    __attribute__((__unused__))
    __attribute__((section(".ctors"))) = { (void *)0 };

static void (*__DTOR_LIST__[1]) __P((void))
    __attribute__((section(".dtors"))) = { (void *)-1 };
*/
extern void (*__DTOR_LIST__[]) __P((void));
static void	__do_global_dtors_aux __P((void));

static void
__do_global_dtors_aux()
{
	void (**p)(void) = __DTOR_LIST__ + 1;

	while (*p)
		(**p++)();
}

static void dummy_fini(void) __attribute__((section(".trash")));

void
dummy_fini(void)
{
	static void (* volatile call__dtors)(void) = __do_global_dtors_aux;
	/*
	 * Call global destructors.
	 */
	/* prevent function pointer constant propagation */
	__asm__ __volatile__ (".section .fini");
	(*call__dtors)();
	__asm__ __volatile__ (".section .trash");

}
