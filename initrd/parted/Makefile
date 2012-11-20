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

extra_bin = $(extra_srcdir)/parted/parted
extra_version = 2.3
extra_archive = parted-$(extra_version).tar.xz
extra_uri = http://ftp.gnu.org/gnu/parted/$(extra_archive)

patch_srcdir = $(extra_srcdir)

include $(topdir)/common.mk
include $(topdir)/extra.mk
include $(topdir)/patch.mk

all-local: $(extra_bin)

$(extra_bin):
	$(MAKE) $(extra_srcdir)/Makefile
	$(MAKE) -C $(extra_srcdir)

$(extra_srcdir)/Makefile: patched-srcdir
	cd $(@D) && \
	  ./configure --disable-device-mapper \
	              --without-readline \
                      --disable-shared

install-local:
	mkdir -p $(DESTDIR)$(initramfsdir)/sbin
	$(install_BIN) --strip $(extra_bin) $(DESTDIR)$(initramfsdir)/sbin/

clean:
	-$(MAKE) -C $(extra_srcdir) clean

CLEANFILES += $(extra_bin)
