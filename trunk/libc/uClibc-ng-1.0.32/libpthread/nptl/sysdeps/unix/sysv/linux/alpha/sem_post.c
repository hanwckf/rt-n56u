/* ??? This is an ass-backwards way to do this.  We should simply define
   the acquire/release semantics of atomic_exchange_and_add.  And even if
   we don't do this, we should be using atomic_full_barrier or otherwise.  */
#define __lll_rel_instr  "mb"
#include "../sem_post.c"
