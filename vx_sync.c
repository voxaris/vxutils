/**
 * Copyright 2008 - 2012 Ampersand, Inc.  All rights reserved.
*/

#include <vx_sync.h>
#include <vxlog.h>

vx_status_t vx_sync_create (vx_sync_t **sync, pthread_mutexattr_t *attr)
{
   int rc;

   (*sync) = (vx_sync_t *) malloc (sizeof (vx_sync_t));
   if ((*sync) == NULL)
   {
      vxlog (LOG_ERR, "{%s:%d} malloc failed", __func__, __LINE__);
      return (VX_ENOMEM);
   }
   rc = pthread_mutex_init (&(*sync)->mutex, attr);
   if (rc)
   {
      vxlog (LOG_ERR, "{%s:%d} pthread_mutex_init failed: %d",
         __func__, __LINE__, rc);
      free (*sync);
      return (VX_FAILURE);
   }
   rc = pthread_cond_init (&(*sync)->cond, NULL);
   if (rc)
   {
      vxlog (LOG_ERR, "{%s:%d} pthread_cond_init failed: %d",
         __func__, __LINE__, rc);
      free (*sync);
      return (VX_FAILURE);
   }
   (*sync)->waiters = 0;
   return (VX_SUCCESS);
}

vx_status_t vx_sync_destroy (vx_sync_t *sync)
{
   if (sync)
      free (sync);
   return (VX_SUCCESS);
}

vx_status_t vx_sync_lock (vx_sync_t *sync)
{
   int rc;
   if ((rc = pthread_mutex_lock (&sync->mutex)))
   {
      vxlog (LOG_ERR, "{%s:%d} pthread_mutex_lock failed: %d",
         __func__, __LINE__, rc);
      return (VX_FAILURE);
   }
   return (VX_SUCCESS);
}

vx_status_t vx_sync_unlock (vx_sync_t *sync)
{
   int rc;
   if ((rc = pthread_mutex_unlock (&sync->mutex)))
   {
      vxlog (LOG_ERR, "{%s:%d} pthread_mutex_unlock failed: %d",
         __func__, __LINE__, rc);
      return (VX_FAILURE);
   }
   return (VX_SUCCESS);
}

vx_status_t vx_sync_wait (vx_sync_t *sync)
{
   int rc;
   if ((rc = pthread_cond_wait (&sync->cond, &sync->mutex)))
   {
      vxlog (LOG_ERR, "{%s:%d} pthread_cond_wait failed: %d",
         __func__, __LINE__, rc);
      return (VX_FAILURE);
   }
   return (VX_SUCCESS);
}
   
vx_status_t vx_sync_timedwait (vx_sync_t *sync, const struct timespec *ts)
{
   int rc;
   rc = pthread_cond_timedwait (&sync->cond, &sync->mutex, ts);
   if (rc == ETIMEDOUT)
   {
      return (VX_TIMEOUT);
   }
   else if (rc)
   {
      vxlog (LOG_ERR, "{%s:%d} pthread_cond_timedwait failed: %d",
         __func__, __LINE__, rc);
      return (VX_FAILURE);
   }
   return (VX_SUCCESS);
}
   
vx_status_t vx_sync_signal (vx_sync_t *sync)
{
   int rc;
   if ((rc = pthread_cond_signal (&sync->cond)))
   {
      vxlog (LOG_ERR, "{%s:%d} pthread_cond_signal failed: %d",
         __func__, __LINE__, rc);
      return (VX_FAILURE);
   }
   return (VX_SUCCESS);
}

vx_status_t vx_sync_broadcast (vx_sync_t *sync)
{
   int rc;
   if ((rc = pthread_cond_signal (&sync->cond)))
   {
      vxlog (LOG_ERR, "{%s:%d} pthread_cond_broadcast failed: %d",
         __func__, __LINE__, rc);
      return (VX_FAILURE);
   }
   return (VX_SUCCESS);
}
