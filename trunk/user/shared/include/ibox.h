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

#ifndef _IBOX_H_
#define _IBOX_H_

#define BYTE uint8_t
#define WORD uint16_t
#define DWORD uint32_t

#define NET_SERVICE_ID_BASE		10 // 0x0A
#define NET_SERVICE_ID_LPT_EMU		NET_SERVICE_ID_BASE + 1
#define NET_SERVICE_ID_IBOX_INFO	NET_SERVICE_ID_BASE + 2

#define NET_PACKET_TYPE_BASE		20 // 0x14
#define NET_PACKET_TYPE_CMD		NET_PACKET_TYPE_BASE + 1
#define NET_PACKET_TYPE_RES		NET_PACKET_TYPE_BASE + 2

#define IBOX_SRV_PORT			9999
#define IBOX_CLI_PORT			9999

enum NET_CMD_ID {			// Dec	Hex
	NET_CMD_ID_BASE = 30,		// 30	0x1E
	NET_CMD_ID_GETINFO,		// 31	0x1F
	NET_CMD_ID_GETINFO_EX,		// 32	0x20
	NET_CMD_ID_GETINFO_SITES,	// 33	0x21
	NET_CMD_ID_SETINFO,		// 34	0x22
	NET_CMD_ID_SETSYSTEM,		// 35	0x23
	NET_CMD_ID_GETINFO_PROF,	// 36	0x24
	NET_CMD_ID_SETINFO_PROF,	// 37	0x25
	NET_CMD_ID_CHECK_PASS,		// 38	0x26
	NET_CMD_ID_SETKEY_EX,		// 39	0x27
	NET_CMD_ID_QUICKGW_EX,		// 40	0x28
	NET_CMD_ID_EZPROBE,		// 41	0x29
	NET_CMD_ID_MANU_BASE=50,	// 50	0x32
	NET_CMD_ID_MANU_CMD,		// 51	0x33
	NET_CMD_ID_GETINFO_MANU,	// 52	0x34
	NET_CMD_ID_GETINFO_EX2,		// 53	0x35
	NET_CMD_ID_MAXIMUM
};

enum  NET_RES_OP {
	NET_RES_OK = 0x0000,
	NET_RES_ERR_PASSWORD = 0x0100,
	NET_RES_ERR_FIELD_UNDEF = 0x0200
};

typedef struct iboxPKT {
	BYTE ServiceID;
	BYTE PacketType;
	WORD OpCode;
	DWORD Info; // Or Transaction ID
} IBOX_COMM_PKT_HDR;

typedef struct iboxPKTRes {
	BYTE ServiceID;
	BYTE PacketType;
	WORD OpCode;
	DWORD Info; // Or Transaction ID
} IBOX_COMM_PKT_RES;

typedef struct iboxPKTEx {
	BYTE ServiceID;
	BYTE PacketType;
	WORD OpCode;
	DWORD Info; // Or Transaction ID
	BYTE MacAddress[6];
	BYTE Password[32];   //NULL terminated string, string length:1~31, cannot be NULL string
} IBOX_COMM_PKT_HDR_EX;

typedef struct iboxPKTExRes {
	BYTE ServiceID;
	BYTE PacketType;
	WORD OpCode;
	DWORD Info; // Or Transaction ID
	BYTE MacAddress[6];
} IBOX_COMM_PKT_RES_EX;

typedef struct PktGetInfo
{
	BYTE PrinterInfo[128];
	BYTE SSID[32];
	BYTE NetMask[32];
	BYTE ProductID[32];
	BYTE FirmwareVersion[16];
	BYTE OperationMode;
	BYTE MacAddress[6];
	BYTE Regulation;
} PKT_GET_INFO;

enum  OPERATION_MODE
{
	OPERATION_MODE_WB=0x00,
	OPERATION_MODE_AP,
	OPERATION_MODE_GATEWAY,
	OPERATION_MODE_ROUTER,
	OPERATION_MODE_PAIRING
};

#endif
