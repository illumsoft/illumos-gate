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
 * Copyright 2007 Sun Microsystems, Inc.  All rights reserved.
 * Use is subject to license terms.
 */

#pragma ident	"%Z%%M%	%I%	%E% SMI"

/*
 * ASSERTION:
 *	Checks that setting "bufresize" to "auto" will cause buffer
 *	allocation to succeed, even for large principal buffer sizes.
 *
 * SECTION: Buffers and Buffering/Buffer Resizing Policy;
 *	Options and Tunables/bufsize;
 *	Options and Tunables/bufresize
 *
 * NOTES:
 *	We use the undocumented "preallocate" option to make sure dtrace(1M)
 *	has enough space in its heap to allocate a buffer as large as the
 *	kernel's trace buffer.
 */

#pragma D option preallocate=100t
#pragma D option bufresize=auto
#pragma D option bufsize=100t

BEGIN
{
	exit(0);
}
