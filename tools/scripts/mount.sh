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

# get the mac address
MAC=`cat /etc/shortmac`

# Other restoration types
SRV=$Next_server

PREFIX=`grep revobase /etc/cmdline | sed 's|.*revobase=\([^ ]*\).*|\1|'`
if [ ! -z "$PREFIX" ]; then
    # get the image UUID
    IMAGE_UUID=`cat /etc/IMAGE_UUID`
    # get the computer UUID
    COMPUTER_UUID=`cat /etc/COMPUTER_UUID`

    if [ -z "$IMAGE_UUID" ]; then
        echo "No image UUID received; giving up !"
        exit 1
    fi

    if [ -z "$COMPUTER_UUID" ]; then
        echo "No computer UUID received; giving up !"
        exit 1
    fi

    SAVEDIR=`grep revosavedir /etc/cmdline | sed 's|.*revosavedir=\([^ ]*\).*|\1|'`
    if [ -z "$SAVEDIR" ]; then
        echo "No SAVEDIR received; giving up !"
        exit 1
    fi
    SAVEDIR="/$SAVEDIR/$IMAGE_UUID"

    INFODIR=`grep revoinfodir /etc/cmdline | sed 's|.*revoinfodir=\([^ ]*\).*|\1|'`
    if [ -z "$INFODIR" ]; then
        echo "No INFODIR received; giving up !"
        exit 1
    fi
    INFODIR="/$INFODIR/$COMPUTER_UUID"

else
    # uses the original boot file name to guess the NFS prefix ("LRS mode")
    PREFIX=`echo $Boot_file | sed 's|/revoboot.pxe$||' | sed 's|/bin$||'`

    # directories below the NFS prefix
    SAVEDIR=`grep revosavedir /etc/cmdline | sed 's|.*revosavedir=\([^ ]*\).*|\1|'`
    INFODIR="$SAVEDIR"

    # shared backup ?
    if echo $SAVEDIR | grep -q /imgbase; then
        INFODIR="/images/$MAC"
    fi

fi
echo "Mounting Storage directory... mount-$TYPE.sh $SRV $PREFIX $SAVEDIR $INFODIR"
while ! mount-$TYPE.sh $SRV $PREFIX $SAVEDIR $INFODIR
do
    sleep 1
done

cat /proc/mounts
