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
# Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
# Use is subject to license terms.
#
# ident	"%Z%%M%	%I%	%E% SMI"
#

LIBRARY= ../libplotbg.a

OBJECTS = arc.o box.o circle.o close.o cont.o dot.o erase.o label.o \
	line.o linemod.o move.o open.o point.o space.o 

# include library definitions
include ../../../../lib/Makefile.lib

C99MODE=	$(C99_DISABLE)

# Specifically request the construction of a static library.
# This library is not installed in the proto area.
LIBS = $(LIBRARY)

BUILD.AR = $(AR) $(ARFLAGS) $@ `$(LORDER) $(AROBJS) | $(TSORT)`

# TXTS allows the AT&T makefile to be bootstrapped into the NSE.

CLOBBERFILES= $(LIBRARY)

.KEEP_STATE:

all: $(LIBS)

include ../../../../lib/Makefile.targ

