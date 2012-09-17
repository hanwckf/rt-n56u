#include "tst-tls10.h"

#ifdef USE_TLS__THREAD
__thread struct A a1 = { 4, 5, 6 };
__thread struct A a2 = { 7, 8, 9 };
#endif
