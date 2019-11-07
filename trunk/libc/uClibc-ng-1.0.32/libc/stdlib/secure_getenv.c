#include <stdlib.h>
#include <unistd.h>

char *secure_getenv(const char *name) {
	if (issetugid()) return NULL;
	return getenv(name);
}
