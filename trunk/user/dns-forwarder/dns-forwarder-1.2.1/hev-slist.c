/*
 ============================================================================
 Name        : hev-slist.h
 Author      : Heiher <r@hev.cc>
 Copyright   : Copyright (c) 2013 everyone.
 Description : Singly-linked list data structure
 ============================================================================
 */

#include <stdlib.h>

#include "hev-slist.h"
#include "hev-memory-allocator.h"

struct _HevSList
{
	void *data;
	HevSList *next;
};

HevSList *
hev_slist_append (HevSList *self, void *data)
{
	HevSList *new = HEV_MEMORY_ALLOCATOR_ALLOC (sizeof (HevSList));
	if (new) {
		new->data = data;
		new->next = NULL;
		if (self) {
			HevSList *last = hev_slist_last (self);
			last->next = new;
			return self;
		} else {
			return new;
		}
	}

	return NULL;
}

HevSList *
hev_slist_prepend (HevSList *self, void *data)
{
	HevSList *new = HEV_MEMORY_ALLOCATOR_ALLOC (sizeof (HevSList));
	if (new) {
		new->data = data;
		if (self) {
			new->next = self;
		} else {
			new->next = NULL;
		}
		return new;
	}

	return NULL;
}

HevSList *
hev_slist_insert (HevSList *self, void *data, unsigned int position)
{
	HevSList *new = HEV_MEMORY_ALLOCATOR_ALLOC (sizeof (HevSList));
	if (new) {
		new->data = data;
		if (self) {
			HevSList *node = NULL, *prev = NULL, *last = NULL;
			unsigned int i = 0;
			for (node=self; node; prev=node,node=node->next,i++) {
				if (i == position)
				  break;
				last = node;
			}
			if (node) {
				new->next = node;
				if (prev)
				  prev->next = new;
				else
				  self = new;
			} else {
				new->next = NULL;
				last->next = new;
			}
			return self;
		} else {
			new->next = NULL;
			return new;
		}
	}

	return NULL;
}

HevSList *
hev_slist_insert_before (HevSList *self, void *data, HevSList *sibling)
{
	HevSList *new = HEV_MEMORY_ALLOCATOR_ALLOC (sizeof (HevSList));
	if (new) {
		new->data = data;
		if (self) {
			HevSList *node = NULL, *prev = NULL, *last = NULL;
			for (node=self; node; prev=node,node=node->next) {
				if (node == sibling)
				  break;
				last = node;
			}
			if (node) {
				new->next = node;
				if (prev)
				  prev->next = new;
				else
				  self = new;
			} else {
				new->next = NULL;
				last->next = new;
			}
			return self;
		} else {
			new->next = NULL;
			return new;
		}
	}

	return NULL;
}

HevSList *
hev_slist_remove (HevSList *self, const void *data)
{
	if (self) {
		HevSList *node = NULL, *prev = NULL;
		for (node=self; node; prev=node,node=node->next) {
			if (data == node->data) {
				if (prev)
				  prev->next = node->next;
				else
				  self = node->next;
				HEV_MEMORY_ALLOCATOR_FREE (node);
				break;
			}
		}
		return self;
	}

	return NULL;
}

HevSList *
hev_slist_remove_all (HevSList *self, const void *data)
{
	if (self) {
		HevSList *node = NULL, *prev = NULL;
		for (node=self; node;) {
			HevSList *curr = node;
			node = node->next;
			if (data == curr->data) {
				if (prev)
				  prev->next = curr->next;
				else
				  self = curr->next;
				HEV_MEMORY_ALLOCATOR_FREE (curr);
			} else {
				prev = curr;
			}
		}
		return self;
	}

	return NULL;
}

HevSList *
hev_slist_last (HevSList *self)
{
	if (self) {
		while (self->next)
		  self = self->next;
	}

	return self;
}

HevSList *
hev_slist_next (HevSList *self)
{
	if (self)
	  return self->next;

	return NULL;
}

unsigned int
hev_slist_length (HevSList *self)
{
	if (self) {
		HevSList *node = NULL;
		unsigned int count = 0;
		for (node=self; node; node=node->next)
		  count ++;
		return count;
	}

	return 0;
}

void
hev_slist_free (HevSList *self)
{
	if (self) {
		HevSList *node = NULL;
		for (node=self; node;) {
			HevSList *curr = node;
			node = node->next;
			HEV_MEMORY_ALLOCATOR_FREE (curr);
		}
	}
}

