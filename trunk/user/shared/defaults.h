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

#ifndef _defaults_h_
#define _defaults_h_

#include <ralink_board.h>

#define SYS_SHELL		"/bin/sh"
#define SYS_EXEC_PATH		"/usr/sbin:/usr/bin:/sbin:/bin"
#define SYS_EXEC_PATH_OPT	"/opt/sbin:/opt/bin:/usr/sbin:/usr/bin:/sbin:/bin"
#define SYS_HOME_PATH_ROOT	"/home/admin"

#define SYS_USER_ROOT		"admin"
#define SYS_GROUP_ROOT		"root"
#define SYS_USER_NOBODY		"nobody"
#define SYS_GROUP_NOGROUP	"nogroup"

#define DEF_LAN_ADDR		"192.168.2.1"
#define DEF_LAN_DHCP_BEG	"192.168.2.100"
#define DEF_LAN_DHCP_END	"192.168.2.244"
#define DEF_LAN_MASK		"255.255.255.0"

#define DEF_WLAN_2G_CC		"CN"
#define DEF_WLAN_5G_CC		"US"
#define DEF_WLAN_2G_SSID	BOARD_PID "_%s"
#define DEF_WLAN_5G_SSID	BOARD_PID "_5G_%s"
#define DEF_WLAN_2G_GSSID	BOARD_PID "_GUEST_%s"
#define DEF_WLAN_5G_GSSID	BOARD_PID "_GUEST_5G_%s"
#define DEF_WLAN_2G_PSK		"1234567890"
#define DEF_WLAN_5G_PSK		"1234567890"

#define DEF_ROOT_PASSWORD	"admin"
#define DEF_SMB_WORKGROUP	"WORKGROUP"
#define DEF_TIMEZONE		"CST-8"
#define DEF_NTP_SERVER0		"ntp1.aliyun.com"
#define DEF_NTP_SERVER1		"2001:470:0:50::2"
#ifdef SUPPORT_OPENSSL_EC
#define DEF_HTTPS_CIPH_LIST	"ECDH+CHACHA20:ECDH+AESGCM:DH+AESGCM:DH+AES256:DH+AES:DH+3DES:RSA+AES:RSA+3DES:!ADH:!MD5:!DSS"
#else
#define DEF_HTTPS_CIPH_LIST	"DH+AESGCM:DH+AES256:DH+AES:DH+3DES:RSA+AES:RSA+3DES:!ADH:!MD5:!DSS"
#endif
#endif
