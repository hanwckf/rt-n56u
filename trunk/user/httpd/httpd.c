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
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <assert.h>
#include <sys/ioctl.h>
#include <shutils.h>

typedef unsigned int __u32;   // 1225 ham

#include <httpd.h>
#include <nvram/bcmnvram.h>
#include <arpa/inet.h>

#define eprintf2(fmt, args...) do{\
	FILE *ffp = fopen("/tmp/detect_wrong.log", "a+");\
	if (ffp) {\
		fprintf(ffp, fmt, ## args);\
		fclose(ffp);\
	}\
}while (0)


#include <error.h>
#include <sys/signal.h>
#include <sys/sysinfo.h>

#include <net/if.h>

#define LOGIN_TIMEOUT 60
#define SERVER_NAME "httpd"
#define SERVER_PORT 80
#define PROTOCOL "HTTP/1.0"
#define RFC1123FMT "%a, %d %b %Y %H:%M:%S GMT"

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
	usockaddr usa;
} conn_item_t;

typedef struct conn_list {
	TAILQ_HEAD(, conn_item) head;
	int count;
} conn_list_t;

/* Globals. */
static int daemon_exit = 0;

static FILE *conn_fp = NULL;
static char auth_userid[AUTH_MAX];
static char auth_passwd[AUTH_MAX];
static char auth_realm[AUTH_MAX];

/* Forwards. */
static int initialize_listen_socket( usockaddr* usaP );
static int auth_check( char* dirname, char* authorization, char* url);
static void send_authenticate( char* realm );
static void send_error( int status, char* title, char* extra_header, char* text );
static void send_headers( int status, char* title, char* extra_header, char* mime_type );
static int b64_decode( const char* str, unsigned char* space, int size );
static int match( const char* pattern, const char* string );
static int match_one( const char* pattern, int patternlen, const char* string );
static void handle_request(void);

char url[128] = {0};
int redirect = 0;
int change_passwd = 0;
int reget_passwd = 0;
int http_port = SERVER_PORT;

static time_t login_timestamp=0;	// the timestamp of the logined ip
static uaddr login_ip;			// the logined ip
static uaddr login_ip_tmp;		// the ip of the current session.
static uaddr last_login_ip;		// the last logined ip 2008.08 magic
static char login_mac[18] = {0};	// the logined mac
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

#ifdef TRANSLATE_ON_FLY
struct language_table language_tables[] = {
	{"en-us", "EN"},
	{"en", "EN"},
	{"ru-ru", "RU"},
	{"ru", "RU"},
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
#endif

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

void fill_login_ip(char *p_login_ip, size_t login_ip_len)
{
#if defined (USE_IPV6)
	char s_addr[INET6_ADDRSTRLEN];
#else
	char s_addr[INET_ADDRSTRLEN];
#endif
	char *p_addr = s_addr;
	
	if (login_ip.len < 1 || !inet_ntop(login_ip.family, &login_ip.addr, s_addr, sizeof(s_addr))) {
		p_login_ip[0] = 0;
		return;
	}
#if defined (USE_IPV6)
	if (login_ip.family == AF_INET6 && strncmp(p_addr, "::ffff:", 7) == 0)
		p_addr += 7;
#endif
	strncpy(p_login_ip, p_addr, login_ip_len);
}

void search_login_mac(void)
{
	FILE *fp;
	char buffer[256], values[5][32];
#if defined (USE_IPV6)
	char s_addr[INET6_ADDRSTRLEN];
	char s_addr2[INET6_ADDRSTRLEN];
#else
	char s_addr[INET_ADDRSTRLEN];
	char s_addr2[INET_ADDRSTRLEN];
#endif

	login_mac[0] = 0;
	fill_login_ip(s_addr, sizeof(s_addr));

	if (!(*s_addr))
		return;

	fp = fopen("/proc/net/arp", "r");
	if (fp) {
		// skip first string
		fgets(buffer, sizeof(buffer), fp);
		
		while (fgets(buffer, sizeof(buffer), fp)) {
			if (sscanf(buffer, "%s%s%s%s%s%s", s_addr2, values[0], values[1], values[2], values[3], values[4]) == 6) {
				if (!strcmp(values[4], IFNAME_BR) && !strcmp(s_addr, s_addr2) && strcmp(values[2], "00:00:00:00:00:00")) {
					strncpy(login_mac, values[2], sizeof(login_mac));
					break;
				}
			}
		}
		
		fclose(fp);
	}
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
	    strcmp(url, "result_of_get_changed_status.asp") && 
	    strcmp(url, "log_content.asp") && 
	    strcmp(url, "WAN_info.asp")) {
		memset(&last_login_ip, 0, sizeof(uaddr));
		if (!is_uaddr_equal(&login_ip, ip)) {
			memcpy(&login_ip, ip, sizeof(uaddr));
			search_login_mac();
		}
		login_timestamp = uptime();
		sprintf(s_login_timestamp, "%lu", login_timestamp);
		nvram_set("login_timestamp", s_login_timestamp);
	}
}

