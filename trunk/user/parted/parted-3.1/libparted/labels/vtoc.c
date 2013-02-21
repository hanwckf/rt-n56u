#include <config.h>
#include <parted/vtoc.h>

#ifdef DEBUG_DASD
#define PDEBUG fprintf(stderr, "%s:%d:%s\n", \
                       __FILE__,                              \
                       __LINE__,                              \
                       __PRETTY_FUNCTION__);
#else
#define PDEBUG
#endif

#include <parted/parted.h>

#include <libintl.h>
#if ENABLE_NLS
#  define _(String) dgettext (PACKAGE, String)
#else
#  define _(String) (String)
#endif /* ENABLE_NLS */

static const unsigned char EBCtoASC[256] =
{
/* 0x00  NUL   SOH   STX   ETX  *SEL    HT  *RNL   DEL */
	0x00, 0x01, 0x02, 0x03, 0x07, 0x09, 0x07, 0x7F,
/* 0x08  -GE  -SPS  -RPT    VT    FF    CR    SO    SI */
	0x07, 0x07, 0x07, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
/* 0x10  DLE   DC1   DC2   DC3  -RES   -NL    BS  -POC
                                -ENP  ->LF             */
	0x10, 0x11, 0x12, 0x13, 0x07, 0x0A, 0x08, 0x07,
/* 0x18  CAN    EM  -UBS  -CU1  -IFS  -IGS  -IRS  -ITB
                                                  -IUS */
	0x18, 0x19, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07,
/* 0x20  -DS  -SOS    FS  -WUS  -BYP    LF   ETB   ESC
                                -INP                   */
	0x07, 0x07, 0x1C, 0x07, 0x07, 0x0A, 0x17, 0x1B,
/* 0x28  -SA  -SFE   -SM  -CSP  -MFA   ENQ   ACK   BEL
                     -SW                               */
	0x07, 0x07, 0x07, 0x07, 0x07, 0x05, 0x06, 0x07,
/* 0x30 ----  ----   SYN   -IR   -PP  -TRN  -NBS   EOT */
	0x07, 0x07, 0x16, 0x07, 0x07, 0x07, 0x07, 0x04,
/* 0x38 -SBS   -IT  -RFF  -CU3   DC4   NAK  ----   SUB */
	0x07, 0x07, 0x07, 0x07, 0x14, 0x15, 0x07, 0x1A,
/* 0x40   SP   RSP           ä              ----       */
	0x20, 0xFF, 0x83, 0x84, 0x85, 0xA0, 0x07, 0x86,
/* 0x48                      .     <     (     +     | */
	0x87, 0xA4, 0x9B, 0x2E, 0x3C, 0x28, 0x2B, 0x7C,
/* 0x50    &                                      ---- */
	0x26, 0x82, 0x88, 0x89, 0x8A, 0xA1, 0x8C, 0x07,
/* 0x58          ß     !     $     *     )     ;       */
	0x8D, 0xE1, 0x21, 0x24, 0x2A, 0x29, 0x3B, 0xAA,
/* 0x60    -     /  ----     Ä  ----  ----  ----       */
	0x2D, 0x2F, 0x07, 0x8E, 0x07, 0x07, 0x07, 0x8F,
/* 0x68             ----     ,     %     _     >     ? */
	0x80, 0xA5, 0x07, 0x2C, 0x25, 0x5F, 0x3E, 0x3F,
/* 0x70  ---        ----  ----  ----  ----  ----  ---- */
	0x07, 0x90, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07,
/* 0x78    *     `     :     #     @     '     =     " */
	0x70, 0x60, 0x3A, 0x23, 0x40, 0x27, 0x3D, 0x22,
/* 0x80    *     a     b     c     d     e     f     g */
	0x07, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67,
/* 0x88    h     i              ----  ----  ----       */
	0x68, 0x69, 0xAE, 0xAF, 0x07, 0x07, 0x07, 0xF1,
/* 0x90    °     j     k     l     m     n     o     p */
	0xF8, 0x6A, 0x6B, 0x6C, 0x6D, 0x6E, 0x6F, 0x70,
/* 0x98    q     r                    ----        ---- */
	0x71, 0x72, 0xA6, 0xA7, 0x91, 0x07, 0x92, 0x07,
/* 0xA0          ~     s     t     u     v     w     x */
	0xE6, 0x7E, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78,
/* 0xA8    y     z              ----  ----  ----  ---- */
	0x79, 0x7A, 0xAD, 0xAB, 0x07, 0x07, 0x07, 0x07,
/* 0xB0    ^                    ----     §  ----       */
	0x5E, 0x9C, 0x9D, 0xFA, 0x07, 0x07, 0x07, 0xAC,
/* 0xB8       ----     [     ]  ----  ----  ----  ---- */
	0xAB, 0x07, 0x5B, 0x5D, 0x07, 0x07, 0x07, 0x07,
/* 0xC0    {     A     B     C     D     E     F     G */
	0x7B, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47,
/* 0xC8    H     I  ----           ö              ---- */
	0x48, 0x49, 0x07, 0x93, 0x94, 0x95, 0xA2, 0x07,
/* 0xD0    }     J     K     L     M     N     O     P */
	0x7D, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F, 0x50,
/* 0xD8    Q     R  ----           ü                   */
	0x51, 0x52, 0x07, 0x96, 0x81, 0x97, 0xA3, 0x98,
/* 0xE0    \           S     T     U     V     W     X */
	0x5C, 0xF6, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58,
/* 0xE8    Y     Z        ----     Ö  ----  ----  ---- */
	0x59, 0x5A, 0xFD, 0x07, 0x99, 0x07, 0x07, 0x07,
/* 0xF0    0     1     2     3     4     5     6     7 */
	0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
/* 0xF8    8     9  ----  ----     Ü  ----  ----  ---- */
	0x38, 0x39, 0x07, 0x07, 0x9A, 0x07, 0x07, 0x07
};

