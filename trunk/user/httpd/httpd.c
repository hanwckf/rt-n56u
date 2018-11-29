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
#include <ifaddrs.h>

#include <bsd_queue.h>

#include "httpd.h"
#include "common.h"

#define LOGIN_TIMEOUT		30
#define SERVER_NAME		"httpd"
#define SERVER_PORT		80
#define SERVER_PORT_SSL		443
#define PROTOCOL		"HTTP/1.0"
#define CACHE_AGE_VAL		(30 * (24*60*60))
#define RFC1123FMT		"%a, %d %b %Y %H:%M:%S GMT"
#define MAX_LISTEN_BACKLOG	511
#define MAX_CONN_ACCEPT		50
#define MAX_CONN_TIMEOUT	30
#define MAX_AUTH_LEN		128

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

enum {
	HTTP_METHOD_GET = 0,
	HTTP_METHOD_HEAD,
	HTTP_METHOD_POST
};

/* Globals. */
char log_header[32] = {0};
int auth_nvram_changed = 0;
int debug_mode = 0;
#if defined (SUPPORT_HTTPS)
int http_is_ssl = 0;
#endif

static int daemon_exit = 0;
static int http_has_lang = 0;
static int http_acl_mode = 0;
static int login_safe = 0;		// the login from LAN/VPN
static time_t login_timestamp = 0;	// the timestamp of the logined ip
static uaddr login_ip;			// the logined ip
static char login_mac[18] = {0};	// the logined mac
static char auth_basic_data[MAX_AUTH_LEN];

#if defined (USE_IPV6)
static const struct in6_addr in6in4addr_loopback = {{{0x00, 0x00, 0x00, 0x00,
                                                      0x00, 0x00, 0x00, 0x00,
                                                      0x00, 0x00, 0xff, 0xff,
                                                      0x7f, 0x00, 0x00, 0x01}}};
#endif

kw_t kw_EN = {0, 0, {0, 0, 0, 0}, NULL, NULL};
kw_t kw_XX = {0, 0, {0, 0, 0, 0}, NULL, NULL};

const int int_1 = 1;

