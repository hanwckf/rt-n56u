
/************************************************************************
 *
 *  sysdefs.h
 *
 *  System definitions
 *
 * ######################################################################
 *
 * mips_start_of_header
 * 
 *  $Id: sysdefs.h,v 1.65 2010-03-12 19:44:16 douglas Exp $
 * 
 * Copyright (c) [Year(s)] MIPS Technologies, Inc. All rights reserved.
 *
 * Unpublished rights reserved under U.S. copyright law.
 *
 * PROPRIETARY/SECRET CONFIDENTIAL INFORMATION OF MIPS TECHNOLOGIES,
 * INC. FOR INTERNAL USE ONLY.
 *
 * Under no circumstances (contract or otherwise) may this information be
 * disclosed to, or copied, modified or used by anyone other than employees
 * or contractors of MIPS Technologies having a need to know.
 *
 * 
 * mips_end_of_header
 *
 ************************************************************************/


#ifndef SYSDEFS_H
#define SYSDEFS_H


/************************************************************************
 *  Include files
 ************************************************************************/

/************************************************************************
 *  Definitions
*************************************************************************/
#ifdef __ASSEMBLER__
#define _ASSEMBLER_
#endif

#ifdef __ghs__
#define _GHS_
#endif

#ifdef _ASSEMBLER_

/******** ASSEMBLER SPECIFIC DEFINITIONS ********/

#ifdef  __ghs__
#define ALIGN(x) .##align (1 << (x))
#else
#define ALIGN(x) .##align (x)
#endif

#ifdef __ghs__
#define SET_MIPS3()
#define SET_MIPS0()
#define SET_PUSH()
#define SET_POP()
#else
#define SET_MIPS3() .##set mips3
#define SET_MIPS0() .##set mips0
#define SET_PUSH()  .##set push
#define SET_POP()   .##set pop
#endif

/*  Different assemblers have different requirements for how to
 *  indicate that the next section is bss : 
 *
 *  Some use :   .bss
 *  Others use : .section bss
 *
 *  We select which to use based on _BSS_OLD_, which may be defined 
 *  in makefile.
 */
#ifdef _BSS_OLD_
#define BSS	.##section bss
#else
#define BSS	.##bss
#endif

#define SWAPEND32( src, tmp0, tmp1 )\
		and	tmp0, src, 0xff;\
		srl	src,  8;\
		sll	tmp0, 8;\
		and	tmp1, src, 0xff;\
		or	tmp0, tmp1;\
		srl	src,  8;\
		sll	tmp0, 8;\
		and	tmp1, src, 0xff;\
		or	tmp0, tmp1;\
		srl	src,  8;\
		sll	tmp0, 8;\
		or	src,  tmp0

#define LEAF(name)\
  		.##text;\
  		.##globl	name;\
  		.##ent	name;\
name:


#define SLEAF(name)\
  		.##text;\
  		.##ent	name;\
name:


#ifdef __ghs__
#define END(name)\
  		.##end	name
#else
#define END(name)\
  		.##size name,.-name;\
  		.##end	name
#endif


#define EXTERN(name)

# else /* not assembler */

/******** C specific definitions ********/

#define SWAPEND32(d)    ((BYTE(d,0) << 24) |\
		         (BYTE(d,1) << 16) |\
		         (BYTE(d,2) << 8)  |\
		         (BYTE(d,3) << 0))

#define SWAPEND64(d)    ((BYTE(d,0) << 56) |\
		         (BYTE(d,1) << 48) |\
		         (BYTE(d,2) << 40)  |\
		         (BYTE(d,3) << 32)  |\
			 (BYTE(d,4) << 24) |\
		         (BYTE(d,5) << 16) |\
		         (BYTE(d,6) << 8)  |\
		         (BYTE(d,7) << 0))

typedef unsigned char		UINT8;
typedef signed char		INT8;
typedef unsigned short		UINT16;
typedef signed short		INT16;
typedef unsigned int		UINT32;
typedef signed int		INT32;
typedef unsigned long long	UINT64;
typedef signed long long	INT64;
typedef UINT8			bool;

#ifndef _SIZE_T
#define _SIZE_T
#ifdef __ghs__
   typedef unsigned int size_t;
#else
   typedef unsigned long size_t;
#endif
#endif

#endif /* #ifdef _ASSEMBLER_ */



/******** DEFINITIONS FOR BOTH ASSEMBLER AND C ********/


#define FALSE		          0
#define TRUE			  (!FALSE)

#define NULL		          ((void *)0)
#define MIN(x,y)		  ((x) < (y) ? (x) : (y))
#define MAX(x,y)      		  ((x) > (y) ? (x) : (y))
 
