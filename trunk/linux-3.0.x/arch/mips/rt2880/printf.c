/**************************************************************************
 *
 *  BRIEF MODULE DESCRIPTION
 *     prom_printf for Ralink RT2880 solution
 *
 *  Copyright 2007 Ralink Inc. (bruce_chang@ralinktech.com.tw)
 *
 *  This program is free software; you can redistribute  it and/or modify it
 *  under  the terms of  the GNU General  Public License as published by the
 *  Free Software Foundation;  either version 2 of the  License, or (at your
 *  option) any later version.
 *
 *  THIS  SOFTWARE  IS PROVIDED   ``AS  IS'' AND   ANY  EXPRESS OR IMPLIED
 *  WARRANTIES,   INCLUDING, BUT NOT  LIMITED  TO, THE IMPLIED WARRANTIES OF
 *  MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN
 *  NO  EVENT  SHALL   THE AUTHOR  BE    LIABLE FOR ANY   DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 *  NOT LIMITED   TO, PROCUREMENT OF  SUBSTITUTE GOODS  OR SERVICES; LOSS OF
 *  USE, DATA,  OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 *  ANY THEORY OF LIABILITY, WHETHER IN  CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 *  THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *  You should have received a copy of the  GNU General Public License along
 *  with this program; if not, write  to the Free Software Foundation, Inc.,
 *  675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *
 **************************************************************************
 * May 2007 Bruce Chang
 *
 * Initial Release
 *
 *
 *
 **************************************************************************
 */

#include <linux/init.h>
#include <linux/kernel.h>
//#include <linux/serial_reg.h>
#include <linux/spinlock.h>
#include <asm/io.h>
#include "serial_rt2880.h"

#include <asm/rt2880/surfboard.h>
#include <asm/rt2880/rt_mmap.h>

static unsigned int uart_base = RALINK_UART_LITE_BASE;


static inline unsigned int serial_in(int offset)
{
	//return inb(PORT(offset));
//	return inb(uart_base + offset);
	return inl( uart_base + offset);
}

static inline void serial_out(int offset, int value)
{
	//outb(value, PORT(offset));
	//outb(value, uart_base + offset);
	outl(value,  uart_base + offset);
}

int putPromChar(char c)
{
	while ((serial_in(UART_LSR) & UART_LSR_THRE) == 0)
		;

	serial_out(UART_TX, c);

	return 1;
}

char getPromChar(void)
{
	while (!(serial_in(UART_LSR) & 1))
		;

	return serial_in(UART_RX);
}

void __init prom_setup_printf(int tty_no)
{
	if (tty_no == 1)
		uart_base = RALINK_UART_LITE_BASE;
	else	/* Default = ttys0 */
		uart_base = RALINK_UART_BASE;
}

static DEFINE_SPINLOCK(con_lock);

static char buf[1024];

/* NOTE:  must call prom_setup_printf before using this function */
void __init prom_printf(char *fmt, ...)
{
	va_list args;
	int l;
	char *p, *buf_end;
	unsigned long flags;

	int putPromChar(char);

	spin_lock_irqsave(&con_lock, flags);
	va_start(args, fmt);
	l = vsprintf(buf, fmt, args); /* hopefully i < sizeof(buf) */
	va_end(args);

	buf_end = buf + l;

	for (p = buf; p < buf_end; p++) {
		/* Crude cr/nl handling is better than none */
		if (*p == '\n')
			putPromChar('\r');
		putPromChar(*p);
	}
	spin_unlock_irqrestore(&con_lock, flags);
}
