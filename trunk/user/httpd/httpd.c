/*
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <time.h>
#include <unistd.h>
#include <ctype.h>
#include <error.h>
#include <sys/signal.h>
#include <sys/sysinfo.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <assert.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <net/if.h>

#include "httpd.h"

/* Basic authorization userid and passwd limit */
#define AUTH_MAX		64

#define LOGIN_TIMEOUT		60
#define SERVER_NAME		"httpd"
#define SERVER_PORT		80
#define SERVER_PORT_SSL		443
#define PROTOCOL		"HTTP/1.0"
#define RFC1123FMT		"%a, %d %b %Y %H:%M:%S GMT"

/* A multi-family in_addr. */
typedef struct {
    union {
        struct in_addr in4;
#if defined (USE_IPV6)
        struct in6_addr in6;
#endif
    } addr;
    int family;
    int len;
} uaddr;

/* A multi-family sockaddr. */
typedef union {
    struct sockaddr sa;
    struct sockaddr_in sa_in;
#if defined (USE_IPV6)
    struct sockaddr_in6 sa_in6;
#endif
} usockaddr;

#include "queue.h"
#define MAX_CONN_ACCEPT 64
#define MAX_CONN_TIMEOUT 30

typedef struct conn_item {
	TAILQ_ENTRY(conn_item) entry;
	int fd;
#if defined (SUPPORT_HTTPS)
	int ssl;
#endif
	usockaddr usa;
} conn_item_t;

typedef struct conn_list {
	TAILQ_HEAD(, conn_item) head;
	int count;
} conn_list_t;

/* Globals. */
static int daemon_exit = 0;
static int reget_passwd = 0;
static char auth_userid[AUTH_MAX];
static char auth_passwd[AUTH_MAX];
static char auth_product[AUTH_MAX];

char log_header[32] = {0};
char url[128] = {0};
int redirect = 0;
int change_passwd = 0;
int debug_mode = 0;

#if defined (SUPPORT_HTTPS)
int http_is_ssl = 0;
#endif

static int http_acl_mode = 0;
static time_t login_timestamp=0;	// the timestamp of the logined ip
static uaddr login_ip;			// the logined ip
static uaddr login_ip_tmp;		// the ip of the current session.
static uaddr last_login_ip;		// the last logined ip 2008.08 magic
static char login_mac[18] = {0};	// the logined mac
static char http_last_dict[16] = {0};	// last loaded XX.dict
#if defined (USE_IPV6)
static const struct in6_addr in6in4addr_loopback = {{{0x00, 0x00, 0x00, 0x00,
                                                      0x00, 0x00, 0x00, 0x00,
                                                      0x00, 0x00, 0xff, 0xff,
                                                      0x7f, 0x00, 0x00, 0x01}}};
#endif
time_t request_timestamp = 0;
time_t turn_off_auth_timestamp = 0;
int temp_turn_off_auth = 0;	// for QISxxx.htm pages

const int int_1 = 1;

struct language_table language_tables[] = {
	{"en-us", "EN"},
	{"en", "EN"},
	{"ru-ru", "RU"},
	{"ru", "RU"},
	{"uk-UA", "UK"},
	{"uk", "UK"},
	{"fr", "FR"},
	{"fr-fr", "FR"},
	{"de-at", "DE"},
	{"de-li", "DE"},
	{"de-lu", "DE"},
	{"de-de", "DE"},
	{"de-ch", "DE"},
	{"de", "DE"},
	{"cs-cz", "CZ"},
	{"cs", "CZ"},
	{"pl-pl", "PL"},
	{"pl", "PL"},
	{"zh-tw", "TW"},
	{"zh", "TW"},
	{"zh-hk", "CN"},
	{"zh-cn", "CN"},
	{"ms", "MS"},
	{"th", "TH"},
	{"th-TH", "TH"},
	{"th-TH-TH", "TH"},
	{"tr", "TR"},
	{"tr-TR", "TR"},
	{"da", "DA"},
	{"da-DK", "DA"},
	{"fi", "FI"},
	{"fi-FI", "FI"},
	{"no", "NO"},
	{"nb-NO", "NO"},
	{"nn-NO", "NO"},
	{"sv", "SV"},
	{"sv-FI", "SV"},
	{"sv-SE", "SV"},
	{"br", "BR"},
	{"pt-BR", "BR"},
	{"ja", "JP"},
	{"ja-JP", "JP"},
	{NULL, NULL}
};

/* Forwards. */
static int b64_decode( const char* str, unsigned char* space, int size );
static int match( const char* pattern, const char* string );
static int match_one( const char* pattern, int patternlen, const char* string );

long uptime(void)
{
	struct sysinfo info;
	sysinfo(&info);

	return info.uptime;
}

int is_uaddr_equal(uaddr *ip1, uaddr *ip2)
{
	if ((ip1->len > 0) && (ip1->len == ip2->len) && (memcmp(&ip1->addr, &ip2->addr, ip1->len) == 0))
		return 1;
	
	return 0;
}

int is_uaddr_localhost(uaddr *ip)
{
	if (
#if defined (USE_IPV6)
	    ((ip->family == AF_INET6) && 
	    ((memcmp(&ip->addr.in6, &in6addr_loopback, sizeof(struct in6_addr)) == 0) ||
	     (memcmp(&ip->addr.in6, &in6in4addr_loopback, sizeof(struct in6_addr)) == 0))) ||
	    ((ip->family == AF_INET) && (ip->addr.in4.s_addr == 0x100007f))
#else
	     (ip->addr.in4.s_addr == 0x100007f)
#endif
	)
		return 1;
	
	return 0;
}

