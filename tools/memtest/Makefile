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
topdir = $(abspath ../..)

extra_bin = $(extra_srcdir)/memtest
extra_version = 4.20
extra_archive = memtest86+-$(extra_version).tar.gz
extra_uri = http://www.memtest.org/download/$(extra_version)/$(extra_archive)
patch_srcdir = $(extra_srcdir)

include $(topdir)/common.mk
include $(topdir)/extra.mk
include $(topdir)/patch.mk

all-local: $(extra_bin)

$(extra_bin):
	$(MAKE) patched-srcdir
	$(MAKE) -C $(extra_srcdir)

install-local:
	mkdir -p $(DESTDIR)$(disklessdir)
	$(install_DATA) $(extra_srcdir)/memtest $(DESTDIR)$(disklessdir)/memtest

CLEANFILES += $(extra_bin)
