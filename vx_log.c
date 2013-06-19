/**
 * Copyright 2008 - 2012 Ampersand, Inc.  All rights reserved.
*/

#include "vxlog.h"


FILE *logfp = NULL;
int loglevel = LOG_INFO;

static char *levstr[] = { "EMRG", "ALRT", "CRIT", "ERRO", "WARN", "NOTI", "INFO", "DBUG" };

int vxlog_init (int level, const char *file)
{
   if (file == NULL)
   {
      logfp = stdout;
   }
   else
   {
      if ((logfp = fopen (file, "a")) == NULL)
      {
         logfp = stdout;
      }
   }
   if ((level >= LOG_EMERG) && (level <= LOG_DEBUG))
   {
      loglevel = level;
   }
   else
   {
      loglevel = LOG_INFO;
   }
   return (0);
}


void vxlog (int level, const char *str, ...)
{
   va_list ap;
   struct tm *lp;
   struct tm res;
   struct timeval tv;
   char output[1024];

   if (logfp == NULL)
      logfp = stderr;

   if (level <= loglevel)
   {
      if (level > LOG_DEBUG)
         level = LOG_DEBUG;

      gettimeofday (&tv, NULL);
      if ((lp = localtime_r ((time_t *) & (tv.tv_sec), &res)))
      {
         sprintf (output, "%02d:%02d:%02d.%06ld %s %s\n",
                  lp->tm_hour, lp->tm_min, lp->tm_sec, tv.tv_usec, levstr[level], str);
      }
      else
      {
         sprintf (output, "??:??:??.?????? %s %s\n", levstr[level], str);
      }

      va_start (ap, str);
      vfprintf (logfp, output, ap);
      va_end (ap);
      fflush (logfp);
   }
}

void vxlog_set_level (int level)
{
   loglevel = level;
}

int vxlog_get_level (void)
{
   return (loglevel);
}