static void
usockaddr_to_uaddr(const usockaddr *usa, uaddr *ip)
{
	ip->family = usa->sa.sa_family;

#if defined (USE_IPV6)
	if (ip->family == AF_INET6) {
		ip->len = sizeof(struct in6_addr);
		memcpy(&ip->addr.in6, &usa->sa_in6.sin6_addr, ip->len);
	} else
#endif
	{
		ip->len = sizeof(struct in_addr);
		ip->addr.in4.s_addr = usa->sa_in.sin_addr.s_addr;
	}
}

static int
convert_ip_to_string(uaddr *ip, char *p_out_ip, size_t out_ip_len)
{
#if defined (USE_IPV6)
	char s_addr[INET6_ADDRSTRLEN];
#else
	char s_addr[INET_ADDRSTRLEN];
#endif
	char *p_addr = s_addr;

	if (ip->len < 1 || !inet_ntop(ip->family, &ip->addr, s_addr, sizeof(s_addr))) {
		p_out_ip[0] = 0;
		return -1;
	}

#if defined (USE_IPV6)
	if (ip->family == AF_INET6 && strncmp(p_addr, "::ffff:", 7) == 0)
		p_addr += 7;
#endif
	strncpy(p_out_ip, p_addr, out_ip_len);

	return 0;
}

static int
find_mac_from_ip(uaddr *ip, unsigned char *p_out_mac, int *p_out_lan)
{
	FILE *fp;
	int result = -1;
	unsigned int arp_flags;
	char buffer[256], arp_mac[32], arp_if[32];
#if defined (USE_IPV6)
	char s_addr1[INET6_ADDRSTRLEN];
	char s_addr2[INET6_ADDRSTRLEN];
#else
	char s_addr1[INET_ADDRSTRLEN];
	char s_addr2[INET_ADDRSTRLEN];
#endif

	if (convert_ip_to_string(ip, s_addr1, sizeof(s_addr1)) < 0)
		return -1;

	if (!(*s_addr1))
		return -1;

	fp = fopen("/proc/net/arp", "r");
	if (fp) {
		// skip first line
		fgets(buffer, sizeof(buffer), fp);
		
		while (fgets(buffer, sizeof(buffer), fp)) {
			arp_flags = 0;
			if (sscanf(buffer, "%s %*s 0x%x %s %*s %s", s_addr2, &arp_flags, arp_mac, arp_if) == 4) {
				if ((arp_flags & 0x02) && !strcmp(s_addr1, s_addr2) && strcmp(arp_mac, "00:00:00:00:00:00")) {
					if (ether_atoe(arp_mac, p_out_mac)) {
						if (p_out_lan)
							*p_out_lan = (strcmp(arp_if, IFNAME_BR) == 0) ? 1 : 0;
						result = 0;
					}
					break;
				}
			}
		}
		
		fclose(fp);
	}

	return result;
}

static int
is_http_client_allowed(const usockaddr *usa)
{
	uaddr uip;
	int mac_in_sta_list, is_lan_client = 0;
	unsigned char mac[8] = {0};

	if (http_acl_mode < 1)
		return 1;

	usockaddr_to_uaddr(usa, &uip);

	/* 1. get MAC from IP (allow if failed) */
	if (find_mac_from_ip(&uip, mac, &is_lan_client) < 0)
		return 1;

	/* 2. do not check wireless sta list if client not from br0 */
	if (!is_lan_client)
		return 1;

	/* 3. check MAC in AP client list */
	mac_in_sta_list = is_mac_in_sta_list(mac);
	if (!mac_in_sta_list)
		return 1;

	if (http_acl_mode == 2) {
		/* LAN users + WiFi main AP users  */
		if (mac_in_sta_list == 1 || mac_in_sta_list == 3)
			return 1;
	}

	return 0;
}

void fill_login_ip(char *p_out_ip, size_t out_ip_len)
{
	convert_ip_to_string(&login_ip, p_out_ip, out_ip_len);
}

const char *get_login_mac(void)
{
	return (const char *)login_mac;
}

void http_login(uaddr *ip, char *url) {
	char s_login_timestamp[32];

	if (is_uaddr_localhost(ip))
		return;

	if (strcmp(url, "system_status_data.asp") && 
	    strcmp(url, "log_content.asp") && 
	    strcmp(url, "status_internet.asp") && 
	    strcmp(url, "status_wanlink.asp")) {
		memset(&last_login_ip, 0, sizeof(uaddr));
		if (!is_uaddr_equal(&login_ip, ip)) {
			unsigned char mac[8] = {0};
			
			memcpy(&login_ip, ip, sizeof(uaddr));
			if (find_mac_from_ip(ip, mac, NULL) == 0)
				ether_etoa(mac, login_mac);
			else
				login_mac[0] = 0;
		}
		login_timestamp = uptime();
		sprintf(s_login_timestamp, "%lu", login_timestamp);
		nvram_set_temp("login_timestamp", s_login_timestamp);
	}
}

