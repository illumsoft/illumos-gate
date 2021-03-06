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
#include	"dthdr.h"

/*	Walk a dictionary and all dictionaries viewed through it.
**	userf:	user function
**
**	Written by Kiem-Phong Vo (5/25/96)
*/

#if __STD_C
int dtwalk(reg Dt_t* dt, int (*userf)(Dt_t*, Void_t*, Void_t*), Void_t* data)
#else
int dtwalk(dt,userf,data)
reg Dt_t*	dt;
int(*		userf)();
Void_t*		data;
#endif
{
	reg Void_t	*obj, *next;
	reg Dt_t*	walk;
	reg int		rv;

	for(obj = dtfirst(dt); obj; )
	{	if(!(walk = dt->walk) )
			walk = dt;
		next = dtnext(dt,obj);
		if((rv = (*userf)(walk, obj, data )) < 0)
			return rv;
		obj = next;
	}
	return 0;
}
