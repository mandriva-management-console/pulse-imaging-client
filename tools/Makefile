#
# (c) 2011 Mandriva, http://www.mandriva.com
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
SUBDIRS = scripts reiserfsprogs revosave autosave autorestore ui_newt bench \
	ntblfix memtest

topdir = $(abspath ..)
include $(topdir)/common.mk

install-local:
	mkdir -p $(DESTDIR)$(libdir) $(DESTDIR)$(usrbindir)
	$(install_DATA) /usr/lib/libnewt.so* $(DESTDIR)$(libdir)/
	$(install_DATA) /lib/libslang.so* $(DESTDIR)$(libdir)/
	$(install_DATA) /usr/lib/libz.so* $(DESTDIR)$(libdir)/
	$(install_BIN) /usr/bin/whiptail $(DESTDIR)$(usrbindir)/
	$(install_DATA) /lib/libext2fs.so.* /usr/lib/libext2fs.so $(DESTDIR)$(libdir)/
	$(install_DATA) /lib/libcom_err.so.2* $(DESTDIR)$(libdir)/
