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
#pragma	ident	"%Z%%M%	%I%	%E% SMI"
#
# cmd/sa/Makefile
#

MANIFEST =	sar.xml
SVCMETHOD =	svc-sar

include ../Makefile.cmd

ROOTMANIFESTDIR = $(ROOTSVCSYSTEM)

GREP=		grep

SADC= 		sadc
SADP= 		sadp
SAR= 		sar
TIMEX=		timex
SAG=		sag
SA1=		sa1
SA2=		sa2

sadc :=		LDLIBS += -lkstat

# Executables produced
BINPROG=	$(TIMEX)
SBINPROG=	$(SAR) $(SAG)
LIBPROG=	$(SADC)
LIBSHELL=	$(SA1) $(SA2)
INITSHELL=	$(PERF)

PROGS=		$(BINPROG) $(SBINPROG) $(LIBPROG)
SHELLS=		$(LIBSHELL)
TXTS= 		$(SADP).c README
ALL=		$(PROGS) $(SHELLS)

# Source files
SAG_OBJECTS=	$(SAG)a.o $(SAG)b.o
SADC_OBJECTS=	$(SADC).o
srcs=		$(TIMEX) $(SAR) $(SADC)
SRCS=		$(srcs:%=%.c) $(SAG_OBJECTS:%.o=%.c)
SHSRCS=		$(SHELLS:%=%.sh)

# Set of target install directories
LIBSAD=		$(ROOT)/usr/lib/sa
CROND=		$(ROOT)/var/spool/cron
CRONTABSD=	$(CROND)/crontabs

# Set of target install files
SYSCRONTAB=	$(CRONTABSD)/sys
ROOTPROG=	$(BINPROG:%=$(ROOTBIN)/%)
ROOTUSBINPROG=	$(SBINPROG:%=$(ROOTUSRSBIN)/%)
ROOTLIBPROG=	$(LIBPROG:%=$(LIBSAD)/%)
ROOTLIBSHELL=	$(LIBSHELL:%=$(LIBSAD)/%)
ROOTSYMLINKS=	$(SBINPROG:%=$(ROOTBIN)/%)

# Performance monitoring should not be enabled by default. Hence, these
# entries are comments. Note the difficulty of inserting a literal #
# in a makefile.... Wonderful parser here....
COMMENT_CHAR:sh=          echo \\043
ENTRY1=		'$(COMMENT_CHAR) 0 * * * 0-6 /usr/lib/sa/sa1'
ENTRY2=		'$(COMMENT_CHAR) 20,40 8-17 * * 1-5 /usr/lib/sa/sa1'
ENTRY3=		'$(COMMENT_CHAR) 5 18 * * 1-5 /usr/lib/sa/sa2 -s 8:00 -e 18:01 -i 1200 -A'

CLOBBERFILES=	$(PROGS) $(SHELLS)

# Conditionals
$(SYSCRONTAB)		:= OWNER = root
$(SYSCRONTAB)     	:= GROUP = sys
$(ROOTBIN)/$(TIMEX)	:= GROUP = sys
# $(ROOTUSRSBIN)/$(SADP) 	:= FILEMODE = 2555
# $(ROOTUSRSBIN)/$(SADP) 	:= GROUP = sys
$(LIBSAD)/$(SADC) 	:= FILEMODE = 0555
$(LIBSAD)/$(SADC) 	:= GROUP = bin

.KEEP_STATE:

all: 		$(ALL) $(TXTS)

$(SAG):		$(SAG_OBJECTS)
	$(LINK.c) -o $@ $(SAG_OBJECTS) $(LDLIBS)
	$(POST_PROCESS)

$(SADC):	$(SADC_OBJECTS)
	$(LINK.c) -o $@ $(SADC_OBJECTS) $(LDLIBS)
	$(POST_PROCESS)

# The edit of SYSCRONTAB must be done unconditionally because of the
# creation of this file by a different component (Adm) and the possible
# backdating.
install:	all $(ROOTPROG) $(ROOTUSBINPROG)     \
		$(ROOTINITSHELL)  $(ROOTLIBSHELL) $(ROOTSYMLINKS) \
		$(ROOTMANIFEST) $(ROOTSVCMETHOD) $(ROOTLIBPROG)
	@if [ -f $(SYSCRONTAB) ]; \
	then \
		if $(GREP) "sa1" $(SYSCRONTAB) >/dev/null 2>&1 ; then :; \
		else \
			echo $(ENTRY1) >> $(SYSCRONTAB); \
			echo $(ENTRY2) >> $(SYSCRONTAB); \
			echo "$(SYSCRONTAB) modified"; \
		fi; \
		if $(GREP) "sa2" $(SYSCRONTAB) >/dev/null 2>&1 ; then :; \
		else \
			echo $(ENTRY3) >> $(SYSCRONTAB); \
		fi; \
	fi

$(LIBSAD)/%: %
	$(INS.file)

$(ROOTSYMLINKS):
	-$(RM) $@; $(SYMLINK) ../sbin/`basename $@` $@

$(ETCINITD)/%: %
	$(INS.file)

check:	$(CHKMANIFEST)

clean:
	$(RM) $(SAG_OBJECTS) $(SADC_OBJECTS) $(PROGS) $(SHELLS) $(SADP)

lint:	lint_SRCS

include ../Makefile.targ
