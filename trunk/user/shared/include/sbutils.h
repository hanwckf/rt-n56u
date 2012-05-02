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
 * Misc utility routines for accessing chip-specific features
 * of Broadcom HNBU SiliconBackplane-based chips.
 *
 * Copyright(c) 2001 ASUSTeK Inc.
 * $Id: sbutils.h,v 1.1 2007/06/08 10:20:42 arthur Exp $
 */

#ifndef	_sbutils_h_
#define	_sbutils_h_

/* Board styles (bustype) */
#define	BOARDSTYLE_SOC		0		/* Silicon Backplane */
#define	BOARDSTYLE_PCI		1		/* PCI/MiniPCI board */
#define	BOARDSTYLE_PCMCIA	2		/* PCMCIA board */
#define	BOARDSTYLE_CARDBUS	3		/* Cardbus board */

/*
 * Many of the routines below take an 'sbh' handle as their first arg.
 * Allocate this by calling sb_attach().  Free it by calling sb_detach().
 * At any one time, the sbh is logically focused on one particular sb core
 * (the "current core").
 * Use sb_setcore() or sb_setcoreidx() to change the association to another core.
 */

/* exported externs */
extern void *sb_attach(uint pcidev, void *osh, void *regs, uint bustype, char **vars, int *varsz);
extern void *sb_kattach(void);
extern void sb_detach(void *sbh);
extern uint sb_chip(void *sbh);
extern uint sb_chiprev(void *sbh);
extern uint sb_boardvendor(void *sbh);
extern uint sb_boardtype(void *sbh);
extern uint sb_boardstyle(void *sbh);
extern uint sb_bus(void *sbh);
extern uint sb_corelist(void *sbh, uint coreid[]);
extern uint sb_coreid(void *sbh);
extern uint sb_coreidx(void *sbh);
extern uint sb_coreunit(void *sbh);
extern uint sb_corevendor(void *sbh);
extern uint sb_corerev(void *sbh);
extern void *sb_coreregs(void *sbh);
extern uint32 sb_coreflags(void *sbh, uint32 mask, uint32 val);
extern uint32 sb_coreflagshi(void *sbh, uint32 mask, uint32 val);
extern bool sb_iscoreup(void *sbh);
extern void *sb_setcoreidx(void *sbh, uint coreidx);
extern void *sb_setcore(void *sbh, uint coreid, uint coreunit);
extern void sb_commit(void *sbh);
extern uint32 sb_base(uint32 admatch);
extern uint32 sb_size(uint32 admatch);
extern void sb_core_reset(void *sbh, uint32 bits);
extern void sb_core_tofixup(void *sbh);
extern void sb_core_disable(void *sbh, uint32 bits);
extern uint32 sb_clock_rate(uint32 pll_type, uint32 n, uint32 m);
extern uint32 sb_clock(void *sbh);
extern void sb_pci_setup(void *sbh, uint32 *dmaoffset, uint coremask);
extern void sb_pcmcia_init(void *sbh);
extern void sb_watchdog(void *sbh, uint ticks);
extern void *sb_gpiosetcore(void *sbh);
extern uint32 sb_gpiocontrol(void *sbh, uint32 mask, uint32 val);
extern uint32 sb_gpioouten(void *sbh, uint32 mask, uint32 val);
extern uint32 sb_gpioout(void *sbh, uint32 mask, uint32 val);
extern uint32 sb_gpioin(void *sbh);
extern uint32 sb_gpiointpolarity(void *sbh, uint32 mask, uint32 val);
extern uint32 sb_gpiointmask(void *sbh, uint32 mask, uint32 val);
extern void sb_dump(void *sbh, char *buf);
extern bool sb_taclear(void *sbh);
extern void sb_pwrctl_init(void *sbh);
extern uint16 sb_pwrctl_fast_pwrup_delay(void *sbh);
extern int sb_pwrctl_clk(void *sbh, uint mode);
extern int sb_pwrctl_xtal(void *sbh, uint on);
extern int sb_pwrctl_dump(void *sbh, char *buf);

/* pwrctl clk mode */
#define	CLK_FAST	0			/* force fast (pll) clock */
#define	CLK_SLOW	1			/* force slow clock */
#define	CLK_DYNAMIC	2			/* enable dynamic power control */

#endif	/* _sbutils_h_ */
