#
# CDDL HEADER START
#
# The contents of this file are subject to the terms of the
# Common Development and Distribution License (the "License").
# You may not use this file except in compliance with the License.
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
# Copyright 2006 Sun Microsystems, Inc.  All rights reserved.
# Use is subject to license terms.
#
# ident	"%Z%%M%	%I%	%E% SMI"
#
# usr/src/lib/hal/libhal-storage/Makefile.com
#

LIBRARY =	libhal-storage.a
VERS =		.1.0.0
VERS_MAJ =	.1
OBJECTS =	libhal-storage.o
LIBPCSRC =	hal-storage.pc

include ../../Makefile.com

LIBS =		$(DYNLIB) $(LINTLIB)
LDLIBS +=	-lc -ldbus-1 -lhal
$(LINTLIB) := 	SRCS = $(SRCDIR)/$(LINTSRC)

SRCDIR =	../common

CFLAGS +=	$(CCVERBOSE)
CFLAGS +=	-_gcc=-Wno-deprecated-declarations
CPPFLAGS +=	-DGETTEXT_PACKAGE=\"$(HAL_GETTEXT_PACKAGE)\" -DENABLE_NLS
CPPFLAGS +=	-DPACKAGE_LOCALE_DIR=\"/usr/lib/locale\"
CPPFLAGS +=	-I$(ROOT)/usr/include/hal

ROOTMAJLINK =	$(ROOTLIBDIR)/$(LIBRARY:.a=.so)$(VERS_MAJ)

.KEEP_STATE:

all:		$(LIBS)

lint:		lintcheck

$(ROOTMAJLINK):
	-$(RM) $@; $(SYMLINK) $(DYNLIB) $@

include $(SRC)/lib/Makefile.targ
