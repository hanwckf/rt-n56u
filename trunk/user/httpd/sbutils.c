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
 * of the SiliconBackplane-based Broadcom chips.
 *
 * Copyright(c) 2001 ASUSTeK Inc..
 */

#include <nvram/typedefs.h>
#include <nvram/bcmutils.h>
#include <bcmdevs.h>
#include <osl.h>
#include <sbconfig.h>
#include <sbpci.h>
#include <sbpcmcia.h>
#include <sbextif.h>
#include <sbutils.h>
#include <bcmendian.h>
#include <nvram/bcmnvram.h>


/* misc sb info needed by some of the routines */
typedef struct sb_info {
	uint	chip;			/* chip number */
	uint	chiprev;		/* chip revision */
	uint16	board;			/* Board id */
	uint	bus;			/* What bus we are going through */

	void	*osh;			/* osl os handle */

	void	*curmap;		/* current regs va */
	void	*origregs;		/* initial core registers va */
	uint	origcoreidx;		/* initial core index */

	uint	numcores;		/* # discovered cores */
	uint	coreid[SB_MAXCORES];	/* id of each core */
} sb_info_t;

/* local prototypes */
static void sb_scan(sb_info_t *si);
static uint sb_pcidev2chip(uint pcidev);
static int sb_initvars(sb_info_t *si, char **vars, int *count);
static int srominitvars(sb_info_t *si, char **vars, int *count);
static int cisinitvars(sb_info_t *si, char **vars, int *count);
/* A cis parsing routine that can be called externally */
int parsecis(uint8 *cis, char **vars, int *count);
static void msecs(uint ms);


#define	SB_INFO(sbh)	(sb_info_t*)sbh
#define	SET_SBREG(sbh, r, mask, val)	W_SBREG((sbh), (r), ((R_SBREG(r) & ~(mask)) | (val)));
#define	GOODCORE(x)	(((x) >= SB_ENUM_BASE) && ((x) <= SB_ENUM_LIM) \
				&& ISALIGNED((x), SB_CORE_SIZE))
#define	GOODREGS(regs)	(regs && ISALIGNED(regs, SB_CORE_SIZE))
#define	REGS2SB(va)	(sbconfig_t*) ((uint)(va) + SBCONFIGOFF)


/* Macros to read/write sbconfig registers. On the PCMCIA core
 * rev 0 we need to treat them differently from other registers.
 * See PR 3863.
 */
#define	R_SBREG(sbr)		R_REG(sbr)
#define	W_SBREG(sbh, sbr, v)	sb_write_sbreg((sbh), (sbr), (v))
#define	AND_SBREG(sbh, sbr, v)	W_SBREG((sbh), (sbr), (R_SBREG(sbr) & (v)))
#define	OR_SBREG(sbh, sbr, v)	W_SBREG((sbh), (sbr), (R_SBREG(sbr) | (v)))

static void
sb_write_sbreg(void *sbh, volatile uint32 *sbr, uint32 v)
{
	sb_info_t *si;
	volatile uint32 dummy;

	si = SB_INFO(sbh);

	if ((si->bus == PCMCIA_BUS) && (sb_corerev(sbh) == 0)) {
#ifdef IL_BIGENDIAN
		dummy = R_REG(sbr);
		W_REG((volatile uint16 *)((uint32)sbr + 2), (uint16)((v >> 16) & 0xffff));
		dummy = R_REG(sbr);
		W_REG((volatile uint16 *)sbr, (uint16)(v & 0xffff));
#else
		dummy = R_REG(sbr);
		W_REG((volatile uint16 *)sbr, (uint16)(v & 0xffff));
		dummy = R_REG(sbr);
		W_REG((volatile uint16 *)((uint32)sbr + 2), (uint16)((v >> 16) & 0xffff));
#endif
	} else
		W_REG(sbr, v);
}

/*
 * Allocate a sb handle.
 * devid - pci device id (used to determine chip#)
 * osh - opaque OS handle
 * regs - virtual address of initial core registers
 * vars - pointer to a pointer area for "environment" variables
 * varsz - pointer to int to return the size of the vars
 */
void*
sb_attach(uint devid, void *osh, void *regs, char **vars, int *varsz)
{
	sb_info_t *si;
	uint32 w;
	uint chiprev;
	uint chip;
	uint16 board;

	ASSERT(GOODREGS(regs));

	/* convert device id to chip id */
	if ((chip = sb_pcidev2chip(devid)) == 0) {
		printf("sb_attach: unrecognized device id 0x%04x\n", devid);
		return (NULL);
	}

	/* alloc sb_info_t */
	if ((si = MALLOC(sizeof (sb_info_t))) == NULL)
		return (NULL);
	bzero((uchar*)si, sizeof (sb_info_t));

	si->chip = chip;
	si->osh = osh;
	si->origregs = si->curmap = regs;

	if (sb_coreid((void *)si) == SB_PCMCIA) {
		si->bus = PCMCIA_BUS;
	} else {
		/* check to see if we are a sb core mimic'ing a pci core */
		if (OSL_PCI_READ_CONFIG(osh, PCI_SPROM_CONTROL, sizeof (uint32)) == 0xffffffff)
			si->bus = SB_BUS;
		else
			si->bus = PCI_BUS;
	}

	/* Initialize the vars now that we have all the parms */
	if (sb_initvars(si, vars, varsz) != 0) {
		printf("sb_attach: sb_initvars failed\n");
		MFREE(si, sizeof (sb_info_t));
		return (NULL);
	}

	/*
	 * The chip revision number is hardwired into all
	 * of the pci function config rev fields and is
	 * independent from the individual core revision numbers.
	 * For PCMCIA we get it from the CIS instead.
	 * For example, the "A0" silicon of each chip is chip rev 0.
	 */
	if (si->bus == PCMCIA_BUS) {
		chiprev = (uint)getintvar(*vars, "chiprev");
		board = (uint16)getintvar(*vars, "prodid");
	} else {
		/* do a pci config read to get pci rev id */
		w = OSL_PCI_READ_CONFIG(osh, 8, sizeof (uint32));
		chiprev = w & 0xff;
		/* do a pci config read to get subsystem id */
		w = OSL_PCI_READ_CONFIG(osh, 0x2c, sizeof (uint32));
		board = (w >> 16) & 0xffff;
	}

	ASSERT(chiprev < 8);	/* sanity check */
	si->chiprev = chiprev;
	si->board = board;

	/* keep and reuse the initial register mapping */
	si->origcoreidx = sb_coreidx((void*)si);

	/* scan for cores */
	sb_scan(si);

	return ((void*)si);
}

