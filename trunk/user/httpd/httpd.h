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

#ifndef _httpd_h_
#define _httpd_h_

#include <ralink_boards.h>
#include <netutils.h>
#include <shutils.h>
#include <nvram/bcmnvram.h>

#include <usb_info.h>

#define SYSLOG_ID_HTTPD		"httpd"

#define STORAGE_HTTPSSL_DIR	"/etc/storage/https"
#define STORAGE_OVPNSVR_DIR	"/etc/storage/openvpn/server"
#define STORAGE_OVPNCLI_DIR	"/etc/storage/openvpn/client"
#define STORAGE_DNSMASQ_DIR	"/etc/storage/dnsmasq"
#define STORAGE_SCRIPTS_DIR	"/etc/storage"

/* Generic MIME type handler */
struct mime_handler {
	char *pattern;
	char *mime_type;
	char *extra_header;
	void (*input)(char *url, FILE *stream, int clen, char *boundary);
	void (*output)(char *url, FILE *stream);
	void (*auth)(int reget);
};

extern struct mime_handler mime_handlers[];

/* CGI helper functions */
extern void init_cgi(char *query);
extern char *get_cgi(char *name);

struct language_table{
	char *Lang;
	char *Target_Lang;
};

extern struct language_table language_tables[];

typedef struct kw_s     {
	int len, tlen;                                          // actually / total
	char dict[4];
	unsigned char **idx;
	unsigned char *buf;
} kw_t, *pkw_t;

extern kw_t kw_EN;
extern kw_t kw_XX;

#define INC_ITEM        128
#define REALLOC_VECTOR(p, len, size, item_size) {                      \
        assert ((len) >= 0 && (len) <= (size));                        \
        if (len == size)        {                                      \
                int new_size;                                          \
                void *np;                                              \
                /* out of vector, reallocate */                        \
                new_size = size + INC_ITEM;                            \
                np = malloc (new_size * (item_size));                  \
                assert (np != NULL);                                   \
                bzero (np, new_size * (item_size));                    \
                memcpy (np, p, len * (item_size));                     \
                free (p);                                              \
                p = np;                                                \
                size = new_size;                                       \
        }    \
}

typedef FILE * webs_t;
typedef char char_t;
#define T(s) (s)
#define __TMPVAR(x) tmpvar ## x
#define _TMPVAR(x) __TMPVAR(x)
#define TMPVAR _TMPVAR(__LINE__)
#define websWrite(wp, fmt, args...) ({ int TMPVAR = fprintf(wp, fmt, ## args); fflush(wp); TMPVAR; })
#define websError(wp, code, msg, args...) fprintf(wp, msg, ## args)
#define websHeader(wp) fputs("<html lang=\"en\">", wp)
#define websFooter(wp) fputs("</html>", wp)
#define websDone(wp, code) fflush(wp)
#define websGetVar(wp, var, default) (get_cgi(var) ? : default)
#define websDefaultHandler(wp, urlPrefix, webDir, arg, url, path, query) ({ do_ej(path, wp); fflush(wp); 1; })
#define websWriteData(wp, buf, nChars) ({ int TMPVAR = fwrite(buf, 1, nChars, wp); fflush(wp); TMPVAR; })
#define websWriteDataNonBlock websWriteData

extern void do_auth(int reget);

/* Regular file handler */
extern void do_file(char *path, FILE *stream);
extern void do_ej(char *path, FILE *stream);

extern int ejArgs(int argc, char_t **argv, char_t *fmt, ...);

struct ej_handler {
	char *pattern;
	int (*output)(int eid, webs_t wp, int argc, char_t **argv);
};

extern struct ej_handler ej_handlers[];

// aspbw.c
extern int f_exists(const char *path);
extern int f_wait_exists(const char *name, int max);
extern void do_f(char *path, webs_t wp);
extern void char_to_ascii(char *output, char *input);

// cgi.c
extern void set_cgi(char *name, char *value);

// crc32.c
extern unsigned long crc32_sp (unsigned long, const unsigned char *, unsigned int);

// ej.c
extern int load_dictionary (char *lang, pkw_t pkw);
extern void release_dictionary (pkw_t pkw);

// httpd.c
extern long uptime(void);
extern int http_login_check(void);
extern void fill_login_ip(char *p_out_ip, size_t out_ip_len);
extern const char *get_login_mac(void);

// ralink.c
struct ifreq;
struct iwreq;
extern void reltime(unsigned long seconds, char *buf);
extern int get_apcli_peer_connected(const char *ifname, struct iwreq *p_wrq);
extern int is_mac_in_sta_list(const unsigned char* p_mac);
extern int ej_lan_leases(int eid, webs_t wp, int argc, char_t **argv);
extern int ej_vpns_leases(int eid, webs_t wp, int argc, char_t **argv);
extern int ej_nat_table(int eid, webs_t wp, int argc, char_t **argv);
extern int ej_route_table(int eid, webs_t wp, int argc, char_t **argv);
extern int ej_conntrack_table(int eid, webs_t wp, int argc, char_t **argv);
extern int wl_ioctl(const char *ifname, int cmd, struct iwreq *pwrq);
extern int ej_wl_status_5g(int eid, webs_t wp, int argc, char_t **argv);
extern int ej_wl_status_2g(int eid, webs_t wp, int argc, char_t **argv);
extern int ej_wl_auth_list(int eid, webs_t wp, int argc, char_t **argv);
extern int ej_wl_scan_5g(int eid, webs_t wp, int argc, char_t **argv);
extern int ej_wl_scan_2g(int eid, webs_t wp, int argc, char_t **argv);
extern int ej_wl_bssid_5g(int eid, webs_t wp, int argc, char_t **argv);
extern int ej_wl_bssid_2g(int eid, webs_t wp, int argc, char_t **argv);

// rtl8367.c or mtk_esw.c
extern int fill_eth_port_status(int port_id, char linkstate[32]);
extern int ej_eth_status_wan(int eid, webs_t wp, int argc, char_t **argv);
extern int ej_eth_status_lan1(int eid, webs_t wp, int argc, char_t **argv);
extern int ej_eth_status_lan2(int eid, webs_t wp, int argc, char_t **argv);
extern int ej_eth_status_lan3(int eid, webs_t wp, int argc, char_t **argv);
extern int ej_eth_status_lan4(int eid, webs_t wp, int argc, char_t **argv);

#if defined (SUPPORT_HTTPS)
extern int ssl_server_init(char* ca_file, char *crt_file, char *key_file, char *dhp_file, char *ssl_cipher_list);
extern void ssl_server_uninit(void);
extern FILE *ssl_server_fopen(int sd);
extern const char* ssl_server_get_ssl_ver(void);
#endif

extern char log_header[];
#define httpd_log(fmt, args...) logmessage(log_header, fmt, ## args);


#endif /* _httpd_h_ */
