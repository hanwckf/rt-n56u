/* C6X DSBT ELF shared library loader suppport.
 *
 * Copyright (C) 2010 Texas Instruments Incorporated
 * Contributed by Mark Salter <msalter@redhat.com>
 *
 * All rights reserved.
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

static const char * const _dl_reltypes_tab[] =
{
  	"R_C6000_NONE",			/* 0 */
	"R_C6000_ABS32",
	"R_C6000_ABS16",
	"R_C6000_ABS8",
	"R_C6000_PCR_S21",
	"R_C6000_PCR_S12",		/* 5 */
	"R_C6000_PCR_S10",
	"R_C6000_PCR_S7",
	"R_C6000_ABS_S16",
	"R_C6000_ABS_L16",
	"R_C6000_ABS_H16",		/* 10 */
	"R_C6000_SBR_U15_B",
	"R_C6000_SBR_U15_H",
	"R_C6000_SBR_U15_W",
	"R_C6000_SBR_S16",
	"R_C6000_SBR_L16_B",		/* 15 */
	"R_C6000_SBR_L16_H",
	"R_C6000_SBR_L16_W",
	"R_C6000_SBR_H16_B",
	"R_C6000_SBR_H16_H",
	"R_C6000_SBR_H16_W",		/* 20 */
	"R_C6000_SBR_GOT_U15_W",
	"R_C6000_SBR_GOT_L16_W",
	"R_C6000_SBR_GOT_H16_W",
	"R_C6000_DSBT_INDEX",
	"R_C6000_PREL31",		/* 25 */
	"R_C6000_COPY",
	"R_C6000_JUMP_SLOT",
	"R_C6000_SBR_GOT32",
	"R_C6000_PCR_H16",
	"R_C6000_PCR_L16",		/* 30 */
#if 0
	"R_C6000_ALIGN",		/* 253 */
	"R_C6000_FPHEAD",		/* 254 */
	"R_C6000_NOCMP",		/* 255 */
#endif
};