static const unsigned char ASCtoEBC[256] =
{
    /*00  NL    SH    SX    EX    ET    NQ    AK    BL */
	0x00, 0x01, 0x02, 0x03, 0x37, 0x2D, 0x2E, 0x2F,
    /*08  BS    HT    LF    VT    FF    CR    SO    SI */
	0x16, 0x05, 0x15, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
    /*10  DL    D1    D2    D3    D4    NK    SN    EB */
	0x10, 0x11, 0x12, 0x13, 0x3C, 0x15, 0x32, 0x26,
    /*18  CN    EM    SB    EC    FS    GS    RS    US */
	0x18, 0x19, 0x3F, 0x27, 0x1C, 0x1D, 0x1E, 0x1F,
    /*20  SP     !     "     #     $     %     &     ' */
	0x40, 0x5A, 0x7F, 0x7B, 0x5B, 0x6C, 0x50, 0x7D,
    /*28   (     )     *     +     ,     -    .      / */
	0x4D, 0x5D, 0x5C, 0x4E, 0x6B, 0x60, 0x4B, 0x61,
    /*30   0     1     2     3     4     5     6     7 */
	0xF0, 0xF1, 0xF2, 0xF3, 0xF4, 0xF5, 0xF6, 0xF7,
    /*38   8     9     :     ;     <     =     >     ? */
	0xF8, 0xF9, 0x7A, 0x5E, 0x4C, 0x7E, 0x6E, 0x6F,
    /*40   @     A     B     C     D     E     F     G */
	0x7C, 0xC1, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7,
    /*48   H     I     J     K     L     M     N     O */
	0xC8, 0xC9, 0xD1, 0xD2, 0xD3, 0xD4, 0xD5, 0xD6,
    /*50   P     Q     R     S     T     U     V     W */
	0xD7, 0xD8, 0xD9, 0xE2, 0xE3, 0xE4, 0xE5, 0xE6,
    /*58   X     Y     Z     [     \     ]     ^     _ */
	0xE7, 0xE8, 0xE9, 0xAD, 0xE0, 0xBD, 0x5F, 0x6D,
    /*60   `     a     b     c     d     e     f     g */
	0x79, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87,
    /*68   h     i     j     k     l     m     n     o */
	0x88, 0x89, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96,
    /*70   p     q     r     s     t     u     v     w */
	0x97, 0x98, 0x99, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6,
    /*78   x     y     z     {     |     }     ~    DL */
	0xA7, 0xA8, 0xA9, 0xC0, 0x4F, 0xD0, 0xA1, 0x07,
	0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F,
	0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F,
	0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F,
	0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F,
	0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F,
	0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F,
	0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F,
	0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F,
	0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F,
	0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F,
	0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F,
	0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F,
	0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F,
	0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F,
	0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F,
	0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0xFF
};

enum failure {
	unable_to_open,
	unable_to_seek,
	unable_to_write,
	unable_to_read
};

static char buffer[89];

static void
vtoc_error (enum failure why, char const *s1, char const *s2)
{
	PDEBUG
	char error[8192];

	switch (why) {
		case unable_to_open:
			sprintf(error, "VTOC: %s -- %s\n%s\n",
				_("opening of device failed"), s1, s2);
			break;
		case unable_to_seek:
			sprintf(error, "VTOC: %s -- %s\n%s\n",
				_("seeking on device failed"), s1, s2);
			break;
		case unable_to_write:
			sprintf(error, "VTOC: %s -- %s\n%s\n",
				_("writing to device failed"), s1, s2);
			break;
		case unable_to_read:
			sprintf(error, "VTOC: %s -- %s\n%s\n",
				_("reading from device failed"), s1, s2);
			break;
		default:
			sprintf(error, "VTOC: %s\n", _("Fatal error"));
	}

	ped_exception_throw(PED_EXCEPTION_ERROR, PED_EXCEPTION_CANCEL, error);
}

char *
vtoc_ebcdic_enc (char const *source, char *target, int l)
{
	PDEBUG
	int i;

	for (i = 0; i < l; i++)
		target[i]=ASCtoEBC[(unsigned char)(source[i])];

	return target;
}

char *
vtoc_ebcdic_dec (char const *source, char *target, int l)
{
	PDEBUG
	int i;

	for (i = 0; i < l; i++)
		target[i]=EBCtoASC[(unsigned char)(source[i])];

	return target;
}

