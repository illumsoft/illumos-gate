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
# Copyright 2007 Sun Microsystems, Inc.  All rights reserved.
# Use is subject to license terms.
#
# uts/sparc/audioixp/Makefile
#
# ident	"%Z%%M%	%I%	%E% SMI"
#
#	This makefile drives the production of hight definition audio
#	driver (audioixp) kernel module.
#
#
#	Path to the base of the uts directory tree (usually /usr/src/uts).
#
UTSBASE = ../..

#
#	Define the module and object file sets.
#
MODULE		= audioixp 
WARLOCK_OUT 	= $(AUDIOIXP_OBJS:%.o=%.ll)
WARLOCK_OK	= $(MODULE).ok
WLCMD_DIR       = $(UTSBASE)/common/io/warlock

#
#	Include common rules.
#
include $(UTSBASE)/sparc/Makefile.sparc

#
# For now, disable these lint checks; maintainers should endeavor
# to investigate and remove these for maximum lint coverage.
# Please do not carry these forward to new Makefiles.
#
LINTTAGS	+= -erroff=E_BAD_PTR_CAST_ALIGN

#
#	Default build targets.
#
.KEEP_STATE:

def:		warlock

clean:		$(CLEAN_DEPS)
		$(RM) $(WARLOCK_OUT) $(WARLOCK_OK)

clobber:	$(CLOBBER_DEPS)
		$(RM) $(WARLOCK_OUT) $(WARLOCK_OK)

#
#	Include common targets.
#
include $(UTSBASE)/sparc/Makefile.targ

#
#	Defines for local commands.
#
TEST		= test
WLCC		= wlcc
TOUCH		= touch
WARLOCK 	= warlock

#
#	Warlock targets
#
MIXER_FILES = $(MIXER_OBJS:%.o=$(UTSBASE)/sparc/mixer/%.ll)
AUDIO_SUP_FILES = $(AUDIO_SUP_OBJS:%.o=$(UTSBASE)/sparc/audiosup/%.ll)
AMSRC2_FILES = $(AMSRC2_OBJS:%.o=$(UTSBASE)/sparc/amsrc2/%.ll)

warlock:	$(WARLOCK_OK)

$(WARLOCK_OK):	$(WARLOCK_OUT) warlock_ddi.files \
	warlock_audiosup.files warlock_mixer.files warlock_amsrc2.files \
	$(WLCMD_DIR)/audioixp_with_sada.wlcmd
	$(WARLOCK) -c $(WLCMD_DIR)/audioixp_with_sada.wlcmd $(WARLOCK_OUT) \
	$(MIXER_FILES) $(AUDIO_SUP_FILES) $(AMSRC2_FILES) \
	-l ../warlock/ddi_dki_impl.ll
	$(TOUCH) $(WARLOCK_OK)

%.ll: $(UTSBASE)/common/io/audio/sada/drv/audioixp/%.c
	$(WLCC) $(CPPFLAGS) -DDEBUG -o $@ $<

warlock_standalone: $(WARLOCK_OUT) warlock_ddi.files $(WLCMD_DIR)/audioixp.wlcmd
	$(WARLOCK) -c $(WLCMD_DIR)/audioixp.wlcmd $(WARLOCK_OUT) \
	-l ../warlock/ddi_dki_impl.ll

warlock_ddi.files:
	@cd ../warlock; pwd; $(MAKE) warlock

warlock_audiosup.files:
	@cd ../audiosup; pwd; $(MAKE) warlock

warlock_amsrc2.files:
	@cd ../amsrc2; pwd; $(MAKE) warlock

warlock_mixer.files:
	@cd ../mixer; pwd; $(MAKE) warlock
