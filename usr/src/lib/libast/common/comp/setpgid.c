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

#include <ast.h>

#if _lib_setpgid

NoN(setpgid)

#else

#include <error.h>

#ifndef ENOSYS
#define ENOSYS		EINVAL
#endif

#if _lib_setpgrp2
#define setpgrp		setpgrp2
#else
#if _lib_BSDsetpgrp
#define _lib_setpgrp2	1
#define setpgrp		BSDsetpgrp
#else
#if _lib_wait3
#define	_lib_setpgrp2	1
#endif
#endif
#endif

#if _lib_setpgrp2
extern int		setpgrp(int, int);
#else
extern int		setpgrp(void);
#endif

/*
 * set process group id
 */

int
setpgid(pid_t pid, pid_t pgid)
{
#if _lib_setpgrp2
	return(setpgrp(pid, pgid));
#else
#if _lib_setpgrp
	int	caller = getpid();

	if ((pid == 0 || pid == caller) && (pgid == 0 || pgid == caller))
		return(setpgrp());
	errno = EINVAL;
#else
	errno = ENOSYS;
#endif
	return(-1);
#endif
}

#endif
