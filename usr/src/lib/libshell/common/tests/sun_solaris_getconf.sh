#!/bin/ksh93

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
# ident	"%Z%%M%	%I%	%E% SMI"
#

#
# sun_solaris_getconf.sh - test the ksh93 getconf builtin for compatibility 
# with /usr/bin/getconf
#

# test setup
function err_exit
{
	print -u2 -n "\t"
	print -u2 -r ${Command}[$1]: "${@:2}"
	let Errors+=1
}
alias err_exit='err_exit $LINENO'

Command=${0##*/}
integer Errors=0


# setup
integer mismatch     # counts mismatches between builtin and external command
integer getconf_keys # counts tests (paranoid check to make sure the test loop works)
export PATH=/usr/bin:/bin

# prechecks
[ ! -f "/bin/getconf" ] && err_exit '/bin/getconf not found.'
[ ! -x "/bin/getconf" ] && err_exit '/bin/getconf not executable.'

# compare builtin getconf output with /usr/bin/getconf
function compare_normal
{
    mismach=0 getconf_keys=0
    /usr/bin/getconf -a | 
        while read i ; do
            let getconf_keys++
            t="${i%:*}"

            a="$(getconf          "$t" 2>/dev/null)"
            b="$(/usr/bin/getconf "$t" 2>/dev/null)"

            if [ "$a" != "$b" ] ; then
                print -u2 "getconf/normal built mismatch: |$t|:|$a| != |$b|"
                let mismatch++
            fi
        done
}

# compare builtin getconf output with /usr/bin/getconf while passing a path argument
function compare_path
{
    mismach=0 getconf_keys=0
    /usr/bin/getconf -a | 
        while read i ; do
            let getconf_keys++
            t="${i%:*}"

            a="$(getconf          "$t" "/tmp" 2>/dev/null)"
            b="$(/usr/bin/getconf "$t" "/tmp" 2>/dev/null)"

            if [ "$a" != "$b" ] ; then
                print -u2 "getconf/path built mismatch: |$t|:|$a| != |$b|"
                let mismatch++
            fi
        done
}

# future versions of this test should test the following ${PATH}s, too:
# "/usr/xpg6/bin:/usr/xpg4/bin:/bin:/usr/bin" \
#"/usr/xpg4/bin:/bin:/usr/bin" \
for i in \
    "/bin:/usr/bin"
do
    export PATH="${i}"

    ## test whether the getconf builtin is available
    if [ "$(builtin | fgrep "/bin/getconf")" = "" ] ; then
        err_exit '/bin/getconf not found in the list of builtins.'
    fi


    ## compare "getconf -a" output
    if [ "$(getconf -a)" != "$(/usr/bin/getconf -a)" ] ; then
        err_exit 'getconf -a output mismatch.'
    fi


    ## check for a key which is only supported by the AST builtin version of getconf:
    if [ "$(getconf LIBPREFIX)" != "lib" ] ; then
        err_exit 'getconf LIBPREFIX did not return "lib".'
    fi


    ## run normal test
    compare_normal
    [ ${getconf_keys} -eq 0 ] && err_exit "getconf/normal not working (PATH=${PATH})."
    [ ${mismatch}     -gt 0 ] && err_exit "getconf/normal test found ${mismatch} differences (PATH=${PATH})."
    
    # run the same test in a seperate shell
    # (we explicitly test this because ast-ksh.2007-01-11 picks up /usr/xpg6/bin/getconf
    # if /usr/xpg6/bin/ comes in ${PATH} before /usr/bin (this happens only of ${PATH}
    # contains /usr/xpg6/bin before ksh93 is started)).
    ${SHELL} -c "integer mismatch ; \
        integer getconf_keys ; \
        $(functions) ; \
        compare_normal ;
        [ \${getconf_keys} -eq 0 ] && err_exit \"getconf/normal not working (PATH=\${PATH}).\" ; \
        [ \${mismatch}     -gt 0 ] && err_exit \"getconf/normal test found \${mismatch} differences (PATH=\${PATH}).\" ; \
        exit $((Errors))"
    let Errors+=$?


    ## run test with path argument
    compare_path
    [ ${getconf_keys} -eq 0 ] && err_exit "getconf/path not working."
    [ ${mismatch}     -gt 0 ] && err_exit "getconf/path test found ${mismatch} differences."

    # run the same test in a seperate shell
    # (see comment above)
    ${SHELL} -c "integer mismatch ; \
        integer getconf_keys ; \
        $(functions) ; \
        compare_path ;
        [ \${getconf_keys} -eq 0 ] && err_exit \"getconf/normal not working (PATH=\${PATH}).\" ; \
        [ \${mismatch}     -gt 0 ] && err_exit \"getconf/normal test found \${mismatch} differences (PATH=\${PATH}).\" ; \
        exit $((Errors))"
    let Errors+=$?
done

# test done
exit $((Errors))

