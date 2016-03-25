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
#include <linux/spinlock.h>
#include <asm/io.h>

#include <asm/rt2880/rt_mmap.h>
#include <asm/rt2880/rt_serial.h>

static DEFINE_SPINLOCK(con_lock);
static char buf[256];
static unsigned int uart_base;

static inline unsigned int serial_in(int offset)
{
	return inl(uart_base + offset);
}

static inline void serial_out(int offset, int value)
{
	outl(value, uart_base + offset);
}

#if defined (CONFIG_EARLY_PRINTK)
void prom_putchar(unsigned char c)
{
	while ((serial_in(UART_LSR) & UART_LSR_THRE) == 0)
		;

	serial_out(UART_TX, c);
}
#endif

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

void __init prom_init_printf(int tty_no)
{
#if !defined (CONFIG_RALINK_GPIOMODE_UARTF) && (CONFIG_SERIAL_8250_NR_UARTS > 1)
	if (tty_no == 1)
		uart_base = RALINK_UART_BASE;
	else
#endif
		uart_base = RALINK_UART_LITE_BASE;
}

void __init prom_printf(char *fmt, ...)
{
	va_list args;
	int l;
	char *p, *buf_end;
	unsigned long flags;

	spin_lock_irqsave(&con_lock, flags);
	va_start(args, fmt);
	l = vsnprintf(buf, sizeof(buf), fmt, args);
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