void
vtoc_set_extent (extent_t *ext, u_int8_t typeind, u_int8_t seqno,
                 cchh_t *lower, cchh_t *upper)
{
	PDEBUG
	ext->typeind = typeind;
	ext->seqno   = seqno;
	memcpy(&ext->llimit,lower,sizeof(cchh_t));
	memcpy(&ext->ulimit,upper,sizeof(cchh_t));
}

void
vtoc_set_cchh (cchh_t *addr, u_int16_t cc, u_int16_t hh)
{
	PDEBUG
	addr->cc = cc;
	addr->hh = hh;
}

static void
vtoc_set_ttr (ttr_t *addr, u_int16_t tt, u_int8_t r)
{
	PDEBUG
	addr->tt = tt;
	addr->r = r;
}

void
vtoc_set_cchhb (cchhb_t *addr, u_int16_t cc, u_int16_t hh, u_int8_t b)
{
	PDEBUG
	addr->cc = cc;
	addr->hh = hh;
	addr->b = b;
}

void
vtoc_set_date (labeldate_t * d, u_int8_t year, u_int16_t day)
{
	PDEBUG
	d->year = year;
	d->day = day;
}

/*
 * initializes the volume label with EBCDIC spaces
 */
void
vtoc_volume_label_init (volume_label_t *vlabel)
{
	PDEBUG
	sprintf(buffer, "%88s", " ");
	vtoc_ebcdic_enc(buffer, buffer, sizeof *vlabel);
	memcpy(vlabel, buffer, sizeof *vlabel);
}

/*
 * reads the volume label from dasd
 */
int
vtoc_read_volume_label (int f, unsigned long vlabel_start,
                        volume_label_t *vlabel)
{

	char str[5];
	unsigned long block_zero;
	typedef struct bogus_label bogus_label_t;
	typedef union vollabel vollabel_t;

	union __attribute__((packed)) vollabel {
		volume_label_t cdl;
		ldl_volume_label_t ldl;
		cms_volume_label_t cms;
	};

	struct  __attribute__((packed)) bogus_label {
	   char overhead[512];
	   vollabel_t actual_label;
	};

	bogus_label_t mybogus;
	bogus_label_t *bogus_ptr = &mybogus;
	vollabel_t *union_ptr = &bogus_ptr->actual_label;
	volume_label_t *cdl_ptr = &union_ptr->cdl;

	PDEBUG
	int rc;

	if (lseek(f, vlabel_start, SEEK_SET) == -1) {
		vtoc_error(unable_to_seek, "vtoc_read_volume_label",
			   _("Could not read volume label."));
		return 1;
	}

	rc = read(f, vlabel, sizeof(volume_label_t));
	if (rc != sizeof(volume_label_t) &&
	/* For CDL we ask to read 88 bytes, but only get 84 */
            rc != sizeof(volume_label_t) - 4) {
		vtoc_error(unable_to_read, "vtoc_read_volume_label",
			   _("Could not read volume label."));
		return 1;
	}

	if (strncmp(vlabel->volkey, vtoc_ebcdic_enc("VOL1", str, 4), 4) == 0
	 || strncmp(vlabel->volkey, vtoc_ebcdic_enc("LNX1", str, 4), 4) == 0
         || strncmp(vlabel->volkey, vtoc_ebcdic_enc("CMS1", str, 4), 4) == 0)
	return 0;

	/*
	   If we didn't find a valid volume label, there is a special case
           we must try before we give up.  For a CMS-formatted disk on FBA
	   DASD using the DIAG driver and a block size greater than 512, we
	   must read the block at offset 0, then look for a label within
	   that block at offset 512.
	*/

	block_zero = 0;

	if (lseek(f, block_zero, SEEK_SET) == -1) {
		vtoc_error(unable_to_seek, "vtoc_read_volume_label",
			   _("Could not read volume label."));
		return 1;
	}

	rc = read(f, bogus_ptr, sizeof(bogus_label_t));
	if (rc != sizeof(bogus_label_t)) {
		vtoc_error(unable_to_read, "vtoc_read_volume_label",
			   _("Could not read volume label."));
		return 1;
	}

	memcpy(vlabel, cdl_ptr, sizeof *vlabel);
	return 0;
}

/*
 * writes the volume label to dasd
 */
int
vtoc_write_volume_label (int f, unsigned long vlabel_start,
                         volume_label_t const *vlabel)
{
	PDEBUG
	int rc;

	if (lseek(f, vlabel_start, SEEK_SET) == -1)
		vtoc_error(unable_to_seek, "vtoc_write_volume_label",
			   _("Could not write volume label."));

	rc = write(f, vlabel, sizeof(volume_label_t) - 4);
	/* Subtract 4 to leave off the "fudge" variable when writing.
           We only write CDL volume labels, never LDL or CMS.  */
	if (rc != sizeof(volume_label_t) - 4)
		vtoc_error(unable_to_write, "vtoc_write_volume_label",
			   _("Could not write volume label."));

	return 0;
}

/*
 * takes a string as input, converts it to uppercase, translates
 * it to EBCDIC and fills it up with spaces before it copies it
 * as volume serial to the volume label
 */
