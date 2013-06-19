/**
 * Copyright 2008 - 2012 Ampersand, Inc.  All rights reserved.
*/

#ifndef _VX_SYNC_H_
#define _VX_SYNC_H_

#include <stdlib.h>
#include <pthread.h>
#include <inttypes.h>
#include <errno.h>

typedef enum vx_status
{
   VX_SUCCESS = 0,
   VX_TIMEOUT,
   VX_FAILURE,
   VX_ENOMEM,
} vx_status_t;

typedef struct vx_sync
{
   pthread_mutex_t mutex;
   pthread_cond_t  cond;
   int waiters;
} vx_sync_t;

vx_status_t vx_sync_create (vx_sync_t **sync, pthread_mutexattr_t *attr);
vx_status_t vx_sync_destroy (vx_sync_t *sync);
vx_status_t vx_sync_lock (vx_sync_t *sync);
vx_status_t vx_sync_unlock (vx_sync_t *sync);
vx_status_t vx_sync_wait (vx_sync_t *sync);
vx_status_t vx_sync_timedwait (vx_sync_t *sync, const struct timespec *ts);
vx_status_t vx_sync_signal (vx_sync_t *sync);
vx_status_t vx_sync_broadcast (vx_sync_t *sync);

#endif
