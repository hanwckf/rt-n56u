#include <errno.h>
#include <stdlib.h>
/*
static void (*__CTOR_END__[1]) __P((void))
    __attribute__((section(".ctors"))) = { (void *)-1 };

static void (*__DTOR_END__[1]) __P((void))
    __attribute__((__unused__))
    __attribute__((section(".dtors"))) = { (void *)0 };
*/
extern void (*__CTOR_END__[]) __P((void));
static void	__do_global_ctors_aux __P((void));

static void
__do_global_ctors_aux()
{
	void (**p)(void) = __CTOR_END__ - 1;

	while (*p)
		(**p--)();
}

static void dummy_init(void) __attribute__((section(".trash")));

void
dummy_init(void)
{
	static int initialized = 0;
	static void (*volatile call__ctors)(void) = __do_global_ctors_aux;
	/*
	 * Call global constructors.
	 * Arrange to call global destructors at exit.
	 */
	/* prevent function pointer constant propagation */
	__asm__ __volatile__ (".section .init");
	
	if (!initialized) {
		initialized = 1;
		(*call__ctors)();
	}
	__asm__ __volatile__ (".section .trash");

}