void http_reset_login(void)
{
	memcpy(&last_login_ip, &login_ip, sizeof(uaddr));
	memset(&login_ip, 0, sizeof(uaddr));
	login_timestamp = 0;

	// load new acl mode
	http_acl_mode = nvram_get_int("http_access");

	nvram_set_temp("login_timestamp", "");

	/* notify about HTTP logout */
	kill_pidfile_s("/var/run/detect_internet.pid", SIGUSR1);

	if (change_passwd == 1) {
		change_passwd = 0;
		reget_passwd = 1;
	}
}

void http_logout(uaddr *ip) {
	if (is_uaddr_equal(ip, &login_ip)) {
		http_reset_login();
	}
}

void http_login_timeout(uaddr *ip)
{
	time_t now;

	now = uptime();

	if (((unsigned long)(now - login_timestamp) > LOGIN_TIMEOUT) && (login_ip.len != 0) && (!is_uaddr_equal(&login_ip, ip)))
		http_logout(&login_ip);
}

// 0: can not login, 1: can login, 2: loginer, 3: not loginer.
int http_login_check(void) 
{
	if (is_uaddr_localhost(&login_ip_tmp))
		return 1;

	http_login_timeout(&login_ip_tmp);

	if (login_ip.len == 0)
		return 1;

	if (is_uaddr_equal(&login_ip, &login_ip_tmp))
		return 2;

	return 3;
}

static int
initialize_listen_socket( usockaddr* usaP, int http_port )
{
	int listen_fd;
	int sa_family;

	sa_family = usaP->sa.sa_family;
	memset( usaP, 0, sizeof(usockaddr) );
#if defined (USE_IPV6)
	if (sa_family == AF_INET6) {
		usaP->sa.sa_family = AF_INET6;
		usaP->sa_in6.sin6_addr = in6addr_any;
		usaP->sa_in6.sin6_port = htons( http_port );
	} else
#endif
	{
		usaP->sa.sa_family = AF_INET;
		usaP->sa_in.sin_addr.s_addr = htonl( INADDR_ANY );
		usaP->sa_in.sin_port = htons( http_port );
	}

	listen_fd = socket( usaP->sa.sa_family, SOCK_STREAM, IPPROTO_TCP );
	if ( listen_fd < 0 )
	{
		perror( "socket" );
		return -1;
	}

	fcntl( listen_fd, F_SETFD, FD_CLOEXEC );

	if ( setsockopt( listen_fd, SOL_SOCKET, SO_REUSEADDR, &int_1, sizeof(int_1) ) < 0 )
	{
		close(listen_fd);	// 1104 chk
		perror( "setsockopt" );
		return -1;
	}

	if ( bind( listen_fd, &usaP->sa, sizeof(usockaddr) ) < 0 )
	{
		close(listen_fd);	// 1104 chk
		perror( "bind" );
		return -1;
	}

	if ( listen( listen_fd, 1024 ) < 0 )
	{
		close(listen_fd);	// 1104 chk
		perror( "listen" );
		return -1;
	}

	return listen_fd;
}

static void
send_headers( int status, char* title, char* extra_header, char* mime_type, FILE *conn_fp )
{
	time_t now;
	char timebuf[100];

	fprintf( conn_fp, "%s %d %s\r\n", PROTOCOL, status, title );
	fprintf( conn_fp, "Server: %s\r\n", SERVER_NAME );
	now = time( (time_t*) 0 );
	strftime( timebuf, sizeof(timebuf), RFC1123FMT, gmtime( &now ) );
	fprintf( conn_fp, "Date: %s\r\n", timebuf );
	if ( extra_header != (char*) 0 )
		fprintf( conn_fp, "%s\r\n", extra_header );
	if ( mime_type != (char*) 0 )
		fprintf( conn_fp, "Content-Type: %s\r\n", mime_type );
	fprintf( conn_fp, "Connection: close\r\n" );
	fprintf( conn_fp, "\r\n" );
}

static void
send_error( int status, char* title, char* extra_header, char* text, FILE *conn_fp )
{
	send_headers( status, title, extra_header, "text/html", conn_fp );
	fprintf( conn_fp, "<HTML><HEAD><TITLE>%d %s</TITLE></HEAD>\n<BODY BGCOLOR=\"#cc9999\"><H4>%d %s</H4>\n", status, title, status, title );
	fprintf( conn_fp, "%s\n", text );
	fprintf( conn_fp, "</BODY></HTML>\n" );
	fflush( conn_fp );
}

static void
send_authenticate( FILE *conn_fp )
{
	char header[8192];

	snprintf(header, sizeof(header), "WWW-Authenticate: Basic realm=\"%s\"", auth_product );
	send_error( 401, "Unauthorized", header, "Authorization required.", conn_fp );
}

static int
auth_check( char* authorization, char* url, FILE *conn_fp )
{
	char authinfo[500];
	char* authpass;
	int l;

	/* Is this directory unprotected? */
	if ( !strlen(auth_passwd) )
		/* Yes, let the request go through. */
		return 1;

	/* Basic authorization info? */
	if ( !authorization || strncmp( authorization, "Basic ", 6 ) != 0) 
	{
		send_authenticate(conn_fp);
		http_logout(&login_ip_tmp);
		memset(&last_login_ip, 0, sizeof(uaddr));
		return 0;
	}

	/* Decode it. */
	l = b64_decode( &(authorization[6]), authinfo, sizeof(authinfo) );
	authinfo[l] = '\0';
	/* Split into user and password. */
	authpass = strchr( authinfo, ':' );
	if ( authpass == (char*) 0 ) {
		/* No colon?  Bogus auth info. */
		send_authenticate(conn_fp);
		http_logout(&login_ip_tmp);
		return 0;
	}
	*authpass++ = '\0';

	/* Is this the right user and password? */
	if ( strcmp( auth_userid, authinfo ) == 0 && strcmp( auth_passwd, authpass ) == 0 )
	{
		/* Is this is the first login after logout */
		if (login_ip.len == 0 && is_uaddr_equal(&last_login_ip, &login_ip_tmp))
		{
			send_authenticate(conn_fp);
			memset(&last_login_ip, 0, sizeof(uaddr));
			return 0;
		}
		return 1;
	}

	send_authenticate(conn_fp);
	http_logout(&login_ip_tmp);

	return 0;
}


