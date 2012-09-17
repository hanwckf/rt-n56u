#include <stdio.h>
#include <stdlib.h>
#include <tls.h>

#define TLS_VAR_INIT_VALUE 99

#ifdef USE_TLS
extern __thread int tls_var;
#endif

int main(void)
{
	int ret = EXIT_SUCCESS;
#ifdef USE_TLS
	if (tls_var != TLS_VAR_INIT_VALUE) {
		printf("tls_var = %d - Expected value = %d\n", tls_var, TLS_VAR_INIT_VALUE);
		ret = EXIT_FAILURE;
	}
#endif
	return ret;
}
