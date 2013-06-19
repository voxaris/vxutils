/**
 * Copyright 2008 - 2012 Ampersand, Inc.  All rights reserved.
*/

#ifndef _LOG_H_
#define _LOG_H_

#include <stdio.h>
#include <stdarg.h>
#include <syslog.h>
#include <time.h>
#include <sys/time.h>

int vxlog_init (int level, const char *file);
void vxlog (int level, const char *fmt, ...);
int vxlog_get_level (void);
void vxlog_set_level (int level);

#endif
