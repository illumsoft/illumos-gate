/*
 * Copyright 2006 Sun Microsystems, Inc.  All rights reserved.
 * Use is subject to license terms.
 */
/*	$OpenBSD: bufaux.h,v 1.18 2002/04/20 09:14:58 markus Exp $	*/

#ifndef	_BUFAUX_H
#define	_BUFAUX_H

#pragma ident	"%Z%%M%	%I%	%E% SMI"

#ifdef __cplusplus
extern "C" {
#endif


/*
 * Author: Tatu Ylonen <ylo@cs.hut.fi>
 * Copyright (c) 1995 Tatu Ylonen <ylo@cs.hut.fi>, Espoo, Finland
 *                    All rights reserved
 *
 * As far as I am concerned, the code I have written for this software
 * can be used freely for any purpose.  Any derived versions of this
 * software must be clearly marked as such, and if the derived work is
 * incompatible with the protocol description in the RFC file, it must be
 * called by a name other than "ssh" or "Secure Shell".
 */

#include "buffer.h"
#include <openssl/bn.h>

void    buffer_put_bignum(Buffer *, const BIGNUM *);
void    buffer_put_bignum2(Buffer *, const BIGNUM *);
void	buffer_get_bignum(Buffer *, BIGNUM *);
void	buffer_get_bignum2(Buffer *, BIGNUM *);

u_short	buffer_get_short(Buffer *);
void	buffer_put_short(Buffer *, u_short);

u_int	buffer_get_int(Buffer *);
void    buffer_put_int(Buffer *, u_int);

#ifdef HAVE_U_INT64_T
u_int64_t buffer_get_int64(Buffer *);
void	buffer_put_int64(Buffer *, u_int64_t);
#endif

int     buffer_get_char(Buffer *);
void    buffer_put_char(Buffer *, int);

void   *buffer_get_string(Buffer *, u_int *);
u_char *buffer_get_utf8_cstring(Buffer *);
char   *buffer_get_ascii_cstring(Buffer *);
void    buffer_put_string(Buffer *, const void *, u_int);
void	buffer_put_cstring(Buffer *, const char *);
void	buffer_put_utf8_cstring(Buffer *, const u_char *);
void	buffer_put_ascii_cstring(Buffer *, const char *);

#if 0
/* If these are needed, then get rid of the #if 0 and this comment */
void	buffer_put_utf8_string(Buffer *, const u_char *, u_int);
void	buffer_put_ascii_string(Buffer *, const char *, u_int);
#endif

#define buffer_skip_string(b) \
    do { u_int l = buffer_get_int(b); buffer_consume(b, l); } while(0)

int	buffer_put_bignum_ret(Buffer *, const BIGNUM *);
int	buffer_get_bignum_ret(Buffer *, BIGNUM *);
int	buffer_put_bignum2_ret(Buffer *, const BIGNUM *);
int	buffer_get_bignum2_ret(Buffer *, BIGNUM *);
int	buffer_get_short_ret(u_short *, Buffer *);
int	buffer_get_int_ret(u_int *, Buffer *);
#ifdef HAVE_U_INT64_T
int	buffer_get_int64_ret(u_int64_t *, Buffer *);
#endif
void	*buffer_get_string_ret(Buffer *, u_int *);
int	buffer_get_char_ret(char *, Buffer *);

#ifdef __cplusplus
}
#endif

#endif /* _BUFAUX_H */
