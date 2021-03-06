/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*           Copyright (c) 1985-2007 AT&T Knowledge Ventures            *
*                      and is licensed under the                       *
*                  Common Public License, Version 1.0                  *
*                      by AT&T Knowledge Ventures                      *
*                                                                      *
*                A copy of the License is available at                 *
*            http://www.opensource.org/licenses/cpl1.0.txt             *
*         (with md5 checksum 059e8cd6165cb4c31e351f2b69388fd9)         *
*                                                                      *
*              Information and Software Systems Research               *
*                            AT&T Research                             *
*                           Florham Park NJ                            *
*                                                                      *
*                 Glenn Fowler <gsf@research.att.com>                  *
*                  David Korn <dgk@research.att.com>                   *
*                   Phong Vo <kpv@research.att.com>                    *
*                                                                      *
***********************************************************************/
#pragma prototyped

#ifndef _NO_LARGEFILE64_SOURCE
#define _NO_LARGEFILE64_SOURCE	1
#endif

#include "stdhdr.h"

long
ftell(Sfio_t* f)
{
	STDIO_INT(f, "ftell", long, (Sfio_t*), (f))

	return (long)sfseek(f, (Sfoff_t)0, SEEK_CUR);
}

#if _typ_int64_t

int64_t
ftell64(Sfio_t* f)
{
	STDIO_INT(f, "ftell64", int64_t, (Sfio_t*), (f))

	return (int64_t)sfseek(f, (Sfoff_t)0, SEEK_CUR);
}

#endif
