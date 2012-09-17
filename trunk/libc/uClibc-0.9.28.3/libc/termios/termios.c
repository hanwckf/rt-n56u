/* Copyright (C) 1992, 1993, 1996, 1997, 1998 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the GNU C Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  
 

   About the only thing remaining here fromthe original Linux-8086 C library
   version by Robert de Bath <robert@mayday.compulink.co.uk>, is the general
   layout.  All else has been recently stolen from GNU libc, since that was
   much more current.
 */

#include <errno.h>
#include <stddef.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <unistd.h>
#include <termios.h>

#ifdef L_isatty
/* Return 1 if FD is a terminal, 0 if not.  */

int isatty(int fd)
{
    struct termios term;
    return (tcgetattr (fd, &term) == 0);
}
#endif

#ifdef L_tcdrain
/* Wait for pending output to be written on FD.  */
int __libc_tcdrain (int fd)
{
      /* With an argument of 1, TCSBRK waits for the output to drain.  */
      return ioctl(fd, TCSBRK, 1);
}
weak_alias(__libc_tcdrain, tcdrain)
#endif

#ifdef L_tcflow
/* Suspend or restart transmission on FD.  */
int tcflow ( int fd, int action)
{
      return ioctl(fd, TCXONC, action);
}
#endif

#ifdef L_tcflush
/* Flush pending data on FD.  */
int tcflush ( int fd, int queue_selector)
{
      return ioctl(fd, TCFLSH, queue_selector);
}
#endif

#ifdef L_tcsendbreak
/* Send zero bits on FD.  */
int tcsendbreak( int fd, int duration)
{
	/* The break lasts 0.25 to 0.5 seconds if DURATION is zero,
	   and an implementation-defined period if DURATION is nonzero.
	   We define a positive DURATION to be number of milliseconds to break.  */
	if (duration <= 0)
		return ioctl(fd, TCSBRK, 0);

#ifdef TCSBRKP
	/* Probably Linux-specific: a positive third TCSBRKP ioctl argument is
	   defined to be the number of 100ms units to break.  */
	return ioctl(fd, TCSBRKP, (duration + 99) / 100);
#else
	/* ioctl can't send a break of any other duration for us.
	   This could be changed to use trickery (e.g. lower speed and
	   send a '\0') to send the break, but for now just return an error.  */
	__set_errno (EINVAL);
	return -1;
#endif
}
#endif

#ifdef L_tcsetpgrp
/* Set the foreground process group ID of FD set PGRP_ID.  */
int tcsetpgrp ( int fd, pid_t pgrp_id)
{
      return ioctl (fd, TIOCSPGRP, &pgrp_id);
}
#endif

#ifdef L_tcgetpgrp
/* Return the foreground process group ID of FD.  */
pid_t tcgetpgrp ( int fd)
{
    int pgrp;

    if (ioctl (fd, TIOCGPGRP, &pgrp) < 0)
	return (pid_t) -1;
    return (pid_t) pgrp;
}
#endif

/* This is a gross hack around a kernel bug.  If the cfsetispeed functions is
 * called with the SPEED argument set to zero this means use the same speed as
 * for output.  But we don't have independent input and output speeds and
 * therefore cannot record this.
 *
 * We use an unused bit in the `c_iflag' field to keep track of this use of
 * `cfsetispeed'.  The value here must correspond to the one used in
 * `tcsetattr.c'.  */
#define IBAUD0  020000000000

#ifdef L_cfgetospeed
/* Return the output baud rate stored in *TERMIOS_P.  */
speed_t cfgetospeed ( const struct termios *termios_p)
{
      return termios_p->c_cflag & (CBAUD | CBAUDEX);
}
#endif

#ifdef L_cfgetispeed

/* Return the input baud rate stored in *TERMIOS_P.
 * Although for Linux there is no difference between input and output
 * speed, the numerical 0 is a special case for the input baud rate. It
 * should set the input baud rate to the output baud rate. */
speed_t cfgetispeed (const struct termios *termios_p)
{
    return ((termios_p->c_iflag & IBAUD0)
	    ? 0 : termios_p->c_cflag & (CBAUD | CBAUDEX));
}
#endif

