#
# CDDL HEADER START
#
# The contents of this file are subject to the terms of the
# Common Development and Distribution License, Version 1.0 only
# (the "License").  You may not use this file except in compliance
# with the License.
#
# You can obtain a copy of the license at usr/src/OPENSOLARIS.LICENSE
# or http://www.opensolaris.org/os/licensing.
# See the License for the specific language governing permissions
# and limitations under the License.
#
# When distributing Covered Code, include this CDDL HEADER in each
# file and include the License file at usr/src/OPENSOLARIS.LICENSE.
# If applicable, add the following below this CDDL HEADER, with the
# fields enclosed by brackets "[]" replaced with your own identifying
# information: Portions Copyright [yyyy] [name of copyright owner]
#
# CDDL HEADER END
#
#
#pragma ident	"%Z%%M%	%I%	%E% SMI"
#
# Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
# Use is subject to license terms.
#
# common/crypto/arcfour/Makefile
#
# include global definitions
include $(SRC)/Makefile.master

.KEEP_STATE:

FRC:

# EXPORT DELETE START
EXPORT_SRC:
	$(RM) Makefile+ arcfour_crypt.c+ sun4u/arcfour_crypt_asm.s+
	sed -e "/EXPORT DELETE START/,/EXPORT DELETE END/d" \
		< arcfour_crypt.c > arcfour_crypt.c+
	$(MV) arcfour_crypt.c+ arcfour_crypt.c
	sed -e "/EXPORT DELETE START/,/EXPORT DELETE END/d" \
		< sun4u/arcfour_crypt_asm.s > sun4u/arcfour_crypt_asm.s+
	$(MV) sun4u/arcfour_crypt_asm.s+ sun4u/arcfour_crypt_asm.s
	sed -e "/^# EXPORT DELETE START/,/^# EXPORT DELETE END/d" \
		< Makefile > Makefile+
	$(RM) Makefile
	$(MV) Makefile+ Makefile 
	$(CHMOD) 444 Makefile arcfour_crypt.c sun4u/arcfour_crypt_asm.s

# EXPORT DELETE END