/* kernel caller variant of sb_attach() */
void*
sb_kattach(uint chip, uint chiprev)
{
	sb_info_t *si;

	/* alloc sb_info_t */
	if ((si = MALLOC(sizeof (sb_info_t))) == NULL)
		return (NULL);
	bzero((uchar*)si, sizeof (sb_info_t));

	si->chip = chip;
	si->chiprev = chiprev;
	si->bus = SB_BUS;

	/* core0 is default */
	si->origcoreidx = 0;
	si->origregs = si->curmap = (void*)CPU_MAP(SB_ENUM_BASE, SB_CORE_SIZE);

	/* scan for cores */
	sb_scan(si);

	return ((void*)si);
}

uint
sb_coreid(void *sbh)
{
	sb_info_t *si;
	sbconfig_t *sb;

	si = SB_INFO(sbh);
	sb = REGS2SB(si->curmap);

	return ((R_SBREG(&(sb)->sbidhigh) & SBIDH_CC_MASK) >> SBIDH_CC_SHIFT);
}

/* return current index of core */
uint
sb_coreidx(void *sbh)
{
	sb_info_t *si;
	sbconfig_t *sb;
	uint32 sbaddr = 0;

	si = SB_INFO(sbh);
	ASSERT(si);

	switch (si->bus) {
	case SB_BUS:
		sb = REGS2SB(si->curmap);
		sbaddr = sb_base(R_SBREG(&sb->sbadmatch0));
		break;

	case PCI_BUS:
		sbaddr = OSL_PCI_READ_CONFIG(si->osh, PCI_BAR0_WIN, sizeof (uint32));
		break;

	case PCMCIA_BUS: {
		uint8 tmp;

		OSL_PCMCIA_READ_ATTR(si->osh, PCMCIA_ADDR0, &tmp, 1);
		sbaddr  = (uint)tmp << 12;
		OSL_PCMCIA_READ_ATTR(si->osh, PCMCIA_ADDR1, &tmp, 1);
		sbaddr |= (uint)tmp << 16;
		OSL_PCMCIA_READ_ATTR(si->osh, PCMCIA_ADDR2, &tmp, 1);
		sbaddr |= (uint)tmp << 24;
		break;
	}

	default:
		ASSERT(0);
	}

	ASSERT(GOODCORE(sbaddr));
	return ((sbaddr - SB_ENUM_BASE)/SB_CORE_SIZE);
}

uint
sb_corevendor(void *sbh)
{
	sb_info_t *si;
	sbconfig_t *sb;

	si = SB_INFO(sbh);
	sb = REGS2SB(si->curmap);

	return ((R_SBREG(&(sb)->sbidhigh) & SBIDH_VC_MASK) >> SBIDH_VC_SHIFT);
}

uint
sb_corerev(void *sbh)
{
	sb_info_t *si;
	sbconfig_t *sb;

	si = SB_INFO(sbh);
	sb = REGS2SB(si->curmap);

	return (R_SBREG(&(sb)->sbidhigh) & SBIDH_RC_MASK);
}

bool
sb_iscoreup(void *sbh)
{
	sb_info_t *si;
	sbconfig_t *sb;

	si = SB_INFO(sbh);
	sb = REGS2SB(si->curmap);

	return ((R_SBREG(&(sb)->sbtmstatelow) & (SBTML_RESET | SBTML_REJ | SBTML_CLK)) == SBTML_CLK);
}

void
sb_enablepme(void *sbh)
{
	sb_info_t *si;
	sbconfig_t *sb;

	si = SB_INFO(sbh);
	sb = REGS2SB(si->curmap);

	OR_SBREG(sbh, &(sb)->sbtmstatelow, SBTML_PE);
}

void
sb_disablepme(void *sbh)
{
	sb_info_t *si;
	sbconfig_t *sb;

	si = SB_INFO(sbh);
	sb = REGS2SB(si->curmap);

	AND_SBREG(sbh, &(sb)->sbtmstatelow, ~SBTML_PE);
}

/* scan the sb enumerated space to find all cores */
static void
sb_scan(sb_info_t *si)
{
	sbconfig_t *sb;
	uint32 sbidhigh;
	void *regs;
	uint idx;
	uint numcores;
	int i;

	numcores = 0;
	si->numcores = SB_MAXCORES;

	/* get current core index */
	idx = sb_coreidx((void*)si);

	for (i = 0; i < SB_MAXCORES; i++) {
		regs = sb_setcoreidx((void*)si, i);
		ASSERT(GOODREGS(regs));
		sb = REGS2SB(regs);

		/* end of enumerated cores? */
		sbidhigh = 0;
		BUSPROBE(sbidhigh, (&sb->sbidhigh));
		if ((sbidhigh == 0xffffffff) || (sbidhigh == 0))
			break;

		/* found a core */
		si->coreid[numcores] = sb_coreid((void *)si);
		numcores++;
	}

	si->numcores = numcores;

	/* restore original core */
	sb_setcoreidx((void*)si, idx);
}

void
sb_detach(void *sbh)
{
	sb_info_t *si;

	si = SB_INFO(sbh);

	if (si == NULL)
		return;

	if ((si->bus == SB_BUS) && si->curmap && (si->curmap != si->origregs)) {
		ASSERT(GOODREGS(si->curmap));
		CPU_UNMAP(si->curmap);
		si->curmap = NULL;
	}

	MFREE(si, sizeof (sb_info_t));
}