#ifdef L_cfsetospeed
/* Set the output baud rate stored in *TERMIOS_P to SPEED.  */
int cfsetospeed  (struct termios *termios_p, speed_t speed)
{
    if ((speed & ~CBAUD) != 0
	    && (speed < B57600 || speed > B460800))
    {
	__set_errno(EINVAL);
	return -1;
    }

    termios_p->c_cflag &= ~(CBAUD | CBAUDEX);
    termios_p->c_cflag |= speed;

    return 0;
}
#endif

#ifdef L_cfsetispeed
/* Set the input baud rate stored in *TERMIOS_P to SPEED.
 *    Although for Linux there is no difference between input and output
 *       speed, the numerical 0 is a special case for the input baud rate.  It
 *          should set the input baud rate to the output baud rate.  */
int cfsetispeed ( struct termios *termios_p, speed_t speed)
{
    if ((speed & ~CBAUD) != 0
	    && (speed < B57600 || speed > B460800))
    {
	__set_errno(EINVAL);
	return -1;
    }

    if (speed == 0)
	termios_p->c_iflag |= IBAUD0;
    else
    {
	termios_p->c_iflag &= ~IBAUD0;
	termios_p->c_cflag &= ~(CBAUD | CBAUDEX);
	termios_p->c_cflag |= speed;
    }

    return 0;
}
#endif

#ifdef L_cfsetspeed
struct speed_struct
{
  speed_t value;
  speed_t internal;
};

static const struct speed_struct speeds[] =
  {
#ifdef B0
    { 0, B0 },
#endif
#ifdef B50
    { 50, B50 },
#endif
#ifdef B75
    { 75, B75 },
#endif
#ifdef B110
    { 110, B110 },
#endif
#ifdef B134
    { 134, B134 },
#endif
#ifdef B150
    { 150, B150 },
#endif
#ifdef B200
    { 200, B200 },
#endif
#ifdef B300
    { 300, B300 },
#endif
#ifdef B600
    { 600, B600 },
#endif
#ifdef B1200
    { 1200, B1200 },
#endif
#ifdef B1200
    { 1200, B1200 },
#endif
#ifdef B1800
    { 1800, B1800 },
#endif
#ifdef B2400
    { 2400, B2400 },
#endif
#ifdef B4800
    { 4800, B4800 },
#endif
#ifdef B9600
    { 9600, B9600 },
#endif
#ifdef B19200
    { 19200, B19200 },
#endif
#ifdef B38400
    { 38400, B38400 },
#endif
#ifdef B57600
    { 57600, B57600 },
#endif
#ifdef B76800
    { 76800, B76800 },
#endif
#ifdef B115200
    { 115200, B115200 },
#endif
#ifdef B153600
    { 153600, B153600 },
#endif
#ifdef B230400
    { 230400, B230400 },
#endif
#ifdef B307200
    { 307200, B307200 },
#endif
#ifdef B460800
    { 460800, B460800 },
#endif
  };


/* Set both the input and output baud rates stored in *TERMIOS_P to SPEED.  */
int cfsetspeed (struct termios *termios_p, speed_t speed)
{
  size_t cnt;

  for (cnt = 0; cnt < sizeof (speeds) / sizeof (speeds[0]); ++cnt)
    if (speed == speeds[cnt].internal)
      {
	cfsetispeed (termios_p, speed);
	cfsetospeed (termios_p, speed);
	return 0;
      }
    else if (speed == speeds[cnt].value)
      {
	cfsetispeed (termios_p, speeds[cnt].internal);
	cfsetospeed (termios_p, speeds[cnt].internal);
	return 0;
      }

  __set_errno (EINVAL);

  return -1;
}
#endif

#ifdef L_cfmakeraw
/* Copyright (C) 1992, 1996, 1997 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
*/
#include <termios.h>

/* Set *T to indicate raw mode.  */
void
cfmakeraw (struct termios *t)
{
  t->c_iflag &= ~(IGNBRK|BRKINT|PARMRK|ISTRIP|INLCR|IGNCR|ICRNL|IXON);
  t->c_oflag &= ~OPOST;
  t->c_lflag &= ~(ECHO|ECHONL|ICANON|ISIG|IEXTEN);
  t->c_cflag &= ~(CSIZE|PARENB);
  t->c_cflag |= CS8;
  t->c_cc[VMIN] = 1;		/* read returns when one char is available.  */
  t->c_cc[VTIME] = 0;
}
#endif

