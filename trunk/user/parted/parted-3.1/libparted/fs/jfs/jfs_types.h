/*
 *   Copyright (c) International Business Machines  Corp., 2000
 *
 *   This program is free software;  you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY;  without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
 *   the GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _H_JFS_TYPES
#define	_H_JFS_TYPES

/*
 *	jfs_types.h:
 *
 * basic type/utility  definitions
 *
 * note: this header file must be the 1st include file
 * of JFS include list in all JFS .c file.
 */

#ifdef _JFS_UTILITY
/* this is defined in asm/byteorder.h for i386, but
 * is NOT defined in asm/byteorder.h for ppc (non-kernel).
 * Until that is changed, we'll define it here.    */
#define __BYTEORDER_HAS_U64__

#include <sys/types.h>
//#include <asm/byteorder.h>
typedef unsigned short UniChar;
#else
#include <linux/types.h>
#include <linux/jfs_fs.h>
#include <linux/nls.h>

#ifndef _ULS_UNICHAR_DEFINED
#if (LINUX_VERSION_CODE > KERNEL_VERSION(2,3,0))
typedef wchar_t UniChar;
#else
typedef unsigned short UniChar;
#endif
#define _ULS_UNICHAR_DEFINED
#endif
#endif
/* #include "endian24.h" */

/*
 *	primitive types
 */
#ifdef _JFS_UTILITY
typedef int8_t s8;
typedef uint8_t u8;
typedef int16_t s16;
typedef uint16_t u16;
typedef int32_t s32;
typedef uint32_t u32;
typedef int64_t s64;
typedef uint64_t u64;

#ifndef _UINT_TYPES
	/*      unicode includes also define these */
typedef u16 uint16;
typedef u32 uint32;
#define _UINT_TYPES
#endif

typedef s8 int8;
typedef u8 uint8;
typedef s16 int16;
typedef s32 int32;
typedef s64 int64;
typedef u64 uint64;

#endif				/* _JFS_UTILITY */
/*
 * Holdovers from OS/2.  Try to get away from using these altogether.
 */
typedef unsigned long ULONG;
typedef unsigned short USHORT;
typedef unsigned char UCHAR;
typedef void *PVOID;
#define MAXPATHLEN      255


/*
 * Almost identical to Linux's timespec, but not quite
 */
struct timestruc_t {
	u32 tv_sec;
	u32 tv_nsec;
};

/*
 *	handy
 */
#undef MIN
#define	MIN(a,b) (((a)<(b))?(a):(b))
#undef MAX
#define	MAX(a,b) (((a)>(b))?(a):(b))
#undef ROUNDUP
#define	ROUNDUP(x, y)	( ((x) + ((y) - 1)) & ~((y) - 1) )

#define LEFTMOSTONE	0x80000000
#define	HIGHORDER	0x80000000u	/* high order bit on            */
#define	ONES		0xffffffffu	/* all bit on                   */

typedef int boolean_t;
#define TRUE 1
#define FALSE 0

/*
 *	logical xd (lxd)
 */
typedef struct {
	unsigned len:24;
	unsigned off1:8;
	u32 off2;
} lxd_t;

/* lxd_t field construction */
#define	LXDlength(lxd, length32)	( (lxd)->len = length32 )
#define	LXDoffset(lxd, offset64)\
{\
	(lxd)->off1 = ((s64)offset64) >> 32;\
	(lxd)->off2 = (offset64) & 0xffffffff;\
}

/* lxd_t field extraction */
#define	lengthLXD(lxd)	( (lxd)->len )
#define	offsetLXD(lxd)\
	( ((s64)((lxd)->off1)) << 32 | (lxd)->off2 )

/* lxd list */
typedef struct {
	s16 maxnlxd;
	s16 nlxd;
	lxd_t *lxd;
} lxdlist_t;

/*
 *	physical xd (pxd)
 */
typedef struct {
	unsigned len:24;
	unsigned addr1:8;
	u32 addr2;
} pxd_t;

/* xd_t field construction */

