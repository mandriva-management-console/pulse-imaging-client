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


# Check argument number
if [ ! $# -eq 2 ]; then
  echo
  echo "Usage: `basename $0` /path/to/etc/modules /path/to/lib/modules"
  echo
  exit 2
fi

# Check arguments looks valid
etcmodule=${1}
kodir=${2}
if [ -f ${etcmodule} ] && [ -d ${kodir} ]; then
  echo
  echo "Checking ${etcmodule} against ${kodir}.."
  echo
else
  echo
  echo "Usage: `basename $0` /path/to/etc/modules /path/to/lib/modules"
  echo
  exit 2
fi

# Here we go !
totalerrors=0
for module in `cat ${etcmodule} | grep -v '^#' | grep -v '^$'`; do
   if [ -f ${kodir}/${module}.ko ]; then
     deps=`strings ${kodir}/${module}.ko | grep '^depends='`
     if [ "${deps}" == "depends=" ]; then
       continue
     else
       deps=`echo ${deps} | sed 's!^depends=!!' | sed 's!,! !g'`
       bad=0
       for dep in ${deps}; do
         grep -q "^${dep}\$" ${etcmodule} || bad=1
       done
       if [ ${bad} -eq 1 ]; then
         echo "${module}: ${deps}: AT LEAST ONE DEPENDENCY MISSING"
         totalerrors=`expr ${totalerrors} + 1`
       fi
     fi
   else
     echo "${module}: ERROR LOADED BUT NOT INSTALLED"
     totalerrors=`expr ${totalerrors} + 1`
   fi
done

echo
exit ${totalerrors}
