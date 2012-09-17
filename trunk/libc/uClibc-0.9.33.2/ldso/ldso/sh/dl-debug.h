/* vi: set sw=4 ts=4: */
/* SuperH ELF shared library loader suppport
 *
 * Copyright (C) 2002, Stefan Allius <allius@atecom.com> and
 *                     Eddie C. Dost <ecd@atecom.com>
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. The name of the above contributors may not be
 *    used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

static const char * const _dl_reltypes_tab[] =
{
  [0]	"R_SH_NONE",	"R_SH_DIR32",	"R_SH_REL32",	"R_SH_DIR8WPN",
  [4]	"R_SH_IND12W",	"R_SH_DIR8WPL",	"R_SH_DIR8WPZ",	"R_SH_DIR8BP",
  [8]	"R_SH_DIR8W",	"R_SH_DIR8L",
 [25]	"R_SH_SWITCH16","R_SH_SWITCH32","R_SH_USES",
 [28]	"R_SH_COUNT",	"R_SH_ALIGN",	"R_SH_CODE",	"R_SH_DATA",
 [32]	"R_SH_LABEL",	"R_SH_SWITCH8",	"R_SH_GNU_VTINHERIT","R_SH_GNU_VTENTRY",
[144]	"R_SH_TLS_GD_32","R_SH_TLS_LD_32", "R_SH_TLS_LDO_32", "R_SH_TLS_IE_32", 
[148]	"R_SH_TLS_LE_32","R_SH_TLS_DTPMOD32", "R_SH_TLS_DTPOFF32", "R_SH_TLS_TPOFF32",
[160]	"R_SH_GOT32",	"R_SH_PLT32",	"R_SH_COPY",	"R_SH_GLOB_DAT",
[164]	"R_SH_JMP_SLOT","R_SH_RELATIVE","R_SH_GOTOFF",	"R_SH_GOTPC",
};
