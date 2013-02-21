/*
    libparted - a library for manipulating disk partitions
    Copyright (C) 2001-2002, 2007, 2009-2012 Free Software Foundation, Inc.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/**
 * \addtogroup PedTimer
 * @{
 */

/** \file timer.h */

#ifndef PED_TIMER_H_INCLUDED
#define PED_TIMER_H_INCLUDED

#include <time.h>

typedef struct _PedTimer PedTimer;

typedef void PedTimerHandler (PedTimer* timer, void* context);

/*
 * Structure keeping track of progress and time
 */
struct _PedTimer {
	float			frac;		/**< fraction of operation done */
	time_t			start;		/**< time of start of op */
	time_t			now;		/**< time of last update (now!) */
	time_t			predicted_end;	/**< expected finish time */
	const char*		state_name;	/**< eg: "copying data" */
	PedTimerHandler*	handler;	/**< who to notify on updates */
	void*			context;	/**< context to pass to handler */
};

extern PedTimer* ped_timer_new (PedTimerHandler* handler, void* context);
extern void ped_timer_destroy (PedTimer* timer);

/* a nested timer automatically notifies it's parent.  You should only
 * create one when you are going to use it (not before)
 */
extern PedTimer* ped_timer_new_nested (PedTimer* parent, float nest_frac);
extern void ped_timer_destroy_nested (PedTimer* timer);

extern void ped_timer_touch (PedTimer* timer);
extern void ped_timer_reset (PedTimer* timer);
extern void ped_timer_update (PedTimer* timer, float new_frac);
extern void ped_timer_set_state_name (PedTimer* timer, const char* state_name);

#endif /* PED_TIMER_H_INCLUDED */


/** @} */
