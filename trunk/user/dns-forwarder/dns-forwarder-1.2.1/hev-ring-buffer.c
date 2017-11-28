/*
 ============================================================================
 Name        : hev-ring-buffer.c
 Author      : Heiher <r@hev.cc>
 Copyright   : Copyright (c) 2013 everyone.
 Description : Ring buffer data structure
 ============================================================================
 */

#include <stdint.h>
#include <assert.h>

#include "hev-ring-buffer.h"
#include "hev-memory-allocator.h"

struct _HevRingBuffer
{
	uint8_t *buffer;
	unsigned int ref_count;

	size_t wp;
	size_t rp;
	size_t len;
	bool full;
};

HevRingBuffer *
hev_ring_buffer_new (size_t len)
{
	HevRingBuffer *self = NULL;
	self = HEV_MEMORY_ALLOCATOR_ALLOC (sizeof (HevRingBuffer) + len);
	if (self) {
		self->ref_count = 1;
		self->wp = 0;
		self->rp = 0;
		self->len = len;
		self->full = false;
		self->buffer = ((void *) self) + sizeof (HevRingBuffer);
	}

	return self;
}

HevRingBuffer *
hev_ring_buffer_ref (HevRingBuffer *self)
{
	if (self)
	  self->ref_count ++;

	return self;
}

void
hev_ring_buffer_unref (HevRingBuffer *self)
{
	if (self) {
		self->ref_count --;
		if (0 == self->ref_count)
		  HEV_MEMORY_ALLOCATOR_FREE (self);
	}
}

void
hev_ring_buffer_reset (HevRingBuffer *self)
{
	if (self) {
		self->wp = 0;
		self->rp = 0;
		self->full = false;
	}
}

size_t
hev_ring_buffer_reading (HevRingBuffer *self, struct iovec *iovec)
{
	if (self && iovec) {
		if (0 == self->rp) {
			/* rp
			 * ||--------------------||
			 * wp */
			if ((self->rp == self->wp) && self->full) {
				iovec[0].iov_base = self->buffer;
				iovec[0].iov_len = self->len;
				return 1;
			}
			/* rp
			 * ||--------------------||
			 *           wp */
			if ((0 < self->wp) && (self->len > self->wp)) {
				iovec[0].iov_base = self->buffer;
				iovec[0].iov_len = self->wp;
				return 1;
			}
			/* rp
			 * ||--------------------||
			 *                       wp */
			if (self->len == self->wp) {
				iovec[0].iov_base = self->buffer;
				iovec[0].iov_len = self->wp;
				return 1;
			}
		}
		if ((0 < self->rp) && (self->len > self->rp)) {
			/*           rp
			 * ||--------------------||
			 * wp */
			if (0 == self->wp) {
				iovec[0].iov_base = self->buffer + self->rp;
				iovec[0].iov_len = self->len - self->rp;
				return 1;
			}
			/*           rp
			 * ||--------------------||
			 *      wp */
			if ((0 < self->wp) && (self->wp < self->rp)) {
				iovec[0].iov_base = self->buffer + self->rp;
				iovec[0].iov_len = self->len - self->rp;
				iovec[1].iov_base = self->buffer;
				iovec[1].iov_len = self->wp;
				return 2;
			}
			/*           rp
			 * ||--------------------||
			 *           wp */
			if ((self->wp == self->rp) && self->full) {
				iovec[0].iov_base = self->buffer + self->rp;
				iovec[0].iov_len = self->len - self->rp;
				iovec[1].iov_base = self->buffer;
				iovec[1].iov_len = self->wp;
				return 2;
			}
			/*           rp
			 * ||--------------------||
			 *                wp */
			if ((self->wp > self->rp) && (self->wp < self->len)) {
				iovec[0].iov_base = self->buffer + self->rp;
				iovec[0].iov_len = self->wp - self->rp;
				return 1;
			}
			/*           rp
			 * ||--------------------||
			 *                       wp */
			if (self->len == self->wp) {
				iovec[0].iov_base = self->buffer + self->rp;
				iovec[0].iov_len = self->wp - self->rp;
				return 1;
			}
		}
		if (self->len == self->rp) {
			/*                       rp
			 * ||--------------------||
			 * wp */
			if (0 == self->wp) {
				return 0;
			}

			/*                       rp
			 * ||--------------------||
			 *           wp */
			if ((0 < self->wp) && (self->wp < self->len)) {
				iovec[0].iov_base = self->buffer;
				iovec[0].iov_len = self->wp;
				return 1;
			}

			/*                       rp
			 * ||--------------------||
			 *                       wp */
			if ((self->wp == self->rp) && self->full) {
				iovec[0].iov_base = self->buffer;
				iovec[0].iov_len = self->wp;
				return 1;
			}
		}
	}

	return 0;
}

