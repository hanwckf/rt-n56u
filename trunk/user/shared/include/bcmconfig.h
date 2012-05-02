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
/*
 * Automatically generated C config: don't edit
 */

/*
 * Base Features
 */
#define __CONFIG_RC__ 1
#define __CONFIG_NVRAM__ 1
#define __CONFIG_SHARED__ 1
#define __CONFIG_BUSYBOX__ 1
#define __CONFIG_BUSYBOX_CONFIG__ "router"
#define __CONFIG_WLCONF__ 1
#define __CONFIG_BRIDGE__ 1
#define __CONFIG_HTTPD__ 1
#define __CONFIG_WWW__ 1
#define __CONFIG_NETCONF__ 1
#undef __CONFIG_IPTABLES__
#define __CONFIG_LIBIPT__ 1
#define __CONFIG_GLIBC__ 1
#undef __CONFIG_UCLIBC__
#define __CONFIG_LIBOPT__ 1

/*
 * Options
 */
#define __CONFIG_VENDOR__ "broadcom"
#define __CONFIG_UDHCPD__ 1
#define __CONFIG_PPP__ 1
#define __CONFIG_UPNP__ 1
#define __CONFIG_NAS__ 1
#define __CONFIG_NTP__ 1
#define __CONFIG_DNSMASQ__ 1
#define __CONFIG_UTILS__ 1

/*
 * Additional C libraries
 */
#undef __CONFIG_LIBCRYPT__
#undef __CONFIG_LIBDL__
#undef __CONFIG_LIBM__
#undef __CONFIG_LIBNSL__
#undef __CONFIG_LIBPTHREAD__
#undef __CONFIG_LIBRESOLV__
#undef __CONFIG_LIBUTIL__

/*
 * Environment
 */
#define __PLATFORM__ "mipsel"
#define __LINUXDIR__ "$(SRCBASE)/linux/linux"
#define __LIBDIR__ "$(TOOLCHAIN)/lib"
#define __USRLIBDIR__ "$(TOOLCHAIN)/usr/lib"