void
vtoc_volume_label_set_volser (volume_label_t *vlabel, char const *volser)
{
	PDEBUG
	int j, i = strlen(volser);
	char s[VOLSER_LENGTH + 1];

	strcpy(s, "      ");
	vtoc_ebcdic_enc(s, s, VOLSER_LENGTH);
	strncpy(vlabel->volid, s, VOLSER_LENGTH);

	if (i > VOLSER_LENGTH)
		i = VOLSER_LENGTH;

	strncpy(s, volser, i);
	for (j=0; j<i; j++)
		s[j] = toupper(s[j]);

	s[VOLSER_LENGTH] = 0x00;
	vtoc_ebcdic_enc(s, s, i);
	strncpy(vlabel->volid, s, i);

	return;
}

/*
 * returns the volume serial number right after it is translated
 * to ASCII
 */
char *
vtoc_volume_label_get_volser (volume_label_t *vlabel, char *volser)
{
	PDEBUG
	vtoc_ebcdic_dec(vlabel->volid, volser, VOLSER_LENGTH);

	return volser;
}

/*
 * sets the volume label key right after
 * it has been translated to EBCDIC
 */
void
vtoc_volume_label_set_key (volume_label_t *vlabel, char const *key)
{
	PDEBUG
	char s[4];

	vtoc_ebcdic_enc(key, s, 4);
	strncpy(vlabel->volkey, s, 4);

	return;
}

/*
 * sets the volume label identifier right
 * after it has been translated to EBCDIC
 */
void
vtoc_volume_label_set_label (volume_label_t *vlabel, char const *lbl)
{
	PDEBUG
	char s[4];

	vtoc_ebcdic_enc(lbl, s, 4);
	strncpy(vlabel->vollbl, s, 4);

	return;
}

/*
 * returns the volume label key = the label identifier
 * right after it has been translated to ASCII
 */
char *
vtoc_volume_label_get_label (volume_label_t *vlabel, char *lbl)
{
	PDEBUG
	vtoc_ebcdic_dec(vlabel->vollbl, lbl, 4);

	return lbl;
}

/*
 * reads either a format4 label or a format1 label
 * from the specified position
 */
void
vtoc_read_label (int f, unsigned long position, format1_label_t *f1,
                 format4_label_t *f4, format5_label_t *f5, format7_label_t *f7)
{
	PDEBUG
	int t;

	if (lseek(f, position, SEEK_SET) == -1)
		vtoc_error(unable_to_seek, "vtoc_read_label",
			   _("Could not read VTOC labels."));

	if (f1 != NULL) {
		t = sizeof(format1_label_t);
		if (read(f, f1, t) != t)
			vtoc_error(unable_to_read, "vtoc_read_label",
				   _("Could not read VTOC FMT1 DSCB."));
	}

	if (f4 != NULL) {
		t = sizeof(format4_label_t);
		if (read(f, f4, t) != t)
			vtoc_error(unable_to_read, "vtoc_read_label",
				   _("Could not read VTOC FMT4 DSCB."));
	}

	if (f5 != NULL) {
		t = sizeof(format5_label_t);
		if (read(f, f5, t) != t)
			vtoc_error(unable_to_read, "vtoc_read_label",
				   _("Could not read VTOC FMT5 DSCB."));
	}

	if (f7 != NULL) {
		t = sizeof(format7_label_t);
		if (read(f, f7, t) != t)
			vtoc_error(unable_to_read, "vtoc_read_label",
				   _("Could not read VTOC FMT7 DSCB."));
	}
}

/*
 * writes either a FMT1, FMT4 or FMT5 label
 * to the specified position
 */
void
vtoc_write_label (int f, unsigned long position,
		  format1_label_t const *f1,
                  format4_label_t const *f4,
		  format5_label_t const *f5,
		  format7_label_t const *f7)
{
	PDEBUG
	int t;

	if (lseek(f, position, SEEK_SET) == -1)
		vtoc_error(unable_to_seek, "vtoc_write_label",
			   _("Could not write VTOC labels."));

	if (f1 != NULL) {
		t = sizeof(format1_label_t);
		if (write(f, f1, t) != t)
			vtoc_error(unable_to_write, "vtoc_write_label",
				   _("Could not write VTOC FMT1 DSCB."));
	}

	if (f4 != NULL) {
		t = sizeof(format4_label_t);
		if (write(f, f4, t) != t)
			vtoc_error(unable_to_write, "vtoc_write_label",
				   _("Could not write VTOC FMT4 DSCB."));
	}

	if (f5 != NULL) {
		t = sizeof(format5_label_t);
		if (write(f, f5, t) != t)
			vtoc_error(unable_to_write, "vtoc_write_label",
				   _("Could not write VTOC FMT5 DSCB."));
	}

	if (f7 != NULL) {
		t = sizeof(format7_label_t);
		if (write(f, f7, t) != t)
			vtoc_error(unable_to_write, "vtoc_write_label",
				   _("Could not write VTOC FMT7 DSCB."));
	}
}

/*
 * initializes a format4 label
 */
