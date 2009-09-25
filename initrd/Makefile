#
# (c) 2009 Mandriva, http://www.mandriva.com
#
# $Id: Makefile 4505 2009-09-23 13:42:56Z nrueff $
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

CC=gcc-3.4

include consts.mk

all: initrd initrdcd
	exit 1

initrd: pump
	exit 1

pump: popt
	[ -e $(PUMP_TARGET)/$(PUMP_TARBALL) ] || wget $(PUMP_URI)/$(PUMP_TARBALL) -P $(PUMP_TARGET)
	[ -d $(PUMP_FOLDER) ] || tar zxf $(PUMP_TARGET)/$(PUMP_TARBALL) -C $(PUMP_TARGET)
	$(MAKE) -C $(PUMP_FOLDER) CC=$(CC) LDFLAGS='-L../../$(POPT_FOLDER)/.libs' DEB_CFLAGS='-I../../$(POPT_FOLDER)'

popt:
	[ -e $(POPT_TARGET)/$(POPT_TARBALL) ] || wget $(POPT_URI)/$(POPT_TARBALL) -P $(POPT_TARGET)
	[ -d $(POPT_FOLDER) ] || tar zxf $(POPT_TARGET)/$(POPT_TARBALL) -C $(POPT_TARGET)
	[ -f $(POPT_FOLDER)/Makefile ] || (cd $(POPT_FOLDER); ./configure)
	$(MAKE) -C $(POPT_FOLDER) CC=$(CC)

clean:
	-$(MAKE) clean -C $(POPT_FOLDER)
	rm -f $(POPT_FOLDER)/Makefile
	-$(MAKE) clean -C $(PUMP_FOLDER)
