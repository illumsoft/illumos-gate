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

#include <unistd.h>
#include <netinet/in.h>
#include <libinetutil.h>

extern int getnetmaskbyaddr(const struct in_addr, struct in_addr *);

/*
 * Generic internet (v4) functions.
 */

/*
 * Given a host-order address, calculate client's default net mask.
 * Consult netmasks database to see if net is further subnetted.
 * We'll only snag the first netmask that matches our criteria.
 * We return the resultant netmask in host order.
 */
void
get_netmask4(const struct in_addr *n_addrp, struct in_addr *s_addrp)
{
	struct in_addr	hp, tp;

	/*
	 * First check if VLSM is in use.
	 */
	hp.s_addr = htonl(n_addrp->s_addr);
	if (getnetmaskbyaddr(hp, &tp) == 0) {
		s_addrp->s_addr = ntohl(tp.s_addr);
		return;
	}

	/*
	 * Fall back on standard classed networks.
	 */
	if (IN_CLASSA(n_addrp->s_addr))
		s_addrp->s_addr = IN_CLASSA_NET;
	else if (IN_CLASSB(n_addrp->s_addr))
		s_addrp->s_addr = IN_CLASSB_NET;
	else if (IN_CLASSC(n_addrp->s_addr))
		s_addrp->s_addr = IN_CLASSC_NET;
	else
		s_addrp->s_addr = IN_CLASSE_NET;
}
