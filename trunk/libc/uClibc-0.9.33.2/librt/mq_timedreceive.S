#include <sysdep-cancel.h>
#ifndef __NR_mq_timedreceive
#error Missing definition of NR_timedreceive needed for cancellation.
#endif
PSEUDO (__mq_timedreceive, mq_timedreceive, 5)
ret
PSEUDO_END(__mq_timedreceive)
librt_hidden_def (__mq_timedreceive)
weak_alias (__mq_timedreceive, mq_timedreceive)
librt_hidden_weak (mq_timedreceive)
