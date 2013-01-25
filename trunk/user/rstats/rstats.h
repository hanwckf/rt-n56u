#ifndef __RSTATS_H__
#define __RSTATS_H__

#include <netinet/in.h>
#include <stdint.h>
#include <errno.h>


//	#define DEBUG_NOISY
//	#define DEBUG_STIME


#ifdef DEBUG_NOISY
#define _dprintf(args...)	cprintf(args)
#else
#define _dprintf(args...)	do { } while (0)
#endif



#define K 1024
#define M (1024 * 1024)
#define G (1024 * 1024 * 1024)

#define SMIN		60
#define	SHOUR		(60 * 60)
#define	SDAY		(60 * 60 * 24)

#define INTERVAL	60

#define MAX_NSPEED	((24 * SHOUR) / INTERVAL)
#define MAX_NDAILY	62
#define MAX_NMONTHLY	25
#define MAX_SPEED_IF	4
#define MAX_ROLLOVER	(225 * M)

#define MAX_COUNTER	2
#define RX 		0
#define TX 		1

#define DAILY		0
#define MONTHLY		1

#define CURRENT_ID	0x31305352


typedef struct {
	uint32_t xtime;
	uint64_t counter[MAX_COUNTER];
} data_t;

typedef struct {
	char ifname[16];
	long utime;
	uint64_t speed[MAX_NSPEED][MAX_COUNTER];
	uint64_t last[MAX_COUNTER];
	int tail;
	int sync;
} speed_t;

typedef struct {
	uint32_t id;
	data_t daily[MAX_NDAILY];
	int dailyp;
	data_t monthly[MAX_NMONTHLY];
	int monthlyp;
} history_t;


#endif
