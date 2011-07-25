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

tools = halt init killall5 runlevel shutdown
extra_bin = $(initramfsdir)/src/init
extra_version = 2.84
extra_archive = sysvinit_$(extra_version).orig.tar.gz
extra_uri = ftp://archive.debian.org/debian-archive/debian/pool/main/s/sysvinit/$(extra_archive)
patch_srcdir = $(extra_srcdir)

include $(topdir)/common.mk
include $(topdir)/extra.mk
include $(topdir)/patch.mk

all-local: $(extra_bin)

$(extra_bin): 
	$(MAKE) patched-srcdir
	$(MAKE) -C $(extra_srcdir)/src

install-local:
	mkdir -p $(DESTDIR)$(initramfsdir)/sbin
	for tool in $(tools); do \
	  $(install_BIN) $(extra_srcdir)/src/$$tool $(DESTDIR)$(initramfsdir)/sbin/; \
	done

CLEANFILES += $(patsubst %,$(extra_srcdir)/src/%,$(tools))