void http_logout(uaddr *ip) {
	if (is_uaddr_equal(ip, &login_ip)) {
		memcpy(&last_login_ip, &login_ip, sizeof(uaddr));
		memset(&login_ip, 0, sizeof(uaddr));
		login_timestamp = 0;
		
		nvram_set("login_timestamp", "");
		
		if (change_passwd == 1) {
			change_passwd = 0;
			reget_passwd = 1;
		}
	}
}

void http_login_timeout(uaddr *ip)
{
	time_t now;

	now = uptime();

	if (((unsigned long)(now - login_timestamp) > LOGIN_TIMEOUT) && (login_ip.len != 0) && (!is_uaddr_equal(&login_ip, ip)))
	{
		http_logout(&login_ip);
	}
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
initialize_listen_socket( usockaddr* usaP )
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

	fcntl( listen_fd, F_SETFD, 1 );

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

static int
auth_check( char* dirname, char* authorization ,char* url)
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
	send_authenticate( dirname );
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
	send_authenticate( dirname );
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
		send_authenticate(dirname);
		memset(&last_login_ip, 0, sizeof(uaddr));
		return 0;
    	}
	return 1;
    }

    send_authenticate( dirname );
    http_logout(&login_ip_tmp);
    return 0;
}


static void
send_authenticate( char* realm )
{
    char header[10000];

    (void) snprintf(
	header, sizeof(header), "WWW-Authenticate: Basic realm=\"%s\"", realm );
    send_error( 401, "Unauthorized", header, "Authorization required." );
}


static void
send_error( int status, char* title, char* extra_header, char* text )
{
    send_headers( status, title, extra_header, "text/html" );
    (void) fprintf( conn_fp, "<HTML><HEAD><TITLE>%d %s</TITLE></HEAD>\n<BODY BGCOLOR=\"#cc9999\"><H4>%d %s</H4>\n", status, title, status, title );
    (void) fprintf( conn_fp, "%s\n", text );
    (void) fprintf( conn_fp, "</BODY></HTML>\n" );
    (void) fflush( conn_fp );
}