#define INCWRAP( ptr, size )      (ptr) = ((ptr)+1) % (size)
#define DECWRAP( ptr, size )      (ptr) = (((ptr) == 0) ? ((size) - 1) : ((ptr) - 1))

#define MAXUINT(w)	(\
		((w) == sizeof(UINT8))  ? 0xFFU :\
		((w) == sizeof(UINT16)) ? 0xFFFFU :\
		((w) == sizeof(UINT32)) ? 0xFFFFFFFFU : 0\
		        )

#define MAXINT(w)	(\
		((w) == sizeof(INT8))  ? 0x7F :\
		((w) == sizeof(INT16)) ? 0x7FFF :\
		((w) == sizeof(INT32)) ? 0x7FFFFFFF : 0\
		        )

#define MSK(n)			  ((1 << (n)) - 1)

#define KUSEG_MSK		  0x80000000
#define KSEG_MSK		  0xE0000000
#define KUSEGBASE		  0x00000000
#define KSEG0BASE		  0x80000000
#define KSEG1BASE		  0xA0000000
#define KSSEGBASE		  0xC0000000
#define KSEG3BASE		  0xE0000000

/*  Below macros perform the following functions :
 *
 *  KSEG0    : Converts KSEG0/1 or physical addr (below 0.5GB) to KSEG0.
 *  KSEG1    : Converts KSEG0/1 or physical addr (below 0.5GB) to KSEG1.
 *  PHYS     : Converts KSEG0/1 or physical addr (below 0.5GB) to physical address.
 *  KSSEG    : Not relevant for converting, but used for determining range.
 *  KSEG3    : Not relevant for converting, but used for determining range.
 *  KUSEG    : Not relevant for converting, but used for determining range.
 *  KSEG0A   : Same as KSEG0 but operates on register rather than constant.
 *  KSEG1A   : Same as KSEG1 but operates on register rather than constant.
 *  PHYSA    : Same as PHYS  but operates on register rather than constant.
 *  CACHED   : Alias for KSEG0 macro .
 *	       (Note that KSEG0 cache attribute is determined by K0
 *	       field of Config register, but this is typically cached).
 *  UNCACHED : Alias for KSEG1 macro .
 */
#ifdef _ASSEMBLER_
#define KSEG0(addr)     (((addr) & ~KSEG_MSK)  | KSEG0BASE)
#define KSEG1(addr)     (((addr) & ~KSEG_MSK)  | KSEG1BASE)
#define KSSEG(addr)     (((addr) & ~KSEG_MSK)  | KSSEGBASE)
#define KSEG3(addr)     (((addr) & ~KSEG_MSK)  | KSEG3BASE)
#define KUSEG(addr)     (((addr) & ~KUSEG_MSK) | KUSEGBASE)
#define PHYS(addr)      ( (addr) & ~KSEG_MSK)
#define KSEG0A(reg) 	and reg, ~KSEG_MSK; or reg, KSEG0BASE
#define KSEG1A(reg) 	and reg, ~KSEG_MSK; or reg, KSEG1BASE
#define PHYSA(reg)	and reg, ~KSEG_MSK
#define KSEG0A2(d,s)	and d, s, ~KSEG_MSK; or d, KSEG0BASE
#define KSEG1A2(d,s)	and d, s, ~KSEG_MSK; or d, KSEG1BASE
#define PHYSA2(d,s)	and d, s, ~KSEG_MSK
#else
#define KSEG0(addr)     (((UINT32)(addr) & ~KSEG_MSK)  | KSEG0BASE)
#define KSEG1(addr)     (((UINT32)(addr) & ~KSEG_MSK)  | KSEG1BASE)
#define KSSEG(addr)	(((UINT32)(addr) & ~KSEG_MSK)  | KSSEGBASE)
#define KSEG3(addr)	(((UINT32)(addr) & ~KSEG_MSK)  | KSEG3BASE)
#define KUSEG(addr)	(((UINT32)(addr) & ~KUSEG_MSK) | KUSEGBASE)
#define PHYS(addr) 	((UINT32)(addr)  & ~KSEG_MSK)
#endif

#define CACHED(addr)	KSEG0(addr)
#define UNCACHED(addr)	KSEG1(addr)


#ifdef _ASSEMBLER_
/* Macroes to access variables at constant addresses 
 * Compensates for signed 16 bit displacement
 * Typical use:	li	a0, HIKSEG1(ATLAS_ASCIIWORD)
 *		sw	v1, LO_OFFS(ATLAS_ASCIIWORD)(a0)
 */
