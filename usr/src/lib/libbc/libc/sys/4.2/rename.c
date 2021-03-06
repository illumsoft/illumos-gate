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
 * Copyright 2006 Sun Microsystems, Inc.  All rights reserved.
 * Use is subject to license terms.
 */

#pragma ident	"%Z%%M%	%I%	%E% SMI"

#include "chkpath.h"
#include <sys/syscall.h>

int
rename(char *path1, char *path2)
{
	char buf2[256];

	CHKNULL(path1);
	CHKNULL(path2);

	/*
	 * with the removal of utmp and wtmp, we now need
	 * to map the utmp or wtmp rename operation into
	 * a rename of utmpx or wtmpx
	 */
	if (strcmp(path1, "/etc/utmp") == 0 ||
	    strcmp(path1, "/var/adm/utmp") == 0) {
		path1 = "/var/adm/utmpx";
		strcpy(buf2, path2);
		strcat(buf2, "x");
		path2 = buf2;
	} else if (strcmp(path1, "/var/adm/wtmp") == 0) {
		path1 = "/var/adm/wtmpx";
		strcpy(buf2, path2);
		strcat(buf2, "x");
		path2 = buf2;
	}

	return (_syscall(SYS_rename, path1, path2));
}
