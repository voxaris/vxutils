/**
 * Copyright 2008 Voxaris Inc, George Howitt.
 */

#ifndef _VX_IOMPLX_H_
#define _VX_IOMPLX_H_

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include <socket.h>

#ifdef __cplusplus
extern "C"
{
#endif

struct vx_iomplx
{
   int iofd;
   
};
typedef struct vx_iomplx vx_iomplx_t;

struct vx_iofd
{
   int fd;
   void *data;
   int16_t events;
   int16_t revents;
};
typedef struct vx_iofd vx_iofd_t;

vx_status_t vx_iomplx_create (vx_iomplx_t **iomplx, uint32_t size, uint32_t flags);
vx_status_t vx_iomplx_destroy (vx_iomplx_t *iomplx);
vx_status_t vx_iomplx_add (vx_iomplx *iomplx, const vx_iofd_t *desc);
vx_status_t vx_iomplx_remove (vx_iomplx *iomplx, const vx_iofd_t *desc);
vx_status_t vx_iomplx_poll (vx_iomplx_t *iomplx, uint16_t msecs, vx_iofd_t *desc, uint32_t maxevents);
vx_status_t vx_iomplx_wake (vx_iomplx_t *iomplx);

#ifdef __cplusplus
}
#endif

#endif
