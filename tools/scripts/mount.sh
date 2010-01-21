#!/bin/sh
#
# (c) 2003-2007 Linbox FAS, http://linbox.com
# (c) 2008-2009 Mandriva, http://www.mandriva.com
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
# Mount helper script
#
# $Id$
#

TYPE=nfs
. /etc/netinfo.sh

# CDROM restoration: already mounted
grep -q revosavedir=/cdrom /etc/cmdline && exit 0

# Other restoration types
SRV=$Next_server
PREFIX=`echo $Boot_file | sed 's|/revoboot.pxe$||' | sed 's|/bin$||'`
DIR=`cat /etc/cmdline | cut -f 1 -d " "|cut -f 2 -d =`
ROOT=`grep revoroot /etc/cmdline | sed 's|.* revoroot=\([^ ]*\).*|\1|'`

# revoroot= override prefix from PXE phase
[ -z "$ROOT" ] || PREFIX="$ROOT"
echo "Mounting Storage directory... mount-$TYPE.sh $SRV $PREFIX $DIR"
while ! mount-$TYPE.sh $SRV $PREFIX $DIR
do
    sleep 1
done

cat /proc/mounts
