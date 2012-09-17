#include <limits.h>
#include <sys/time.h>
#include <sys/timex.h>
#include <errno.h>

#define MAX_SEC	(LONG_MAX / 1000000L - 2)
#define MIN_SEC	(LONG_MIN / 1000000L + 2)

#ifndef MOD_OFFSET
#define modes mode
#endif

int
adjtime(const struct timeval * itv, struct timeval * otv)
{
  struct timex tntx;

  if (itv)
  {
    struct timeval tmp;

    /* We will do some check here. */
    tmp.tv_sec = itv->tv_sec + itv->tv_usec / 1000000L;
    tmp.tv_usec = itv->tv_usec % 1000000L;
    if (tmp.tv_sec > MAX_SEC || tmp.tv_sec < MIN_SEC)
    {
	__set_errno(EINVAL);
	return -1;
    }
    tntx.offset = tmp.tv_usec + tmp.tv_sec * 1000000L;
    tntx.modes = ADJ_OFFSET_SINGLESHOT;
  }
  else
  {
    tntx.modes = 0;
  }
  if (adjtimex(&tntx) < 0) return -1;
  if (otv) {
    if (tntx.offset < 0)
      {
	otv->tv_usec = -(-tntx.offset % 1000000);
	otv->tv_sec  = -(-tntx.offset / 1000000);
      }
    else
      {
	otv->tv_usec = tntx.offset % 1000000;
	otv->tv_sec  = tntx.offset / 1000000;
      }
  }
  return 0;
}

