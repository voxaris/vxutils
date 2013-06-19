/**
 * Copyright 2008 - 2012 Ampersand, Inc.  All rights reserved.
*/

#include <vx_ring.h>
#include <vxlog.h>

vx_status_t vx_ring_create  (vx_ring_t **ring)
{
   vx_status_t rc;
   size_t index;
   vx_ring_ele_t *tmp;
   (*ring) = (vx_ring_t *) malloc (sizeof (vx_ring_t));

   if ( (*ring) == NULL)
      return (VX_ENOMEM);

   if ((rc = vx_sync_create (&(*ring)->sync, NULL)) != VX_SUCCESS)
   {
      free (*ring);
      return (rc);
   }
      
   (*ring)->head = malloc (sizeof (vx_ring_ele_t));

   if ((*ring)->head == NULL)
      return (VX_ENOMEM);

   tmp = (*ring)->head;
   for (index = 1; index < VX_RING_INIT; index++)
   {
      (*ring)->tail = malloc (sizeof (vx_ring_ele_t));

      if ((*ring)->tail == NULL)
         return (VX_ENOMEM);

      tmp->next = (*ring)->tail;
      tmp = (*ring)->tail;
   }
   tmp->next = (*ring)->head;
   (*ring)->tail = (*ring)->head;
   (*ring)->count = 0;
   (*ring)->waiters = 0;
   (*ring)->size = VX_RING_INIT;
   return (VX_SUCCESS);
}

vx_status_t vx_ring_destroy (vx_ring_t *ring)
{
   size_t index;
   vx_ring_ele_t *tmp;
   vx_ring_ele_t *link = ring->head;
   vx_sync_lock (ring->sync);
   for (index = 0; index < ring->size; index++)
   {
      tmp = link->next;
      free (link);
      link = tmp;
   }
   vx_sync_unlock (ring->sync);

   vx_sync_destroy (ring->sync);

   free (ring);

   return (VX_SUCCESS);
}

vx_status_t vx_ring_push    (vx_ring_t *ring, void *data)
{
   size_t index;
   vx_ring_ele_t *tmp, *link;
   vx_sync_lock (ring->sync);
   if (ring->count == (ring->size - 1))
   {
      vxlog (LOG_WARNING, "{%s:%d} ring is full [%d:%d], adding nodes", 
         __func__, __LINE__, ring->size, ring->count);
      link = (vx_ring_ele_t *) malloc (sizeof (vx_ring_ele_t));

      if (link == NULL)
         return (VX_ENOMEM);

      ring->tail->next = link;
      for (index = 1; index < VX_RING_INCR; index++)
      {
         tmp = (vx_ring_ele_t *) malloc (sizeof (vx_ring_ele_t));

         if (tmp == NULL)
            return (VX_ENOMEM);

         link->next = tmp;
         link = tmp;
      }
      link->next = ring->head;
      ring->size += VX_RING_INCR;
   }
   ring->tail->data = data;
   ring->count++;
   ring->tail = ring->tail->next;
   vx_sync_signal (ring->sync);
   vx_sync_unlock (ring->sync);
   return (VX_SUCCESS);
}

vx_status_t vx_ring_pop (vx_ring_t *ring, void **data)
{
   vx_sync_lock (ring->sync);
   while (ring->count == 0)
   {
      ring->waiters++;
      vx_sync_wait (ring->sync);
      ring->waiters--;
   }
   (*data) = ring->head->data;
   ring->head = ring->head->next;
   ring->count--;
   vx_sync_unlock (ring->sync);
   return (VX_SUCCESS);
}

#define THOUSAND 1000
#define MILLION  1000000
#define BILLION  1000000000
vx_status_t vx_ring_pop_timed (vx_ring_t *ring, void **data, uint32_t msec)
{
   vx_status_t rc;
   struct timespec ts;

   clock_gettime (CLOCK_REALTIME, &ts);
   ts.tv_sec += (msec + ts.tv_nsec/MILLION)/THOUSAND;
   msec = msec % THOUSAND;
   ts.tv_nsec = (MILLION*msec + ts.tv_nsec) % BILLION;

   vx_sync_lock (ring->sync);
   while (ring->count == 0)
   {
      ring->waiters++;
      rc = vx_sync_timedwait (ring->sync, &ts);
      ring->waiters--;
      if (rc == VX_TIMEOUT)
      {
         vx_sync_unlock (ring->sync);
         return (rc);
      }
   }
   (*data) = ring->head->data;
   ring->head = ring->head->next;
   ring->count--;
   vx_sync_unlock (ring->sync);
   return (VX_SUCCESS);
}