#define	PXDlength(pxd, length32)	((pxd)->len = __cpu_to_le24(length32))
#define	PXDaddress(pxd, address64)\
{\
	(pxd)->addr1 = ((s64)address64) >> 32;\
	(pxd)->addr2 = __cpu_to_le32((address64) & 0xffffffff);\
}

/* xd_t field extraction */
#define	lengthPXD(pxd)	__le24_to_cpu((pxd)->len)
#define	addressPXD(pxd)\
	( ((s64)((pxd)->addr1)) << 32 | __le32_to_cpu((pxd)->addr2))

/* pxd list */
typedef struct {
	s16 maxnpxd;
	s16 npxd;
	pxd_t pxd[8];
} pxdlist_t;


/*
 *	data extent descriptor (dxd)
 */
typedef struct {
	unsigned flag:8;	/* 1: flags */
	unsigned rsrvd:24;	/* 3: */
	u32 size;		/* 4: size in byte */
	unsigned len:24;	/* 3: length in unit of fsblksize */
	unsigned addr1:8;	/* 1: address in unit of fsblksize */
	u32 addr2;		/* 4: address in unit of fsblksize */
} dxd_t;			/* - 16 - */

/* dxd_t flags */
#define	DXD_INDEX	0x80	/* B+-tree index */
#define	DXD_INLINE	0x40	/* in-line data extent */
#define	DXD_EXTENT	0x20	/* out-of-line single extent */
#define	DXD_FILE	0x10	/* out-of-line file (inode) */
#define DXD_CORRUPT	0x08	/* Inconsistency detected */

/* dxd_t field construction
 *	Conveniently, the PXD macros work for DXD
 */
#define	DXDlength	PXDlength
#define	DXDaddress	PXDaddress
#define	lengthDXD	lengthPXD
#define	addressDXD	addressPXD

/*
 *      directory entry argument
 */
typedef struct component_name {
	int namlen;
	UniChar *name;
} component_t;


/*
 *	DASD limit information - stored in directory inode
 */
typedef struct dasd {
	u8 thresh;		/* Alert Threshold (in percent) */
	u8 delta;		/* Alert Threshold delta (in percent)   */
	u8 rsrvd1;
	u8 limit_hi;		/* DASD limit (in logical blocks)       */
	u32 limit_lo;		/* DASD limit (in logical blocks)       */
	u8 rsrvd2[3];
	u8 used_hi;		/* DASD usage (in logical blocks)       */
	u32 used_lo;		/* DASD usage (in logical blocks)       */
} dasd_t;

#define DASDLIMIT(dasdp) \
	(((u64)((dasdp)->limit_hi) << 32) + __le32_to_cpu((dasdp)->limit_lo))
#define setDASDLIMIT(dasdp, limit)\
{\
	(dasdp)->limit_hi = ((u64)limit) >> 32;\
	(dasdp)->limit_lo = __cpu_to_le32(limit);\
}
#define DASDUSED(dasdp) \
	(((u64)((dasdp)->used_hi) << 32) + __le32_to_cpu((dasdp)->used_lo))
#define setDASDUSED(dasdp, used)\
{\
	(dasdp)->used_hi = ((u64)used) >> 32;\
	(dasdp)->used_lo = __cpu_to_le32(used);\
}

/*
 *		circular doubly-linked list (cdll)
 *
 * A circular doubly-linked list (cdll) is anchored by a pair of pointers,
 * one to the head of the list and the other to the tail of the list.
 * The elements are doubly linked so that an arbitrary element can be
 * removed without a need to traverse the list.
 * New elements can be added to the list before or after an existing element,
 * at the head of the list, or at the tail of the list.
 * A circle queue may be traversed in either direction.
 *
 * +----------+        +-------------------------------------+
 * |          |        |                                     |
 * +->+-----+ |        +->+-----+  +->+-----+    +->+-----+  |
 * |  |  h  +-+        |  |  h  +--+  |  n  +----+  |  n  +--+
 * |  +-----+          |  +-----+  |  +-----+    |  +-----+
 * |  |  t  +-+     +-----+  t  |  |  |  p  +--+ |  |  p  +--+
 * |  +-----+ |     |  |  +-----+  |  +-----+  | |  +-----+  |
 * +----------+     |  +-----------------------+ |           |
 *                  |              |             |           |
 *                  |              +-------------------------+
 *                  |                            |
 *                  +----------------------------+
 */