void
hev_ring_buffer_read_finish (HevRingBuffer *self, size_t inc_len)
{
	if (self && (0 < inc_len)) {
		if (0 == self->rp) {
			/* rp
			 * ||--------------------||
			 * wp */
			if ((self->rp == self->wp) && self->full) {
				assert (self->len >= inc_len);
				self->rp = inc_len;
				self->full = false;
				return;
			}
			/* rp
			 * ||--------------------||
			 *           wp */
			if ((0 < self->wp) && (self->len > self->wp)) {
				assert (self->wp >= inc_len);
				self->rp = inc_len;
				return;
			}
			/* rp
			 * ||--------------------||
			 *                       wp */
			if (self->len == self->wp) {
				assert (self->len >= inc_len);
				self->rp = inc_len;
				return;
			}
		}
		if ((0 < self->rp) && (self->len > self->rp)) {
			/*           rp
			 * ||--------------------||
			 * wp */
			if (0 == self->wp) {
				assert ((self->len - self->rp) >= inc_len);
				self->rp += inc_len;
				return;
			}
			/*           rp
			 * ||--------------------||
			 *      wp */
			if ((0 < self->wp) && (self->wp < self->rp)) {
				assert ((self->len - self->rp + self->wp) >= inc_len);
				self->rp += inc_len;
				if (self->rp > self->len)
				  self->rp -= self->len;
				return;
			}
			/*           rp
			 * ||--------------------||
			 *           wp */
			if ((self->wp == self->rp) && self->full) {
				assert ((self->len - self->rp + self->wp) >= inc_len);
				self->rp += inc_len;
				if (self->rp > self->len)
				  self->rp -= self->len;
				self->full = false;
				return;
			}
			/*           rp
			 * ||--------------------||
			 *                wp */
			if ((self->wp > self->rp) && (self->wp < self->len)) {
				assert ((self->wp - self->rp) >= inc_len);
				self->rp += inc_len;
				return;
			}
			/*           rp
			 * ||--------------------||
			 *                       wp */
			if (self->len == self->wp) {
				assert ((self->wp - self->rp) >= inc_len);
				self->rp += inc_len;
				return;
			}
		}
		if (self->len == self->rp) {
			/*                       rp
			 * ||--------------------||
			 * wp */
			if (0 == self->wp) {
				return;
			}

			/*                       rp
			 * ||--------------------||
			 *           wp */
			if ((0 < self->wp) && (self->wp < self->len)) {
				assert (self->wp >= inc_len);
				self->rp = inc_len;
				return;
			}

			/*                       rp
			 * ||--------------------||
			 *                       wp */
			if ((self->wp == self->rp) && self->full) {
				assert (self->wp >= inc_len);
				self->rp = inc_len;
				self->full = false;
				return;
			}
		}
	}
}

size_t
hev_ring_buffer_writing (HevRingBuffer *self, struct iovec *iovec)
{
	if (self && iovec) {
		if (0 == self->rp) {
			/* rp
			 * ||--------------------||
			 * wp */
			if ((self->rp == self->wp) && !self->full) {
				iovec[0].iov_base = self->buffer;
				iovec[0].iov_len = self->len;
				return 1;
			}
			/* rp
			 * ||--------------------||
			 *           wp */
			if ((0 < self->wp) && (self->len > self->wp)) {
				iovec[0].iov_base = self->buffer + self->wp;
				iovec[0].iov_len = self->len - self->wp;
				return 1;
			}
			/* rp
			 * ||--------------------||
			 *                       wp */
			if (self->len == self->wp) {
				return 0;
			}
		}
		if ((0 < self->rp) && (self->len > self->rp)) {
			/*           rp
			 * ||--------------------||
			 * wp */
			if (0 == self->wp) {
				iovec[0].iov_base = self->buffer;
				iovec[0].iov_len = self->rp - self->wp;
				return 1;
			}
			/*           rp
			 * ||--------------------||
			 *      wp */
			if ((0 < self->wp) && (self->wp < self->rp)) {
				iovec[0].iov_base = self->buffer + self->wp;
				iovec[0].iov_len = self->rp - self->wp;
				return 1;
			}
			/*           rp
			 * ||--------------------||
			 *           wp */
			if ((self->wp == self->rp) && !self->full) {
				iovec[0].iov_base = self->buffer + self->wp;
				iovec[0].iov_len = self->len - self->wp;
				iovec[1].iov_base = self->buffer;
				iovec[1].iov_len = self->rp;
				return 2;
			}
			/*           rp
			 * ||--------------------||
			 *                wp */
			if ((self->wp > self->rp) && (self->wp < self->len)) {
				iovec[0].iov_base = self->buffer + self->wp;
				iovec[0].iov_len = self->len - self->wp;
				iovec[1].iov_base = self->buffer;
				iovec[1].iov_len = self->rp;
				return 2;
			}
			/*           rp
			 * ||--------------------||
			 *                       wp */
			if (self->len == self->wp) {
				iovec[0].iov_base = self->buffer;
				iovec[0].iov_len = self->rp;
				return 1;
			}
		}
		if (self->len == self->rp) {
			/*                       rp
			 * ||--------------------||
			 * wp */
			if (0 == self->wp) {
				iovec[0].iov_base = self->buffer;
				iovec[0].iov_len = self->rp;
				return 1;
			}

			/*                       rp
			 * ||--------------------||
			 *           wp */
			if ((0 < self->wp) && (self->wp < self->len)) {
				iovec[0].iov_base = self->buffer + self->wp;
				iovec[0].iov_len = self->rp - self->wp;
				return 1;
			}

			/*                       rp
			 * ||--------------------||
			 *                       wp */
			if ((self->wp == self->rp) && !self->full) {
				iovec[0].iov_base = self->buffer;
				iovec[0].iov_len = self->rp;
				return 1;
			}
		}
	}

	return 0;
}