void
vtoc_init_format4_label (format4_label_t *f4, unsigned int usable_partitions,
                         unsigned int cylinders, unsigned int tracks,
                         unsigned int blocks, unsigned int blksize,
                         u_int16_t dev_type)
{
	PDEBUG
	int i;

	cchh_t lower = {VTOC_START_CC, VTOC_START_HH};
	cchh_t upper = {VTOC_START_CC, VTOC_START_HH};

	for (i=0; i<44; i++) f4->DS4KEYCD[i] = 0x04;
        f4->DS4IDFMT = 0xf4;

	vtoc_set_cchhb(&f4->DS4HPCHR, 0x0000, 0x0000, 0x00);
	f4->DS4DSREC = blocks - 2;
	/* free space starts right behind VTOC
	   vtoc_set_cchh(&f4->DS4HCCHH, VTOC_START_CC, VTOC_START_HH + 1);*/
	vtoc_set_cchh(&f4->DS4HCCHH, 0x0000, 0x0000);
	f4->DS4NOATK = 0x0000;
	f4->DS4VTOCI = 0x00;
	f4->DS4NOEXT = 0x01;
	f4->DS4SMSFG = 0x00;
	f4->DS4DEVAC = 0x00;

	/* -- begin f4->DS4DEVCT -- */
	f4->DS4DEVCT.DS4DSCYL = cylinders;
	f4->DS4DEVCT.DS4DSTRK = tracks;

	switch (dev_type) {
		case DASD_3380_TYPE:
			f4->DS4DEVCT.DS4DEVTK = DASD_3380_VALUE;
			break;
		case DASD_3390_TYPE:
			f4->DS4DEVCT.DS4DEVTK = DASD_3390_VALUE;
			break;
		case DASD_9345_TYPE:
			f4->DS4DEVCT.DS4DEVTK = DASD_9345_VALUE;
			break;
		default:
			f4->DS4DEVCT.DS4DEVTK = blocks * blksize;;
	}

	f4->DS4DEVCT.DS4DEVI  = 0x00;
	f4->DS4DEVCT.DS4DEVL  = 0x00;
	f4->DS4DEVCT.DS4DEVK  = 0x00;
	f4->DS4DEVCT.DS4DEVFG = 0x30;
	f4->DS4DEVCT.DS4DEVTL = 0x0000;
	f4->DS4DEVCT.DS4DEVDT = blocks;
	f4->DS4DEVCT.DS4DEVDB = 0x00;
	/* -- end f4->DS4DEVCT -- */

	bzero(f4->DS4AMTIM, sizeof(f4->DS4AMTIM));
	bzero(f4->DS4AMCAT, sizeof(f4->DS4AMCAT));
	bzero(f4->DS4R2TIM, sizeof(f4->DS4R2TIM));
	bzero(f4->res1, sizeof(f4->res1));
	bzero(f4->DS4F6PTR, sizeof(f4->DS4F6PTR));

	/* -- begin f4lbl->DS4VTOCE -- */
	vtoc_set_extent(&f4->DS4VTOCE, 0x01, 0x00, &lower, &upper);
	/* -- end f4lbl->DS4VTOCE -- */

	bzero(f4->res2, sizeof(f4->res2));
	f4->DS4EFLVL = 0x00;
	bzero(&f4->DS4EFPTR, sizeof(f4->DS4EFPTR));
	bzero(f4->res3, sizeof(f4->res3));
}

/*
 * initializes a format5 label
 */
void
vtoc_init_format5_label (format5_label_t *f5)
{
	PDEBUG
	int i;

	bzero(f5, sizeof(format5_label_t));
	for (i=0; i<4; i++)
		f5->DS5KEYID[i] = 0x05;
	f5->DS5FMTID = 0xf5;
}

/*
 * initializes a format7 label
 */
void
vtoc_init_format7_label (format7_label_t *f7)
{
	PDEBUG
	int i;

	bzero(f7, sizeof(format7_label_t));
	for (i=0; i<4; i++)
		f7->DS7KEYID[i] = 0x07;
	f7->DS7FMTID = 0xf7;
}

/*
 * initializes a format1 label
 */
void
vtoc_init_format1_label (char *volid, unsigned int blksize,
                         extent_t *part_extent, format1_label_t *f1)
{
	PDEBUG
	struct tm * creatime;
	time_t t;
	char str[80];

	/* get actual date */
	t = time(NULL);
	creatime = gmtime(&t);

	bzero(f1->DS1DSNAM, sizeof(f1->DS1DSNAM));
	sprintf(str, "PART    .NEW                                ");
	vtoc_ebcdic_enc(str, str, 44);
	strncpy(f1->DS1DSNAM, str, 44);
	f1->DS1FMTID = 0xf1;
	strncpy(f1->DS1DSSN, "      ", 6);
	f1->DS1VOLSQ = 0x0001;

	vtoc_set_date(&f1->DS1CREDT, (u_int8_t) creatime->tm_year,
				  (u_int16_t) creatime->tm_yday);
	/* expires never - 99 365 */
	vtoc_set_date(&f1->DS1EXPDT, 0x63, 0x016D);
	f1->DS1NOEPV = 0x01;
	f1->DS1NOBDB = 0x00;
	f1->DS1FLAG1 = 0x00;
	vtoc_ebcdic_enc("IBM LINUX    ", str, 13);
	strncpy(f1->DS1SYSCD, str, 13);
	vtoc_set_date(&f1->DS1REFD, (u_int8_t) creatime->tm_year,
				  (u_int16_t) creatime->tm_yday);
	f1->DS1SMSFG = 0x00;
	f1->DS1SCXTF = 0x00;
	f1->DS1SCXTV = 0x0000;
	f1->DS1DSRG1 = 0x00;
	f1->DS1DSRG2 = 0x00;
	f1->DS1RECFM = 0x88;
	f1->DS1OPTCD = 0x00;
	f1->DS1BLKL  = blksize;
	f1->DS1LRECL = blksize;
	f1->DS1KEYL  = 0x00;
	f1->DS1RKP   = 0x0000;
	f1->DS1DSIND = 0x80; /* last volume for this dataset */
	f1->DS1SCAL1 = 0x80;
	bzero(&f1->DS1SCAL3, sizeof(f1->DS1SCAL3));
	vtoc_set_ttr(&f1->DS1LSTAR, 0x0000, 0x00);
	f1->DS1TRBAL = 0x00;
	bzero(&f1->res1, sizeof(f1->res1));
	memcpy(&f1->DS1EXT1, part_extent, sizeof(extent_t));
	bzero(&f1->DS1EXT2, sizeof(extent_t));
	bzero(&f1->DS1EXT3, sizeof(extent_t));
	vtoc_set_cchhb(&f1->DS1PTRDS, 0x0000, 0x0000, 0x00);
}