/*
 * Stupid table to convert pci device id to chip id
 * since that's the only way to do it for all the chips.
 */
static uint
sb_pcidev2chip(uint pcidev)
{
	if ((pcidev >= BCM4710_DEVICE_ID) && (pcidev <= BCM47XX_USB_ID))
		return (0x4710);
	if ((pcidev >= BCM4307_V90_ID) && (pcidev <= BCM4307_D11B_ID))
		return (0x4307);
	if (pcidev == BCM4301_DEVICE_ID)
		return (0x4301);
	/* add new chips here */

	return (0);
}

/* return index of core, or -1 if not found */
int
sb_findcoreidx(void *sbh, uint coreid, uint coreunit)
{
	sb_info_t *si;
	uint found;
	uint i;

	si = SB_INFO(sbh);
	found = 0;

	for (i = 0; i < si->numcores; i++)
		if (si->coreid[i] == coreid) {
			if (found == coreunit)
				return (i);
			found++;
		}

	return (-1);
}

/* change logical "focus" to the indiciated core */
void*
sb_setcoreidx(void *sbh, uint coreidx)
{
	sb_info_t *si;
	uint32 sbaddr;
	uint8 tmp;

	si = SB_INFO(sbh);

	if (coreidx >= si->numcores)
		return (NULL);

	sbaddr = SB_ENUM_BASE + (coreidx * SB_CORE_SIZE);

	switch (si->bus) {
	case SB_BUS:
		/* unmap any previous one */
		if (si->curmap && (si->curmap != si->origregs)) {
			ASSERT(GOODREGS(si->curmap));
			CPU_UNMAP(si->curmap);
			si->curmap = NULL;
		}

		/* keep and reuse the initial register mapping */
		if (coreidx == si->origcoreidx) {
			si->curmap = si->origregs;
			return (si->curmap);
		}

		/* map new one */
		si->curmap = (void*)CPU_MAP(sbaddr, SB_CORE_SIZE);
		ASSERT(GOODREGS(si->curmap));
		break;

	case PCI_BUS:
		/* point bar0 window */
		OSL_PCI_WRITE_CONFIG(si->osh, PCI_BAR0_WIN, 4, sbaddr);
		break;

	case PCMCIA_BUS:
		tmp = (sbaddr >> 12) & 0x0f;
		OSL_PCMCIA_WRITE_ATTR(si->osh, PCMCIA_ADDR0, &tmp, 1);
		tmp = (sbaddr >> 16) & 0xff;
		OSL_PCMCIA_WRITE_ATTR(si->osh, PCMCIA_ADDR1, &tmp, 1);
		tmp = (sbaddr >> 24) & 0xff;
		OSL_PCMCIA_WRITE_ATTR(si->osh, PCMCIA_ADDR2, &tmp, 1);
		break;
	}

	return (si->curmap);
}

/* change logical "focus" to the indicated core */
void*
sb_setcore(void *sbh, uint coreid, uint coreunit)
{
	sb_info_t *si;
	int idx;

	si = SB_INFO(sbh);

	idx = sb_findcoreidx(sbh, coreid, coreunit);
	if (idx < 0)
		return (NULL);

	return (sb_setcoreidx(sbh, idx));
}

/* return chip number */
uint
sb_chip(void *sbh)
{
	sb_info_t *si;

	si = SB_INFO(sbh);
	return (si->chip);
}

/* return chip rev id */
uint
sb_chiprev(void *sbh)
{
	sb_info_t *si;

	si = SB_INFO(sbh);
	return (si->chiprev);
}

/* return chip rev id */
uint16
sb_board(void *sbh)
{
	sb_info_t *si;

	si = SB_INFO(sbh);
	return (si->board);
}

/* return boolean if sbh device is in pci hostmode or client mode */
uint
sb_bus(void *sbh)
{
	sb_info_t *si;

	si = SB_INFO(sbh);
	return (si->bus);
}

/* return list of found cores */
uint
sb_corelist(void *sbh, uint coreid[])
{
	sb_info_t *si;

	si = SB_INFO(sbh);

	bcopy((uchar*)si->coreid, (uchar*)coreid, (si->numcores * sizeof (uint)));
	return (si->numcores);
}

/* do buffered registers update */
void
sb_commit(void *sbh)
{
	sb_info_t *si;
	sbpciregs_t *pciregs;
	int idx;

	si = SB_INFO(sbh);
	idx = -1;

	/* switch over to the pci core if necessary */
	if (sb_coreid(sbh) == SB_PCI)
		pciregs = (sbpciregs_t*) si->curmap;
	else {
		/* get the current core index */
		idx = sb_coreidx(sbh);
		ASSERT(idx >= 0);

		/* switch over to pci core */
		pciregs = (sbpciregs_t*) sb_setcore(sbh, SB_PCI, 0);
	}

	/* do the buffer registers update */
	W_REG(&pciregs->bcastaddr, SB_COMMIT);
	W_REG(&pciregs->bcastdata, 0x0);

	/* switch back to original core if necessary */
	if (idx >= 0)
		sb_setcoreidx(sbh, idx);
}

