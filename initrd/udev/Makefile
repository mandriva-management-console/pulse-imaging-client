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

extra_bin = $(extra_srcdir)/udev/udevd $(extra_srcdir)/udev/udevadm

extra_version = 164
extra_archive = udev_$(extra_version).orig.tar.gz
extra_uri = http://ftp.de.debian.org/debian/pool/main/u/udev/$(extra_archive)

patch_srcdir = $(extra_srcdir)

include $(topdir)/common.mk
include $(topdir)/extra.mk
include $(topdir)/patch.mk

all-local: $(extra_bin)

$(extra_bin): 
	$(MAKE) $(extra_srcdir)/Makefile
	$(MAKE) -C $(extra_srcdir) install DESTDIR=$(CURDIR)/$(extra_srcdir)/make_install
	strip $@

$(extra_srcdir)/Makefile: patched-srcdir
	cd $(@D) && \
	  ./configure --disable-introspection --disable-extras --disable-logging --libexecdir=/lib/udev --prefix=/

install-local:
	mkdir -p $(DESTDIR)$(initramfsdir)/sbin
	$(install_BIN) $(extra_srcdir)/make_install/sbin/udevd $(DESTDIR)$(initramfsdir)/sbin/
	$(install_BIN) $(extra_srcdir)/make_install/sbin/udevadm $(DESTDIR)$(initramfsdir)/sbin/
	mkdir -p $(DESTDIR)$(initramfsdir)/lib/udev
	cp -a $(extra_srcdir)/make_install/lib/udev/* $(DESTDIR)$(initramfsdir)/lib/udev/
	chown -R root:root $(DESTDIR)$(initramfsdir)/lib/udev/
	find $(DESTDIR)$(initramfsdir)/lib/udev/ -type f -exec chmod 644 {} \;
	find $(DESTDIR)$(initramfsdir)/lib/udev/ -type d -exec chmod 755 {} \;
