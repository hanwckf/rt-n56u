

/*
 *	.......... F U N C T I O N S ..........
 */

/* ................... Begin of alaw_compress() ..................... */
/*
  ==========================================================================

   FUNCTION NAME: alaw_compress

   DESCRIPTION: ALaw encoding rule according ITU-T Rec. G.711.

   PROTOTYPE: void alaw_compress(long lseg, short *linbuf, short *logbuf)

   PARAMETERS:
     lseg:	(In)  number of samples
     linbuf:	(In)  buffer with linear samples (only 12 MSBits are taken
                      into account)
     logbuf:	(Out) buffer with compressed samples (8 bit right justified,
                      without sign extension)

   RETURN VALUE: none.

   HISTORY:
   10.Dec.91	1.0	Separated A-law compression function

  ==========================================================================
*/
void            alaw_compress(lseg, linbuf, logbuf)
  long            lseg;
  short          *linbuf;
  short          *logbuf;
{
  short           ix, iexp;
  long            n;

  for (n = 0; n < lseg; n++)
  {
    ix = linbuf[n] < 0		/* 0 <= ix < 2048 */
      ? (~linbuf[n]) >> 4	/* 1's complement for negative values */
      : (linbuf[n]) >> 4;

    /* Do more, if exponent > 0 */
    if (ix > 15)		/* exponent=0 for ix <= 15 */
    {
      iexp = 1;			/* first step: */
      while (ix > 16 + 15)	/* find mantissa and exponent */
      {
	ix >>= 1;
	iexp++;
      }
      ix -= 16;			/* second step: remove leading '1' */

      ix += iexp << 4;		/* now compute encoded value */
    }
    if (linbuf[n] >= 0)
      ix |= (0x0080);		/* add sign bit */

    logbuf[n] = ix ^ (0x0055);	/* toggle even bits */
  }
}
/* ................... End of alaw_compress() ..................... */


/* ................... Begin of alaw_expand() ..................... */
/*
  ==========================================================================

   FUNCTION NAME: alaw_expand

   DESCRIPTION: ALaw decoding rule according ITU-T Rec. G.711.

   PROTOTYPE: void alaw_expand(long lseg, short *logbuf, short *linbuf)

   PARAMETERS:
     lseg:	(In)  number of samples
     logbuf:	(In)  buffer with compressed samples (8 bit right justified,
                      without sign extension)
     linbuf:	(Out) buffer with linear samples (13 bits left justified)

   RETURN VALUE: none.

   HISTORY:
   10.Dec.91	1.0	Separated A-law expansion function

  ============================================================================
*/
void            alaw_expand(lseg, logbuf, linbuf)
  long            lseg;
  short          *linbuf;
  short          *logbuf;
{
  short           ix, mant, iexp;
  long            n;

  for (n = 0; n < lseg; n++)
  {
    ix = logbuf[n] ^ (0x0055);	/* re-toggle toggled bits */

    ix &= (0x007F);		/* remove sign bit */
    iexp = ix >> 4;		/* extract exponent */
    mant = ix & (0x000F);	/* now get mantissa */
    if (iexp > 0)
      mant = mant + 16;		/* add leading '1', if exponent > 0 */

    mant = (mant << 4) + (0x0008);	/* now mantissa left justified and */
    /* 1/2 quantization step added */
    if (iexp > 1)		/* now left shift according exponent */
      mant = mant << (iexp - 1);

    linbuf[n] = logbuf[n] > 127	/* invert, if negative sample */
      ? mant
      : -mant;
  }
}
/* ................... End of alaw_expand() ..................... */

