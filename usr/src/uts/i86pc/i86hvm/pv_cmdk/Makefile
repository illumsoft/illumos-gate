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
# uts/i86pc/pv_cmdk/Makefile
#
# Copyright 2008 Sun Microsystems, Inc.  All rights reserved.
# Use is subject to license terms.
#
#ident	"%Z%%M%	%I%	%E% SMI"
#
#	This makefile drives the production of the xdc driver.
#
#	i86pc implementation architecture dependent
#

#
#	Path to the base of the uts directory tree (usually /usr/src/uts).
#
UTSBASE	= ../../..

#
#	Define the module and object file sets.
#
MODULE		= cmdk
OBJECTS		= $(PV_CMDK_OBJS:%=$(OBJS_DIR)/%)
LINTS		= $(PV_CMDK_OBJS:%.o=$(LINTS_DIR)/%.ln)
ROOTMODULE	= $(ROOT_HVM_DRV_DIR)/$(MODULE)

#
#	Include common rules.
#
include $(UTSBASE)/i86pc/i86hvm/Makefile.i86hvm

#
# When generating lint libraries, we want the name of the lint module
# that will be generated to by pv_cmdk and not cmdk, so override the
# default lint module name here.
#
LINT_MODULE	= pv_cmdk

#
#	Define targets
#
ALL_TARGET	= $(BINARY)
LINT_TARGET	= $(LINT_MODULE).lint
INSTALL_TARGET	= $(BINARY) $(ROOTMODULE)

LDFLAGS		+= -dy -Nmisc/strategy -Nmisc/cmlb
LDFLAGS		+= -Ndrv/xpvd -Ndrv/xdf

#
# The Xen header files do not lint cleanly.  Since the troublesome
# structures form part of the externally defined interface to the
# hypervisor, we're stuck with the noise.
#
LINTTAGS	+= -erroff=E_BAD_PTR_CAST_ALIGN
LINTTAGS	+= -erroff=E_SUPPRESSION_DIRECTIVE_UNUSED
LINTTAGS	+= -erroff=E_ASSIGN_NARROW_CONV

#
#	Default build targets.
#
.KEEP_STATE:

def:		$(DEF_DEPS)

all:		$(ALL_DEPS)

clean:		$(CLEAN_DEPS)

clobber:	$(CLOBBER_DEPS)

lint:		$(LINT_DEPS)

modlintlib:	$(MODLINTLIB_DEPS)

clean.lint:	$(CLEAN_LINT_DEPS)

install:	$(INSTALL_DEPS)

#
#	Include common targets.
#
include $(UTSBASE)/i86pc/i86hvm/Makefile.targ
