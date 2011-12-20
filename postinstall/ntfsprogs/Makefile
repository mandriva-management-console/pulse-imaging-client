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

extra_bin = $(extra_srcdir)/ntfsprogs/.libs/ntfsresize
extra_version = 2.0.0
extra_archive = ntfsprogs-$(extra_version).tar.bz2
extra_uri = http://downloads.sourceforge.net/project/linux-ntfs/NTFS%20Tools%20and%20Library/$(extra_archive)

# TODO: enable-crypto
configure_opts = --disable-crypto

include $(topdir)/common.mk
include $(topdir)/extra.mk

all-local: $(extra_bin)

$(extra_bin): $(extra_srcdir)/Makefile
	$(MAKE) -C $(extra_srcdir)

$(extra_srcdir)/Makefile:
	$(MAKE) $(extra_srcdir)
	cd $(extra_srcdir) && ./configure $(configure_opts)

install-local:
	mkdir -p $(DESTDIR)$(postinstdir)/bin
	$(install_BIN) $(extra_bin) $(DESTDIR)$(postinstdir)/bin/ntfsresize
	mkdir -p $(DESTDIR)$(postinstdir)/lib
	$(install_DATA) $(extra_srcdir)/libntfs/.libs/libntfs.so.10.0.0 \
	  $(DESTDIR)$(postinstdir)/lib/libntfs.so.10.0.0
	ln -sf libntfs.so.10.0.0 $(DESTDIR)$(postinstdir)/lib/libntfs.so.10
	ln -sf libntfs.so.10.0.0 $(DESTDIR)$(postinstdir)/lib/libntfs.so

clean:
	$(MAKE) -C $(extra_srcdir) clean

