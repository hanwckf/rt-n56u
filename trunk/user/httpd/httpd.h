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
#include <ralink_priv.h>
#include <netutils.h>
#include <rtutils.h>
#include <shutils.h>
#include <nvram_linux.h>

#define SYSLOG_ID_HTTPD		"httpd"

#define STORAGE_HTTPSSL_DIR	"/etc/storage/https"
#define STORAGE_OVPNSVR_DIR	"/etc/storage/openvpn/server"
#define STORAGE_OVPNCLI_DIR	"/etc/storage/openvpn/client"
#define STORAGE_DNSMASQ_DIR	"/etc/storage/dnsmasq"
#define STORAGE_SCRIPTS_DIR	"/etc/storage"
#define STORAGE_CRONTAB_DIR	"/etc/storage/cron/crontabs"

#define PROFILE_FIFO_UPLOAD	"/tmp/settings_u.prf"
#define PROFILE_FIFO_DOWNLOAD	"/tmp/settings_d.prf"
#define STORAGE_FIFO_FILENAME	"/tmp/storage.tar.bz2"

/* Generic MIME type handler */
struct mime_handler {
	char *pattern;
	char *mime_type;
	char *extra_header;
	void (*input)(const char *url, FILE *stream, int clen, char *boundary);
	void (*output)(const char *url, FILE *stream);
	int need_auth;
};

extern struct mime_handler mime_handlers[];

/* CGI helper functions */
extern void init_cgi(char *query);
extern char *get_cgi(char *name);

struct language_table{
	char *Lang;
	char *Target_Lang;
};

extern const struct language_table language_tables[];

typedef struct kw_s     {
	int len, tlen;                                          // actually / total
	char dict[4];
	unsigned char **idx;
	unsigned char *buf;
} kw_t, *pkw_t;

extern kw_t kw_EN;
extern kw_t kw_XX;

#define INC_ITEM	256
#define REALLOC_VECTOR(p, len, size, item_size) {		\
	assert ((len) >= 0 && (len) <= (size));			\
	if (len == size) {					\
		int new_size = size + INC_ITEM;			\
		void *np = malloc(new_size * (item_size));	\
		assert(np != NULL);				\
		bzero(np, new_size * (item_size));		\
		if (p) {					\
			memcpy(np, p, len * (item_size));	\
			free(p);				\
		}						\
		p = np;						\
		size = new_size;				\
	}							\
}

typedef FILE * webs_t;
#define T(s) (s)
#define __TMPVAR(x) tmpvar ## x
#define _TMPVAR(x) __TMPVAR(x)
#define TMPVAR _TMPVAR(__LINE__)
#define websWrite(wp, fmt, args...) ({ int TMPVAR = fprintf(wp, fmt, ## args); fflush(wp); TMPVAR; })
#define websError(wp, code, msg, args...) fprintf(wp, msg, ## args)
#define websDone(wp, code) fflush(wp)
#define websGetVar(wp, var, default) (get_cgi(var) ? : default)

/* Regular file handler */
extern void do_file(const char *url, FILE *stream);
extern void do_ej(const char *url, FILE *stream);

extern int ejArgs(int argc, char **argv, char *fmt, ...);

struct ej_handler {
	char *pattern;
	int (*output)(int eid, webs_t wp, int argc, char **argv);
};

extern struct ej_handler ej_handlers[];

// aidisk.c
#if defined (USE_USB_SUPPORT)
extern int ej_get_usb_ports_info(int eid, webs_t wp, int argc, char **argv);
#endif
#if defined (USE_STORAGE)
extern int ej_disk_pool_mapping_info(int eid, webs_t wp, int argc, char **argv);
extern int ej_available_disk_names_and_sizes(int eid, webs_t wp, int argc, char **argv);
extern int ej_get_storage_share_list(int eid, webs_t wp, int argc, char **argv);
extern int ej_get_AiDisk_status(int eid, webs_t wp, int argc, char **argv);
extern int ej_set_AiDisk_status(int eid, webs_t wp, int argc, char **argv);
extern int ej_get_all_accounts(int eid, webs_t wp, int argc, char **argv);
extern int ej_safely_remove_disk(int eid, webs_t wp, int argc, char **argv);
extern int ej_get_permissions_of_account(int eid, webs_t wp, int argc, char **argv);
extern int ej_set_account_permission(int eid, webs_t wp, int argc, char **argv);
extern int ej_get_folder_tree(int eid, webs_t wp, int argc, char **argv);
extern int ej_get_share_tree(int eid, webs_t wp, int argc, char **argv);
extern int ej_initial_account(int eid, webs_t wp, int argc, char **argv);
extern int ej_create_account(int eid, webs_t wp, int argc, char **argv);
extern int ej_delete_account(int eid, webs_t wp, int argc, char **argv);
extern int ej_modify_account(int eid, webs_t wp, int argc, char **argv);
extern int ej_create_sharedfolder(int eid, webs_t wp, int argc, char **argv);
extern int ej_delete_sharedfolder(int eid, webs_t wp, int argc, char **argv);
extern int ej_modify_sharedfolder(int eid, webs_t wp, int argc, char **argv);
extern int ej_set_share_mode(int eid, webs_t wp, int argc, char **argv);
#endif

