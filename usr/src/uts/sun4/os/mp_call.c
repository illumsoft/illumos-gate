/*
 * CDDL HEADER START
 *
 * The contents of this file are subject to the terms of the
 * Common Development and Distribution License, Version 1.0 only
 * (the "License").  You may not use this file except in compliance
 * with the License.
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
 * Copyright 2005 Sun Microsystems, Inc.  All rights reserved.
 * Use is subject to license terms.
 */

#pragma ident	"%Z%%M%	%I%	%E% SMI"

/*
 * Facilities for cross-processor subroutine calls using "mailbox" interrupts.
 */

#include <sys/types.h>
#include <sys/thread.h>
#include <sys/cpuvar.h>
#include <sys/x_call.h>
#include <sys/systm.h>
#include <sys/machsystm.h>
#include <sys/intr.h>

/*
 * Interrupt another CPU.
 * 	This is useful to make the other CPU go through a trap so that
 *	it recognizes an address space trap (AST) for preempting a thread.
 *
 *	It is possible to be preempted here and be resumed on the CPU
 *	being poked, so it isn't an error to poke the current CPU.
 *	We could check this and still get preempted after the check, so
 *	we don't bother.
 */
void
poke_cpu(int cpun)
{
	uint32_t *ptr = (uint32_t *)&cpu[cpun]->cpu_m.poke_cpu_outstanding;

	/*
	 * If panicstr is set or a poke_cpu is already pending,
	 * no need to send another one. Use atomic swap to protect
	 * against multiple CPUs sending redundant pokes.
	 */
	if (panicstr || *ptr == B_TRUE ||
	    atomic_swap_32(ptr, B_TRUE) == B_TRUE)
		return;

	xt_one(cpun, setsoftint_tl1, poke_cpu_inum, 0);
}
