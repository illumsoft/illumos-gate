/*
 * Copyright (c) 2006 Damien Miller. All rights reserved.
 * Copyright (c) 2005 Anil Madhavapeddy. All rights reserved.
 * Copyright (c) 1995,1999 Theo de Raadt.  All rights reserved.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "includes.h"
RCSID("$OpenBSD: atomicio.c,v 1.10 2001/05/08 22:48:07 markus Exp $");

#pragma ident	"%Z%%M%	%I%	%E% SMI"

#include "atomicio.h"

/*
 * ensure all of data on socket comes through. f==read || f==write
 */
ssize_t
atomicio(f, fd, _s, n)
	ssize_t (*f) ();
	int fd;
	void *_s;
	size_t n;
{
	char *s = _s;
	ssize_t res, pos = 0;

	while (n > pos) {
		res = (f) (fd, s + pos, n - pos);
		switch (res) {
		case -1:
#ifdef EWOULDBLOCK
			if (errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK)
#else
			if (errno == EINTR || errno == EAGAIN)
#endif
				continue;
			/* FALLTHROUGH */
		case 0:
			return (res);
		default:
			pos += res;
		}
	}
	return (pos);
}

/*
 * ensure all of data on socket comes through. f==readv || f==writev
 */
size_t
atomiciov(ssize_t (*f) (int, const struct iovec *, int), int fd,
    const struct iovec *_iov, int iovcnt)
{
	size_t pos = 0, rem;
	ssize_t res;
	struct iovec iov_array[IOV_MAX], *iov = iov_array;

	if (iovcnt > IOV_MAX) {
		errno = EINVAL;
		return 0;
	}
	/* Make a copy of the iov array because we may modify it below */
	memcpy(iov, _iov, iovcnt * sizeof(*_iov));

	for (; iovcnt > 0 && iov[0].iov_len > 0;) {
		res = (f) (fd, iov, iovcnt);
		switch (res) {
		case -1:
			if (errno == EINTR || errno == EAGAIN)
				continue;
			return 0;
		case 0:
			errno = EPIPE;
			return pos;
		default:
			rem = (size_t)res;
			pos += rem;
			/* skip completed iov entries */
			while (iovcnt > 0 && rem >= iov[0].iov_len) {
				rem -= iov[0].iov_len;
				iov++;
				iovcnt--;
			}
			/* This shouldn't happen... */
			if (rem > 0 && (iovcnt <= 0 || rem > iov[0].iov_len)) {
				errno = EFAULT;
				return 0;
			}
			if (iovcnt == 0)
				break;
			/* update pointer in partially complete iov */
			iov[0].iov_base = ((char *)iov[0].iov_base) + rem;
			iov[0].iov_len -= rem;
		}
	}
	return pos;
}
