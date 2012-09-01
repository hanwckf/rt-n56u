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
/* milli_httpd - pretty small HTTP server
** A combination of
** micro_httpd - really small HTTP server
** and
** mini_httpd - small HTTP server
**
** Copyright ?1999,2000 by Jef Poskanzer <jef@acme.com>.
** All rights reserved.
**
** Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions
** are met:
** 1. Redistributions of source code must retain the above copyright
**    notice, this list of conditions and the following disclaimer.
** 2. Redistributions in binary form must reproduce the above copyright
**    notice, this list of conditions and the following disclaimer in the
**    documentation and/or other materials provided with the distribution.
**
** THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
** ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
** IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
** ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
** FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
** DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
** OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
** HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
** LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
** OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
** SUCH DAMAGE.
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
#ifndef SIOCETHTOOL
#define SIOCETHTOOL 0x8946
#endif

#define ETCPHYRD	14
#define SIOCGETCPHYRD   0x89FE

#define SERVER_NAME "httpd"
#define SERVER_PORT 80
#define PROTOCOL "HTTP/1.0"
#define RFC1123FMT "%a, %d %b %Y %H:%M:%S GMT"

/* A multi-family sockaddr. */
typedef union {
    struct sockaddr sa;
    struct sockaddr_in sa_in;
} usockaddr;

/* Globals. */
static int daemon_exit = 0;

static FILE *conn_fp = NULL;
static char auth_userid[AUTH_MAX];
static char auth_passwd[AUTH_MAX];
static char auth_realm[AUTH_MAX];
#ifdef TRANSLATE_ON_FLY
char Accept_Language[16];
#endif //TRANSLATE_ON_FLY

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

int redirect = 0;
int change_passwd = 0;
int reget_passwd = 0;
char url[128];
int http_port=SERVER_PORT;

unsigned int login_ip=0; // the logined ip
time_t login_timestamp=0; // the timestamp of the logined ip
unsigned int login_ip_tmp=0; // the ip of the current session.
unsigned int login_try=0;
unsigned int last_login_ip = 0;	// the last logined ip 2008.08 magic

time_t request_timestamp = 0;
time_t turn_off_auth_timestamp = 0;
int temp_turn_off_auth = 0;	// for QISxxx.htm pages

void http_login(unsigned int ip, char *url);
void http_login_timeout(unsigned int ip);
void http_logout(unsigned int ip);

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

