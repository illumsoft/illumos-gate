/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*           Copyright (c) 1982-2007 AT&T Knowledge Ventures            *
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
*                  David Korn <dgk@research.att.com>                   *
*                                                                      *
***********************************************************************/
#pragma prototyped
/*
 * sleep delay
 *
 *   David Korn
 *   AT&T Labs
 *   research!dgk
 *
 */

#define sleep	______sleep
#include	"defs.h"
#undef	sleep
#include	<error.h>
#include	<errno.h>
#include	"builtins.h"
#include	"FEATURE/time"
#include	"FEATURE/poll"
#ifdef _NEXT_SOURCE
#   define sleep	_ast_sleep
#endif /* _NEXT_SOURCE */
#ifdef _lib_poll_notimer
#   undef _lib_poll
#endif /* _lib_poll_notimer */

int	b_sleep(register int argc,char *argv[],void *extra)
{
	register char *cp;
	register double d;
	register Shell_t *shp = (Shell_t*)extra;
	time_t tloc = 0;
	while((argc = optget(argv,sh_optsleep))) switch(argc)
	{
		case ':':
			errormsg(SH_DICT,2, "%s", opt_info.arg);
			break;
		case '?':
			errormsg(SH_DICT,ERROR_usage(2), "%s", opt_info.arg);
			break;
	}
	argv += opt_info.index;
	if(error_info.errors || !(cp= *argv) || !(strmatch(cp,e_numeric)))
		errormsg(SH_DICT,ERROR_usage(2),"%s",optusage((char*)0));
	if((d=strtod(cp, (char**)0)) > .10)
	{
		sfsync(shp->outpool);
		time(&tloc);
		tloc += (time_t)(d+.5);
	}
	while(1)
	{
		time_t now;
		errno = 0;
		shp->lastsig=0;
		sh_delay(d);
		if(tloc==0 || errno!=EINTR || shp->lastsig)
			break;
		sh_sigcheck();
		if(tloc < (now=time(NIL(time_t*))))
			break;
		d = (double)(tloc-now);
		if(shp->sigflag[SIGALRM]&SH_SIGTRAP)
			sh_timetraps();
	}
	return(0);
}

static void completed(void * handle)
{
	char *expired = (char*)handle;
	*expired = 1;
}

unsigned int sleep(unsigned int sec)
{
	pid_t newpid, curpid=getpid();
	void *tp;
	char expired = 0;
	sh.lastsig = 0;
	tp = (void*)sh_timeradd(1000*sec, 0, completed, (void*)&expired);
	do
	{
		if(!sh.waitevent || (*sh.waitevent)(-1,-1L,0)==0)
			pause();
		if(sh.sigflag[SIGALRM]&SH_SIGTRAP)
			sh_timetraps();
		if((newpid=getpid()) != curpid)
		{
			curpid = newpid;
			sh.lastsig = 0;
			sh.trapnote &= ~SH_SIGSET;
			if(expired)
				expired = 0;
			else
				timerdel(tp);
			tp = (void*)sh_timeradd(1000*sec, 0, completed, (void*)&expired);
		}
	}
	while(!expired && sh.lastsig==0);
	if(!expired)
		timerdel(tp);
	sh_sigcheck();
	return(0);
}

/*
 * delay execution for time <t>
 */

void	sh_delay(double t)
{
	register int n = (int)t;
#ifdef _lib_poll
	struct pollfd fd;
	if(t<=0)
		return;
	else if(n > 30)
	{
		sleep(n);
		t -= n;
	}
	if(n=(int)(1000*t))
	{
		if(!sh.waitevent || (*sh.waitevent)(-1,(long)n,0)==0)
			poll(&fd,0,n);
	}
#else
#   if defined(_lib_select) && defined(_mem_tv_usec_timeval)
	struct timeval timeloc;
	if(t<=0)
		return;
	if(n=(int)(1000*t) && sh.waitevent && (*sh.waitevent)(-1,(long)n,0))
		return;
	n = (int)t;
	timeloc.tv_sec = n;
	timeloc.tv_usec = 1000000*(t-(double)n);
	select(0,(fd_set*)0,(fd_set*)0,(fd_set*)0,&timeloc);
#   else
#	ifdef _lib_select
		/* for 9th edition machines */
		if(t<=0)
			return;
		if(n > 30)
		{
			sleep(n);
			t -= n;
		}
		if(n=(int)(1000*t))
		{
			if(!sh.waitevent || (*sh.waitevent)(-1,(long)n,0)==0)
				select(0,(fd_set*)0,(fd_set*)0,n);
		}
#	else
		struct tms tt;
		if(t<=0)
			return;
		sleep(n);
		t -= n;
		if(t)
		{
			clock_t begin = times(&tt);
			if(begin==0)
				return;
			t *= sh.lim.clk_tck;
			n += (t+.5);
			while((times(&tt)-begin) < n);
		}
#	endif
#   endif
#endif /* _lib_poll */
}