void
hev_ring_buffer_write_finish (HevRingBuffer *self, size_t inc_len)
{
	if (self && (0 < inc_len)) {
		if (0 == self->rp) {
			/* rp
			 * ||--------------------||
			 * wp */
			if ((self->rp == self->wp) && !self->full) {
				assert (self->len >= inc_len);
				self->wp = inc_len;
				return;
			}
			/* rp
			 * ||--------------------||
			 *           wp */
			if ((0 < self->wp) && (self->len > self->wp)) {
				assert ((self->len - self->wp) >= inc_len);
				self->wp += inc_len;
				return;
			}
			/* rp
			 * ||--------------------||
			 *                       wp */
			if (self->len == self->wp) {
				return;
			}
		}
		if ((0 < self->rp) && (self->len > self->rp)) {
			/*           rp
			 * ||--------------------||
			 * wp */
			if (0 == self->wp) {
				assert (self->rp >= inc_len);
				self->wp += inc_len;
				if (self->wp == self->rp)
				  self->full = true;
				return;
			}
			/*           rp
			 * ||--------------------||
			 *      wp */
			if ((0 < self->wp) && (self->wp < self->rp)) {
				assert ((self->rp - self->wp) >= inc_len);
				self->wp += inc_len;
				if (self->wp == self->rp)
				  self->full = true;
				return;
			}
			/*           rp
			 * ||--------------------||
			 *           wp */
			if ((self->wp == self->rp) && !self->full) {
				assert (self->len >= inc_len);
				self->wp += inc_len;
				if (self->wp > self->len)
				  self->wp -= self->len;
				if (self->len == inc_len)
				  self->full = true;
				return;
			}
			/*           rp
			 * ||--------------------||
			 *                wp */
			if ((self->wp > self->rp) && (self->wp < self->len)) {
				assert ((self->len - self->wp + self->rp) >= inc_len);
				self->wp += inc_len;
				if (self->wp > self->len)
				  self->wp -= self->len;
				if (self->wp == self->rp)
				  self->full = true;
				return;
			}
			/*           rp
			 * ||--------------------||
			 *                       wp */
			if (self->len == self->wp) {
				assert (self->rp >= inc_len);
				self->wp = inc_len;
				if (self->wp == self->rp)
				  self->full = true;
				return;
			}
		}
		if (self->len == self->rp) {
			/*                       rp
			 * ||--------------------||
			 * wp */
			if (0 == self->wp) {
				assert (self->rp >= inc_len);
				self->wp = inc_len;
				if (self->wp == self->rp)
				  self->full = true;
				return;
			}

			/*                       rp
			 * ||--------------------||
			 *           wp */
			if ((0 < self->wp) && (self->wp < self->len)) {
				assert ((self->len - self->wp) >= inc_len);
				self->wp += inc_len;
				if (self->wp == self->rp)
				  self->full = true;
				return;
			}

			/*                       rp
			 * ||--------------------||
			 *                       wp */
			if ((self->wp == self->rp) && !self->full) {
				assert (self->len >= inc_len);
				self->wp = inc_len;
				if (self->len == inc_len)
				  self->full = true;
				return;
			}
		}
	}
}

