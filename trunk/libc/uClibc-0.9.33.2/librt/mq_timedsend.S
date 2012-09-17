#include <sysdep-cancel.h>
#ifndef __NR_mq_timedsend
#error Missing definition of NR_timedsend needed for cancellation.
#endif
PSEUDO (__mq_timedsend, mq_timedsend, 5)
ret
PSEUDO_END(__mq_timedsend)
librt_hidden_def (__mq_timedsend)
weak_alias (__mq_timedsend, mq_timedsend)
librt_hidden_weak (mq_timedsend)