static void
send_headers( int status, char* title, char* extra_header, char* mime_type )
{
    time_t now;
    char timebuf[100];

    (void) fprintf( conn_fp, "%s %d %s\r\n", PROTOCOL, status, title );
    (void) fprintf( conn_fp, "Server: %s\r\n", SERVER_NAME );
    now = time( (time_t*) 0 );
    (void) strftime( timebuf, sizeof(timebuf), RFC1123FMT, gmtime( &now ) );
    (void) fprintf( conn_fp, "Date: %s\r\n", timebuf );
    if ( extra_header != (char*) 0 )
	(void) fprintf( conn_fp, "%s\r\n", extra_header );
    if ( mime_type != (char*) 0 )
	(void) fprintf( conn_fp, "Content-Type: %s\r\n", mime_type );
    (void) fprintf( conn_fp, "Connection: close\r\n" );
    (void) fprintf( conn_fp, "\r\n" );
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

#ifdef TRANSLATE_ON_FLY
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
#endif

time_t detect_timestamp, detect_timestamp_old, signal_timestamp;
char detect_timestampstr[32];

static void
handle_request(void)
{
	static char line[10000];
	char *method, *path, *protocol, *authorization, *boundary;
	char *cur, *end, *cp, *file;
	int len, login_state;
	struct mime_handler *handler;
	int cl = 0, flags, has_lang;

	/* Initialize the request variables. */
	authorization = boundary = NULL;

	/* Parse the first line of the request. */
	if (!fgets( line, sizeof(line), conn_fp )) {
		send_error( 400, "Bad Request", (char*) 0, "No request found." );
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
		send_error( 400, "Bad Request", (char*) 0, "Can't parse request." );
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
#ifdef TRANSLATE_ON_FLY
		else if ((!has_lang) && (strncasecmp(cur, "Accept-Language:", 16) == 0)) {
			has_lang = set_preferred_lang(cur + 16);
		}
#endif
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
			cl = strtoul( cp, NULL, 0 );
		}
		else if ((cp = strstr( cur, "boundary=" ))) {
			boundary = cp + 9;
			for ( cp = cp + 9; *cp && *cp != '\r' && *cp != '\n'; cp++ );
			*cp = '\0';
			cur = ++cp;
		}
	}

	if ( strcasecmp( method, "get" ) != 0 && strcasecmp(method, "post") != 0 ) {
		send_error( 501, "Not Implemented", (char*) 0, "That method is not implemented." );
		return;
	}

	if ( path[0] != '/' ) {
		send_error( 400, "Bad Request", (char*) 0, "Bad filename." );
		return;
	}

	file = &(path[1]);
	len = strlen( file );
	if ( file[0] == '/' || strcmp( file, ".." ) == 0 || strncmp( file, "../", 3 ) == 0 || strstr( file, "/../" ) != (char*) 0 || strcmp( &(file[len-3]), "/.." ) == 0 ) {
		send_error( 400, "Bad Request", (char*) 0, "Illegal filename." );
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
		if ((strstr(url, ".htm") != NULL
					&& !(!strcmp(url, "error_page.htm")
						|| (strstr(url, "QIS_") != NULL && nvram_match("x_Setting", "0") && login_ip.len == 0)
						|| !strcmp(url, "gotoHomePage.htm")
						)
					)
				|| (strstr(url, ".asp") != NULL && login_ip.len != 0)
				) {
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
							|| !strcmp(url, "result_of_get_changed_status.asp")
							|| !strcmp(url, "result_of_get_changed_status_QIS.asp")
							|| !strcmp(url, "detectWAN.asp")
							|| !strcmp(url, "WAN_info.asp")
							|| !strcmp(url, "start_apply.htm")
							|| !strcmp(url, "start_apply2.htm")
							|| !strcmp(url, "detectWAN2.asp")
							|| !strcmp(url, "automac.asp")
							|| !strcmp(url, "setting_lan.htm")
							|| !strcmp(url, "status.asp")
							|| !strcmp(url, "httpd_check.htm")
							)
					) {
				turn_off_auth_timestamp = request_timestamp;
				temp_turn_off_auth = 1; // no auth
				redirect = !strcmp(url, "Logout.asp");
			}
			else if(!strcmp(url, "error_page.htm")
					|| !strcmp(url, "jquery.js") // 2010.09 James.
					|| !strcmp(url, "Nologin.asp")
					|| !strcmp(url, "gotoHomePage.htm")
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
					handler->auth(auth_userid, auth_passwd, auth_realm);
					if (!auth_check(auth_realm, authorization, url))
						return;
				}
			
				if (!redirect)
				{
					if (is_firsttime ())
					{
						if (strstr(url, "QIS_wizard.htm"))
							;
						else if (	strcasestr(url, ".asp") != NULL ||
								strcasestr(url, ".cgi") != NULL ||
								strcasestr(url, ".htm") != NULL ||
								strcasestr(url, ".CFG") != NULL	)
							http_login(&login_ip_tmp, url);
					}
					else
						http_login(&login_ip_tmp, url);
				}
			}
			
			if (strcasecmp(method, "post") == 0 && !handler->input) {
				send_error(501, "Not Implemented", NULL, "That method is not implemented.");
				return;
			}
			
			if (handler->input) {
				handler->input(file, conn_fp, cl, boundary);
				if ((flags = fcntl(fileno(conn_fp), F_GETFL)) != -1 &&
						fcntl(fileno(conn_fp), F_SETFL, flags | O_NONBLOCK) != -1) {
					/* Read up to two more characters */
					if (fgetc(conn_fp) != EOF)
						(void)fgetc(conn_fp);
					
					fcntl(fileno(conn_fp), F_SETFL, flags);
				}
			}
