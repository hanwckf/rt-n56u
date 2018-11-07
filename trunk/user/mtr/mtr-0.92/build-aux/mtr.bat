@echo off
rem
rem  mtr  --  a network diagnostic tool
rem  Copyright (C) 2016  Matt Kimball
rem
rem  This program is free software; you can redistribute it and/or modify
rem  it under the terms of the GNU General Public License version 2 as
rem  published by the Free Software Foundation.
rem
rem  This program is distributed in the hope that it will be useful,
rem  but WITHOUT ANY WARRANTY; without even the implied warranty of
rem  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
rem  GNU General Public License for more details.
rem
rem  You should have received a copy of the GNU General Public License
rem  along with this program; if not, write to the Free Software
rem  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
rem

rem Assume the path of this batch file is the mtr installation location
set "MTR_DIR=%~dp0"

set "MTR_BIN=%MTR_DIR%\bin"

rem ncurses needs to locate the cygwin terminfo file
set "TERMINFO=%MTR_DIR%\terminfo"

rem mtr needs to know the location to the packet generator
set "MTR_PACKET=%MTR_BIN%\mtr-packet.exe"

rem Pass along commandline arguments
"%MTR_BIN%\mtr" %*