#define HIKSEG0(addr)	((KSEG0(addr) + 0x8000) & 0xffff0000)
#define HIKSEG1(addr)	((KSEG1(addr) + 0x8000) & 0xffff0000)
#define HI_PART(addr)	(((addr) + 0x8000) & 0xffff0000)
#define LO_OFFS(addr)	((addr) & 0xffff)
#endif


/* Most/Least significant 32 bit from 64 bit double word */
#define HI32(data64)		  ((UINT32)(data64 >> 32))
#define LO32(data64)		  ((UINT32)(data64 & 0xFFFFFFFF))

#define REG8( addr )		  (*(volatile UINT8 *) (addr))
#define REG16( addr )		  (*(volatile UINT16 *)(addr))
#define REG32( addr )		  (*(volatile UINT32 *)(addr))
#define REG64( addr )		  (*(volatile UINT64 *)(addr))


/* Register field mapping */
#define REGFIELD(reg, rfld)	  (((reg) & rfld##_MSK) >> rfld##_SHF)

/* absolute register address, access 					*/
#define	REGA(addr)		  REG32(addr)

/* physical register address, access: base address + offsett		*/
#define	REGP(base,phys)	REG32( (UINT32)(base) + (phys) )

/* relative register address, access: base address + offsett		*/
#define REG(base,offs)  REG32( (UINT32)(base) + offs##_##OFS )

/**************************************
 * Macroes not used by YAMON any more
 * (kept for backwards compatibility)
 */
/* register read field							*/
#define	REGARD(addr,fld)	((REGA(addr) & addr##_##fld##_##MSK) 	\
			 >> addr##_##fld##_##SHF)

