;   libparted - a library for manipulating disk partitions
;   Copyright (C) 1999-2000, 2007, 2009-2012 Free Software Foundation, Inc.
;
;   This program is free software; you can redistribute it and/or modify
;   it under the terms of the GNU General Public License as published by
;   the Free Software Foundation; either version 3 of the License, or
;   (at your option) any later version.
;
;   This program is distributed in the hope that it will be useful,
;   but WITHOUT ANY WARRANTY; without even the implied warranty of
;   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;   GNU General Public License for more details.
;
;   You should have received a copy of the GNU General Public License
;   along with this program.  If not, see <http://www.gnu.org/licenses/>.

; NOTE: I build this with:
;	$ as86 -b /dev/stdout mbr.s | hexdump -e '8/1 "0x%02x, " "\n"'
;
; The build isn't done automagically by make, because as86 may not be on many
; machines (particularly non-x86).  Also, it seems rather difficult to get
; as86 to build object files that can be linked, especially as it's 16 bit
; code...

USE16

; This code, plus the partition table is loaded into 0000:7C00 by the BIOS

.text

; set top of stack to 1000:B000

	cli

	mov	ax, #0x1000
	mov	ss, ax
	mov	sp, #0xB000

	mov	ax, #0x0000
	mov	ds, ax
	mov	es, ax

	sti

; Copy what the BIOS loaded (i.e. the MBR + head of partition table) from
; 0000:7c00 to 0000:0600

	mov	si, #0x7c00
	mov	di, #0x0600
	mov	cx, #0x200
	rep
	movsb

; Jump to the copy of the MBR

	jmp	0x0000:find_boot_partition + 0x0600

find_boot_partition:
	mov	si, #0x07BE

check_next_bootable:
	cmp	[si], al
	jnz	found_bootable
	add	si, #0x0010
	cmp	si, #0x07FE
	jnz	check_next_bootable
	jmp	error

found_bootable:

; Load in the boot sector at 0000:7c00

	mov	ah, #2			; BIOS command (read)
	mov	al, #1			; count
	mov	bx, #0x7c00		; destination pointer
	mov	dl, #0x80		; drive
	mov	dh, byte ptr [si + 1]	; head
	mov	cx, word ptr [si + 2]	; sector / cylinder
	int	#0x13			; BIOS read interrupt

	jmp	0x0000:0x7c00		; hand control to boot sector

error:
	jmp	error