/* Base-64 decoding.  This represents binary data as printable ASCII
** characters.  Three 8-bit binary bytes are turned into four 6-bit
** values, like so:
**
**   [11111111]  [22222222]  [33333333]
**
**   [111111] [112222] [222233] [333333]
**
** Then the 6-bit values are represented using the characters "A-Za-z0-9+/".
*/

static int b64_decode_table[256] = {
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* 00-0F */
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* 10-1F */
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,62,-1,-1,-1,63,  /* 20-2F */
    52,53,54,55,56,57,58,59,60,61,-1,-1,-1,-1,-1,-1,  /* 30-3F */
    -1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,  /* 40-4F */
    15,16,17,18,19,20,21,22,23,24,25,-1,-1,-1,-1,-1,  /* 50-5F */
    -1,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,  /* 60-6F */
    41,42,43,44,45,46,47,48,49,50,51,-1,-1,-1,-1,-1,  /* 70-7F */
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* 80-8F */
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* 90-9F */
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* A0-AF */
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* B0-BF */
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* C0-CF */
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* D0-DF */
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* E0-EF */
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1   /* F0-FF */
    };

/* Do base-64 decoding on a string.  Ignore any non-base64 bytes.
** Return the actual number of bytes generated.  The decoded size will
** be at most 3/4 the size of the encoded, and may be smaller if there
** are padding characters (blanks, newlines).
*/
static int
b64_decode( const char* str, unsigned char* space, int size )
{
    const char* cp;
    int space_idx, phase;
    int d, prev_d=0;
    unsigned char c;

    space_idx = 0;
    phase = 0;
    for ( cp = str; *cp != '\0'; ++cp )
	{
	d = b64_decode_table[(int)*cp];
	if ( d != -1 )
	    {
	    switch ( phase )
		{
		case 0:
		++phase;
		break;
		case 1:
		c = ( ( prev_d << 2 ) | ( ( d & 0x30 ) >> 4 ) );
		if ( space_idx < size )
		    space[space_idx++] = c;
		++phase;
		break;
		case 2:
		c = ( ( ( prev_d & 0xf ) << 4 ) | ( ( d & 0x3c ) >> 2 ) );
		if ( space_idx < size )
		    space[space_idx++] = c;
		++phase;
		break;
		case 3:
		c = ( ( ( prev_d & 0x03 ) << 6 ) | d );
		if ( space_idx < size )
		    space[space_idx++] = c;
		phase = 0;
		break;
		}
	    prev_d = d;
	    }
	}
    return space_idx;
}


/* Simple shell-style filename matcher.  Only does ? * and **, and multiple
** patterns separated by |.  Returns 1 or 0.
*/
int
match( const char* pattern, const char* string )
{
    const char* or;

    for (;;)
	{
	or = strchr( pattern, '|' );
	if ( or == (char*) 0 )
	    return match_one( pattern, strlen( pattern ), string );
	if ( match_one( pattern, or - pattern, string ) )
	    return 1;
	pattern = or + 1;
	}
}


static int
match_one( const char* pattern, int patternlen, const char* string )
{
    const char* p;

    for ( p = pattern; p - pattern < patternlen; ++p, ++string )
	{
	if ( *p == '?' && *string != '\0' )
	    continue;
	if ( *p == '*' )
	    {
	    int i, pl;
	    ++p;
	    if ( *p == '*' )
		{
		/* Double-wildcard matches anything. */
		++p;
		i = strlen( string );
		}
	    else
		/* Single-wildcard matches anything but slash. */
		i = strcspn( string, "/" );
	    pl = patternlen - ( p - pattern );
	    for ( ; i >= 0; --i )
		if ( match_one( p, pl, &(string[i]) ) )
		    return 1;
	    return 0;
	    }
	if ( *p != *string )
	    return 0;
	}
    if ( *string == '\0' )
	return 1;
    return 0;
}

int do_fwrite(const char *buffer, int len, FILE *stream)
{
	int n = len;
	int r = 0;
 
	while (n > 0) {
		r = fwrite(buffer, 1, n, stream);
		if ((r == 0) && (errno != EINTR)) return -1;
		buffer += r;
		n -= r;
	}

	return r;
}

void do_file(char *path, FILE *stream)
{
	FILE *fp;
	char buf[1024];
	int nr;

	if ((fp = fopen(path, "r")) != NULL) {
		while ((nr = fread(buf, 1, sizeof(buf), fp)) > 0)
			do_fwrite(buf, nr, stream);
		fclose(fp);
	}
}

