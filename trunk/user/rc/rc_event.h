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
#ifndef _rc_event_h_
#define _rc_event_h_

#define EVT_MEM_OUT		301
#define USB_PLUG_ON		10
#define USB_PLUG_OFF		11
#define USB_PRT_PLUG_ON		12
#define USB_PRT_PLUG_OFF	13
#define USB_SERIAL_PLUG_ON      14
#define USB_SERIAL_PLUG_OFF     15
#define USB_3G_PLUG_ON          21
#define USB_3G_PLUG_OFF         22
#define USB_HUB_RE_ENABLE       30

int event_code;

#endif
