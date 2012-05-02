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
#ifndef _DISK_SWAP_
#define _DISK_SWAP_

#define SWAP_MIN 1
#define SWAP_MAX 1024*1024

int start_swap(const char *, const int);
int stop_swap_from_proc();
int stop_swap(const char *);
int run_swap(const int);
int swap_check();
extern int do_swap_for_format(const char *);

#endif // _DISK_SWAP_