void do_auth(int reget)
{
	if (!reget)
		return;
	
	snprintf(auth_userid, sizeof(auth_userid), "%s", nvram_safe_get("http_username"));
	snprintf(auth_passwd, sizeof(auth_passwd), "%s", nvram_safe_get("http_passwd"));
}

int set_preferred_lang(char* cur)
{
	char *p, *p_lang;
	char lang_buf[64], lang_file[16];
	struct language_table *p_lt;

	memset(lang_buf, 0, sizeof(lang_buf));
	strncpy(lang_buf, cur, sizeof(lang_buf)-1);

	p = lang_buf;
	p_lang = NULL;
	while (p != NULL)
	{
		p = strtok (p, "\r\n ,;");
		if (p == NULL)
			break;
		
		for (p_lt = language_tables; p_lt->Lang != NULL; ++p_lt) 
		{
			if (strcasecmp(p, p_lt->Lang)==0)
			{
				p_lang = p_lt->Target_Lang;
				break;
			}
		}
		
		if (p_lang)
			break;
		
		p+=strlen(p)+1;
	}
	
	if (p_lang)
	{
		snprintf(lang_file, sizeof(lang_file), "%s.dict", p_lang);
		if (f_exists(lang_file))
			nvram_set("preferred_lang", p_lang);
		else
			nvram_set("preferred_lang", "EN");
		
		return 1;
	}
	
	return 0;
}

static void
handle_request(FILE *conn_fp, int conn_fd)
{
	char line[8192];
	char *method, *path, *protocol, *authorization, *boundary;
	char *cur, *end, *cp, *file;
	int len, login_state;
	struct mime_handler *handler;
	int clen = 0, flags, has_lang;

	/* Initialize the request variables. */
	authorization = boundary = NULL;

	/* Parse the first line of the request. */
	if (!fgets( line, sizeof(line), conn_fp)) {
		send_error( 400, "Bad Request", (char*) 0, "No request found.", conn_fp);
		return;
	}

	method = path = line;
	strsep(&path, " ");
	while (*path == ' ') path++;
	protocol = path;
	strsep(&protocol, " ");
	while (*protocol == ' ') protocol++;
	cp = protocol;
	strsep(&cp, " ");

	if ( !method || !path || !protocol ) {
		send_error( 400, "Bad Request", (char*) 0, "Can't parse request.", conn_fp );
		return;
	}

	has_lang = (strlen(nvram_safe_get("preferred_lang")) > 1) ? 1 : 0;

	cur = protocol + strlen(protocol) + 1;
	end = line + sizeof(line) - 1;

	while ( (cur < end) && (fgets(cur, line + sizeof(line) - cur, conn_fp)) )
	{
		if ( strcmp( cur, "\n" ) == 0 || strcmp( cur, "\r\n" ) == 0 ) {
			break;
		}
		else if ((!has_lang) && (strncasecmp(cur, "Accept-Language:", 16) == 0)) {
			has_lang = set_preferred_lang(cur + 16);
		}
		else if (strncasecmp( cur, "Authorization:", 14) == 0)
		{
			cp = cur + 14;
			cp += strspn( cp, " \t" );
			authorization = cp;
			cur = cp + strlen(cp) + 1;
		}
		else if (strncasecmp( cur, "Content-Length:", 15) == 0) {
			cp = cur + 15;
			cp += strspn( cp, " \t" );
			clen = strtoul( cp, NULL, 0 );
		}
		else if ((cp = strstr( cur, "boundary=" ))) {
			boundary = cp + 9;
			for ( cp = cp + 9; *cp && *cp != '\r' && *cp != '\n'; cp++ );
			*cp = '\0';
			cur = ++cp;
		}
	}

	if ( strcasecmp( method, "get" ) != 0 && strcasecmp(method, "post") != 0 ) {
		send_error( 501, "Not Implemented", (char*) 0, "That method is not implemented.", conn_fp );
		return;
	}

	if ( path[0] != '/' ) {
		send_error( 400, "Bad Request", (char*) 0, "Bad filename.", conn_fp );
		return;
	}

	file = &(path[1]);
	len = strlen( file );
	if ( file[0] == '/' || strcmp( file, ".." ) == 0 || strncmp( file, "../", 3 ) == 0 || strstr( file, "/../" ) != (char*) 0 || strcmp( &(file[len-3]), "/.." ) == 0 ) {
		send_error( 400, "Bad Request", (char*) 0, "Illegal filename.", conn_fp );
		return;
	}

	if (file[0] == '\0' || file[len-1] == '/')
		file = "index.asp";
	
	char *query;
	int file_len;
	
	if ((query = index(file, '?')) != NULL) {
		file_len = strlen(file)-strlen(query);
		
		strncpy(url, file, file_len);
	}
	else
	{
		strcpy(url, file);
	}

	login_state = http_login_check();
	if (login_state == 3)
	{
		if ( (login_ip.len != 0) && (strstr(url, ".htm") != NULL || strstr(url, ".asp") != NULL) ) {
			file = "Nologin.asp";
			strcpy(url, file);
		}
	}
	
	for (handler = mime_handlers; handler->pattern; handler++) 
	{
		if (match(handler->pattern, url))
		{
			request_timestamp = uptime();
			
			if ((login_state == 1 || login_state == 2)
					/* modify QIS authentication flow */
					&& (strstr(url, "QIS_") != NULL   // to avoid the interference of the other logined browser. 2008.11 magic
						|| !strcmp(url, "Logout.asp")
						|| !strcmp(url, "log_content.asp")
						|| !strcmp(url, "system_status_data.asp")
						|| !strcmp(url, "status_internet.asp")
						|| !strcmp(url, "status_wanlink.asp")
						|| !strcmp(url, "start_apply.htm")
						|| !strcmp(url, "httpd_check.htm")
						)
					) {
				turn_off_auth_timestamp = request_timestamp;
				temp_turn_off_auth = 1; // no auth
				redirect = !strcmp(url, "Logout.asp");
			}
			else if(!strcmp(url, "jquery.js")
					|| !strcmp(url, "Nologin.asp")
					) {
				;	// do nothing.
			}
			else if (strstr(url, ".asp") != NULL
					|| strstr(url, ".cgi") != NULL
					|| strstr(url, ".htm") != NULL
					|| strstr(url, ".CFG") != NULL
					|| strstr(url, "QIS_") != NULL  /* modify QIS authentication flow */
					) {
				switch(login_state) {
					case 0:
						return;
					case 1:
					case 2:
						turn_off_auth_timestamp = 0;
						temp_turn_off_auth = 0;
						redirect = 0;
						break;
					case 3:
						break;
				}
			}
			else if (login_state == 2
					&& temp_turn_off_auth
					&& (unsigned long)(request_timestamp-turn_off_auth_timestamp) > 10
					) {
				http_logout(&login_ip_tmp);
				turn_off_auth_timestamp = 0;
				temp_turn_off_auth = 0;
				redirect = 0;
			}
			
			if (handler->auth) {
				if (!temp_turn_off_auth) {
					handler->auth(reget_passwd);
					reget_passwd = 0;
					if (!auth_check(authorization, url, conn_fp))
						return;
				}
				
				if (!redirect)
					http_login(&login_ip_tmp, url);
			}
			
			if (strcasecmp(method, "post") == 0 && !handler->input) {
				send_error(501, "Not Implemented", NULL, "That method is not implemented.", conn_fp);
				return;
			}
			
			if (handler->input) {
				handler->input(file, conn_fp, clen, boundary);
				if ((flags = fcntl(conn_fd, F_GETFL)) != -1 && fcntl(conn_fd, F_SETFL, flags | O_NONBLOCK) != -1) {
					/* Read up to two more characters */
					if (fgetc(conn_fp) != EOF)
						(void)fgetc(conn_fp);
					
					fcntl(conn_fd, F_SETFL, flags);
				}
			}
			
			send_headers( 200, "Ok", handler->extra_header, handler->mime_type, conn_fp );
			if (handler->output) {
				handler->output(file, conn_fp);
			}
			
			break;
		}
	}
	
	if (!handler->pattern) 
		send_error( 404, "Not Found", (char*) 0, "File not found.", conn_fp );
	
	if (!strcmp(file, "Logout.asp")) {
		http_logout(&login_ip_tmp);
	}
	
	if (!strcmp(file, "Reboot.asp")) {
		system("reboot");
	}
}

