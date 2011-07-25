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

extra_bin = $(extra_srcdir)/pump
extra_version = 0.8.24
extra_archive = pump-$(extra_version)-patched-2009.tar.gz
extra_uri = http://www.linuxfocus.org/~guido/$(extra_archive)

patch_srcdir = $(extra_srcdir)
popt_srcdir = $(topdir)/initrd/popt/$(extra_srcdir)

include $(topdir)/common.mk
include $(topdir)/extra.mk
include $(topdir)/patch.mk

all-local: $(extra_bin)

$(extra_bin):
	$(MAKE) $(popt_srcdir)/.libs/libpopt.so
	$(MAKE) patched-srcdir
	$(MAKE) -C $(extra_srcdir) \
	  LDFLAGS='-L$(popt_srcdir)/.libs' \
	  DEB_CFLAGS='-I$(popt_srcdir)'
	strip $@

$(popt_srcdir)/.libs/libpopt.so:
	$(MAKE) -C $(topdir)/initrd/popt

install-local:
	mkdir -p $(DESTDIR)$(initramfsdir)/bin
	$(install_BIN) $(extra_srcdir)/pump $(DESTDIR)$(initramfsdir)/bin/

CLEANFILES += $(extra_bin)