#if (!defined(W7_LOGO) && !defined(WIFI_LOGO))
			if (	nvram_match("wan_route_x", "IP_Routed") &&
				(strstr(file, "result_of_get_changed_status.asp") || strstr(file, "result_of_get_changed_status_QIS.asp") || strstr(file, "detectWAN2.asp") /*|| strstr(file, "ajax_status.asp")*/)
			)
			{
				if (!is_phyconnected())
				{
					nvram_set("link_internet", "0");
					goto no_detect_internet;
				}

				detect_timestamp_old = detect_timestamp;
				detect_timestamp = uptime();
				memset(detect_timestampstr, 0, 32);
				sprintf(detect_timestampstr, "%lu", detect_timestamp);
				nvram_set("detect_timestamp", detect_timestampstr);

				if (!signal_timestamp || ((detect_timestamp - signal_timestamp) > (60 - 1)))
				{
					signal_timestamp = uptime();
					kill_pidfile_s("/var/run/detect_internet.pid", SIGUSR1);
				}
			}
no_detect_internet:
#endif
			send_headers( 200, "Ok", handler->extra_header, handler->mime_type );
			if (handler->output) {
				handler->output(file, conn_fp);
			}
			
			break;
		}
	}
	
	if (!handler->pattern) 
		send_error( 404, "Not Found", (char*) 0, "File not found." );
	
	if (!strcmp(file, "Logout.asp")) {
		http_logout(&login_ip_tmp);
	}
	
	if (!strcmp(file, "Reboot.asp")) {
		system("reboot");
	}
}

void http_login_cache(usockaddr *usa)
{
	login_ip_tmp.family = usa->sa.sa_family;

#if defined (USE_IPV6)
	if (login_ip_tmp.family == AF_INET6) {
		login_ip_tmp.len = sizeof(struct in6_addr);
		memcpy(&login_ip_tmp.addr.in6, &usa->sa_in6.sin6_addr, login_ip_tmp.len);
	} else
#endif
	{
		login_ip_tmp.len = sizeof(struct in_addr);
		login_ip_tmp.addr.in4.s_addr = usa->sa_in.sin_addr.s_addr;
	}
}

int is_phyconnected(void)
{
	int ret = 0;

	if (nvram_match("link_wan", "1"))
		ret = 1;
	else if(is_usb_modem_ready() && (nvram_get_int("modem_rule") > 0))
		ret = 1;

	return ret;
}

int is_fileexist(char *filename)
{
	FILE *fp;

	fp=fopen(filename, "r");

	if (fp==NULL) return 0;
	fclose(fp);
	return 1;
}

int is_firsttime(void)
{
	if (strcmp(nvram_safe_get("w_Setting"), "1")==0)
		return 0;
	else
		return 1;
}