int is_fileexist(char *filename)
{
	FILE *fp;

	fp=fopen(filename, "r");

	if (fp==NULL) return 0;
	fclose(fp);
	return 1;
}

int
load_dictionary (char *lang, pkw_t pkw)
{
	char dfn[16];
	char *p, *q;
	FILE *dfp = NULL;
	int dict_size = 0;
	const char *eng_dict = "EN.dict";

	if (lang == NULL || strlen(lang) == 0) {
		snprintf(dfn, sizeof (dfn), eng_dict);
	} else {
		snprintf(dfn, sizeof (dfn), "%s.dict", lang);
	}

	if (strcmp (dfn, http_last_dict) == 0) {
		return 1;
	}

	release_dictionary (pkw);

	do {
		dfp = fopen (dfn, "r");
		if (dfp != NULL) {
			snprintf(http_last_dict, sizeof(http_last_dict), "%s", dfn);
			break;
		}
		
		if (dfp == NULL && strcmp (dfn, eng_dict) == 0) {
			return 0;
		} else {
			// If we can't open specified language file, use English as default
			snprintf (dfn, sizeof (dfn), eng_dict);
		}
	} while (1);

	memset (pkw, 0, sizeof (kw_t));
	fseek (dfp, 0L, SEEK_END);
	dict_size = ftell (dfp) + 128;
	REALLOC_VECTOR (pkw->idx, pkw->len, pkw->tlen, sizeof (unsigned char*));
	pkw->buf = q = malloc (dict_size);

	fseek (dfp, 0L, SEEK_SET);

	while ((fscanf(dfp, "%[^\n]", q)) != EOF) {
		fgetc(dfp);

		// if pkw->idx is not enough, add 32 item to pkw->idx
		REALLOC_VECTOR (pkw->idx, pkw->len, pkw->tlen, sizeof (unsigned char*));

		if ((p = strchr (q, '=')) != NULL) {
			pkw->idx[pkw->len] = q;
			pkw->len++;
			q = p + strlen (p);
			*q = '\0';
			q++;
		}
	}

	fclose (dfp);

	return 1;
}

void
release_dictionary (pkw_t pkw)
{
	if (pkw == NULL)
		return;

	pkw->len = pkw->tlen = 0;

	if (pkw->idx != NULL)   {
		free (pkw->idx);
		pkw->idx = NULL;
	}

	if (pkw->buf != NULL)   {
		free (pkw->buf);
		pkw->buf = NULL;
	}
}