/* reset and re-enable a core */
void
sb_core_reset(void *sbh, uint32 bits)
{
	sb_info_t *si;
	sbconfig_t *sb;
	volatile uint32 dummy;

	si = SB_INFO(sbh);
	ASSERT(GOODREGS(si->curmap));
	sb = REGS2SB(si->curmap);

	/*
	 * Must do the disable sequence first to work for arbitrary current core state.
	 */
	sb_core_disable(sbh);

	/*
	 * Now do the initialization sequence.
	 */

	/* set reset while enabling the clock and forcing them on throughout the core */
	W_SBREG(sbh, &sb->sbtmstatelow, (SBTML_FGC | SBTML_CLK | SBTML_RESET | bits));
	dummy = R_SBREG(&sb->sbtmstatelow);

	/* PR 6594: iline100 overloads the clock enable bit with the
	 * oscillator/pll power up function which needs a much longer
	 * delay
	 */
	if (sb_coreid(sbh) == SB_ILINE100) {
		msecs(50);
	} else {
		DELAY(1);
	}

	if (R_SBREG(&sb->sbtmstatehigh) & SBTMH_SERR) {
		W_SBREG(sbh, &sb->sbtmstatehigh, 0);
	}
	if ((dummy = R_SBREG(&sb->sbimstate)) & (SBIM_IBE | SBIM_TO)) {
		AND_SBREG(sbh, &sb->sbimstate, ~(SBIM_IBE | SBIM_TO));
	}

	/* clear reset and allow it to propagate throughout the core */
	W_SBREG(sbh, &sb->sbtmstatelow, (SBTML_FGC | SBTML_CLK | bits));
	dummy = R_SBREG(&sb->sbtmstatelow);
	DELAY(1);

	/* leave clock enabled */
	W_SBREG(sbh, &sb->sbtmstatelow, (SBTML_CLK | bits));
	dummy = R_SBREG(&sb->sbtmstatelow);
	DELAY(1);

	if ((si->bus == SB_BUS) || (sb_coreid(sbh) == SB_PCI)) {
		SET_SBREG(sbh, &sb->sbimconfiglow, SBIMCL_RTO_MASK, (0x3 << SBIMCL_RTO_SHIFT));
	}
	else {
		SET_SBREG(sbh, &sb->sbimconfiglow, SBIMCL_RTO_MASK, 0);
	}
	sb_commit(sbh);
}

void
sb_core_disable(void *sbh)
{
	sb_info_t *si;
	volatile uint32 dummy;
	sbconfig_t *sb;

	si = SB_INFO(sbh);

	ASSERT(GOODREGS(si->curmap));
	sb = REGS2SB(si->curmap);

	/* must return if core is already in reset */
	if (R_SBREG(&sb->sbtmstatelow) & SBTML_RESET)
		return;

	/* set the reject bit */
	W_SBREG(sbh, &sb->sbtmstatelow, (SBTML_CLK | SBTML_REJ));

	/* spin until reject is set */
	while ((R_SBREG(&sb->sbtmstatelow) & SBTML_REJ) == 0)
		DELAY(1);

	/* spin until sbtmstatehigh.busy is clear */
	while (R_SBREG(&sb->sbtmstatehigh) & SBTMH_BUSY)
		DELAY(1);

	/* set reset and reject while enabling the clocks */
	W_SBREG(sbh, &sb->sbtmstatelow, (SBTML_FGC | SBTML_CLK | SBTML_REJ | SBTML_RESET));
	dummy = R_SBREG(&sb->sbtmstatelow);
	DELAY(10);

	/* leave reset and reject asserted */
	W_SBREG(sbh, &sb->sbtmstatelow, (SBTML_REJ | SBTML_RESET));
	DELAY(1);
}

void
sb_chip_reset(void *sbh)
{
	extifregs_t *eir;

	/* switch to extif core */
	eir = (extifregs_t *) sb_setcore(sbh, SB_EXTIF, 0);
	ASSERT(GOODREGS(eir));

	/* instant NMI */
	W_REG(&eir->watchdog, 1);
}

/*
 * Initialize the pcmcia core.
 */
void
sb_pcmcia_init(void *sbh)
{
	sb_info_t *si;
	void *regs;
	uint8 cor;

	si = SB_INFO(sbh);

	/* Set the d11 core instead of pcmcia */
	regs = sb_setcore(sbh, SB_D11, 0);
	ASSERT(GOODREGS(regs));

	/* Reset origregs, curmap and origcoreidx */
	si->origregs = si->curmap = regs;
	si->origcoreidx = sb_coreidx((void*)si);


	/* Enable interrupts via function 2 FCR */
	OSL_PCMCIA_READ_ATTR(si->osh, PCMCIA_FCR0 + PCMCIA_COR, &cor, 1);
	OSL_PCMCIA_READ_ATTR(si->osh, PCMCIA_FCR2 + PCMCIA_COR, &cor, 1);
	cor |= COR_IRQEN | COR_FUNEN;
	OSL_PCMCIA_WRITE_ATTR(si->osh, PCMCIA_FCR2 + PCMCIA_COR, &cor, 1);

}

/*
 * Configure the pci core for pci client (NIC) action
 * and get appropriate dma offset value.
 */
void
sb_pci_setup(void *sbh, uint32 *dmaoffset)
{
	sb_info_t *si;
	sbconfig_t *sb;
	sbpciregs_t *pciregs;
	uint32 sbflagnum0;
	uint idx;

	si = SB_INFO(sbh);

	*dmaoffset = 0;

	/* if directly on SB, we're done */
	if (si->bus == SB_BUS)
		return;

	/* get index of the current core */
	idx = sb_coreidx(sbh);

	/* we interrupt on this backplane flag number */
	ASSERT(GOODREGS(si->curmap));
	sb = REGS2SB(si->curmap);
	sbflagnum0 = R_SBREG(&sb->sbtpsflag) & SBTPS_NUM0_MASK;

	/* switch over to pci core */
	pciregs = (sbpciregs_t*) sb_setcore(sbh, SB_PCI, 0);
	ASSERT(GOODREGS(pciregs));
	sb = REGS2SB(pciregs);

	/* enable sb->pci interrupts */
	OR_SBREG(sbh, &sb->sbintvec, (1 << sbflagnum0));

	/* enable prefetch and bursts for sonics-to-pci translation 2 */
	OR_REG(&pciregs->sbtopci2, (SBTOPCI_PREF|SBTOPCI_BURST));

	SET_SBREG(sbh, &sb->sbimconfiglow, SBIMCL_RTO_MASK, (0x3 << SBIMCL_RTO_SHIFT));
	sb_commit(sbh);

	/* switch back to previous core */
	sb_setcoreidx(sbh, idx);

	/* use large sb pci dma window */
	*dmaoffset = SB_PCI_DMA;
}

