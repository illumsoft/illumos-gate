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
# Copyright 2008 Sun Microsystems, Inc.  All rights reserved.
# Use is subject to license terms.
#
#pragma ident	"@(#)Makefile	1.1	08/06/25 SMI"
#

include ../Makefile.com

DATAFILES += i.manifest r.manifest i.logadmconf
LICENSEFILES += $(OSBL)
CDDL=

MMSHDR = $(ROOT)/usr/include/mms

FILES += $(MMSHDR)/mms.h
FILES += $(MMSHDR)/mms_api.h
FILES += $(MMSHDR)/mms_api_msg.h
FILES += $(MMSHDR)/mms_cat.h
FILES += $(MMSHDR)/mms_dmd.h
FILES += $(MMSHDR)/mms_err.h
FILES += $(MMSHDR)/mms_list.h
FILES += $(MMSHDR)/mms_network.h
FILES += $(MMSHDR)/mms_parser.h
FILES += $(MMSHDR)/mms_strapp.h
FILES += $(MMSHDR)/mms_sym.h
FILES += $(MMSHDR)/mms_trace.h

all: $(FILES) depend

install: all pkg

include ../Makefile.targ

$(MMSHDR):
	$(INS.dir)

$(MMSHDR)/%: $(MMSHDR) $(SRC)/common/mms/mms/%
	$(INS.file)
