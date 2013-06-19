/**
 * Copyright 2008 - 2012 Ampersand, Inc.  All rights reserved.
*/

#ifndef _VX_RING_H_
#define _VX_RING_H_

#include <vx_sync.h>

#define VX_RING_INIT   256
#define VX_RING_INCR   32

typedef struct vx_ring_ele
{
   void *data;
   struct vx_ring_ele *next;
} vx_ring_ele_t;

typedef struct vx_ring
{
   vx_ring_ele_t *head;
   vx_ring_ele_t *tail;
   vx_sync_t *sync;
   size_t size;
   size_t count;
   size_t waiters;
} vx_ring_t;

vx_status_t vx_ring_create  (vx_ring_t **ring);
vx_status_t vx_ring_destroy (vx_ring_t *ring);
vx_status_t vx_ring_push    (vx_ring_t *ring, void *data);
vx_status_t vx_ring_pop     (vx_ring_t *ring, void **data);
vx_status_t vx_ring_pop_timed (vx_ring_t *ring, void **data, uint32_t sec);

#endif