static int
initialize_listen_socket( usockaddr* usaP )
{
    int listen_fd;
    int i;

    memset( usaP, 0, sizeof(usockaddr) );
    usaP->sa.sa_family = AF_INET;
    usaP->sa_in.sin_addr.s_addr = htonl( INADDR_ANY );
    usaP->sa_in.sin_port = htons( http_port );

    listen_fd = socket( usaP->sa.sa_family, SOCK_STREAM, 0 );
    if ( listen_fd < 0 )
	{
	perror( "socket" );
	return -1;
	}
    fcntl( listen_fd, F_SETFD, 1 );
    i = 1;
    if ( setsockopt( listen_fd, SOL_SOCKET, SO_REUSEADDR, (char*) &i, sizeof(i) ) < 0 )
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

	//printf("[httpd] auth chk:%s, %s\n", dirname, url);	// tmp test
    /* Is this directory unprotected? */
    if ( !strlen(auth_passwd) )
	/* Yes, let the request go through. */
	return 1;

    /* Basic authorization info? */
    if ( !authorization || strncmp( authorization, "Basic ", 6 ) != 0) 
    {
	send_authenticate( dirname );
	http_logout(login_ip_tmp);
	last_login_ip = 0;	// 2008.08 magic
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
	http_logout(login_ip_tmp);
	return 0;
    }
    *authpass++ = '\0';

    /* Is this the right user and password? */
    if ( strcmp( auth_userid, authinfo ) == 0 && strcmp( auth_passwd, authpass ) == 0 )
    {
    	//fprintf(stderr, "login check : %x %x\n", login_ip, last_login_ip);
    	/* Is this is the first login after logout */
    	if (login_ip==0 && last_login_ip==login_ip_tmp)
    	{
		send_authenticate(dirname);
		last_login_ip=0;
		return 0;
    	}
	return 1;
    }

    send_authenticate( dirname );
    http_logout(login_ip_tmp);
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


time_t detect_timestamp, detect_timestamp_old, signal_timestamp;
char detect_timestampstr[32];

static void
handle_request(void)
{
    char line[10000], *cur;
    char *method, *path, *protocol, *authorization, *boundary, *alang;
    char *cp;
    char *file;
    int len;
    struct mime_handler *handler;
    int cl = 0, flags;

    /* Initialize the request variables. */
    authorization = boundary = NULL;
    bzero( line, sizeof line );

    /* Parse the first line of the request. */
    if ( fgets( line, sizeof(line), conn_fp ) == (char*) 0 ) {
	send_error( 400, "Bad Request", (char*) 0, "No request found." );
	return;
    }

    method = path = line;
    strsep(&path, " ");
    while (path && *path == ' ') path++;	// oleg patch
    protocol = path;
    strsep(&protocol, " ");
    while (protocol && *protocol == ' ') protocol++;    // oleg pat
    cp = protocol;
    strsep(&cp, " ");
    if ( !method || !path || !protocol ) {
	send_error( 400, "Bad Request", (char*) 0, "Can't parse request." );
	return;
    }
    cur = protocol + strlen(protocol) + 1;

#ifdef TRANSLATE_ON_FLY
	memset(Accept_Language, 0, sizeof(Accept_Language));
#endif

    /* Parse the rest of the request headers. */
    while ( fgets( cur, line + sizeof(line) - cur, conn_fp ) != (char*) 0 )
	{
		

	if ( strcmp( cur, "\n" ) == 0 || strcmp( cur, "\r\n" ) == 0 ) {
	    break;
	}
#ifdef TRANSLATE_ON_FLY
	else if ( strncasecmp( cur, "Accept-Language:",16) ==0) {
		char *p;
		struct language_table *pLang;
		char lang_buf[256];
		memset(lang_buf, 0, sizeof(lang_buf));
		alang = &cur[16];
		strcpy(lang_buf, alang);
		p = lang_buf;
		while (p != NULL)
		{
			p = strtok (p, "\r\n ,;");
			if (p == NULL)  break;
			int i, len=strlen(p);
			for (i=0;i<len;++i)
				if (isupper(p[i])) {
					p[i]=tolower(p[i]);
				}

			for (pLang = language_tables; pLang->Lang != NULL; ++pLang)
			{
				if (strcasecmp(p, pLang->Lang)==0)
				{
					snprintf(Accept_Language,sizeof(Accept_Language),"%s",pLang->Target_Lang);
					if (is_firsttime ())    {
						nvram_set("preferred_lang", Accept_Language);
					}
					break;
				}
			}

			if (Accept_Language[0] != 0)    {
				break;
			}
			p+=strlen(p)+1;
		}

		if (Accept_Language[0] == 0)    {
			// If all language setting of user's browser are not supported, use English.
//			printf ("Auto detect language failed. Use English.\n");			
			strcpy (Accept_Language, "EN");

				if (is_firsttime())
					nvram_set("preferred_lang", "EN");
		}
	}
#endif


	else if ( strncasecmp( cur, "Authorization:", 14 ) == 0 )
	    {
	    cp = &cur[14];
	    cp += strspn( cp, " \t" );
	    authorization = cp;
	    cur = cp + strlen(cp) + 1;
	    }
	else if (strncasecmp( cur, "Content-Length:", 15 ) == 0) {
		cp = &cur[15];
		cp += strspn( cp, " \t" );
		cl = strtoul( cp, NULL, 0 );
	}
	else if ((cp = strstr( cur, "boundary=" ))) {
	    boundary = &cp[9];
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
	
	memset(url, 0, 128);
	if ((query = index(file, '?')) != NULL) {
		file_len = strlen(file)-strlen(query);
		
		strncpy(url, file, file_len);
	}
	else
	{
		strcpy(url, file);
	}
// 2007.11 James. }

	http_login_timeout(login_ip_tmp);	// 2008.07 James.
	
	if (http_login_check() == 3) {
		if ((strstr(url, ".htm") != NULL
					&& !(!strcmp(url, "error_page.htm")
						|| (strstr(url, "QIS_") != NULL && nvram_match("x_Setting", "0") && login_ip == 0)
//						|| (strstr(url, "Logout.asp") != NULL && nvram_match("r_Setting", "0") && login_ip == 0)//1202
						|| !strcmp(url, "gotoHomePage.htm")
						)
					)
				|| (strstr(url, ".asp") != NULL && login_ip != 0)
				) {
			file = "Nologin.asp";
			
			memset(url, 0, 128);
			strcpy(url, file);
		}
	}
	
	for (handler = &mime_handlers[0]; handler->pattern; handler++) 
	{
		if (match(handler->pattern, url))
		{
//			request_timestamp = time((time_t *)0);
			request_timestamp = uptime();
			
			int login_state = http_login_check();

			if ((login_state == 1 || login_state == 2)
					//&& !nvram_match("x_Setting", "1") 	// user: step 0
					/* modify QIS authentication flow */
					&& (strstr(url, "QIS_") != NULL   // to avoid the interference of the other logined browser. 2008.11 magic
							|| !strcmp(url, "survey.htm")	
							|| !strcmp(url, "ureip.asp")	
							|| !strcmp(url, "Logout.asp")	
							|| !strcmp(url, "aplist.asp")	
							|| !strcmp(url, "wlconn_apply.htm")
							|| !strcmp(url, "result_of_get_changed_status.asp")
							|| !strcmp(url, "result_of_get_changed_status_QIS.asp")
							|| !strcmp(url, "detectWAN.asp")
							|| !strcmp(url, "WPS_info.asp")
							|| !strcmp(url, "WAN_info.asp")
							|| !strcmp(url, "result_of_detect_client.asp")
							|| !strcmp(url, "start_apply.htm")
							|| !strcmp(url, "start_apply2.htm")
							|| !strcmp(url, "detectWAN2.asp")
							|| !strcmp(url, "automac.asp")
							|| !strcmp(url, "setting_lan.htm")
							|| !strcmp(url, "status.asp")
							|| !strcmp(url, "httpd_check.htm")
//							|| !strcmp(url, "ajax_status.asp")
							)
					) {
/* hacker issue patch start */
                                                if((nvram_match("x_Setting", "1")) && login_state == 1){ // user : Step1, hacker : Step2
                                                                if (!strcmp(url, "start_apply.htm") || !strcmp(url, "start_apply2.htm") || !strcmp(url, "wlconn_apply.htm")){
                                                                temp_turn_off_auth = 0; // auth
                                                                turn_off_auth_timestamp = request_timestamp;
                                                                redirect = 0;
                                                                }
                                                                else{ // user : Step2
                                                                turn_off_auth_timestamp = request_timestamp;
                                                                temp_turn_off_auth = 1; // no auth
                                                                redirect = 0;
                                                                }
                                                }
                                                else{ // user : Step3
                                                                turn_off_auth_timestamp = request_timestamp;
                                                                temp_turn_off_auth = 1; // no auth
                                                                redirect = 0;
                                                }
/* hacker issue patch end */
			}
			else if(!strcmp(url, "error_page.htm")
					|| !strcmp(url, "jquery.js") // 2010.09 James.
					|| !strcmp(url, "Nologin.asp")
					|| !strcmp(url, "gotoHomePage.htm")
					) {
				;	// do nothing.
			}
			else if (login_state == 2
					&& !strcmp(url, "Logout.asp")) {
				turn_off_auth_timestamp = 0;
				temp_turn_off_auth = 0;
				redirect = 1;
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
						printf("System error! the url: %s would be changed to Nologin.asp in this case!\n", url);
						break;
					default:
						printf("System error! the login_state is wrong!\n");
				}
			}
			else if (login_state == 2
					&& temp_turn_off_auth
					&& (unsigned long)(request_timestamp-turn_off_auth_timestamp) > 10
					) {
				http_logout(login_ip_tmp);
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
							http_login(login_ip_tmp, url);
					}
					else
						http_login(login_ip_tmp, url);
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
					if (nvram_match("di_debug", "1")) fprintf(stderr, "refresh timer of detect_internet\n");

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
		http_logout(login_ip_tmp);
	}
	
	if (!strcmp(file, "Reboot.asp")) {
		system("reboot");
	}
//2008.08 magic}
}

//2008 magic{
void http_login_cache(usockaddr *u) {
	struct in_addr temp_ip_addr;
	char *temp_ip_str;
	
	login_ip_tmp = (unsigned int)(u->sa_in.sin_addr.s_addr);
	temp_ip_addr.s_addr = login_ip_tmp;
	temp_ip_str = inet_ntoa(temp_ip_addr);
}


void http_login(unsigned int ip, char *url) {
	struct in_addr login_ip_addr;
	char *login_ip_str;
	
	if (ip == 0x100007f)
		return;
	
	login_ip = ip;
	last_login_ip = 0;
	
	login_ip_addr.s_addr = login_ip;
	login_ip_str = inet_ntoa(login_ip_addr);
	nvram_set("login_ip_srt", login_ip_str);
	
	if (strcmp(url, "result_of_get_changed_status.asp")
			&& strcmp(url, "WPS_info.asp")
			&& strcmp(url, "WAN_info.asp")) //2008.11 magic
//		login_timestamp = time((time_t *)0);
		login_timestamp = uptime();
	
	char login_ipstr[32], login_timestampstr[32];
	
	memset(login_ipstr, 0, 32);
	sprintf(login_ipstr, "%u", login_ip);
	nvram_set("login_ip", login_ipstr);

	if (strcmp(url, "result_of_get_changed_status.asp")
			&& strcmp(url, "WPS_info.asp")
			&& strcmp(url, "WAN_info.asp")) {//2008.11 magic
		memset(login_timestampstr, 0, 32);
		sprintf(login_timestampstr, "%lu", login_timestamp);
		nvram_set("login_timestamp", login_timestampstr);
	}
}

// 0: can not login, 1: can login, 2: loginer, 3: not loginer.
int http_login_check(void) {
	if (login_ip_tmp == 0x100007f)
		return 1;
	
	//http_login_timeout(login_ip_tmp);	// 2008.07 James.
	
	if (login_ip == 0)
		return 1;
	else if (login_ip == login_ip_tmp)
		return 2;
	
	return 3;
}

void http_login_timeout(unsigned int ip)
{
	time_t now;

	if (ip == 0x100007f)
		return;

//	time(&now);
	now = uptime();
	
	//if (login_ip!=ip && (unsigned long)(now-login_timestamp) > 60) //one minitues
	if ((login_ip != 0 && login_ip != ip) && ((unsigned long)(now-login_timestamp) > 60)) //one minitues
	{
		http_logout(login_ip);
	}
}

void http_logout(unsigned int ip) {
	if (ip == login_ip) {
		last_login_ip = login_ip;
		login_ip = 0;
		login_timestamp = 0;
		
		nvram_set("login_ip", "");
		nvram_set("login_timestamp", "");
		
		if (change_passwd == 1) {
			change_passwd = 0;
			reget_passwd = 1;
		}
	}
}

int is_auth(void)
{
	return 1;
}

int is_phyconnected(void)
{
	int ret = 0;

	if (nvram_match("link_wan", "1"))
		ret = 1;
	else if(is_usb_modem_ready())
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
#ifndef RELOAD_DICT
	static char loaded_dict[12] = {'\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0'};
#endif  // RELOAD_DICT

	if (lang == NULL || (lang != NULL && strlen (lang) == 0))       {
		// if "lang" is invalid, use English as default
		snprintf (dfn, sizeof (dfn), eng_dict);
	} else {
		snprintf (dfn, sizeof (dfn), "%s.dict", lang);
	}

#ifndef RELOAD_DICT
	if (strcmp (dfn, loaded_dict) == 0)     {
		return 1;
	}
	release_dictionary (pkw);
#endif  // RELOAD_DICT

	do      {
		dfp = fopen (dfn, "r");
		if (dfp != NULL)	{
#ifndef RELOAD_DICT
			snprintf (loaded_dict, sizeof (loaded_dict), "%s", dfn);
#endif  // RELOAD_DICT
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


static void handle_sigchld( int sig )
{
	const int oerrno = errno;
	pid_t pid;
	int status;
	
	/* Set up handler again. */
	signal(SIGCHLD, handle_sigchld);
	
	/* Reap defunct children until there aren't any more. */
	for (;;)
	{
		pid = waitpid( (pid_t) -1, &status, WNOHANG );
		if ( (int) pid == 0 )		/* none left */
			break;
		
		if ( (int) pid < 0 )
		{
			if ( errno == EINTR || errno == EAGAIN )
				continue;
			/* ECHILD shouldn't happen with the WNOHANG option,
			** but with some kernels it does anyway.  Ignore it.
			*/
			if ( errno != ECHILD )
			{
				perror( "child wait" );
			}
			break;
		}
	}
	
	/* Restore previous errno. */
	errno = oerrno;
}

static void handle_sigterm( int sig )
{
	daemon_exit = 1;
}

int main(int argc, char **argv)
{
	FILE *pid_fp;
	char pidfile[32];
	usockaddr usa;
	fd_set rfds, listen_rfds;
	int _port, selected, listen4_fd, client4_fd, max_fd;
	socklen_t sz;
	
	// usage: httpd [port]
	if (argc>1) {
		_port = atoi(argv[1]);
		if (_port >= 80 && _port <= 65535)
			http_port = _port;
	}
	
	//2008.08 magic
	nvram_unset("login_timestamp");
	nvram_unset("login_ip");
	nvram_unset("login_ip_str");

	detect_timestamp_old = 0;
	detect_timestamp = 0;
	signal_timestamp = 0;

	/* Ignore broken pipes */
	signal(SIGPIPE, SIG_IGN);
	signal(SIGHUP,  SIG_IGN);
	signal(SIGUSR1, SIG_IGN);
	signal(SIGUSR2, SIG_IGN);
	signal(SIGTERM, handle_sigterm);
	signal(SIGCHLD, handle_sigchld);
	
	/* Initialize listen socket */
	if ((listen4_fd = initialize_listen_socket(&usa)) < 0) {
		fprintf(stderr, "can't bind to any address\n" );
		exit(errno);
	}
	
	if (daemon(1, 0) < 0) {
		perror("daemon");
		close(listen4_fd);
		exit(errno);
	}
	
	if (http_port==SERVER_PORT)
		strcpy(pidfile, "/var/run/httpd.pid");
	else
		sprintf(pidfile, "/var/run/httpd-%d.pid", http_port);
	
	if (!(pid_fp = fopen(pidfile, "w"))) {
		perror(pidfile);
		close(listen4_fd);
		exit(errno);
	}
	
	fprintf(pid_fp, "%d", getpid());
	fclose(pid_fp);
	
	chdir("/www");
	
	FD_ZERO(&rfds);
	FD_SET(listen4_fd, &rfds);
	max_fd = listen4_fd;
	client4_fd = -1;
	sz = sizeof(usa);
	
	/* Loop forever handling requests */
	while (!daemon_exit) 
	{
		listen_rfds = rfds;
		selected = select(max_fd + 1, &listen_rfds, NULL, NULL, NULL);
		if (selected < 0)
		{
			if ( errno == EINTR || errno == EAGAIN )
				continue;
			
			break;
		}
		
		/* Check and accept new connection */
		if (selected && FD_ISSET(listen4_fd, &listen_rfds))
		{
			client4_fd = accept(listen4_fd, &usa.sa, &sz);
			if (client4_fd < 0)
				continue;
			
			/* Pending request, process it */
			conn_fp = fdopen(client4_fd, "r+");
			if (conn_fp) {
				/* Process HTTP request */
				http_login_cache(&usa);
				handle_request();
				
				/* fclose already closed file descriptor */
				fflush(conn_fp);
				fclose(conn_fp);
				conn_fp = NULL;
			}
		}
	}
	
	shutdown(listen4_fd, 2);
	close(listen4_fd);
	
	return 0;
}
