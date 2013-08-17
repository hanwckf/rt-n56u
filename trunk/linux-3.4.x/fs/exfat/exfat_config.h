/*
 *  Copyright (C) 2012-2013 Samsung Electronics Co., Ltd.
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version 2
 *  of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef _EXFAT_CONFIG_H
#define _EXFAT_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

#define OS_NONOS                1
#define OS_LINUX                2

#define FFS_CONFIG_OS           OS_LINUX

#define FFS_CONFIG_LITTLE_ENDIAN        1
#define FFS_CONFIG_LEGACY_32BIT_API     0
#define FFS_CONFIG_LEGACY_32BIT_API     0
#define FFS_CONFIG_SUPPORT_CP1250       1 
#define FFS_CONFIG_SUPPORT_CP1251       1
#define FFS_CONFIG_SUPPORT_CP1252       1
#define FFS_CONFIG_SUPPORT_CP1253       1
#define FFS_CONFIG_SUPPORT_CP1254       1
#define FFS_CONFIG_SUPPORT_CP1255       1
#define FFS_CONFIG_SUPPORT_CP1256       1
#define FFS_CONFIG_SUPPORT_CP1257       1
#define FFS_CONFIG_SUPPORT_CP1258       1
#define FFS_CONFIG_SUPPORT_CP874        1
#define FFS_CONFIG_SUPPORT_CP932        1
#define FFS_CONFIG_SUPPORT_CP936        1
#define FFS_CONFIG_SUPPORT_CP949        1
#define FFS_CONFIG_SUPPORT_CP950        1
#define FFS_CONFIG_SUPPORT_UTF8         1
#define EXFAT_CONFIG_DISCARD		1
#define EXFAT_CONFIG_KERNEL_DEBUG	0
#define EXFAT_CONFIG_DEBUG_MSG		0

#ifdef __cplusplus
}
#endif

#endif 
