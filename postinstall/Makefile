#
# (c) 2010-2011 Mandriva, http://www.mandriva.com
#
# Author(s):
#  Nicolas Rueff
#  Jean Parpaillon
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
SUBDIRS = chntpw fuse ntfs3g parted ntfsprogs ddrescue

topdir = $(abspath ..)
include $(topdir)/common.mk
include $(topdir)/consts.mk

install-local:
	mkdir -p $(DESTDIR)$(postinstalldir)/bin
	$(install_BIN) bin/mountwin $(DESTDIR)$(postinstalldir)/bin/mountwin
	mkdir -p $(DESTDIR)$(postinstalldir)/lib
	$(install_DATA) lib/libpostinst.sh $(DESTDIR)$(postinstalldir)/lib/libpostinst.sh
	mkdir -p $(DESTDIR)$(postinstalldir)/winutils
	$(install_DATA) winutils/newsid.exe $(DESTDIR)$(postinstalldir)/winutils/newsid.exe