uint32
sb_base(uint32 admatch)
{
	uint32 base;
	uint type;

	type = admatch & SBAM_TYPE_MASK;
	ASSERT(type < 3);

	base = 0;

	if (type == 0) {
		base = admatch & SBAM_BASE0_MASK;
	} else if (type == 1) {
		ASSERT(!(admatch & SBAM_ADNEG));	/* neg not supported */
		base = admatch & SBAM_BASE1_MASK;
	} else if (type == 2) {
		ASSERT(!(admatch & SBAM_ADNEG));	/* neg not supported */
		base = admatch & SBAM_BASE2_MASK;
	}

	return (base);
}

uint32
sb_size(uint32 admatch)
{
	uint32 size;
	uint type;

	type = admatch & SBAM_TYPE_MASK;
	ASSERT(type < 3);

	size = 0;

	if (type == 0) {
		size = 1 << (((admatch & SBAM_ADINT0_MASK) >> SBAM_ADINT0_SHIFT) + 1);
	} else if (type == 1) {
		ASSERT(!(admatch & SBAM_ADNEG));	/* neg not supported */
		size = 1 << (((admatch & SBAM_ADINT1_MASK) >> SBAM_ADINT1_SHIFT) + 1);
	} else if (type == 2) {
		ASSERT(!(admatch & SBAM_ADNEG));	/* neg not supported */
		size = 1 << (((admatch & SBAM_ADINT2_MASK) >> SBAM_ADINT2_SHIFT) + 1);
	}

	return (size);
}

/* return the core-type instantiation # of the current core */
uint
sb_coreunit(void *sbh)
{
	sb_info_t *si;
	uint idx;
	uint coreid;
	uint coreunit;
	uint i;

	si = SB_INFO(sbh);
	coreunit = 0;

	idx = sb_coreidx(sbh);

	ASSERT(GOODREGS(si->curmap));
	coreid = sb_coreid(sbh);

	/* count the cores of our type */
	for (i = 0; i < idx; i++)
		if (si->coreid[i] == coreid)
			coreunit++;

	return (coreunit);
}

static INLINE uint32
factor6(uint32 x)
{
	switch (x) {
	case CC_F6_2:	return 2;
	case CC_F6_3:	return 3;
	case CC_F6_4:	return 4;
	case CC_F6_5:	return 5;
	case CC_F6_6:	return 6;
	case CC_F6_7:	return 7;
	default:	return 0;
	}
}

/* calculate the speed the SB would run at given a set of clockcontrol values */
uint32
sb_clock_rate(uint32 n, uint32 m)
{
	uint32 n1, n2, fastclock, m1, m2, m3;

	n1 = factor6(n & CN_N1_MASK);
	n2 = ((n & CN_N2_MASK) >> CN_N2_SHIFT) + CC_F5_BIAS;

	fastclock = CC_CLOCK_BASE * n1 * n2;

	if (fastclock == 0)
		return 0;

	m1 = factor6(m & CC_M1_MASK);
	m2 = ((m & CC_M2_MASK) >> CC_M2_SHIFT) + CC_F5_BIAS;
	m3 = factor6((m & CC_M3_MASK) >> CC_M3_SHIFT);

	switch ((m & CC_MC_MASK) >> CC_MC_SHIFT) {
	case CC_MC_BYPASS:	return (fastclock);
	case CC_MC_M1:		return (fastclock / m1);
	case CC_MC_M1M2:	return (fastclock / (m1 * m2));
	case CC_MC_M1M2M3:	return (fastclock / (m1 * m2 * m3));
	case CC_MC_M1M3:	return (fastclock / (m1 * m3));
	default:		return (0);
	}
}

/* returns the current speed the SB is running at */
uint32
sb_clock(void *sbh)
{
	extifregs_t *eir;
	uint idx;
	uint32 rate;

	/* get index of the current core */
	idx = sb_coreidx(sbh);

	/* switch to extif core */
	eir = (extifregs_t *) sb_setcore(sbh, SB_EXTIF, 0);
	ASSERT(GOODREGS(eir));

	/* calculate rate */
	rate = sb_clock_rate(R_REG(&eir->clockcontrol_n), R_REG(&eir->clockcontrol_sb));

	/* switch back to previous core */
	sb_setcoreidx(sbh, idx);

	return rate;
}