const struct language_table language_tables[] = {
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

long
uptime(void)
{
	struct sysinfo info;
	sysinfo(&info);

	return info.uptime;
}

static int
is_uaddr_equal(const uaddr *ip1, const uaddr *ip2)
{
	if ((ip1->len > 0) && (ip1->len == ip2->len) && (memcmp(&ip1->addr, &ip2->addr, ip1->len) == 0))
		return 1;

	return 0;
}

static int
is_uaddr_localhost(const uaddr *ip)
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
convert_ip_to_string(const uaddr *ip, char *p_out_ip, size_t out_ip_len)
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
find_mac_from_ip(const uaddr *ip, unsigned char *p_out_mac, int *p_out_lan)
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
			if (sscanf(buffer, "%s %*s 0x%x %31s %*s %31s", s_addr2, &arp_flags, arp_mac, arp_if) == 4) {
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

static int
is_safe_ifname(const char *ifname)
{
	if (strcmp(ifname, IFNAME_BR) == 0)
		return 1;
#if defined(APP_OPENVPN)
	if (strcmp(ifname, IFNAME_SERVER_TUN) == 0)
		return 1;
#endif
	if (strncmp(ifname, "ppp", 3) == 0 && atoi(ifname+3) >= 10)
		return 1;
	return 0;
}

static int
is_safe_ipaddr(const uaddr *ip)
{
	int result = 0;
	struct ifaddrs *ifa, *ife;
	const struct in_addr *addr4, *mask4, *ip4 = NULL;
#if defined (USE_IPV6)
	struct in_addr ip4_6;
	const struct in6_addr *addr6, *mask6, *ip6 = NULL;

	if (ip->family == AF_INET6) {
		ip6 = &ip->addr.in6;
		
		/* check IPv4-Mapped IPv6 Addresses */
		if (IN6_IS_ADDR_V4MAPPED(ip6)) {
			ip4_6.s_addr = ip6->s6_addr32[3];
			ip4 = &ip4_6;
			ip6 = NULL;
		}
	} else
#endif
	if (ip->family == AF_INET)
		ip4 = &ip->addr.in4;
	else
		return 0;

	if (getifaddrs(&ifa) < 0)
		return 0;

	for (ife = ifa; ife; ife = ife->ifa_next) {
		if (!ife->ifa_addr)
			continue;
		if (!(ife->ifa_flags & IFF_UP))
			continue;
		if (!ife->ifa_name)
			continue;
		if (!is_safe_ifname(ife->ifa_name))
			continue;
#if defined (USE_IPV6)
		if (ip6 && ife->ifa_addr->sa_family == AF_INET6) {
			addr6 = &((struct sockaddr_in6 *)ife->ifa_addr)->sin6_addr;
			mask6 = &((struct sockaddr_in6 *)ife->ifa_netmask)->sin6_addr;
			if (IN6_IS_ADDR_LINKLOCAL(addr6))
				continue;
			
			if ((addr6->s6_addr32[0] & mask6->s6_addr32[0]) == (ip6->s6_addr32[0] & mask6->s6_addr32[0]) &&
			    (addr6->s6_addr32[1] & mask6->s6_addr32[1]) == (ip6->s6_addr32[1] & mask6->s6_addr32[1]) &&
			    (addr6->s6_addr32[2] & mask6->s6_addr32[2]) == (ip6->s6_addr32[2] & mask6->s6_addr32[2]) &&
			    (addr6->s6_addr32[3] & mask6->s6_addr32[3]) == (ip6->s6_addr32[3] & mask6->s6_addr32[3])) {
				result = 1;
				break;
			}
		} else
#endif
		if (ip4 && ife->ifa_addr->sa_family == AF_INET) {
			if ((ife->ifa_flags & IFF_POINTOPOINT) && ife->ifa_dstaddr)
				addr4 = &((struct sockaddr_in *)ife->ifa_dstaddr)->sin_addr;
			else
				addr4 = &((struct sockaddr_in *)ife->ifa_addr)->sin_addr;
			mask4 = &((struct sockaddr_in *)ife->ifa_netmask)->sin_addr;
			if (mask4->s_addr == INADDR_ANY || addr4->s_addr == INADDR_BROADCAST)
				continue;
			
			if ((addr4->s_addr & mask4->s_addr) == (ip4->s_addr & mask4->s_addr)) {
				result = 1;
				break;
			}
		}
	}

	freeifaddrs(ifa);

	return result;
}

void
fill_login_ip(char *p_out_ip, size_t out_ip_len)
{
	convert_ip_to_string(&login_ip, p_out_ip, out_ip_len);
}

const char *
get_login_mac(void)
{
	return login_mac;
}

int
get_login_safe(void)
{
	if (login_ip.len == 0)
		return 0;

#if defined (SUPPORT_HTTPS)
	if (http_is_ssl)
		return 1;
#endif

	return login_safe;
}

static void
http_login(const uaddr *ip_now)
{
	char s_lts[32];
	unsigned char mac[8] = {0};

	memcpy(&login_ip, ip_now, sizeof(uaddr));

	if (find_mac_from_ip(ip_now, mac, NULL) == 0)
		ether_etoa(mac, login_mac);
	else
		login_mac[0] = 0;

	if (!get_ap_mode())
		login_safe = is_safe_ipaddr(ip_now);
	else
		login_safe = 1;

	login_timestamp = uptime();

	sprintf(s_lts, "%lu", login_timestamp);
	nvram_set_temp("login_timestamp", s_lts);
}

static void
load_nvram_auth(void)
{
	char *pw_str;
	size_t pw_len, bs_len;

	memset(auth_basic_data, 0, sizeof(auth_basic_data));
	snprintf(auth_basic_data, sizeof(auth_basic_data)-1, "%s:", nvram_safe_get("http_username"));

	bs_len = strlen(auth_basic_data);

	pw_str = nvram_safe_get("http_passwd");
	pw_len = MIN(sizeof(auth_basic_data)-bs_len-1, strlen(pw_str));
	strncpy(&auth_basic_data[bs_len], pw_str, pw_len);
	auth_basic_data[bs_len+pw_len] = '\0';
}

static void
reset_login_data(void)
{
	// load new acl mode
	http_acl_mode = nvram_get_int("http_access");
	http_has_lang = (strlen(nvram_safe_get("preferred_lang")) > 1) ? 1 : 0;

	memset(&login_ip, 0, sizeof(uaddr));

	login_safe = 0;
	login_timestamp = 0;

	nvram_set_temp("login_timestamp", "");

	if (auth_nvram_changed) {
		auth_nvram_changed = 0;
		load_nvram_auth();
	}
}

static void
http_logout(const uaddr *ip_now)
{
	if (is_uaddr_equal(ip_now, &login_ip))
		reset_login_data();
}


/*
 * attempt login check, result
 * 0: can not login, has other loginer
 * 1: can login, this is localhost (always allow w/o auth)
 * 2: can login, no loginer
 * 3: can login, loginer is our
 */
static int
http_login_check(const uaddr *ip_now)
{
	if (is_uaddr_localhost(ip_now))
		return 1;

	if (login_ip.len == 0)
		return 2;

	if (is_uaddr_equal(&login_ip, ip_now))
		return 3;

	if ((unsigned long)(uptime() - login_timestamp) > LOGIN_TIMEOUT) {
		reset_login_data();
		return 2;
	}

	return 0;
}

static int
initialize_listen_socket(usockaddr* usaP, int http_port)
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

	if ( listen( listen_fd, MAX_LISTEN_BACKLOG ) < 0 )
	{
		close(listen_fd);	// 1104 chk
		perror( "listen" );
		return -1;
	}

	return listen_fd;
}

static void
send_headers( int status, const char *title, const char *extra_header, const char *mime_type, const struct stat *st, FILE *conn_fp )
{
	time_t now;
	char timebuf[64];

	now = time(NULL);
	strftime( timebuf, sizeof(timebuf), RFC1123FMT, gmtime( &now ) );

	fprintf( conn_fp, "%s %d %s\r\n", PROTOCOL, status, title );
	fprintf( conn_fp, "Server: %s\r\n", SERVER_NAME );
	fprintf( conn_fp, "Date: %s\r\n", timebuf );
	if (extra_header) {
		fprintf( conn_fp, "%s\r\n", extra_header );
	} else if (st) {
		now += CACHE_AGE_VAL;
		strftime( timebuf, sizeof(timebuf), RFC1123FMT, gmtime( &now ) );
		fprintf( conn_fp, "Cache-Control: max-age=%u\r\n", CACHE_AGE_VAL );
		fprintf( conn_fp, "Expires: %s\r\n", timebuf );
		if (st->st_mtime != 0) {
			now = st->st_mtime;
			strftime( timebuf, sizeof(timebuf), RFC1123FMT, gmtime( &now ) );
			fprintf( conn_fp, "Last-Modified: %s\r\n", timebuf );
		}
		if (st->st_size > 0)
			fprintf( conn_fp, "Content-Length: %lu\r\n", st->st_size );
	}
	if (mime_type)
		fprintf( conn_fp, "Content-Type: %s\r\n", mime_type );
	fprintf( conn_fp, "Connection: close\r\n" );
	fprintf( conn_fp, "\r\n" );
}

static void
send_error( int status, const char *title, const char *extra_header, const char *text, FILE *conn_fp )
{
	send_headers( status, title, extra_header, "text/html", NULL, conn_fp );
	fprintf( conn_fp, "<HTML><HEAD><TITLE>%d %s</TITLE></HEAD>\n<BODY BGCOLOR=\"#cc9999\"><H4>%d %s</H4>\n", status, title, status, title );
	fprintf( conn_fp, "%s\n", text );
	fprintf( conn_fp, "</BODY></HTML>\n" );
	fflush( conn_fp );
}

static void
send_authenticate( FILE *conn_fp )
{
	char header[128], *realm;

	realm = nvram_safe_get("computer_name");
	if (strlen(realm) < 1)
		realm = nvram_safe_get("productid");

	snprintf(header, sizeof(header), "WWW-Authenticate: Basic realm=\"%s\"", realm);
	send_error( 401, "Unauthorized", header, "Authorization required.", conn_fp );
}

static int
auth_check( const char *authorization )
{
	char authinfo[256];
	int auth_len;

	/* Basic authorization info? */
	if (!authorization || strncmp(authorization, "Basic ", 6) != 0)
		return 0;

	/* Decode it. */
	auth_len = b64_decode(authorization+6, authinfo, sizeof(authinfo)-1);
	authinfo[auth_len] = '\0';

	/* Is this the right user and password? */
	if (strcmp(authinfo, auth_basic_data) == 0)
		return 1;

	return 0;
}

static int
match_one( const char *pattern, int patternlen, const char *string )
{
	const char* p;

	for ( p = pattern; p - pattern < patternlen; ++p, ++string ) {
		if ( *p == '?' && *string != '\0' )
			continue;
		if ( *p == '*' ) {
			int i, pl;
			++p;
			if ( *p == '*' ) {
				/* Double-wildcard matches anything. */
				++p;
				i = strlen( string );
			} else {
				/* Single-wildcard matches anything but slash. */
				i = strcspn( string, "/" );
			}
			
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

/* Simple shell-style filename matcher.  Only does ? * and **, and multiple
** patterns separated by |.  Returns 1 or 0.
*/
static int
match( const char *pattern, const char *string )
{
	const char *or;

	for (;;) {
		or = strchr( pattern, '|' );
		if ( or == (char*) 0 )
			return match_one( pattern, strlen( pattern ), string );
		if ( match_one( pattern, or - pattern, string ) )
			return 1;
		pattern = or + 1;
	}
}

static void
eat_post_data(FILE *conn_fp, int clen)
{
	char fake_buf[128];

	do_cgi_clear();

	if (!fgets(fake_buf, MIN(clen+1, sizeof(fake_buf)), conn_fp))
		return;

	clen -= strlen(fake_buf);
	while (clen--)
		fgetc(conn_fp);
}

static void
try_pull_data(FILE *conn_fp, int conn_fd)
{
	int flags = fcntl(conn_fd, F_GETFL);

	/* Read up to two more characters */
	if (flags != -1 && fcntl(conn_fd, F_SETFL, flags | O_NONBLOCK) != -1) {
		if (fgetc(conn_fp) != EOF)
			fgetc(conn_fp);
		
		fcntl(conn_fd, F_SETFL, flags);
	}
}

int
do_fwrite(const char *buffer, int len, FILE *stream)
{
	int n = len;
	int r = 0;

	while (n > 0) {
		r = fwrite(buffer, 1, n, stream);
		if ((r == 0) && (errno != EINTR))
			return -1;
		buffer += r;
		n -= r;
	}

	return r;
}

void
do_file(const char *url, FILE *stream)
{
	FILE *fp;
	char buf[1024];
	int nr;

	if ((fp = fopen(url, "r")) != NULL) {
		while ((nr = fread(buf, 1, sizeof(buf), fp)) > 0)
			do_fwrite(buf, nr, stream);
		fclose(fp);
	}
}

static int
set_preferred_lang(char *cur)
{
	char *p, *p_lang;
	char lang_buf[64], lang_file[16];
	const struct language_table *p_lt;

	memset(lang_buf, 0, sizeof(lang_buf));
	strncpy(lang_buf, cur, sizeof(lang_buf)-1);

	p = lang_buf;
	p_lang = NULL;
	while (p != NULL) {
		p = strtok (p, "\r\n ,;");
		if (p == NULL)
			break;
		
		for (p_lt = language_tables; p_lt->Lang != NULL; ++p_lt) {
			if (strcasecmp(p, p_lt->Lang)==0) {
				p_lang = p_lt->Target_Lang;
				break;
			}
		}
		
		if (p_lang)
			break;
		
		p+=strlen(p)+1;
	}

	if (p_lang) {
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
handle_request(FILE *conn_fp, const conn_item_t *item)
{
	char line[4096];
	char *method, *path, *protocol, *authorization, *boundary;
	char *cur, *end, *cp, *file, *query;
	int len, login_state, method_id, do_logout, clen = 0;
	time_t if_modified_since = (time_t)-1;
	struct mime_handler *handler;
	struct stat st, *p_st = NULL;
	uaddr conn_ip;

	/* Initialize the request variables. */
	authorization = boundary = NULL;

	/* Parse the first line of the request. */
	if (!fgets(line, sizeof(line), conn_fp)) {
		send_error( 400, "Bad Request", NULL, "No request found.", conn_fp);
		return;
	}

	method = path = line;
	strsep(&path, " ");
	while (path && *path == ' ') path++;

	protocol = path;
	strsep(&protocol, " ");
	while (protocol && *protocol == ' ') protocol++;

	cp = protocol;
	strsep(&cp, " ");

	if ( !method || !path || !protocol ) {
		send_error( 400, "Bad Request", NULL, "Can't parse request.", conn_fp );
		return;
	}

	cur = protocol + strlen(protocol) + 1;
	end = line + sizeof(line) - 1;

	while ( (cur < end) && (fgets(cur, line + sizeof(line) - cur, conn_fp)) ) {
		if ( strcmp( cur, "\n" ) == 0 || strcmp( cur, "\r\n" ) == 0 ) {
			break;
		}
		
		if (strncasecmp(cur, "Accept-Language:", 16) == 0) {
			if (!http_has_lang)
				http_has_lang = set_preferred_lang(cur + 16);
		}
		else if (strncasecmp( cur, "Authorization:", 14) == 0) {
			cp = cur + 14;
			cp += strspn( cp, " \t" );
			authorization = cp;
			cur = cp + strlen(cp) + 1;
		}
		else if (strncasecmp( cur, "Content-Length:", 15) == 0) {
			cp = cur + 15;
			cp += strspn( cp, " \t" );
			clen = strtoul( cp, NULL, 0 );
			if ((clen < 0) || (clen > 50000000)) {
				send_error( 400, "Bad Request", NULL, "Content length invalid.", conn_fp);
				return;
			}
		}
		else if (strncasecmp( cur, "If-Modified-Since:", 18) == 0) {
			cp = cur + 18;
			cp += strspn( cp, " \t" );
			if_modified_since = tdate_parse(cp);
		}
		else if ((cp = strstr( cur, "boundary=" ))) {
			boundary = cp + 9;
			for ( cp = cp + 9; *cp && *cp != '\r' && *cp != '\n'; cp++ );
			*cp = '\0';
			cur = ++cp;
		}
	}

	if (strcasecmp(method, "get") == 0)
		method_id = HTTP_METHOD_GET;
	else if (strcasecmp(method, "head") == 0)
		method_id = HTTP_METHOD_HEAD;
	else if (strcasecmp(method, "post") == 0)
		method_id = HTTP_METHOD_POST;
	else {
		send_error( 501, "Not Implemented", NULL, "Unsupported method.", conn_fp );
		return;
	}

	if ( path[0] != '/' ) {
		send_error( 400, "Bad Request", NULL, "Bad URL.", conn_fp );
		return;
	}

	file = path + 1;
	len = strlen(file);

	if (file[0] == '/' || strcmp(file, "..") == 0 || strncmp(file, "../", 3) == 0 || strstr(file, "/../") != NULL) {
		send_error( 400, "Bad Request", NULL, "Illegal URL.", conn_fp );
		return;
	}

	if (len > 0 && file[len-1] == '/') {
		send_error( 400, "Bad Request", NULL, "Illegal URL.", conn_fp );
		return;
	}

	if (len > 2 && strcmp(&(file[len-3]), "/.." ) == 0) {
		send_error( 400, "Bad Request", NULL, "Illegal URL.", conn_fp );
		return;
	}

	if (len < 1)
		file = "index.asp";

	query = file;
	strsep(&query, "?");

	usockaddr_to_uaddr(&item->usa, &conn_ip);

	login_state = http_login_check(&conn_ip);
	
	if (login_state == 0) {
		if (strstr(file, ".htm") != NULL || strstr(file, ".asp") != NULL) {
			file = "Nologin.asp";
			query = NULL;
		}
	}
	

	/* special case for reset browser credentials */
	if (strcmp(file, "logout") == 0) {
		send_headers( 401, "Unauthorized", NULL, NULL, NULL, conn_fp );
		return;
	}

	for (handler = mime_handlers; handler->pattern; handler++) {
		if (match(handler->pattern, file))
			break;
	}

	if (!handler->pattern) {
		send_error( 404, "Not Found", NULL, "URL was not found.", conn_fp );
		return;
	}

#if defined (SUPPORT_HTTPS)
	http_is_ssl = item->ssl;
#endif

	do_logout = (strcmp(file, "Logout.asp") == 0) ? 1 : 0;

	if (handler->need_auth && login_state > 1 && !do_logout) {
		if (!auth_check(authorization)) {
			http_logout(&conn_ip);
			if (method_id == HTTP_METHOD_POST)
				eat_post_data(conn_fp, clen);
			send_authenticate(conn_fp);
			return;
		}
		
		if (login_state == 2)
			http_login(&conn_ip);
	}

	if (method_id == HTTP_METHOD_POST) {
		if (handler->input)
			handler->input(file, conn_fp, clen, boundary);
		else
			eat_post_data(conn_fp, clen);
		try_pull_data(conn_fp, item->fd);
	} else {
		if (query)
			do_uncgi_query(query);
		else if (handler->output == do_ej)
			do_cgi_clear();
	}

	if (handler->output == do_file) {
		if (stat(file, &st) == 0 && !S_ISDIR(st.st_mode)) {
			p_st = &st;
			if (!handler->extra_header && if_modified_since != (time_t)-1 && if_modified_since == st.st_mtime) {
				st.st_size = 0; /* not send Content-Length */
				send_headers( 304, "Not Modified", NULL, handler->mime_type, p_st, conn_fp );
				return;
			}
		}
	}

	send_headers( 200, "OK", handler->extra_header, handler->mime_type, p_st, conn_fp );

	if (method_id != HTTP_METHOD_HEAD) {
		if (handler->output)
			handler->output(file, conn_fp);
	}

	if (do_logout)
		http_logout(&conn_ip);
}

static void
reset_lang_dict(void)
{
	// reset last loaded XX.dict
	memset(kw_XX.dict, 0, sizeof(kw_XX.dict));
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
		reset_lang_dict();
	}
	else if (sig == SIGUSR1)
	{
		reset_login_data();
	}
	else if (sig == SIGUSR2)
	{
		;
	}
}

int
main(int argc, char **argv)
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

	if ((pid_fp = fopen("/var/run/httpd.pid", "w"))) {
		fprintf(pid_fp, "%d", pid);
		fclose(pid_fp);
	}

	reset_login_data();
	load_nvram_auth();

	chdir("/www");

	FD_ZERO(&active_rfds);
	TAILQ_INIT(&pool.head);
	pool.count = 0;
	sz = sizeof(usa);

	load_dictionary("EN", &kw_EN);

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
			else
				httpd_log("Failed to select open sockets (errno: %d). EXITING", errno);
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
				if (item->ssl)
					conn_fp = ssl_server_fopen(item->fd);
				else
#endif
				conn_fp = fdopen(item->fd, "r+");
				if (conn_fp) {
					handle_request(conn_fp, item);
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
	TAILQ_FOREACH_SAFE(item, &pool.head, entry, next) {
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

	init_cgi(NULL);
	release_dictionary(&kw_XX);
	release_dictionary(&kw_EN);

	return 0;
}

