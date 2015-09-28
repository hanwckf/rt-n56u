#ifndef _CMD_TFTPSVR_H
#define _CMD_TFTPSVR_H

#include <common.h>
#include <command.h>
#include <net.h>
#include <image.h>
//#include "bootp.h"
#include <gpio.h>

#define PORT_TFTP	69		/* Well known TFTP port #		*/
#define TIMEOUT		2		/* Seconds to timeout for a lost pkt	*/
#define TIMEOUT_COUNT	10		/* # of timeouts before giving up  */

					/* (for checking the image size)	*/
#define HASHES_PER_LINE	65		/* Number of "loading" hashes per line	*/

/*
 *	TFTP operations.
 */
#define TFTP_RRQ	1
#define TFTP_WRQ	2
#define TFTP_DATA	3
#define TFTP_ACK	4
#define TFTP_ERROR	5
#define TFTP_FINISHACK	6

#define STATE_RRQ	1
#define STATE_WRQ	2
#define STATE_DATA	3
#define STATE_TOO_LARGE	4
#define STATE_BAD_MAGIC	5
#define STATE_FINISHACK	6


#define TFTP_BLOCK_SIZE		512		    /* default TFTP block size	*/
#define TFTP_SEQUENCE_SIZE	((ulong)(1<<16))    /* sequence number is 16 bit */

#define DEFAULT_NAME_LEN	(8 + 4 + 1)

#define TRX_MAGIC		0x56190527      /* Image Magic Number */
/**********************************************************************/
/*
 *	Global functions and variables.
 */

/* tftpd.c */
int do_tftpd(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[]);
void TftpdStart(void);// Begin TFTP get

/**********************************************************************/

extern void LED_POWER_ON(void);
extern void LED_ALERT_ON(void);
extern void LED_ALERT_OFF(void);


/*
Error message defined in RFC1350. 
char *tftp_errmsg[9] = {
     "Undefined error code",
     "File not found",
     "Access violation",
     "Disk full or allocation exceeded",
     "Illegal TFTP operation",
     "Unknown transfer ID",
     "File already exists",
     "No such user",
     "Failure to negotiate RFC1782 options",
};
*/

#endif
