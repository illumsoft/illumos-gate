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
# lib/libprtdiag_psr/sparc/montecarlo/Makefile

LIBBASE   = ../../../../../src/lib
UTSBASE   = ../../../../uts
PLATFORM_OBJECTS= montecarlo.o

include ../Makefile.com

IFLAGS = -I$(USR_PLAT_DIR)/sun4u/include -I../../../libprtdiag/inc -I$(LIBBASE)/libdevinfo
IFLAGS += -I$(UTSBASE)/sun4u/sys
IFLAGS += -I$(UTSBASE)/sun4u/montecarlo/sys
LINTFLAGS += $(IFLAGS)
LDLIBS += -L$(LIBBASE)/libdevinfo -ldevinfo -L$(LIBBASE)/libcfgadm -lcfgadm \
	  -lkstat
#
# links in /usr/platform
#
LINKED_PLATFORMS	= SUNW,UltraSPARC-IIe-NetraCT-40

LINKED_DIRS	= $(LINKED_PLATFORMS:%=$(USR_PLAT_DIR)/%)
LINKED_LIB_DIRS	= $(LINKED_PLATFORMS:%=$(USR_PLAT_DIR)/%/lib)
LINKED_PRTDIAG_DIRS	= \
	$(LINKED_PLATFORMS:%=$(USR_PLAT_DIR)/%/lib/libprtdiag_psr.so.1)

#
# SUNW,UltraSPARC-IIi-Netract and SUNW,UltraSPARC-IIe-NetraCT-40 platform
# should install into SUNW,UltraSPARC-IIi-Netract.
# SUNW,UltraSPARC-IIe-NetraCT-40 platform can link to
# /usr/platform/SUNW,UltraSPARC-IIi-Netract/lib/libprtdiag_psr.so
#
PLATFORM=SUNW,UltraSPARC-IIi-Netract

.KEEP_STATE:

PLATLIBS= $(PLATFORM:%=$(USR_PLAT_DIR)/%/lib/)

install:	all $(PLATLIBS) $(USR_PSM_LIBS) \
		$(LINKED_PRTDIAG_DIRS)

#
# install rules for SUNW,UltraSPARC-IIi-Netract/lib/libprtdiag_psr.so
#
$(PLATLIBS):
	$(INS.dir)

$(USR_PSM_LIB_DIR):
	cd $(UTSBASE)/sun4u/montecarlo; $(MAKE) $(USR_PSM_LIB_DIR)

$(USR_PSM_LIB_DIR)/%: % $(USR_PSM_LIB_DIR)
	$(INS.file)

$(LINKED_DIRS):	$(USR_PLAT_DIR)
	-$(INS.dir.root.sys)

$(LINKED_LIB_DIRS):	$(LINKED_DIRS)
	-$(INS.dir.root.sys)

$(LINKED_PRTDIAG_DIRS):	$(LINKED_LIB_DIRS)
	-$(INS.slink6)


# New additions to generate msg file
POFILE =	libprtdiag_psr_montecarlo.po
POFILES =	montecarlo.po

_msg:	$(MSGDOMAIN) $(POFILE)
	$(RM) $(MSGDOMAIN)/$(POFILE)
	$(CP) $(POFILE) $(MSGDOMAIN)

$(POFILE):	$(POFILES)
		$(RM) $@
		$(CAT) $(POFILES) > $@
 
$(POFILES):
	$(RM) messages.po
	$(XGETTEXT) $(XGETFLAGS) `$(GREP) -l gettext common/montecarlo.c`
	$(SED) -e '/^# msg/d' -e '/^domain/d' messages.po > $@
	$(RM) messages.po