/*
 * do some updates to the VTOC format4 label
 */
void
vtoc_update_format4_label (format4_label_t *f4, cchhb_t *highest_f1,
                           u_int16_t unused_update)
{
	PDEBUG
	/* update highest address of a format 1 label */
	memcpy(&f4->DS4HPCHR, highest_f1, sizeof(cchhb_t));

	/* update unused DSCB count */
	f4->DS4DSREC = unused_update;
}

/*
 * reorganizes all extents within a FMT5 label
 */
static void
vtoc_reorganize_FMT5_extents (format5_label_t *f5)
{
	PDEBUG
	ds5ext_t *ext, *last, tmp;
	int i, j;

	for (i=0; i<26; i++) {
		if (i==0)
			last = &f5->DS5AVEXT;
		else if ((i > 0) && (i < 8))
			last = &f5->DS5EXTAV[i-1];
		else
			last = &f5->DS5MAVET[i-8];

		for (j=i; j<26; j++) {
			if (j==0)
				ext = &f5->DS5AVEXT;
			else if ((j > 0) && (j < 8))
				ext = &f5->DS5EXTAV[j-1];
			else
				ext = &f5->DS5MAVET[j-8];

			if (((ext->t > 0) && (last->t == 0)) ||
			    ((ext->t > 0) && (ext->t < last->t)))
			{
				tmp.t  = last->t;
				tmp.fc = last->fc;
				tmp.ft = last->ft;
				last->t  = ext->t;
				last->fc = ext->fc;
				last->ft = ext->ft;
				ext->t  = tmp.t;
				ext->fc = tmp.fc;
				ext->ft = tmp.ft;
			}
		}
	}
}

/*
 * add a free space extent description to the VTOC FMT5 DSCB
 */
void
vtoc_update_format5_label_add (format5_label_t *f5, int verbose, int cyl,
                               int trk, u_int16_t a, u_int16_t b, u_int8_t c)
{
	PDEBUG
	ds5ext_t *ext = NULL, *tmp = NULL;
	int i;

	for (i=0; i<26; i++) {
		if (i==0)
			ext = &f5->DS5AVEXT;
		else if ((i > 0) && (i < 8))
			ext = &f5->DS5EXTAV[i-1];
		else
			ext = &f5->DS5MAVET[i-8];

		if (((a < ext->t) && (a + b*trk + c > ext->t)) ||
		    ((a > ext->t) && (ext->t + ext->fc*trk + ext->ft > a)))
		{
			puts ("BUG: overlapping free space extents "
			      "in FMT5 DSCB!\nexiting...");
			exit(EXIT_FAILURE);
		}

		if ((ext->t + ext->fc + ext->ft) == 0x0000) {
			ext->t  = a;
			ext->fc = b;
			ext->ft = c;
			tmp = ext;
			if (verbose)
                                puts ("FMT5 add extent: add new extent");
			break;
		}
	}

	if (tmp == NULL) {
		/* BUG: no free extent found */
		puts ("BUG: no free FMT5 DSCB extent found!\nexiting...");
		exit(EXIT_FAILURE);
	}

	for (i=0; i<26; i++) {
		if (i==0)
			ext = &f5->DS5AVEXT;
		else if ((i > 0) && (i < 8))
			ext = &f5->DS5EXTAV[i-1];
		else
			ext = &f5->DS5MAVET[i-8];

		if ((ext->t + ext->fc + ext->ft) == 0x0000)
			continue;

		if ((ext->t + ext->fc*trk + ext->ft) == tmp->t) {
			/* this extent precedes the new one */
			ext->fc += (tmp->fc + (tmp->ft + ext->ft)/trk);
			ext->ft = (tmp->ft + ext->ft) % trk;
			bzero(tmp, sizeof(ds5ext_t));
			tmp = ext;

			if (verbose)
				puts ("FMT5 add extent: "
                                      "merge with predecessor");

			i = -1;
			continue;
		}

		if ((tmp->t + tmp->fc*trk + tmp->ft) == ext->t) {
			/* this extent succeeds the new one */
			ext->t = tmp->t;
			ext->fc += (tmp->fc + (tmp->ft + ext->ft)/trk);
			ext->ft = (tmp->ft + ext->ft) % trk;
			bzero(tmp, sizeof(ds5ext_t));
			tmp = ext;

			if (verbose)
				puts ("FMT5 add extent: "
				      "merge with successor");

			i = -1;
			continue;
		}
	}
}