/*
 *	define header
 *
 * list header field definition in header element:
 *
 * type - type of list element struct embedding the link field
 */
#define CDLL_HEADER(type)\
struct {\
	struct type *head;\
	struct type *tail;\
}

struct cdll_header {
	struct cdll_header *head;
	struct cdll_header *tail;
};

/*
 *	define link
 *
 * list link field definition in list element:
 *
 * type - type of parent list element struct embedding the link field
 */
#define CDLL_ENTRY(type)\
struct {\
	struct type *next;\
	struct type *prev;\
}

struct cdll_entry {
	struct cdll_entry *next;
	struct cdll_entry *prev;
};

/*
 *	initialize header
 *
 * header - ptr to the header field in the header element
 */
#define	CDLL_INIT(header) {\
	(header)->head = (void *)(header);\
	(header)->tail = (void *)(header);\
}

/*
 *	scan list
 *
 * header - ptr to the header field in the header element
 * elm - ptr to the element to be inserted
 * field - name of the link field in the list element
 *
 * struct header_container	*container;
 * struct header_type	*header;
 * struct element_type	*elm;
 *
 * header = &container->header_field;
 * for (elm = header->head; elm != (void *)header; elm = elm->field.next)
 */

/*
 *	insert <elm> at head of list anchored at <header>
 *
 * header - ptr to the header field in the header element
 * elm - ptr to the list element to be inserted
 * field - name of the link field in the list element
 */
#define CDLL_INSERT_HEAD(header, elm, field) {\
	(elm)->field.next = (header)->head;\
	(elm)->field.prev = (void *)(header);\
	if ((header)->tail == (void *)(header))\
		(header)->tail = (elm);\
	else\
		(header)->head->field.prev = (elm);\
	(header)->head = (elm);\
}

/*
 *	insert <elm> at tail of list anchored at <header>
 *
 * header - ptr to the header field in the header element
 * elm - ptr to the list element to be inserted
 * field - name of the link field in the list element
 */
#define CDLL_INSERT_TAIL(header, elm, field) {\
	(elm)->field.next = (void *)(header);\
	(elm)->field.prev = (header)->tail;\
	if ((header)->head == (void *)(header))\
		(header)->head = (elm);\
	else\
		(header)->tail->field.next = (elm);\
	(header)->tail = (elm);\
}

/*
 *	insert <elm> after <listelm> of list anchored at <header>
 *
 * header - ptr to the header field in the header element
 * listelm - ptr to the list element at insertion point
 * elm - ptr to the list element to be inserted
 * field - name of the link field in the list element
 */
#define CDLL_INSERT_AFTER(header, listelm, elm, field) {\
	(elm)->field.next = (listelm)->field.next;\
	(elm)->field.prev = (listelm);\
	if ((listelm)->field.next == (void *)(header))\
		(header)->tail = (elm);\
	else\
		(listelm)->field.next->field.prev = (elm);\
	(listelm)->field.next = (elm);\
}

/*
 *	insert <elm> before <listelm> of list anchored at <header>
 *
 * header - ptr to the header field in the header element
 * listelm - ptr to list element at insertion point
 * elm - ptr to the element to be inserted
 * field - name of the link field in the list element
 */
#define CDLL_INSERT_BEFORE(header, listelm, elm, field) {\
	(elm)->field.next = (listelm);\
	(elm)->field.prev = (listelm)->field.prev;\
	if ((listelm)->field.prev == (void *)(header))\
		(header)->head = (elm);\
	else\
		(listelm)->field.prev->field.next = (elm);\
	(listelm)->field.prev = (elm);\
}

/*
 *	remove <elm> from list anchored at <header>
 *
 * header - ptr to the header field in the header element
 * elm - ptr to the list element to be removed
 * field - name of the link field in the list element
 */