char*
search_desc (pkw_t pkw, char *name)
{
	int i;
	char *p, *ret = NULL;

	if (pkw == NULL || (pkw != NULL && pkw->len <= 0))      {
		return NULL;
	}
	for (i = 0; i < pkw->len; ++i)  {
		p = pkw->idx[i];
		if (strncmp (name, p, strlen (name)) == 0)      {
			ret = p + strlen (name);
			break;
		}
	}

	return ret;
}

static void
http_reload_params(void)
{
	// reset last loaded XX.dict
	memset(http_last_dict, 0, sizeof(http_last_dict));
}


static void
catch_sig(int sig)
{
	if (sig == SIGTERM)
	{
		daemon_exit = 1;
		httpd_log("Received %s, terminating.", "SIGTERM");
	}
	else if (sig == SIGHUP)
	{
		http_reload_params();
	}
	else if (sig == SIGUSR1)
	{
		http_reset_login();
	}
	else if (sig == SIGUSR2)
	{
	}
}

int main(int argc, char **argv)
{
	FILE *pid_fp;
	struct timeval tv;
	fd_set active_rfds;
	usockaddr usa[2];
	int listen_fd[2], http_port[2];
	int i, c, tmp, max_fd, cnt_fd, selected;
	pid_t pid;
	socklen_t sz;
	conn_list_t pool;
	conn_item_t *item, *next;

	snprintf(log_header, sizeof(log_header), "%s[%d]", SYSLOG_ID_HTTPD, getpid());

	http_port[0] = 0;
	http_port[1] = 0;

	// usage : httpd -p [port] -s [port]
	if(argc) {
		while ((c = getopt(argc, argv, "p:s:d")) != -1) {
			switch (c) {
			case 'p':
				tmp = atoi(optarg);
				if (tmp > 0 && tmp < 65536)
					http_port[0] = tmp;
				else
					http_port[0] = SERVER_PORT;
				break;
#if defined (SUPPORT_HTTPS)
			case 's':
				tmp = atoi(optarg);
				if (tmp > 0 && tmp < 65536)
					http_port[1] = tmp;
				else
					http_port[1] = SERVER_PORT_SSL;
				break;
#endif
			case 'd':
				debug_mode = 1;
				break;
			}
		}
	}

	if (!http_port[0] && !http_port[1])
		http_port[0] = SERVER_PORT;

#if defined (SUPPORT_HTTPS)
	if (http_port[1] == http_port[0]) {
		if (http_port[0] != SERVER_PORT_SSL)
			http_port[1] = SERVER_PORT_SSL;
		else
			http_port[1] = 0;
	}

	if (http_port[1]) {
		char path_ca[64], path_crt[64], path_key[64], path_dhp[64];
		sprintf(path_ca,  "%s/%s", STORAGE_HTTPSSL_DIR, "ca.crt");
		sprintf(path_crt, "%s/%s", STORAGE_HTTPSSL_DIR, "server.crt");
		sprintf(path_key, "%s/%s", STORAGE_HTTPSSL_DIR, "server.key");
		sprintf(path_dhp, "%s/%s", STORAGE_HTTPSSL_DIR, "dh1024.pem");
		if (ssl_server_init(path_ca, path_crt, path_key, path_dhp, nvram_get("https_clist")) != 0) {
			http_port[1] = 0;
			/* avoid httpd unload */
			if (!http_port[0])
				http_port[0] = SERVER_PORT;
			if (nvram_get_int("http_proto") == 1)
				nvram_set_int("http_proto", 0);
		}
	}
#endif

	memset(&login_ip, 0, sizeof(uaddr));
	memset(&login_ip_tmp, 0, sizeof(uaddr));
	memset(&last_login_ip, 0, sizeof(uaddr));

#if defined (USE_IPV6)
	usa[0].sa.sa_family = (get_ipv6_type() != IPV6_DISABLED) ? AF_INET6 : AF_INET;
	usa[1].sa.sa_family = usa[0].sa.sa_family;
#endif

	listen_fd[0] = -1;
	listen_fd[1] = -1;

	if (http_port[0]) {
		if ((listen_fd[0] = initialize_listen_socket(&usa[0], http_port[0])) < 0) {
			perror("bind");
			httpd_log("ERROR: can't bind listening port %d to any address!", http_port[0]);
			http_port[0] = 0;
			if (!http_port[1])
				exit(errno);
		}
	}

#if defined (SUPPORT_HTTPS)
	if (http_port[1]) {
		if ((listen_fd[1] = initialize_listen_socket(&usa[1], http_port[1])) < 0) {
			perror("bind");
			httpd_log("ERROR: can't bind listening port %d to any address!", http_port[1]);
			if (listen_fd[0] < 0) {
				ssl_server_uninit();
				exit(errno);
			}
		}
	}
#endif

	signal(SIGPIPE, SIG_IGN);
	signal(SIGHUP,  catch_sig);
	signal(SIGUSR1, catch_sig);
	signal(SIGUSR2, catch_sig);
	signal(SIGTERM, catch_sig);

	cnt_fd = (listen_fd[1] >= 0) ? 2 : 1;

	if (!debug_mode && daemon(1, 0) < 0) {
		perror("daemon");
		for (i=0; i<cnt_fd; i++) {
			if (listen_fd[i] >= 0) {
				shutdown(listen_fd[i], SHUT_RDWR);
				close(listen_fd[i]);
			}
		}
#if defined (SUPPORT_HTTPS)
		if (http_port[1])
			ssl_server_uninit();
#endif
		exit(errno);
	}

	pid = getpid();
	snprintf(log_header, sizeof(log_header), "%s[%d]", SYSLOG_ID_HTTPD, pid);

	do_auth(1);
	snprintf(auth_product, sizeof(auth_product), "%s", nvram_safe_get("productid"));

	if ((pid_fp = fopen("/var/run/httpd.pid", "w"))) {
		fprintf(pid_fp, "%d", pid);
		fclose(pid_fp);
	}

	http_acl_mode = nvram_get_int("http_access");
	nvram_set_temp("login_timestamp", "");

	chdir("/www");

	FD_ZERO(&active_rfds);
	TAILQ_INIT(&pool.head);
	pool.count = 0;
	sz = sizeof(usa);

	if (http_port[0] && listen_fd[0] >= 0)
		httpd_log("Server listening port %d (%s).", http_port[0], "HTTP");
#if defined (SUPPORT_HTTPS)
	if (http_port[1] && listen_fd[1] >= 0)
		httpd_log("Server listening port %d (%s). %s.", http_port[1], "HTTPS", ssl_server_get_ssl_ver());
#endif

	while (!daemon_exit) {
		fd_set rfds;
		
		rfds = active_rfds;
		max_fd = -1;
		if (pool.count < MAX_CONN_ACCEPT) {
			for (i=0; i<cnt_fd; i++) {
				if (listen_fd[i] >= 0) {
					FD_SET(listen_fd[i], &rfds);
					max_fd = (listen_fd[i] > max_fd) ? listen_fd[i] : max_fd;
				}
			}
		}
		
		TAILQ_FOREACH(item, &pool.head, entry)
			max_fd = (item->fd > max_fd) ? item->fd : max_fd;
		
		/* wait for new connection or incoming request */
		tv.tv_sec = MAX_CONN_TIMEOUT;
		tv.tv_usec = 0;
		selected = select(max_fd + 1, &rfds, NULL, NULL, &tv);
		if (selected < 0) {
			if (errno == EINTR || errno == EAGAIN)
				continue;
			if (debug_mode)
				perror("select");
			break;
		}
		
		/* check and accept new connection */
		if (selected) {
			int is_accept = 0;
			for (i=0; i<cnt_fd; i++) {
				if (listen_fd[i] >= 0 && FD_ISSET(listen_fd[i], &rfds)) {
					item = malloc(sizeof(*item));
					if (!item)
						continue;
					
					item->fd = accept(listen_fd[i], &item->usa.sa, &sz);
					if (item->fd >= 0) {
#if defined (SUPPORT_HTTPS)
						item->ssl = (i > 0) ? 1 : 0;
#endif
						if (is_http_client_allowed(&item->usa)) {
							setsockopt(item->fd, SOL_SOCKET, SO_KEEPALIVE, &int_1, sizeof(int_1));
							FD_SET(item->fd, &active_rfds);
							TAILQ_INSERT_TAIL(&pool.head, item, entry);
							pool.count++;
							is_accept++;
						} else {
							shutdown(item->fd, SHUT_RDWR);
							close(item->fd);
							free(item);
						}
					} else {
						if (errno != EINTR && errno != EAGAIN) {
							if (debug_mode)
								perror("accept");
						}
						free(item);
					}
				}
			}
			
			/* Continue waiting */
			if (is_accept)
				continue;
		}
		
		/* Check and process pending or expired requests */
		TAILQ_FOREACH_SAFE(item, &pool.head, entry, next) {
			if (selected && !FD_ISSET(item->fd, &rfds))
				continue;
			
			FD_CLR(item->fd, &active_rfds);
			TAILQ_REMOVE(&pool.head, item, entry);
			pool.count--;
			
			if (selected) {
				FILE *conn_fp;
#if defined (SUPPORT_HTTPS)
				http_is_ssl = item->ssl;
				if (item->ssl)
					conn_fp = ssl_server_fopen(item->fd);
				else
#endif
				conn_fp = fdopen(item->fd, "r+");
				if (conn_fp) {
					usockaddr_to_uaddr(&item->usa, &login_ip_tmp);
					handle_request(conn_fp, item->fd);
					fflush(conn_fp);
#if defined (SUPPORT_HTTPS)
					http_is_ssl = 0;
					if (!item->ssl)
#endif
					shutdown(item->fd, SHUT_RDWR);
					fclose(conn_fp);
					conn_fp = NULL;
					item->fd = -1;
				}
				if (--selected == 0)
					next = NULL;
			}
			
			if (item->fd >= 0) {
				shutdown(item->fd, SHUT_RDWR);
				close(item->fd);
			}
			
			free(item);
		}
	}

	/* free all pending requests */
	TAILQ_FOREACH(item, &pool.head, entry) {
		if (item->fd >= 0) {
			shutdown(item->fd, SHUT_RDWR);
			close(item->fd);
		}
		
		free(item);
	}

	for (i=0; i<cnt_fd; i++) {
		if (listen_fd[i] >= 0) {
			shutdown(listen_fd[i], SHUT_RDWR);
			close(listen_fd[i]);
		}
	}

#if defined (SUPPORT_HTTPS)
	if (http_port[1])
		ssl_server_uninit();
#endif

	return 0;
}

