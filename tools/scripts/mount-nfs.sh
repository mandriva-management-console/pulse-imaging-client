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
# Mount helper script for NFS

SIP=$1
PREFIX=$2
SAVEDIR=$3
INFODIR=$4

. /usr/lib/revolib.sh
# Get DHCP options
. /etc/netinfo.sh

[ -n "$Option_177" ] && SIP=`echo $Option_177|cut -d : -f 1`

RPCINFO=`rpcinfo -p $SIP`

logger "rpcinfo:"
logger "$RPCINFO"

# check nfs
echo
echo
sleep 1
if echo "$RPCINFO"|grep -q nfs
then
    echo "*** NFS service seems to be ok on $SIP"
else
    echo "*** Warning : the NFS service does not seem to work on the LRS !"
    echo "*** IP configuration :"
    cat /etc/netinfo.log
    sleep 10
fi

# NFS base options
NFSOPT="hard,intr,ac,nfsvers=3,async"

# Get NFS proto
if grep -q 'revoproto=nfsudp' /etc/cmdline; then
    NFSOPT="$NFSOPT,proto=udp"
    echo "*** Using NFS over UDP"
else
    if echo "$RPCINFO" | grep nfs | grep -q tcp; then
        NFSOPT="$NFSOPT,proto=tcp"
        echo "*** Using NFS over TCP"
    else
        NFSOPT="$NFSOPT,proto=udp"
        echo "*** Using NFS over UDP"
    fi
fi

# Get NFS block size
if grep -q 'revonfsbsize' /etc/cmdline; then
    NFSBSIZE=`sed -e 's/.*revonfsbsize=\([^ ]*\).*/\1/' < /etc/cmdline`
    NFSOPT="$NFSOPT,rsize=$NFSBSIZE,wsize=$NFSBSIZE"
    echo "*** Using blocks of $NFSBSIZE bytes"
else
    echo "*** Autonegociating block size"
fi

# Full NFS bypass, if needed
if grep -q 'revonfsopts' /etc/cmdline; then
    NFSOPT=`sed -e 's/.*revonfsopts=\([^ ]*\).*/\1/' < /etc/cmdline`
    echo "*** Bypassing NFS options : $NFSOPT"
fi

echo "*** Using the following NFS options : $NFSOPT"

if [ -z "$Option_177" ]
then
    echo "*** Using $SIP:$PREFIX as backup dir"
    mount -t nfs $SIP:$PREFIX$INFODIR /revoinfo -o hard,intr,nolock,sync,$NFSOPT
    mount -t nfs $SIP:$PREFIX$SAVEDIR /revosave -o hard,intr,nolock,sync,$NFSOPT
else
    echo "*** Using Option 177: $Option_177 as backup dir"
    mount -t nfs $Option_177$INFODIR /revoinfo -o hard,intr,nolock,sync,$NFSOPT
    mount -t nfs $Option_177$SAVEDIR /revosave -o hard,intr,nolock,sync,$NFSOPT
fi

EXITCODE=$?

exit $EXITCODE