#define	CDLL_REMOVE(header, elm, field) {\
	if ((elm)->field.next == (void *)(header))\
		(header)->tail = (elm)->field.prev;\
	else\
		(elm)->field.next->field.prev = (elm)->field.prev;\
	if ((elm)->field.prev == (void *)(header))\
		(header)->head = (elm)->field.next;\
	else\
		(elm)->field.prev->field.next = (elm)->field.next;\
}

#define CDLL_MOVE_TO_HEAD(header, elm, field) {\
	if ((elm)->field.prev != (void *)(header))\
	{\
		if ((elm)->field.next == (void *)(header))\
			(header)->tail = (elm)->field.prev;\
		else\
			(elm)->field.next->field.prev = (elm)->field.prev;\
		(elm)->field.prev->field.next = (elm)->field.next;\
		(elm)->field.next = (header)->head;\
		(elm)->field.prev = (void *)(header);\
		(header)->head->field.prev = (elm);\
		(header)->head = (elm);\
	}\
}

#define CDLL_MOVE_TO_TAIL(header, elm, field) {\
	if ((elm)->field.next != (void *)(header))\
	{\
		(elm)->field.next->field.prev = (elm)->field.prev;\
		if ((elm)->field.prev == (void *)(header))\
			(header)->head = (elm)->field.next;\
		else\
			(elm)->field.prev->field.next = (elm)->field.next;\
		(elm)->field.next = (void *)(header);\
		(elm)->field.prev = (header)->tail;\
		(header)->tail->field.next = (elm);\
		(header)->tail = (elm);\
	}\
}

/*
 *	orphan list element
 */
#define	CDLL_SELF(elm, field)\
	(elm)->field.next = (elm)->field.prev = (elm);


/*
 *		single head doubly-linked list
 *
 * A list is headed by a single head pointer.
 * The elements are doubly linked so that an arbitrary element can be
 * removed without a need to traverse the list.
 * New elements can be added to the list at the head of the list, or
 * after an existing element (NO insert at tail).
 * A list may only be traversed in the forward direction.
 * (note: the list is NULL terminated in next field.)
 *
 *   +-----+          +->+-----+  +->+-----+    +->+-----+
 *   | NULL|          |  |  h  +--+  |  n  +----+  | NULL|
 *   +-----+          |  +-----+  |  +-----+       +-----+
 *                    |           |  |  p  +--+    |  p  +--+
 *                    |           |  +-----+  |    +-----+  |
 *                    +-----------------------+             |
 *                                |                         |
 *                                +-------------------------+
 */
#define LIST_HEADER(type)\
struct {\
	struct type *head;\
}

#define LIST_ENTRY(type)\
struct {\
	struct type *next;\
	struct type **prev;\
}

#define	LIST_INIT(header)	{ (header)->head = NULL; }

/*
 *	scan list
 *
 * header - ptr to the header (field in header element)
 * elm - ptr to the element to be inserted
 * field - name of the link field in list element
 *
 * struct header_container	*container;
 * struct header_type	*header;
 * struct element_type	*elm;
 *
 * header = &container->header_field;
 * for (elm = header->head; elm; elm = elm->field.next)
 */

#define LIST_INSERT_HEAD(header, elm, field) {\
	if (((elm)->field.next = (header)->head) != NULL)\
		(header)->head->field.prev = &(elm)->field.next;\
	(header)->head = (elm);\
	(elm)->field.prev = &(header)->head;\
}

#define LIST_INSERT_AFTER(listelm, elm, field) {\
	if (((elm)->field.next = (listelm)->field.next) != NULL)\
		(listelm)->field.next->field.prev = &(elm)->field.next;\
	(listelm)->field.next = (elm);\
	(elm)->field.prev = &(listelm)->field.next;\
}

#define LIST_REMOVE(elm, field) {\
	if ((elm)->field.next != NULL)\
		(elm)->field.next->field.prev = (elm)->field.prev;\
	*(elm)->field.prev = (elm)->field.next;\
}

#define LIST_SELF(elm, field) {\
	(elm)->field.next = NULL;\
	(elm)->field.prev = &(elm)->field.next;\
}

#endif				/* !_H_JFS_TYPES */
