#ifndef IPT2SOCKS_LOGUTILS_H
#define IPT2SOCKS_LOGUTILS_H

#define _GNU_SOURCE
#include <stdio.h>
#include <time.h>
#undef _GNU_SOURCE

#define LOGINF(fmt, ...)                                                     \
    do {                                                                     \
        struct tm *tm = localtime_r(&(time_t){time(NULL)}, &(struct tm){0}); \
        printf("\e[1;32m%04d-%02d-%02d %02d:%02d:%02d INF:\e[0m " fmt "\n",  \
                tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday,             \
                tm->tm_hour,        tm->tm_min,     tm->tm_sec,              \
                ##__VA_ARGS__);                                              \
    } while (0)

#define LOGERR(fmt, ...)                                                     \
    do {                                                                     \
        struct tm *tm = localtime_r(&(time_t){time(NULL)}, &(struct tm){0}); \
        printf("\e[1;35m%04d-%02d-%02d %02d:%02d:%02d ERR:\e[0m " fmt "\n",  \
                tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday,             \
                tm->tm_hour,        tm->tm_min,     tm->tm_sec,              \
                ##__VA_ARGS__);                                              \
    } while (0)

#endif
