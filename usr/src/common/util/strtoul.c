/*
 * CDDL HEADER START
 *
 * The contents of this file are subject to the terms of the
 * Common Development and Distribution License (the "License").
 * You may not use this file except in compliance with the License.
 *
 * You can obtain a copy of the license at usr/src/OPENSOLARIS.LICENSE
 * or http://www.opensolaris.org/os/licensing.
 * See the License for the specific language governing permissions
 * and limitations under the License.
 *
 * When distributing Covered Code, include this CDDL HEADER in each
 * file and include the License file at usr/src/OPENSOLARIS.LICENSE.
 * If applicable, add the following below this CDDL HEADER, with the
 * fields enclosed by brackets "[]" replaced with your own identifying
 * information: Portions Copyright [yyyy] [name of copyright owner]
 *
 * CDDL HEADER END
 */

/*
 * Copyright 2008 Sun Microsystems, Inc.  All rights reserved.
 * Use is subject to license terms.
 */

/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

#pragma ident	"%Z%%M%	%I%	%E% SMI"

#if !defined(_BOOT) && !defined(_KMDB)
#include "lint.h"
#endif /* !_BOOT && !_KMDB */
#include <errno.h>
#include <ctype.h>
#include <limits.h>
#include <stdlib.h>
#include <sys/types.h>

#define	DIGIT(x)	\
	(isdigit(x) ? (x) - '0' : islower(x) ? (x) + 10 - 'a' : (x) + 10 - 'A')

#define	MBASE	('z' - 'a' + 1 + 10)

/*
 * The following macro is a local version of isalnum() which limits
 * alphabetic characters to the ranges a-z and A-Z; locale dependent
 * characters will not return 1. The members of a-z and A-Z are
 * assumed to be in ascending order and contiguous
 */
#define	lisalnum(x)	\
	(isdigit(x) || ((x) >= 'a' && (x) <= 'z') || ((x) >= 'A' && (x) <= 'Z'))

unsigned long
strtoul(const char *str, char **nptr, int base)
{
	unsigned long val;
	int c;
	int xx;
	unsigned long	multmax;
	int neg = 0;
	const char **ptr = (const char **)nptr;
	const unsigned char	*ustr = (const unsigned char *)str;

	if (ptr != (const char **)0)
		*ptr = (char *)ustr; /* in case no number is formed */
	if (base < 0 || base > MBASE || base == 1) {
		errno = EINVAL;
		return (0); /* base is invalid -- should be a fatal error */
	}
	if (!isalnum(c = *ustr)) {
		while (isspace(c))
			c = *++ustr;
		switch (c) {
		case '-':
			neg++;
			/* FALLTHROUGH */
		case '+':
			c = *++ustr;
		}
	}
	if (base == 0)
		if (c != '0')
			base = 10;
		else if (ustr[1] == 'x' || ustr[1] == 'X')
			base = 16;
		else
			base = 8;
	/*
	 * for any base > 10, the digits incrementally following
	 *	9 are assumed to be "abc...z" or "ABC...Z"
	 */
	if (!lisalnum(c) || (xx = DIGIT(c)) >= base)
		return (0); /* no number formed */
	if (base == 16 && c == '0' && (ustr[1] == 'x' || ustr[1] == 'X') &&
	    isxdigit(ustr[2]))
		c = *(ustr += 2); /* skip over leading "0x" or "0X" */

	multmax = ULONG_MAX / (unsigned long)base;
	val = DIGIT(c);
	for (c = *++ustr; lisalnum(c) && (xx = DIGIT(c)) < base; ) {
		if (val > multmax)
			goto overflow;
		val *= base;
		if (ULONG_MAX - val < xx)
			goto overflow;
		val += xx;
		c = *++ustr;
	}
	if (ptr != (const char **)0)
		*ptr = (char *)ustr;
	return (neg ? -val : val);

overflow:
	for (c = *++ustr; lisalnum(c) && (xx = DIGIT(c)) < base; (c = *++ustr))
		;
	if (ptr != (const char **)0)
		*ptr = (char *)ustr;
	errno = ERANGE;
	return (ULONG_MAX);
}
