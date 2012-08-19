#ifndef __RSTATS_H__
#define __RSTATS_H__

// #include <tomato_profile.h>

#include <netinet/in.h>
#include <stdint.h>
#include <errno.h>

#define Y2K			946684800UL		// seconds since 1970

#define ASIZE(array)	(sizeof(array) / sizeof(array[0]))



// misc.c
#define	WP_DISABLED		0		// order must be synced with def in misc.c
#define	WP_STATIC		1
#define WP_DHCP			2
#define	WP_L2TP			3
#define	WP_PPPOE		4
#define	WP_PPTP			5

enum {
	ACT_IDLE,
	ACT_TFTP_UPGRADE_UNUSED,
	ACT_WEB_UPGRADE,
	ACT_WEBS_UPGRADE_UNUSED,
	ACT_SW_RESTORE,
	ACT_HW_RESTORE,
	ACT_ERASE_NVRAM,
	ACT_NVRAM_COMMIT,
	ACT_REBOOT,
	ACT_UNKNOWN
};

typedef struct {
	int count;
	struct in_addr dns[3];
} dns_list_t;



#define SUP_SES			(1 << 0)
#define SUP_BRAU		(1 << 1)
#define SUP_AOSS_LED	(1 << 2)
#define SUP_WHAM_LED	(1 << 3)
#define SUP_HPAMP		(1 << 4)
#define SUP_NONVE		(1 << 5)
#define SUP_80211N		(1 << 6)

extern int check_hw_type(void);
//	extern int get_hardware(void) __attribute__ ((weak, alias ("check_hw_type")));
extern int get_model(void);
extern int supports(unsigned long attr);


#endif