// aspbw.c
extern int f_exists(const char *path);
extern int f_wait_exists(const char *name, int max);
extern int do_f(const char *path, webs_t wp);
extern void char_to_ascii(char *output, uint8_t *input);

// cgi.c
extern void set_cgi(char *name, char *value);

// crc32.c
extern unsigned long crc32_sp (unsigned long, const unsigned char *, unsigned int);

// base64.c
extern int b64_decode( const char* str, unsigned char* space, int size );

// ej.c
extern int load_dictionary (char *lang, pkw_t pkw);
extern void release_dictionary (pkw_t pkw);
extern char *get_alert_msg_from_dict(const char *msg_id);

// tdate_parse.c
extern time_t tdate_parse(char *str);

// httpd.c
extern long uptime(void);
extern void fill_login_ip(char *p_out_ip, size_t out_ip_len);
extern const char *get_login_mac(void);
extern int get_login_safe(void);

// initial_web_hook.c
extern char *initial_disk_pool_mapping_info(void);
extern char *initial_blank_disk_names_and_sizes(void);
extern char *initial_available_disk_names_and_sizes(void);

// ralink.c
struct ifreq;
struct iwreq;
extern int get_apcli_peer_connected(const char *ifname, struct iwreq *p_wrq);
extern int get_apcli_wds_entry(const char *ifname, RT_802_11_MAC_ENTRY *pme);
extern int is_mac_in_sta_list(const unsigned char* p_mac);
extern int ej_lan_leases(int eid, webs_t wp, int argc, char **argv);
extern int ej_vpns_leases(int eid, webs_t wp, int argc, char **argv);
extern int ej_nat_table(int eid, webs_t wp, int argc, char **argv);
extern int ej_route_table(int eid, webs_t wp, int argc, char **argv);
extern int ej_conntrack_table(int eid, webs_t wp, int argc, char **argv);
extern int wl_ioctl(const char *ifname, int cmd, struct iwreq *pwrq);
extern int ej_wl_auth_list(int eid, webs_t wp, int argc, char **argv);
#if BOARD_HAS_5G_RADIO
extern int ej_wl_status_5g(int eid, webs_t wp, int argc, char **argv);
extern int ej_wl_scan_5g(int eid, webs_t wp, int argc, char **argv);
extern int ej_wl_bssid_5g(int eid, webs_t wp, int argc, char **argv);
#endif
extern int ej_wl_status_2g(int eid, webs_t wp, int argc, char **argv);
extern int ej_wl_scan_2g(int eid, webs_t wp, int argc, char **argv);
extern int ej_wl_bssid_2g(int eid, webs_t wp, int argc, char **argv);

// rtl8367.c or mtk_esw.c
extern int get_eth_port_bytes(int port_id_uapi, uint64_t *rx, uint64_t *tx);
extern int fill_eth_port_status(int port_id_uapi, char linkstate[40]);
extern int fill_eth_status(int port_id_uapi, webs_t wp);

// upload.c
extern void do_upgrade_fw_post(const char *url, FILE *stream, int clen, char *boundary);
extern void do_restore_nv_post(const char *url, FILE *stream, int clen, char *boundary);
extern void do_restore_st_post(const char *url, FILE *stream, int clen, char *boundary);

// web_ex.c
extern void nvram_commit_safe(void);
extern void do_uncgi_query(const char *query);
extern void do_cgi_clear(void);

#if defined (SUPPORT_HTTPS)
extern int ssl_server_init(char* ca_file, char *crt_file, char *key_file, char *dhp_file, char *ssl_cipher_list);
extern void ssl_server_uninit(void);
extern FILE *ssl_server_fopen(int sd);
extern const char* ssl_server_get_ssl_ver(void);
#endif

extern char log_header[];
#define httpd_log(fmt, args...) logmessage(log_header, fmt, ## args);


#endif /* _httpd_h_ */