/* ................... Begin of ulaw_compress() ..................... */
/*
  ==========================================================================

   FUNCTION NAME: ulaw_compress

   DESCRIPTION: Mu law encoding rule according ITU-T Rec. G.711.

   PROTOTYPE: void ulaw_compress(long lseg, short *linbuf, short *logbuf)

   PARAMETERS:
     lseg:	(In)  number of samples
     linbuf:	(In)  buffer with linear samples (only 12 MSBits are taken
                      into account)
     logbuf:	(Out) buffer with compressed samples (8 bit right justified,
                      without sign extension)

   RETURN VALUE: none.

   HISTORY:
   10.Dec.91	1.0	Separated mu-law compression function

  ==========================================================================
*/
void            ulaw_compress(lseg, linbuf, logbuf)
  long            lseg;
  short          *linbuf;
  short          *logbuf;
{
  long            n;            /* samples's count */
  short           i;		/* aux.var. */
  short           absno;	/* absolute value of linear (input) sample */
  short           segno;	/* segment (Table 2/G711, column 1) */
  short           low_nibble;	/* low  nibble of log companded sample */
  short           high_nibble;	/* high nibble of log companded sample */

  for (n = 0; n < lseg; n++)
  {
    /* -------------------------------------------------------------------- */
    /* Change from 14 bit left justified to 14 bit right justified */
    /* Compute absolute value; adjust for easy processing */
    /* -------------------------------------------------------------------- */
    absno = linbuf[n] < 0	/* compute 1's complement in case of  */
      ? ((~linbuf[n]) >> 2) + 33/* negative samples */
      : ((linbuf[n]) >> 2) + 33;/* NB: 33 is the difference value */
    /* between the thresholds for */
    /* A-law and u-law. */
    if (absno > (0x1FFF))	/* limitation to "absno" < 8192 */
      absno = (0x1FFF);

    /* Determination of sample's segment */
    i = absno >> 6;
    segno = 1;
    while (i != 0)
    {
      segno++;
      i >>= 1;
    }

    /* Mounting the high-nibble of the log-PCM sample */
    high_nibble = (0x0008) - segno;

    /* Mounting the low-nibble of the log PCM sample */
    low_nibble = (absno >> segno)	/* right shift of mantissa and */
      & (0x000F);		/* masking away leading '1' */
    low_nibble = (0x000F) - low_nibble;

    /* Joining the high-nibble and the low-nibble of the log PCM sample */
    logbuf[n] = (high_nibble << 4) | low_nibble;

    /* Add sign bit */
    if (linbuf[n] >= 0)
      logbuf[n] = logbuf[n] | (0x0080);
  }
}
/* ................... End of ulaw_compress() ..................... */



/* ................... Begin of ulaw_expand() ..................... */
/*
  ==========================================================================

   FUNCTION NAME: ulaw_expand

   DESCRIPTION: Mu law decoding rule according ITU-T Rec. G.711.

   PROTOTYPE: void ulaw_expand(long lseg, short *logbuf, short *linbuf)

   PARAMETERS:
     lseg:	(In)  number of samples
     logbuf:	(In)  buffer with compressed samples (8 bit right justified,
                      without sign extension)
     linbuf:	(Out) buffer with linear samples (14 bits left justified)

   RETURN VALUE: none.

   HISTORY:
   10.Dec.91	1.0	Separated mu law expansion function

  ============================================================================
*/

void            ulaw_expand(lseg, logbuf, linbuf)
  long            lseg;
  short          *logbuf;
  short          *linbuf;
{
  long            n;		/* aux.var. */
  short           segment;	/* segment (Table 2/G711, column 1) */
  short           mantissa;	/* low  nibble of log companded sample */
  short           exponent;	/* high nibble of log companded sample */
  short           sign;		/* sign of output sample */
  short           step;

  for (n = 0; n < lseg; n++)
  {
    sign = logbuf[n] < (0x0080)	/* sign-bit = 1 for positiv values */
      ? -1
      : 1;
    mantissa = ~logbuf[n];	/* 1's complement of input value */
    exponent = (mantissa >> 4) & (0x0007);	/* extract exponent */
    segment = exponent + 1;	/* compute segment number */
    mantissa = mantissa & (0x000F);	/* extract mantissa */

    /* Compute Quantized Sample (14 bit left justified!) */
    step = (4) << segment;	/* position of the LSB */
    /* = 1 quantization step) */
    linbuf[n] = sign *		/* sign */
      (((0x0080) << exponent)	/* '1', preceding the mantissa */
       + step * mantissa	/* left shift of mantissa */
       + step / 2		/* 1/2 quantization step */
       - 4 * 33
      );
  }
}
/* ................... End of ulaw_expand() ..................... */