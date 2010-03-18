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
. /usr/lib/revolib.sh
. /etc/netinfo.sh

# CDROM restoration: already mounted
grep -q revosavedir=/cdrom /etc/cmdline && exit 0

# Check if we are saving or restoring
grep -q revorestore /etc/cmdline && I_M_RESTORING=1

# get the mac address
MAC=`cat /etc/shortmac`

# Other restoration types
SRV=$Next_server

PREFIX=`grep revobase /etc/cmdline | sed 's|.*revobase=\([^ ]*\).*|\1|'`
if [ ! -z "$PREFIX" ]; then

    # get the base image dir
    SAVEDIR=`grep revosavedir /etc/cmdline | sed 's|.*revosavedir=\([^ ]*\).*|\1|'`
    if [ -z "$SAVEDIR" ]; then
	whiptail --title 'Error' --msgbox '   It seems that something weird happened : \n\n\n        I did not received SAVEDIR\n\n\n  Please contact your system administrator.\n\n          Press any key to reboot.' 15 50
        exit 1
    fi

    # get the base info dir
    INFODIR=`grep revoinfodir /etc/cmdline | sed 's|.*revoinfodir=\([^ ]*\).*|\1|'`
    if [ -z "$INFODIR" ]; then
	whiptail --title 'Error' --msgbox '   It seems that something weird happened : \n\n\n        I did not received INFODIR\n\n\n  Please contact your system administrator.\n\n          Press any key to reboot.' 15 50
        exit 1
    fi

    # get the computer UUID
    COMPUTER_UUID=`cat /etc/COMPUTER_UUID`
    if [ -z "$COMPUTER_UUID" ]; then
	whiptail --title 'Error' --msgbox '   It seems that something weird happened : \n\n\n     I did not received a Computer UUID\n\n\n  Please contact your system administrator.\n\n          Press any key to reboot.' 15 50
        exit 1
    fi
    INFODIR="/$INFODIR/$COMPUTER_UUID"

    if [ -z "$I_M_RESTORING" ]; then # get the image UUID, if we are saving
	IMAGE_UUID=`cat /etc/IMAGE_UUID`
	if [ -z "$IMAGE_UUID" ]; then
	    whiptail --title 'Error' --msgbox '   It seems that something weird happened : \n\n\n     I did not received an Image UUID\n\n\n  Please contact your system administrator.\n\n          Press any key to reboot.' 15 50
            exit 1
	fi
	SAVEDIR="/$SAVEDIR/$IMAGE_UUID"
    else # image uuid given on the command line
	IMAGE_UUID=`grep revoimage /etc/cmdline | sed 's|.*revoimage=\([^ ]*\).*|\1|'`
	if [ -z "$IMAGE_UUID" ]; then
	    whiptail --title 'Error' --msgbox '   It seems that something weird happened : \n\n\n      I did not received an Image UUID\n\n\n  Please contact your system administrator.\n\n          Press any key to reboot.' 15 50
            exit 1
	fi
	SAVEDIR="/$SAVEDIR/$IMAGE_UUID"
    fi

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
