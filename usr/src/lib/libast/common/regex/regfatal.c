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

/*
 * posix regex fatal error interface to error()
 */

#include "reglib.h"

#include <error.h>

void
regfatalpat(regex_t* p, int level, int code, const char* pat)
{
	char	buf[128];

	regerror(code, p, buf, sizeof(buf));
	regfree(p);
	if (pat)
		error(level, "regular expression: %s: %s", pat, buf);
	else
		error(level, "regular expression: %s", buf);
}

void
regfatal(regex_t* p, int level, int code)
{
	regfatalpat(p, level, code, NiL);
}
