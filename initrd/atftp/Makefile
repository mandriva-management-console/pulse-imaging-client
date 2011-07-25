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

extra_bin = $(extra_srcdir)/atftp
extra_version = 0.7
extra_archive = atftp_$(extra_version).orig.tar.gz
extra_uri = ftp://archive.debian.org/debian-archive/debian/pool/main/a/atftp/$(extra_archive)
patch_srcdir = $(extra_srcdir)

include $(topdir)/common.mk
include $(topdir)/extra.mk
include $(topdir)/patch.mk

all-local: $(extra_bin)

$(extra_bin): 
	$(MAKE) $(extra_srcdir)/Makefile
	$(MAKE) -C $(extra_srcdir) atftp
	strip $@

$(extra_srcdir)/Makefile: patched-srcdir
	cd $(extra_srcdir) && ./configure --disable-libreadline

install-local:
	mkdir -p $(DESTDIR)$(initramfsdir)/bin
	$(install_BIN) $(extra_srcdir)/atftp $(DESTDIR)$(initramfsdir)/bin/

CLEANFILES += $(extra_bin)
