#include <stdio.h>
#include <tls.h>

#define TLS_VAR_INIT_VALUE 99

#ifdef USE_TLS
__thread int tls_var  __attribute__((tls_model("global-dynamic")));
static __thread int local_tls_var __attribute__((tls_model("local-dynamic")));
#endif

void __attribute__((constructor)) libtls_ctor(void);
void libtls_ctor(void)
{
	printf("libtls: constructor!\n");
#ifdef USE_TLS
	local_tls_var = TLS_VAR_INIT_VALUE;
	tls_var = local_tls_var;
#endif
}

void __attribute__((destructor)) libtls_dtor(void);
void libtls_dtor(void)
{
	printf("libtls: destructor!\n");
}
