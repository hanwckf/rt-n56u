/*
    libparted - a library for manipulating disk partitions
    Copyright (C) 2001, 2007, 2009-2012 Free Software Foundation, Inc.

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

/** \file timer.c */

/**
 * \addtogroup PedTimer
 *
 * \brief A PedTimer keeps track of the progress of a single (possibly
 * compound) operation.
 *
 * The user of libparted constructs a PedTimer, and passes it to libparted
 * functions that are likely to be expensive operations
 * (like ped_file_system_resize).  Use of timers is optional... you may
 * pass NULL instead.
 *
 * When you create a PedTimer, you must specify a timer handler function.
 * This will be called when there's an update on how work is progressing.
 *
 * Timers may be nested.  When a timer is constructed, you can choose
 * to assign it a parent, along with an estimate of what proportion of
 * the total (parent's) time will be used in the nested operation.  In
 * this case, the nested timer's handler is internal to libparted,
 * and simply updates the parent's progress, and calls its handler.
 *
 * @{
 */


#include <config.h>
#include <parted/parted.h>
#include <parted/debug.h>

typedef struct {
	PedTimer*	parent;
	float		nest_frac;
	float		start_frac;
} NestedContext;


/**
 * \brief Creates a timer.
 *
 * Context will be passed in the \p context
 *         argument to the \p handler, when it is invoked.
 *
 * \return a new PedTimer
 */
PedTimer*
ped_timer_new (PedTimerHandler* handler, void* context)
{
	PedTimer*	timer;

	PED_ASSERT (handler != NULL);

	timer = (PedTimer*) ped_malloc (sizeof (PedTimer));
	if (!timer)
		return NULL;

	timer->handler = handler;
	timer->context = context;
	ped_timer_reset (timer);
	return timer;
}


/**
 * \brief Destroys a \p timer.
 */
void
ped_timer_destroy (PedTimer* timer)
{
	if (!timer)
		return;

	free (timer);
}

/* This function is used by ped_timer_new_nested() as the timer->handler
 * function.
 */
static void
_nest_handler (PedTimer* timer, void* context)
{
	NestedContext*	ncontext = (NestedContext*) context;

	ped_timer_update (
		ncontext->parent,
		ncontext->start_frac + ncontext->nest_frac * timer->frac);
}


/**
 * \brief Creates a new nested timer.
 *
 * This function creates a "nested" timer that describes the progress
 * of a subtask. \p parent is the parent timer, and \p nested_frac is
 * the estimated proportion (between 0 and 1) of the time that will be
 * spent doing the nested timer's operation. The timer should only be
 * constructed immediately prior to starting the nested operation.
 * (It will be inaccurate, otherwise).
 * Updates to the progress of the subtask are propagated
 * back through to the parent task's timer.
 *
 * \return nested timer
 */
PedTimer*
ped_timer_new_nested (PedTimer* parent, float nest_frac)
{
	NestedContext*	context;

	if (!parent)
		return NULL;

	PED_ASSERT (nest_frac >= 0.0f);
	PED_ASSERT (nest_frac <= 1.0f);

	context = (NestedContext*) ped_malloc (sizeof (NestedContext));
	if (!context)
		return NULL;
	context->parent = parent;
	context->nest_frac = nest_frac;
	context->start_frac = parent->frac;

	return ped_timer_new (_nest_handler, context);
}

/**
 * \brief Destroys a nested \p timer.
 */
void
ped_timer_destroy_nested (PedTimer* timer)
{
	if (!timer)
		return;

	free (timer->context);
	ped_timer_destroy (timer);
}

/**
 * \internal
 *
 * \brief This function calls the update handler, making sure that it has
 * 	the latest time.
 *
 * First it updates \p timer->now and recomputes \p timer->predicted_end,
 * and then calls the handler.
 */
void
ped_timer_touch (PedTimer* timer)
{
	if (!timer)
	       return;

	timer->now = time (NULL);
	if (timer->now > timer->predicted_end)
		timer->predicted_end = timer->now;

	timer->handler (timer, timer->context);
}

/**
 * \internal
 *
 * \brief This function sets the \p timer into a "start of task" position.
 *
 * It resets the \p timer, by setting \p timer->start and \p timer->now
 * to the current time.
 */
void
ped_timer_reset (PedTimer* timer)
{
	if (!timer)
	       return;

	timer->start = timer->now = timer->predicted_end = time (NULL);
	timer->state_name = NULL;
	timer->frac = 0;

	ped_timer_touch (timer);
}

/**
 * \internal
 *
 * \brief This function tells a \p timer what fraction \p frac of the task
 * has been completed.
 *
 * Sets the new \p timer->frac, and calls ped_timer_touch().
 */
void
ped_timer_update (PedTimer* timer, float frac)
{
	if (!timer)
	       return;

	timer->now = time (NULL);
	timer->frac = frac;

	if (frac)
		timer->predicted_end
			= timer->start
			  + (long) ((timer->now - timer->start) / frac);

	ped_timer_touch (timer);
}

/**
 * \internal
 *
 * \brief This function changes the description of the current task that the
 * 	\p timer describes.
 *
 * Sets a new name - \p state_name - for the current "phase" of the operation,
 * and calls ped_timer_touch().
 */
void
ped_timer_set_state_name (PedTimer* timer, const char* state_name)
{
	if (!timer)
	       return;

	timer->state_name = state_name;
	ped_timer_touch (timer);
}

/** @} */
