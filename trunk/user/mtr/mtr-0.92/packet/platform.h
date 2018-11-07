/*
    mtr  --  a network diagnostic tool
    Copyright (C) 2016  Matt Kimball

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License version 2 as
    published by the Free Software Foundation.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#ifndef PLATFORM_H
#define PLATFORM_H

/*
	Determine the most appropriate PLATFORM_* define for our
	current target.
*/

#if defined(__CYGWIN__)

#define PLATFORM_CYGWIN

#elif defined(__APPLE__) && defined(__MACH__)

#define PLATFORM_OS_X

#elif defined(__gnu_linux__)

#define PLATFORM_LINUX

#elif defined (__FreeBSD__)

#define PLATFORM_FREEBSD

#elif defined(__unix__)

#define PLATFORM_UNIX_UNKNOWN

#else

#error Unsupported platform

#endif

#endif