/* set the current speed of the SB to the desired rate (as closely as possible) */
uint32
sb_setclock(void *sbh, uint32 sb, uint32 pci)
{
	extifregs_t *eir;
	uint idx, i;
	struct {
		uint32 clock;
		uint16 n;
		uint32 sb;
		uint32 pci33;
		uint32 pci25;
	} sb_clock_table[] = {
		{  96000000, 0x0303, 0x04020011, 0x11030011, 0x11050011 }, /*  96.000 32.000 24.000 */
		{ 100000000, 0x0009, 0x04020011, 0x11030011, 0x11050011 }, /* 100.000 33.333 25.000 */
		{ 104000000, 0x0802, 0x04020011, 0x11050009, 0x11090009 }, /* 104.000 31.200 24.960 */
		{ 108000000, 0x0403, 0x04020011, 0x11050009, 0x02000802 }, /* 108.000 32.400 24.923 */
		{ 112000000, 0x0205, 0x04020011, 0x11030021, 0x02000403 }, /* 112.000 32.000 24.889 */
		{ 115200000, 0x0303, 0x04020009, 0x11030011, 0x11050011 }, /* 115.200 32.000 24.000 */
		{ 120000000, 0x0011, 0x04020011, 0x11050011, 0x11090011 }, /* 120.000 30.000 24.000 */
		{ 124800000, 0x0802, 0x04020009, 0x11050009, 0x11090009 }, /* 124.800 31.200 24.960 */
		{ 128000000, 0x0305, 0x04020011, 0x11050011, 0x02000305 }, /* 128.000 32.000 24.000 */
		{ 132000000, 0x0603, 0x04020011, 0x11050011, 0x02000305 }, /* 132.000 33.000 24.750 */
		{ 136000000, 0x0c02, 0x04020011, 0x11090009, 0x02000603 }, /* 136.000 32.640 24.727 */
		{ 140000000, 0x0021, 0x04020011, 0x11050021, 0x02000c02 }, /* 140.000 30.000 24.706 */
		{ 144000000, 0x0405, 0x04020011, 0x01020202, 0x11090021 }, /* 144.000 30.857 24.686 */
		{ 150857142, 0x0605, 0x04020021, 0x02000305, 0x02000605 }, /* 150.857 33.000 24.000 */
		{ 152000000, 0x0e02, 0x04020011, 0x11050021, 0x02000e02 }, /* 152.000 32.571 24.000 */
		{ 156000000, 0x0802, 0x04020005, 0x11050009, 0x11090009 }, /* 156.000 31.200 24.960 */
		{ 160000000, 0x0309, 0x04020011, 0x11090011, 0x02000309 }, /* 160.000 32.000 24.000 */
		{ 163200000, 0x0c02, 0x04020009, 0x11090009, 0x02000603 }, /* 163.200 32.640 24.727 */
		{ 168000000, 0x0205, 0x04020005, 0x11030021, 0x02000403 }, /* 168.000 32.000 24.889 */
		{ 176000000, 0x0602, 0x04020003, 0x11050005, 0x02000602 }, /* 176.000 33.000 24.000 */
	};

	/* get index of the current core */
	idx = sb_coreidx(sbh);

	/* switch to extif core */
	eir = (extifregs_t *) sb_setcore(sbh, SB_EXTIF, 0);
	ASSERT(GOODREGS(eir));

	for (i = 1; i < (sizeof(sb_clock_table)/sizeof(sb_clock_table[0])); i++) {
		if ((sb >= sb_clock_table[i-1].clock) && (sb < sb_clock_table[i].clock)) {
			ASSERT(sb_clock_table[i-1].clock ==
			       sb_clock_rate(sb_clock_table[i-1].n, sb_clock_table[i-1].sb));
			W_REG(&eir->clockcontrol_n, sb_clock_table[i-1].n);
			W_REG(&eir->clockcontrol_sb, sb_clock_table[i-1].sb);
			if (pci == 25000000)
				W_REG(&eir->clockcontrol_pci, sb_clock_table[i-1].pci25);
			else
				W_REG(&eir->clockcontrol_pci, sb_clock_table[i-1].pci33);
			break;
		}
	}

	/* switch back to previous core */
	sb_setcoreidx(sbh, idx);

	/* return current speed */
	if (i < (sizeof(sb_clock_table)/sizeof(sb_clock_table[0])))
		return (sb_clock_table[i-1].clock);
	else
		return (0);
}

#include <proto/ethernet.h>	/* for sprom content groking */

/*
 * Read in and validate sprom.
 * Return 0 on success, nonzero on error.
 */
static int
spromread(uint16 *sprom, uint byteoff, uint16 *buf, uint nbytes)
{
	int off, nw;
	uint8 chk8;
	int i;

	off = byteoff / 2;
	nw = ROUNDUP(nbytes, 2) / 2;

	/* read the sprom and fixup the endianness so crc8 will pass */
	for (i = 0; i < nw; i++)
		buf[i] = htol16(R_REG(&sprom[off + i]));

	if ((chk8 = crc8((uchar*)buf, nbytes, CRC8_INIT_VALUE)) != CRC8_GOOD_VALUE)
		return (1);

#ifdef IL_BIGENDIAN
	/* now correct the endianness of the byte array */
	for (i = 0; i < nw; i++)
		buf[i] = ltoh16(buf[i]);
#endif

	return (0);
}

#define	VARS_MAX	4096

/*
 * Initialize nonvolatile variable table from sprom.
 * Return 0 on success, nonzero on error.
 */

