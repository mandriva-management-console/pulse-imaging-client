#
# (c) 2010 Mandriva, http://www.mandriva.com
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

CC		:= gcc-3.4
SVNREV		:=$(shell echo $Rev: 4491 $ | tr -cd [[:digit:]])

BUILD_FOLDER	= $(shell pwd)/build
3RDPARTY_FOLDER	= $(shell pwd)/3rd_party

GRUB_VERSION	= 0.95
GRUB_URI	= ftp://alpha.gnu.org/gnu/grub/
GRUB_FOLDER	= grub-$(GRUB_VERSION)
GRUB_TARBALL	= grub-$(GRUB_VERSION).tar.gz

all:
	[ -e $(GRUB_TARBALL) ] || wget $(GRUB_URI)/$(GRUB_TARBALL) -P .
	[ -d $(GRUB_FOLDER) ] || tar zxf $(GRUB_TARBALL)
	(for file in builtins_pulse2.c builtins_pulse2.h shared-pulse2.h zlib ;\
		do \
		[ -e $(GRUB_FOLDER)/stage2/"$$file" ] || cp -a "$$file" $(GRUB_FOLDER)/stage2/ ;\
		done \
	)
	-cat pulse2.diff | patch -N -p1 -d $(GRUB_FOLDER)
	[ -f  $(GRUB_FOLDER)/Makefile ] || (cd $(GRUB_FOLDER); CC=$(CC) ./configure)
	$(MAKE) -C $(GRUB_FOLDER)/stage2/ CC=$(CC) stage2_size.h stage2_eltorito
	cp -a $(GRUB_FOLDER)/stage2/stage2_eltorito stage2_eltorito-$(SVNREV)

clean:
	$(MAKE) clean -C $(GRUB_FOLDER)
	rm -fr $(GRUB_FOLDER)
	rm -f stage2_eltorito-*

distclean:
	$(MAKE) distclean -C $(GRUB_FOLDER)
