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

extra_bin = $(extra_srcdir)/tools/lvm
extra_version = 2.02.97
extra_archive = LVM2.$(extra_version).tgz
extra_uri = ftp://sources.redhat.com/pub/lvm2/$(extra_archive)

patch_srcdir = $(extra_srcdir)

include $(topdir)/common.mk
include $(topdir)/extra.mk
include $(topdir)/patch.mk

all-local: $(extra_bin)

$(extra_bin): 
	$(MAKE) $(extra_srcdir)/Makefile
	$(MAKE) -C $(extra_srcdir) tools
	strip $@

$(extra_srcdir)/Makefile: patched-srcdir
	cd $(@D) && \
	  ./configure --disable-selinux --disable-readline --enable-static_link

install-local:
	mkdir -p $(DESTDIR)$(initramfsdir)/bin
	cp $(extra_srcdir)/tools/lvm.static $(extra_srcdir)/tools/lvm
	$(install_BIN) $(extra_srcdir)/tools/lvm $(DESTDIR)$(initramfsdir)/bin/

CLEANFILES += $(extra_bin)