/*
 * remove a free space extent description from the VTOC FMT5 DSCB
 */
void
vtoc_update_format5_label_del (format5_label_t *f5, int verbose, int cyl,
                               int trk, u_int16_t a, u_int16_t b, u_int8_t c)
{
	PDEBUG
	ds5ext_t *ext;
	int i, counter=0;

	for (i=0; i<26; i++) {
		if (i==0)
			ext = &f5->DS5AVEXT;
		else if ((i > 0) && (i < 8))
			ext = &f5->DS5EXTAV[i-1];
		else
			ext = &f5->DS5MAVET[i-8];

		if ((a == ext->t) && (b == ext->fc) && (c == ext->ft)) {
			/* fills up whole free space gap */
			bzero(ext, sizeof(ds5ext_t));

			if (verbose)
				puts ("FMT5 del extent: fills whole gap");

			counter++;
			break;
		}

		if ((a == ext->t) && ((b < ext->fc) || (c < ext->ft))) {
			/* left-bounded in free space gap */
			ext->t = ext->t + b*trk + c;

			if (c > ext->ft) {
				ext->fc -= (b + 1);
				ext->ft -= (c - trk);
			} else {
				ext->fc -= b;
				ext->ft -= c;
			}

			if (verbose)
				puts ("FMT5 del extent: left bounded");

			counter++;
			break;
		}

		if ((ext->t < a)
		    && ((ext->t + ext->fc*trk + ext->ft) == (a + b*trk + c)))
		{
			/* right-bounded in free space gap */
			if (c > ext->ft) {
				ext->fc -= (b + 1);
				ext->ft -= (c - trk);
			} else {
				ext->fc -= b;
				ext->ft -= c;
			}

			if (verbose)
				puts ("FMT5 del extent: right bounded");

			counter++;
			break;
		}

		if ((a > ext->t)
			&& ((ext->t + ext->fc*trk + ext->ft) > (a + b*trk + c)))
		{
			/* partition devides free space into 2 pieces */
			u_int16_t x = a + b*trk + c;
			u_int16_t w,y;
			u_int8_t z;

			w = (ext->t + ext->fc*trk + ext->ft) - (a + b*trk + c);
			y = w / trk;
			z = w % trk;

			ext->fc = (a - ext->t) / trk;
			ext->ft = (a - ext->t) % trk;

			vtoc_update_format5_label_add(f5, verbose,
						      cyl, trk, x, y, z);

			if (verbose)
				puts ("FMT5 del extent: 2 pieces");

			counter++;
			break;
		}

		if ((a < ext->t) && (a + b*trk + c > ext->t)
		    && (a + b*trk + c < ext->t + ext->fc*trk + ext->ft))
		{
			puts ("BUG: corresponding free space extent "
			      "doesn't match free space currently shown "
			      "in FMT5 DSCB!\nexiting...");
			exit(EXIT_FAILURE);
		}

		if ((a > ext->t) && (a < ext->t + ext->fc*trk + ext->ft)
		    && (a + b*trk + c > ext->t + ext->fc*trk + ext->ft))
		{
			puts ("BUG: specified free space extent for "
			      "deleting doesn't match free space "
			      "currently shown in FMT5 DSCB!\n"
			      "exiting...");
			exit(EXIT_FAILURE);
		}
	}

	if (counter > 0)
		return;

	puts ("BUG: specified free space extent for "
	      "deleting not found in FMT5 DSCB!\n"
	      "exiting...");
	exit(EXIT_FAILURE);
}

/*
 * reorganizes all extents within a FMT7 label
 */
static void
vtoc_reorganize_FMT7_extents (format7_label_t *f7)
{
	PDEBUG
	ds7ext_t *ext, *last, tmp;
	int i, j;

	for (i=0; i<16; i++) {
		if (i<5)
			last = &f7->DS7EXTNT[i];
		else
			last = &f7->DS7ADEXT[i-5];

		for (j=i; j<16; j++) {
			if (j<5)
				ext = &f7->DS7EXTNT[j];
			else
				ext = &f7->DS7ADEXT[j-5];

			if (((ext->a > 0) && (last->a == 0))
			    || ((ext->a > 0) && (ext->a < last->a)))
			{
				tmp.a = last->a;
				tmp.b = last->b;
				last->a = ext->a;
				last->b = ext->b;
				ext->a = tmp.a;
				ext->b = tmp.b;
			}
		}
	}
}

/*
 * add a free space extent description to the VTOC FMT7 DSCB
 */
