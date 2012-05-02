#ifndef _IP_CONNTRACK_TALK_H
#define _IP_CONNTRACK_TALK_H
/* TALK tracking. */

#ifdef __KERNEL__
#include <linux/in.h>
#include <linux/netfilter_ipv4/lockhelp.h>

/* Protects talk part of conntracks */
DECLARE_LOCK_EXTERN(ip_talk_lock);
#endif


#define TALK_PORT	517
#define NTALK_PORT	518

/* talk structures and constants from <protocols/talkd.h> */

/*
 * 4.3BSD struct sockaddr
 */
struct talk_addr {
	u_int16_t ta_family;
	u_int16_t ta_port;
	u_int32_t ta_addr;
	u_int32_t ta_junk1;
	u_int32_t ta_junk2;
};

#define	TALK_OLD_NSIZE	9
#define	TALK_NSIZE	12
#define	TALK_TTY_NSIZE	16

/*
 * Client->server request message formats.
 */
struct talk_msg {
	u_char	type;		/* request type, see below */
	char	l_name[TALK_OLD_NSIZE];/* caller's name */
	char	r_name[TALK_OLD_NSIZE];/* callee's name */
	u_char	pad;
	u_int32_t id_num;	/* message id */
	int32_t	pid;		/* caller's process id */
	char	r_tty[TALK_TTY_NSIZE];/* callee's tty name */
	struct	talk_addr addr;		/* old (4.3) style */
	struct	talk_addr ctl_addr;	/* old (4.3) style */
};

struct ntalk_msg {
	u_char	vers;		/* protocol version */
	u_char	type;		/* request type, see below */
	u_char	answer;		/* not used */
	u_char	pad;
	u_int32_t id_num;	/* message id */
	struct	talk_addr addr;		/* old (4.3) style */
	struct	talk_addr ctl_addr;	/* old (4.3) style */
	int32_t	pid;		/* caller's process id */
	char	l_name[TALK_NSIZE];/* caller's name */
	char	r_name[TALK_NSIZE];/* callee's name */
	char	r_tty[TALK_TTY_NSIZE];/* callee's tty name */
};

struct ntalk2_msg {
	u_char	vers;		/* talk protocol version    */
	u_char	type;		/* request type             */
	u_char	answer;		/*  */
	u_char	extended;	/* !0 if additional parts   */
	u_int32_t id_num;	/* message id number (dels) */
	struct	talk_addr addr;		/* target address   */
	struct	talk_addr ctl_addr;	/* reply to address */
	int32_t	pid;		/* caller's process id */
	char	l_name[TALK_NSIZE];  /* caller's name */
	char	r_name[TALK_NSIZE];  /* callee's name */
	char	r_tty[TALK_TTY_NSIZE];    /* callee's tty */
};

/*
 * Server->client response message formats.
 */
struct talk_response {
	u_char	type;		/* type of request message, see below */
	u_char	answer;		/* response to request message, see below */
	u_char	pad[2];
	u_int32_t id_num;	/* message id */
	struct	talk_addr addr;	/* address for establishing conversation */
};

struct ntalk_response {
	u_char	vers;		/* protocol version */
	u_char	type;		/* type of request message, see below */
	u_char	answer;		/* response to request message, see below */
	u_char	pad;
	u_int32_t id_num;	/* message id */
	struct	talk_addr addr;	/* address for establishing conversation */
};

struct ntalk2_response {
	u_char	vers;		/* protocol version         */
	u_char	type;		/* type of request message  */
	u_char	answer;		/* response to request      */
	u_char	rvers;		/* Version of answering vers*/
	u_int32_t id_num;	/* message id number        */
	struct	talk_addr addr;	/* address for connection   */
	/* This is at the end to compatiblize this with NTALK version.   */
	char	r_name[TALK_NSIZE]; /* callee's name            */
};

#define TALK_STR(data, talk_str, member) ((struct talk_str *)data)->member)
#define TALK_RESP(data, ver, member) (ver ? ((struct ntalk_response *)data)->member : ((struct talk_response *)data)->member)
#define TALK_MSG(data, ver, member) (ver ? ((struct ntalk_msg *)data)->member : ((struct talk_msg *)data)->member)

#define	TALK_VERSION	0		/* protocol versions */
#define	NTALK_VERSION	1
#define	NTALK2_VERSION	2

/* message type values */
#define LEAVE_INVITE	0	/* leave invitation with server */
#define LOOK_UP		1	/* check for invitation by callee */
#define DELETE		2	/* delete invitation by caller */
#define ANNOUNCE	3	/* announce invitation by caller */
/* NTALK2 */
#define REPLY_QUERY	4	/* request reply data from local daemon */

/* answer values */
#define SUCCESS		0	/* operation completed properly */
#define NOT_HERE	1	/* callee not logged in */
#define FAILED		2	/* operation failed for unexplained reason */
#define MACHINE_UNKNOWN	3	/* caller's machine name unknown */
#define PERMISSION_DENIED 4	/* callee's tty doesn't permit announce */
#define UNKNOWN_REQUEST	5	/* request has invalid type value */
#define	BADVERSION	6	/* request has invalid protocol version */
#define	BADADDR		7	/* request has invalid addr value */
#define	BADCTLADDR	8	/* request has invalid ctl_addr value */
/* NTALK2 */
#define NO_CALLER	9	/* no-one calling answer from REPLY   */
#define TRY_HERE	10	/* Not on this machine, try this      */
#define SELECTIVE_REFUSAL 11	/* User Filter refusal.               */
#define MAX_RESPONSE_TYPE 11	/* Make sure this is updated          */

/* We don't really need much for talk */
struct ip_ct_talk_expect
{
	/* Port that was to be used */
	u_int16_t port;
};

/* This structure exists only once per master */
struct ip_ct_talk_master
{
};

#endif /* _IP_CONNTRACK_TALK_H */