/* register write numeric field value					*/
#define	REGAWRI(addr,fld,intval) ((REGA(addr) & ~(addr##_##fld##_##MSK))\
				 | ((intval) << addr##_##fld##_##SHF))

/* register write enumerated field value				*/
#define	REGAWRE(addr,fld,enumval) ((REGA(addr) & ~(addr##_##fld##_##MSK))\
			| ((addr##_##fld##_##enumval) << addr##_##fld##_##SHF))


/* Examples:
 * 
 *	exccode = REGARD(CPU_CAUSE,EXC);
 *
 *	REGA(SDR_CONTROL) = REGAWRI(OSG_CONTROL,TMO,17)
 *			 | REGAWRE(OSG_CONTROL,DTYPE,PC1);
 */


/* register read field							*/
#define	REGRD(base,offs,fld) ((REG(base,offs) & offs##_##fld##_##MSK) 	\
			 >> offs##_##fld##_##SHF)

/* register write numeric field value					*/
#define	REGWRI(base,offs,fld,intval)((REG(base,offs)& ~(offs##_##fld##_##MSK))\
                                 | (((intval) << offs##_##fld##_##SHF) & offs##_##fld##_##MSK))

/* register write enumerated field value				*/
#define	REGWRE(base,offs,fld,enumval)((REG(base,offs) & ~(offs##_##fld##_##MSK))\
				| ((offs##_##fld##_##enumval) << offs##_##fld##_##SHF))


/* physical register read field							*/
#define	REGPRD(base,phys,fld) ((REGP(base,phys) & phys##_##fld##_##MSK) 	\
			 >> phys##_##fld##_##SHF)

/* physical register write numeric field value					*/
#define	REGPWRI(base,phys,fld,intval)((REGP(base,phys)& ~(phys##_##fld##_##MSK))\
				 | ((intval) << phys##_##fld##_##SHF))

/* physical register write enumerated field value				*/
#define	REGPWRE(base,phys,fld,enumval)((REGP(base,phys) & ~(phys##_##fld##_##MSK))\
				| ((phys##_##fld##_##enumval) << phys##_##fld##_##SHF))
/*
 * End of macroes not used by YAMON any more
 *********************************************/

/* Endian related macros */

#define SWAP_BYTEADDR32( addr )   ( (addr) ^ 0x3 )
#define SWAP_UINT16ADDR32( addr ) ( (addr) ^ 0x2 )

#define BYTE(d,n)		  (((d) >> ((n) << 3)) & 0xFF)

/* Set byte address to little endian format */
#ifdef EL
#define SWAP_BYTEADDR_EL(addr) 	  addr
#else
#define SWAP_BYTEADDR_EL(addr)    SWAP_BYTEADDR32( addr )
#endif

/* Set byte address to big endian format */
#ifdef EB
#define SWAP_BYTEADDR_EB(addr) 	  addr
#else
#define SWAP_BYTEADDR_EB(addr)    SWAP_BYTEADDR32( addr )
#endif

/* Set UINT16 address to little endian format */
#ifdef EL
#define SWAP_UINT16ADDR_EL(addr)  addr
#else
#define SWAP_UINT16ADDR_EL(addr)  SWAP_UINT16ADDR32( addr )
#endif

/* Set UINT16 address to big endian format */
#ifdef EB
#define SWAP_UINT16ADDR_EB(addr)  addr
#else
#define SWAP_UINT16ADDR_EB(addr)  SWAP_UINT16ADDR32( addr )
#endif

#ifdef EL
#define REGW32LE(addr, data)      REG32(addr) = (data)
#define REGR32LE(addr, data)      (data)      = REG32(addr)
#else
#define REGW32LE(addr, data)      REG32(addr) = SWAPEND32(data)
#define REGR32LE(addr, data)      (data)      = REG32(addr), (data) = SWAPEND32(data)
#endif

/* Set of 'LE'-macros, convert by BE: */
#ifdef EL
#define CPU_TO_LE32( value ) (value)
#define LE32_TO_CPU( value ) (value)

#define CPU_TO_LE16( value ) (value)
#define LE16_TO_CPU( value ) (value)
#else
#define CPU_TO_LE32( value ) ( (                ((UINT32)value)  << 24) |   \
                               ((0x0000FF00UL & ((UINT32)value)) <<  8) |   \
                               ((0x00FF0000UL & ((UINT32)value)) >>  8) |   \
                               (                ((UINT32)value)  >> 24)   )
#define LE32_TO_CPU( value ) CPU_TO_LE32( value )

#define CPU_TO_LE16( value ) ( ((UINT16)(((UINT16)value)  << 8)) |   \
                               ((UINT16)(((UINT16)value)  >> 8))   )
#define LE16_TO_CPU( value ) CPU_TO_LE16( value )
#endif

/* Set of 'BE'-macros, convert by LE: */
#ifdef EB
#define CPU_TO_BE32( value ) (value)
#define BE32_TO_CPU( value ) (value)

#define CPU_TO_BE16( value ) (value)
#define BE16_TO_CPU( value ) (value)
#else
#define CPU_TO_BE32( value ) ( (                ((UINT32)value)  << 24) |   \
                               ((0x0000FF00UL & ((UINT32)value)) <<  8) |   \
                               ((0x00FF0000UL & ((UINT32)value)) >>  8) |   \
                               (                ((UINT32)value)  >> 24)   )
#define BE32_TO_CPU( value ) CPU_TO_BE32( value )

#define CPU_TO_BE16( value ) ( ((UINT16)(((UINT16)value)  << 8)) |   \
                               ((UINT16)(((UINT16)value)  >> 8))   )
#define BE16_TO_CPU( value ) CPU_TO_BE16( value )
#endif

#ifdef _ASSEMBLER_
EXTERN(default_port)
EXTERN(debug_port)
EXTERN(default_gdb_port)
#else
extern UINT8 default_port;
extern UINT8 debug_port;
extern UINT8 default_gdb_port;
#endif

/* Standard ports */
#define PORT_TTY0	 0
#define PORT_TTY1	 1
#define PORT_NET	 2
#define DEFAULT_PORT	 default_port
#define DEBUG_PORT	 degug_port
#define DEFAULT_GDB_PORT default_gdb_port

/* Pick either TTY1 or TTY0 as the default port */
#define DEFAULT_TTY_PORT       PORT_TTY0

/* Env. variable for default serial port used for load */
#define DEFAULT_BOOTPORT_ENV   "tty0"

/* Control characters */
#define CTRL_A          ('A'-0x40)
#define CTRL_B          ('B'-0x40)
#define CTRL_C          ('C'-0x40)
#define CTRL_D          ('D'-0x40)
#define CTRL_E          ('E'-0x40)
#define CTRL_F          ('F'-0x40)
#define CTRL_H          ('H'-0x40)
#define CTRL_K          ('K'-0x40)
#define CTRL_N          ('N'-0x40)
#define CTRL_P          ('P'-0x40)
#define CTRL_U          ('U'-0x40)
#define DEL             0x7F
#define TAB             0x09
#define CR              0x0D
#define LF              0x0A
#define ESC             0x1B
#define SP              0x20
#define CSI             0x9B
#define EOF_SREC        -2
#define UART_ERROR      -1


/* DEF2STR(x) converts #define symbol to string */
#define DEF2STR1(x) #x
#define DEF2STR(x)  DEF2STR1(x)


#endif /* #ifndef SYSDEFS_H */

