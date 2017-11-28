/*
 ============================================================================
 Name        : hev-ring-buffer.h
 Author      : Heiher <r@hev.cc>
 Copyright   : Copyright (c) 2013 everyone.
 Description : Ring buffer data structure
 ============================================================================
 */

#ifndef __HEV_RING_BUFFER_H__
#define __HEV_RING_BUFFER_H__

#include <stddef.h>
#include <stdbool.h>
#include <sys/uio.h>

typedef struct _HevRingBuffer HevRingBuffer;

HevRingBuffer * hev_ring_buffer_new (size_t len);

HevRingBuffer * hev_ring_buffer_ref (HevRingBuffer *self);
void hev_ring_buffer_unref (HevRingBuffer *self);

void hev_ring_buffer_reset (HevRingBuffer *self);

size_t hev_ring_buffer_reading (HevRingBuffer *self, struct iovec *iovec);
void hev_ring_buffer_read_finish (HevRingBuffer *self, size_t inc_len);

size_t hev_ring_buffer_writing (HevRingBuffer *self, struct iovec *iovec);
void hev_ring_buffer_write_finish (HevRingBuffer *self, size_t inc_len);

#endif /* __HEV_RING_BUFFER_H__ */

