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

include ../../Makefile.lx

DISTROS =	centos35.distro centos36.distro centos37.distro \
		centos38.distro rhel35.distro rhel36.distro rhel37.distro \
		rhel38.distro rhel_centos_common

ROOTDISTRODIR=	$(ROOTBRANDDIR)/distros
ROOTDISTROS=	$(DISTROS:%=$(ROOTDISTRODIR)/%)

$(ROOTDISTROS) :=	FILEMODE = 444
$(ROOTDISTROS) :=	OWNER = root
$(ROOTDISTROS) :=	GROUP = bin

$(ROOTDISTRODIR):
	$(INS.dir)

$(ROOTDISTRODIR)/%: %
	$(INS.file)

install: $(ROOTDISTROS)

lint clean all:

clobber:
	-$(RM) $(ROOTDISTROS)

