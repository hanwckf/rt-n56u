/*
 ============================================================================
 Name        : hev-memory-allocator.c
 Author      : Heiher <r@hev.cc>
 Copyright   : Copyright (c) 2013 everyone.
 Description : Memory allocator
 ============================================================================
 */

#include <string.h>
#include "hev-memory-allocator.h"

HevMemoryAllocator *
hev_memory_allocator_default (void)
{
	static HevMemoryAllocator *allocator = NULL;

	if (!allocator)
	  allocator = hev_memory_allocator_new ();

	return allocator;
}

HevMemoryAllocator *
hev_memory_allocator_new (void)
{
	HevMemoryAllocator *self = NULL;

	self = malloc (sizeof (HevMemoryAllocator));
	if (self)
	  self->ref_count = 1;

	return self;
}

HevMemoryAllocator *
hev_memory_allocator_ref (HevMemoryAllocator *self)
{
	if (self) {
		self->ref_count ++;
		return self;
	}

	return NULL;
}

void
hev_memory_allocator_unref (HevMemoryAllocator *self)
{
	if (self) {
		self->ref_count --;
		if (0 == self->ref_count)
		  free (self);
	}
}

void *
hev_memory_allocator_alloc (HevMemoryAllocator *self, size_t size)
{
	return malloc (size);
}

void
hev_memory_allocator_free (HevMemoryAllocator *self, void *ptr)
{
	free (ptr);
}

void *
hev_malloc (size_t size)
{
	return HEV_MEMORY_ALLOCATOR_ALLOC (size);
}

void *
hev_malloc0 (size_t size)
{
	void *data = HEV_MEMORY_ALLOCATOR_ALLOC (size);
	if (data)
	  memset (data, 0, size);
	return data;
}

void
hev_free (void *ptr)
{
	HEV_MEMORY_ALLOCATOR_FREE (ptr);
}