void
vtoc_update_format7_label_add (format7_label_t *f7, int verbose,
                               u_int32_t a, u_int32_t b)
{
	PDEBUG
	ds7ext_t *ext = NULL, *tmp = NULL;
	int i;

	for (i=0; i<16; i++) {
		if (i<5)
			ext = &f7->DS7EXTNT[i];
		else
			ext = &f7->DS7ADEXT[i-5];

		if (((a < ext->a) && (b > ext->a) && (b < ext->b))
		    || ((a > ext->a) && (a < ext->b) && (b > ext->b)))
		{
			puts ("BUG: overlapping free space extents "
			      "in FMT7 DSCB!\nexiting...");
			exit(EXIT_FAILURE);
		}

		if ((ext->a + ext->b) == 0x00000000) {
			ext->a = a;
			ext->b = b;
			tmp = ext;

			if (verbose)
				puts ("FMT7 add extent: add new extent");

			break;
		}
	}

	if (tmp == NULL) {
		/* BUG: no free extent found */
		puts ("BUG: no free FMT7 DSCB extent found!\nexiting...");
		exit(EXIT_FAILURE);
	}

	for (i=0; i<16; i++) {
		if (i<5)
			ext = &f7->DS7EXTNT[i];
		else
			ext = &f7->DS7ADEXT[i-5];

		if ((ext->a + ext->b) == 0x00000000)
			continue;

		if ((ext->b + 1) == tmp->a) {
			/* this extent precedes the new one */
			ext->b = tmp->b;
			bzero(tmp, sizeof(ds7ext_t));
			tmp = ext;

			if (verbose)
                                puts ("FMT7 add extent: "
                                      "merge with predecessor");

			i = -1;
			continue;
		}

		if (ext->a == (tmp->b + 1)) {
			/* this extent succeeds the new one */
			ext->a = tmp->a;
			bzero(tmp, sizeof(ds7ext_t));
			tmp = ext;

			if (verbose)
				puts ("FMT7 add extent: merge with successor");

			i = -1;
			continue;
		}
	}
}

/*
 * remove a free space extent description from the VTOC FMT7 DSCB
 */
void
vtoc_update_format7_label_del (format7_label_t *f7, int verbose,
                               u_int32_t a, u_int32_t b)
{
	PDEBUG
	ds7ext_t *ext;
	int i, counter=0;

	for (i=0; i<16; i++) {
		if (i<5)
			ext = &f7->DS7EXTNT[i];
		else
			ext = &f7->DS7ADEXT[i-5];

		if ((a == ext->a) && (b == ext->b)) {
			/* fills up whole free space gap */
			bzero(ext, sizeof(ds7ext_t));

			if (verbose)
				puts ("FMT7 del extent: fills whole gap");

			counter++;
			break;
		}

		if ((a == ext->a) && (b < ext->b)) {
			/* left-bounded in free space gap */
			ext->a = b + 1;

			if (verbose)
				puts ("FMT7 add extent: left-bounded");

			counter++;
			break;
		}

		if ((a > ext->a) && (b == ext->b)) {
			/* right-bounded in free space gap */
			ext->b = a - 1;

			if (verbose)
				puts ("FMT7 add extent: right-bounded");

			counter++;
			break;
		}

		if ((a > ext->a) && (b < ext->b)) {
			/* partition devides free space into 2 pieces */
			vtoc_update_format7_label_add(f7, verbose, b+1, ext->b);
			ext->b = a - 1;

			if (verbose)
				puts ("FMT7 add extent: 2 pieces");

			counter++;
			break;
		}

		if (((a < ext->a) && (b > ext->a)) || ((a < ext->b) && (b > ext->b))) {
                        puts ("BUG: specified free space extent for deleting "
                              "doesn't match free space currently shown in "
                              "FMT7 DSCB!\nexiting...");
			printf ("%d %d %d %d\n", a, b, ext->a, ext->b);
			exit(EXIT_FAILURE);
		}
	}

	if (counter > 0)
		return;

	puts ("BUG: specified free space extent for "
	      "deleting not found in FMT7 DSCB!\n"
	      "exiting...");
	exit(EXIT_FAILURE);
}

void
vtoc_set_freespace(format4_label_t *f4, format5_label_t *f5,
                   format7_label_t *f7, char ch, int verbose,
                   u_int32_t start, u_int32_t stop, int cyl, int trk)
{
	PDEBUG
	if ((cyl * trk) > BIG_DISK_SIZE) {
		if (ch == '+')
			vtoc_update_format7_label_add(f7, verbose, start, stop);
		else if (ch == '-')
			vtoc_update_format7_label_del(f7, verbose, start, stop);
		else
			puts ("BUG: syntax error in vtoc_set_freespace call");

		vtoc_reorganize_FMT7_extents (f7);

		f4->DS4VTOCI = 0xa0;
		f4->DS4EFLVL = 0x07;
		vtoc_set_cchhb(&f4->DS4EFPTR, 0x0000, 0x0001, 0x03);
	} else {
		u_int16_t x,y;
		u_int8_t z;

		x = (u_int16_t) start;
		y = (u_int16_t) ((stop - start + 1) / trk);
		z =  (u_int8_t) ((stop - start + 1) % trk);

		if (ch == '+')
			vtoc_update_format5_label_add(f5, verbose, cyl, trk, x, y, z);
		else if (ch == '-')
			vtoc_update_format5_label_del(f5, verbose, cyl, trk, x, y, z);
		else
			puts ("BUG: syntax error in vtoc_set_freespace call");

		vtoc_reorganize_FMT5_extents (f5);
	}
}