#ifdef TRANSLATE_ON_FLY
int
load_dictionary (char *lang, pkw_t pkw)
{
	char dfn[16];
	char *p, *q;
	FILE *dfp = NULL;
	int dict_size = 0;
	const char *eng_dict = "EN.dict";
	static char loaded_dict[16] = {0};

	if (lang == NULL || strlen(lang) == 0) {
		snprintf(dfn, sizeof (dfn), eng_dict);
	} else {
		snprintf(dfn, sizeof (dfn), "%s.dict", lang);
	}

	if (strcmp (dfn, loaded_dict) == 0) {
		return 1;
	}

	release_dictionary (pkw);

	do {
		dfp = fopen (dfn, "r");
		if (dfp != NULL) {
			snprintf(loaded_dict, sizeof (loaded_dict), "%s", dfn);
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
	if (pkw == NULL)	{
		return;
	}

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
#endif //TRANSLATE_ON_FLY

static void handle_sigterm( int sig )
{
	daemon_exit = 1;
}

int main(int argc, char **argv)
{
	FILE *pid_fp;
	char pidfile[32];
	usockaddr usa;
	socklen_t sz;
	fd_set active_rfds;
	int _port, listen_fd, max_fd, selected;
	conn_list_t pool;
	conn_item_t *item, *next;
	struct timeval tv;
	
	// usage: httpd [port]
	if (argc>1) {
		_port = atoi(argv[1]);
		if (_port >= 80 && _port <= 65535)
			http_port = _port;
	}

	memset(&login_ip, 0, sizeof(uaddr));
	memset(&login_ip_tmp, 0, sizeof(uaddr));
	memset(&last_login_ip, 0, sizeof(uaddr));

	nvram_set("login_timestamp", "");

	detect_timestamp_old = 0;
	detect_timestamp = 0;
	signal_timestamp = 0;

	signal(SIGPIPE, SIG_IGN);
	signal(SIGHUP,  SIG_IGN);
	signal(SIGUSR1, SIG_IGN);
	signal(SIGUSR2, SIG_IGN);
	signal(SIGTERM, handle_sigterm);
	
#if defined (USE_IPV6)
	usa.sa.sa_family = (get_ipv6_type() != IPV6_DISABLED) ? AF_INET6 : AF_INET;
#endif
	if ((listen_fd = initialize_listen_socket(&usa)) < 0) {
		fprintf(stderr, "can't bind to any address\n" );
		exit(errno);
	}
	
	if (daemon(1, 0) < 0) {
		perror("daemon");
		close(listen_fd);
		exit(errno);
	}
	
	if (http_port==SERVER_PORT)
		strcpy(pidfile, "/var/run/httpd.pid");
	else
		sprintf(pidfile, "/var/run/httpd-%d.pid", http_port);
	
	if (!(pid_fp = fopen(pidfile, "w"))) {
		perror(pidfile);
		close(listen_fd);
		exit(errno);
	}
	
	fprintf(pid_fp, "%d", getpid());
	fclose(pid_fp);
	
	chdir("/www");
	
	FD_ZERO(&active_rfds);
	TAILQ_INIT(&pool.head);
	pool.count = 0;
	sz = sizeof(usa);
	
	while (!daemon_exit) {
		fd_set rfds;
		
		rfds = active_rfds;
		if (pool.count < MAX_CONN_ACCEPT) {
			FD_SET(listen_fd, &rfds);
			max_fd = listen_fd;
		} else
			max_fd = -1;
		
		TAILQ_FOREACH(item, &pool.head, entry)
			max_fd = (item->fd > max_fd) ? item->fd : max_fd;
		
		/* wait for new connection or incoming request */
		tv.tv_sec = MAX_CONN_TIMEOUT;
		tv.tv_usec = 0;
		selected = select(max_fd + 1, &rfds, NULL, NULL, &tv);
		if (selected < 0) {
			if (errno == EINTR || errno == EAGAIN)
				continue;
			perror("select");
			break;
		}
		
		/* check and accept new connection */
		if (selected && FD_ISSET(listen_fd, &rfds)) {
			item = malloc(sizeof(*item));
			if (!item) {
				perror("malloc");
				break;
			}
			item->fd = accept(listen_fd, &item->usa.sa, &sz);
			if (item->fd < 0) {
				if (errno != EINTR && errno != EAGAIN)
					perror("accept");
				free(item);
				continue;
			}
			
			setsockopt(item->fd, SOL_SOCKET, SO_KEEPALIVE, &int_1, sizeof(int_1));
			FD_SET(item->fd, &active_rfds);
			TAILQ_INSERT_TAIL(&pool.head, item, entry);
			pool.count++;
			
			/* Continue waiting */
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
				conn_fp = fdopen(item->fd, "r+");
				if (conn_fp) {
					http_login_cache(&item->usa);
					handle_request();
					fflush(conn_fp);
					shutdown(item->fd, 2);
					fclose(conn_fp);
					item->fd = -1;
					conn_fp = NULL;
				}
				if (--selected == 0)
					next = NULL;
			}
			
			if (item->fd >= 0) {
				shutdown(item->fd, 2);
				close(item->fd);
			}
			
			free(item);
		}
	}
	
	/* free all pending requests */
	TAILQ_FOREACH(item, &pool.head, entry) {
		if (item->fd >= 0) {
			shutdown(item->fd, 2);
			close(item->fd);
		}
		
		free(item);
	}
	
	shutdown(listen_fd, 2);
	close(listen_fd);
	
	return 0;
}


