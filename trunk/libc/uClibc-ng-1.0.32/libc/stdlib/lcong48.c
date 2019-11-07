/* Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball. */

#include <features.h>

#if defined __USE_SVID || defined __USE_XOPEN
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

static int __lcong48_r (unsigned short int param[7], struct drand48_data *buffer)
{
	/* Store the given values. */
	memcpy (buffer->__x, &param[0], sizeof (buffer->__x));
	buffer->__a = ((uint64_t) param[5] << 32 | (uint32_t) param[4] << 16 | param[3]);
	buffer->__c = param[6];
	buffer->__init = 1;

	return 0;
}
# ifdef __USE_MISC
strong_alias(__lcong48_r,lcong48_r)
# endif

void lcong48 (unsigned short int param[7])
{
	(void) __lcong48_r (param, &__libc_drand48_data);
}
#endif
