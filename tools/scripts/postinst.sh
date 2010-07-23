#!/bin/sh
#
# (c) 2003-2007 Linbox FAS, http://linbox.com
# (c) 2008-2010 Mandriva, http://www.mandriva.com
#
# $Id$
#
# This file is part of Pulse 2, http://pulse2.mandriva.org
#
# Pulse 2 is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# Pulse 2 is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with Pulse 2; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
# MA 02110-1301, USA.
#
# Launch all postinstall scripts
#
# two cases : 
# "file"   : we "just" source it
# "folder" : launch "run-parts" over it

# Functions which can be interesting in the postinst scripts

# try either to source $basename, or to run-parts over $basename.d

. /usr/lib/revolib.sh
. /opt/lib/libpostinst.sh

getmac

run_script() {
    basename="$1"
    message="$2"

    # executable
    if [ -x $basename ]
    then
	pretty_warn "Executing $message post-installation script"
	/bin/revosendlog 6
        $basename
	/bin/revosendlog 7
    fi

    # not executable, source it
    if [ -r $basename ]
    then
	pretty_warn "Executing $message post-installation script"
	/bin/revosendlog 6
	set -v
        . $basename
	set +v
	/bin/revosendlog 7
    fi

    # not executable, source it
    if [ -d $basename.d ]
    then
	pretty_warn "Executing $message post-installation script"
	/bin/revosendlog 6
	/bin/run-parts -t $basename.d
	/bin/run-parts $basename.d
	/bin/revosendlog 7
    fi
}

run_script "/revoinfo/preinst" "Pre"
grep -q revosavedir /proc/cmdline && run_script "/revosave/postinst" "image"
grep -q revosavedir /proc/cmdline && run_script "/revoinfo/$MAC/postinst" "computer"
run_script "/revoinfo/postinst" "global"
