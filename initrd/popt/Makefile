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

extra_base = libpopt.so
extra_bin = $(extra_srcdir)/.libs/$(extra_base).0.0.0
extra_version = 1.15
extra_archive = popt-$(extra_version).tar.gz
extra_uri = http://rpm5.org/files/popt/$(extra_archive)

include $(topdir)/common.mk
include $(topdir)/extra.mk

all-local: $(extra_bin)

$(extra_bin): 
	$(MAKE) $(extra_srcdir)/Makefile
	$(MAKE) -C $(extra_srcdir)
	strip $@

$(extra_srcdir)/Makefile: $(extra_srcdir)
	cd $(@D) && ./configure

install-local:
	mkdir -p $(DESTDIR)$(initramfsdir)/lib
	$(install_DATA) $(extra_bin) $(DESTDIR)$(initramfsdir)/lib/
	ln -sf libpopt.so.0.0.0 $(DESTDIR)$(initramfsdir)/lib/libpopt.so.0

CLEANFILES += $(extra_bin)*
