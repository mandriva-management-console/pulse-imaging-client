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

for module in `cat ../../src_initrd/trunk/tree/etc/modules | grep -v '^#' | grep -v '^$'`; do
   if [ -f build/modules/$module.ko ]; then
     deps=`strings build/modules/$module.ko | grep '^depends='`
     if [ "$deps" == "depends=" ]; then
       echo "$module: OK"
     else
       deps=`echo $deps | sed 's!^depends=!!' | sed 's!,! !g'`
       bad=0
       for dep in $deps; do
         grep -q "^$dep\$" ../../src_initrd/trunk/tree/etc/modules || bad=1
       done
       if [ $bad -eq 1 ]; then
         echo "$module: $deps: DEPS NOT LOADED"
       else
         echo "$module: $deps: OK"
       fi
       continue
     fi
   else
     echo "$module: ERROR LOADED BUT NOT INSTALLED"
   fi
done
