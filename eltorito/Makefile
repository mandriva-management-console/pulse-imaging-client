#
# (c) 2010-2011 Mandriva, http://www.mandriva.com
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
topdir = $(abspath ..)

extra_bin = $(extra_srcdir)/stage2/stage2_eltorito
extra_version = 0.97
extra_archive = grub-$(extra_version).tar.gz
extra_uri = ftp://alpha.gnu.org/gnu/grub/$(extra_archive)
patch_srcdir = $(extra_srcdir)

include $(topdir)/common.mk
include $(topdir)/extra.mk
include $(topdir)/patch.mk

CLEANFILES += $(extra_bin)

all-local: $(extra_bin)

$(extra_bin): $(extra_srcdir)/Makefile
	$(MAKE) -C $(extra_srcdir)/stage2 stage2_size.h
	$(MAKE) -C $(extra_srcdir)/stage2 stage2_eltorito

$(extra_srcdir)/Makefile:
	$(MAKE) patched-srcdir
	cd $(extra_srcdir) && \
	  aclocal-1.9 && automake-1.9 && autoconf && \
	  ./configure

install-local:
	mkdir -p $(bootloaderdir)
	$(install_ro_DATA) $(extra_bin) $(bootloaderdir)/cdrom_boot
