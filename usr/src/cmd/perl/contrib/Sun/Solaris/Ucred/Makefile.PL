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
#ident	"%Z%%M%	%I%	%E% SMI"
#
# Makefile.PL for ::Ucred
#

require 5.6.1;
use strict;
use warnings;
use ExtUtils::MakeMaker;

# #defines.
my @defines = ( DEFINE => exists($ENV{RELEASE_BUILD}) ? '-DNDEBUG' : '' );

# List of POD pages to install.
my @man3pods = ( MAN3PODS => {} );

#
# If not building as part of ON.
#
if (! exists($ENV{ENVCPPFLAGS1})) {

	#
	# Suppress the setting of LD_RUN_PATH.  The ON build environment
	# contains a modified MakeMaker that does this automatically, so we
	# only need to do this if we are building outside of ON.
	#
	package MY;
	no warnings qw(once);

	# Override const_loadlibs to remove LD_RUN_PATH cruft.
	*const_loadlibs = sub
	{
		my $self = shift(@_);
		delete($self->{LD_RUN_PATH});
		return($self->SUPER::const_loadlibs(@_));
	};

	# Override dynamic_lib to get rid of LD_RUN_PATH cruft.
	*dynamic_lib = sub
	{
		my $self = shift(@_);
		my $txt = $self->SUPER::dynamic_lib(@_);
		$txt =~ s/LD_RUN_PATH=\S*\s*//;
		return($txt);
	};

	# Turn off debugging.
	@defines = ();

	#
	# Install the POD documentation for non-ON builds.
	#
	my $man3pfx = '$(INST_MAN3DIR)/Sun::Solaris::Ucred';
	@man3pods = (
	    MAN3PODS => { 'pod/Ucred.pod' => $man3pfx . '.$(MAN3EXT)' }
	);
}

WriteMakefile(
    NAME         => 'Sun::Solaris::Ucred',
    VERSION_FROM => 'Ucred.pm',
    #PREREQ_PM    => { 'Sun::Solaris::Privilege' => '1.1' },

    @defines,
    @man3pods,
);