static int
srominitvars(sb_info_t *si, char **vars, int *count)
{
	uint16 w, b[64];
	uint8 spromversion;
	struct ether_addr ea;
	char eabuf[32];
	int c, woff, i;
	char *vp, *base;

	if (spromread((void *)((uint)si->curmap + PCI_BAR0_SPROM_OFFSET), 0, b, sizeof (b)))
		return (-1);

	/* top word of sprom contains version and crc8 */
	spromversion = b[63] & 0xff;
	if (spromversion != 1) {
		return (-2);
	}

	ASSERT(vars);
	ASSERT(count);

	base = vp = MALLOC(VARS_MAX);
	ASSERT(vp);

	/* parameter section of sprom starts at byte offset 72 */
	woff = 72/2;

	/* first 6 bytes are il0macaddr */
	ea.octet[0] = (b[woff] >> 8) & 0xff;
	ea.octet[1] = b[woff] & 0xff;
	ea.octet[2] = (b[woff+1] >> 8) & 0xff;
	ea.octet[3] = b[woff+1] & 0xff;
	ea.octet[4] = (b[woff+2] >> 8) & 0xff;
	ea.octet[5] = b[woff+2] & 0xff;
	woff += ETHER_ADDR_LEN/2 ;
	bcm_ether_ntoa((uchar*)&ea, eabuf);
	vp += sprintf(vp, "il0macaddr=%s", eabuf);
	vp++;

	/* next 6 bytes are et0macaddr */
	ea.octet[0] = (b[woff] >> 8) & 0xff;
	ea.octet[1] = b[woff] & 0xff;
	ea.octet[2] = (b[woff+1] >> 8) & 0xff;
	ea.octet[3] = b[woff+1] & 0xff;
	ea.octet[4] = (b[woff+2] >> 8) & 0xff;
	ea.octet[5] = b[woff+2] & 0xff;
	woff += ETHER_ADDR_LEN/2 ;
	bcm_ether_ntoa((uchar*)&ea, eabuf);
	vp += sprintf(vp, "et0macaddr=%s", eabuf);
	vp++;

	/* next 6 bytes are et1macaddr */
	ea.octet[0] = (b[woff] >> 8) & 0xff;
	ea.octet[1] = b[woff] & 0xff;
	ea.octet[2] = (b[woff+1] >> 8) & 0xff;
	ea.octet[3] = b[woff+1] & 0xff;
	ea.octet[4] = (b[woff+2] >> 8) & 0xff;
	ea.octet[5] = b[woff+2] & 0xff;
	woff += ETHER_ADDR_LEN/2 ;
	bcm_ether_ntoa((uchar*)&ea, eabuf);
	vp += sprintf(vp, "et1macaddr=%s", eabuf);
	vp++;

	/*
	 * Enet phy settings one or two singles or a dual
	 * Bits 4-0 : MII address for enet0 (0x1f for not there)
	 * Bits 9-5 : MII address for enet1 (0x1f for not there)
	 * Bit 14   : Mdio for enet0
	 * Bit 15   : Mdio for enet1
	 */
	w = b[woff];
	vp += sprintf(vp, "et0phyaddr=%d", (w & 0x1f));
	vp++;
	vp += sprintf(vp, "et1phyaddr=%d", ((w >> 5) & 0x1f));
	vp++;
	vp += sprintf(vp, "et0mdcport=%d", ((w >> 14) & 0x1));
	vp++;
	vp += sprintf(vp, "et1mdcport=%d", ((w >> 15) & 0x1));
	vp++;

	/* Word 46 has board rev, antennas 0/1 & Country code lock */
	w = b[46];
	vp += sprintf(vp, "boardrev=%d", w & 0xff);
	vp++;

	vp += sprintf(vp, "cclock=%d", (w >> 8) & 0xf);
	vp++;

	vp += sprintf(vp, "aa0=%d", (w >> 12) & 0x3);
	vp++;

	vp += sprintf(vp, "aa1=%d", (w >> 14) & 0x3);
	vp++;

	/* Word 52 is max power 0/1 */
	w = b[52];
	vp += sprintf(vp, "pa0maxpwr=%d", w & 0xff);
	vp++;
	vp += sprintf(vp, "pa1maxpwr=%d", (w >> 8) & 0xff);
	vp++;

	/* set the (wl) pa settings */
	woff = 94/2; /* start of pa param's section */
	
	for (i = 0; i < 5; i++) {
		vp += sprintf(vp, "pa0b%d=%d", i, b[woff+i]);
		vp++;
		vp += sprintf(vp, "pa1b%d=%d", i, b[woff+i+6]);
		vp++;
	}
	
	/* set the oem string */
	vp += sprintf(vp, "oem=%02x%02x%02x%02x%02x%02x%02x%02x",
				  ((b[woff] >> 8) & 0xff), (b[woff] & 0xff),
				  ((b[woff+1] >> 8) & 0xff), (b[woff+1] & 0xff),
				  ((b[woff+2] >> 8) & 0xff), (b[woff+2] & 0xff),
				  ((b[woff+3] >> 8) & 0xff), (b[woff+3] & 0xff));
	vp++;

	/* final nullbyte terminator */
	*vp++ = '\0';

	c = vp - base;
	ASSERT(c <= VARS_MAX);

	if (c == VARS_MAX) {
		*vars = base;
	} else {
		vp = MALLOC(c);
		ASSERT(vp);
		bcopy(base, vp, c);
		MFREE(base, VARS_MAX);
		*vars = vp;
	}
	*count = c;

	return (0);
}

void
sromread(void *sbh, uint byteoff, uint nbytes, uint16 *buf)
{
	sb_info_t *si = SB_INFO(sbh);
	uint16 *srom = (void *)((uint)si->curmap + PCI_BAR0_SPROM_OFFSET);
	int off, nw;
	int i;

	off = byteoff / 2;
	nw  = ROUNDUP(nbytes, 2) / 2;
	/* read srom and fixup endianess */
	for (i = 0; i < nw; i++)
		buf[i] = htol16(R_REG(&srom[off + i]));
#ifdef IL_BIGENDIAN
	for (i = 0; i < nw; i++)
		buf[i] = ltoh16(buf[i]);
#endif
}

/* support only 16-bit word write into srom */
void
sromwrite(void *sbh, uint byteoff, uint nbytes, uint16 *buf)
{
	sb_info_t *si = SB_INFO(sbh);
	uint16 *srom = (void *)((uint)si->curmap + PCI_BAR0_SPROM_OFFSET);
	int off, nw;
	int i;
	uint16 image[SPROM_SIZE];
	uint8  crc;
	volatile uint32 val32;

	/* first read entire srom */
	for (i = 0; i < SPROM_SIZE; i++)
		image[i] = htol16(R_REG(&srom[i]));
#ifdef IL_BIGENDIAN
	for (i = 0; i < SPROM_SIZE; i++)
		image[i] = ltoh16(image[i]);
#endif

	off = byteoff / 2;
	nw  = ROUNDUP(nbytes, 2) / 2;
	for (i = 0; i < nw; i++)
		image[off+i] = buf[i];

	crc = ~crc8((uint8 *)image, SPROM_BYTES - 1, CRC8_INIT_VALUE);
	image[SPROM_SIZE - 1] = (crc << 8) | (image[SPROM_SIZE - 1] & 0xff);

	/* Enable writes to the SPROM */
	val32 = OSL_PCI_READ_CONFIG(si->osh, PCI_SPROM_CONTROL, sizeof(uint32));
	val32 |= SPROM_WRITEEN;
	OSL_PCI_WRITE_CONFIG(si->osh, PCI_SPROM_CONTROL, sizeof(uint32), val32);
	msecs(500);

	/* write srom */
#ifdef IL_BIGENDIAN
	for (i = 0; i < SPROM_SIZE; i++)
		image[i] = htol16(image[i]);
#endif
	for (i = 0; i < SPROM_SIZE; i++) {
		W_REG(&srom[i], ltoh16(image[i]));
		msecs(20);
	}

	/* Disable writes to the SPROM */
	OSL_PCI_WRITE_CONFIG(si->osh, PCI_SPROM_CONTROL,
				sizeof(uint32), val32 & ~SPROM_WRITEEN);
	msecs(500);
}

