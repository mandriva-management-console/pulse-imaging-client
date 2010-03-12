#!/bin/bash -e

#
# (c) 2010 Mandriva, http://www.mandriva.com
#
# $Id$
#
# This file is part of Pulse 2.
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
# along with Pulse 2.  If not, see <http://www.gnu.org/licenses/>.

export LANG=C
export LC_ALL=C

echo "Pulse 2 Imaging client basic auto-installation script"
echo

if [ ! -f "/bin/lsb_release" ];
then
    echo "Please install lsb_release."
    echo "urpmi lsb-release"
    exit 1
fi	

if [ -z $TMPCO ];
    then
    TMPCO=`mktemp -d`
    TMPREMOVE=1
    pushd $TMPCO
    # Check out Pulse 2 imaging client
    svn co http://mds.mandriva.org/svn/mmc-projects/pulse2/client/imaging/trunk pulse2-client-imaging
else
    pushd $TMPCO
fi

pushd pulse2-client-imaging
make install-prebuild PREFIX=/usr
popd

popd

if [ ! -z $TMPREMOVE ];
    then
    rm -fr $TMPCO
fi

echo "Installation done successfully"

exit 0