int
parsecis(uint8 *cis, char **vars, int *count)
{
	char eabuf[32];
	char *vp, *base;
	uint8 tup, tlen;
	int i, j;

	ASSERT(vars);
	ASSERT(count);

	base = vp = MALLOC(VARS_MAX);
	ASSERT(vp);

	i = 0;
	do {
		tup = cis[i++];
		tlen = cis[i++];

		switch (tup) {
		case CISTPL_MANFID:
			vp += sprintf(vp, "manfid=%02x%02x", cis[i + 1], cis[i]);
			vp++;
			vp += sprintf(vp, "prodid=%02x%02x", cis[i + 3], cis[i + 2]);
			vp++;
			break;

		case CISTPL_FUNCE:
			if (cis[i] == LAN_NID) {
				ASSERT(cis[i + 1] == ETHER_ADDR_LEN);
				bcm_ether_ntoa((uchar*)&cis[i + 2], eabuf);
				vp += sprintf(vp, "il0macaddr=%s", eabuf);
				vp++;
			}
			break;

		case CISTPL_BRCM_HNBU:
			switch (cis[i]) {
			case HNBU_CHIPID:
				vp += sprintf(vp, "vendid=%02x%02x", cis[i + 2], cis[i + 1]);
				vp++;
				vp += sprintf(vp, "devid=%02x%02x", cis[i + 4], cis[i + 3]);
				vp++;
				vp += sprintf(vp, "chiprev=%d", (cis[i + 6] << 8) + cis[i + 5]);
				vp++;
				break;

			case HNBU_BOARDREV:
				vp += sprintf(vp, "boardrev=%d", cis[i + 1]);
				vp++;
				break;

			case HNBU_PAPARMS:
				for (j = 0; j < 5; j++) {
					vp += sprintf(vp, "pa0b%d=%02x%02x", j,
						      cis[i + (j * 2) + 2], cis[i + (j * 2) + 1]);
					vp++;
				}
				vp += sprintf(vp, "pa0maxpwr=%d", cis[i + 11]);
				vp++;
				break;

			case HNBU_OEM:
				vp += sprintf(vp, "oem=%02x%02x%02x%02x%02x%02x%02x%02x",
					      cis[i + 1], cis[i + 2], cis[i + 3], cis[i + 4],
					      cis[i + 5], cis[i + 6], cis[i + 7], cis[i + 8]);
				vp++;
				break;
			}
			break;

		}
		i += tlen;
	} while (tup != 0xff);

	/* final nullbyte terminator */
	*vp++ = '\0';
	i++;

	ASSERT(i < VARS_MAX);

	if (i == VARS_MAX) {
		*vars = base;
	} else {
		vp = MALLOC(i);
		ASSERT(vp);
		bcopy(base, vp, i);
		MFREE(base, VARS_MAX);
		*vars = vp;
	}
	*count = i;

	return (0);
}

/*
 * Read the cis and call parsecis to initialize the vars.
 * Return 0 on success, nonzero on error.
 */
static int
cisinitvars(sb_info_t *si, char **vars, int *count)
{
	uint8 *cis = NULL;
	int rc;

	if ((cis = MALLOC(CIS_SIZE)) == NULL)
		return (-1);

	OSL_PCMCIA_READ_ATTR(si->osh, 0, cis, CIS_SIZE);

	rc = parsecis(cis, vars, count);

	MFREE(cis, CIS_SIZE);

	return (rc);
}

/*
 * Initialize the vars from the right source for this platform.
 * Return 0 on success, nonzero on error.
 */
static int
sb_initvars(sb_info_t *si, char **vars, int *count)
{
	if (vars == NULL)
		return (0);

	switch (si->bus) {
	case SB_BUS:
		/* These two could be asserts ... */
		*vars = NULL;
		*count = 0;
		return (0);

	case PCI_BUS:
		return (srominitvars(si, vars, count));

	case PCMCIA_BUS:
		return (cisinitvars(si, vars, count));

	default:
		ASSERT(0);
	}
	return (-1);
}

/*
 * Search the vars for a specific one and return its value.
 * Returns NULL if not found.
 */
char*
getvar(char *vars, char *name)
{
	char *s;
	int len;

	len = strlen(name);
	printf("GetVar\n");
	if (vars == NULL)
	{
		return (NULL);
	}	

	for (s = vars; s && *s; ) {
		if ((bcmp(s, name, len) == 0)
		    && (s[len] == '='))
			return (&s[len+1]);

		while (*s++)
			;
	}
	return (NULL);
}

/*
 * Search the vars for a specific one and return its value as
 * an integer. Returns 0 if not found.
 */
int
getintvar(char *vars, char *name)
{
	char *val;

	if ((val = getvar(vars, name)) == NULL)
		return (0);

	return (bcm_atoi(val));
}

void
sb_dump(void *sbh, char *buf)
{
	sb_info_t *si;
	uint i;

	si = SB_INFO(sbh);

	buf += sprintf(buf, "si 0x%x chip 0x%x chiprev 0x%x bus %d\n",
		(uint)si, si->chip, si->chiprev, si->bus);
	buf += sprintf(buf, "osh 0x%x curmap 0x%x origregs 0x%x origcoreidx %d\n",
		(uint)si->osh, (uint)si->curmap, (uint)si->origregs, si->origcoreidx);
	buf += sprintf(buf, "cores:  ");
	for (i = 0; i < si->numcores; i++)
		buf += sprintf(buf, "0x%x ", si->coreid[i]);
	buf += sprintf(buf, "\n");
}

/* Doing really long delays with DELAY breaks Linux:
 * it claims they would not be accurate on fast machines.
 */
static void
msecs(uint ms)
{
	uint i;

	for (i = 0; i < ms; i++) {
		DELAY(1000);
	}
}

