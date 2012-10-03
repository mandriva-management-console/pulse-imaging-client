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

extra_bin = $(extra_srcdir)/dban.bzi
extra_version = 2.2.7
extra_archive = dban-$(extra_version)_i586.iso
extra_uri = http://garr.dl.sourceforge.net/project/dban/dban/dban-$(extra_version)/$(extra_archive)

include $(topdir)/common.mk
include $(topdir)/extra.mk
include $(topdir)/patch.mk

all-local: $(extra_bin)

$(extra_srcdir): $(extra_archive)
	tmpdir=$(shell mktemp -d --tmpdir=$(CURDIR)) && \
	mount -oloop $< $$tmpdir/ && \
	cp -a $$tmpdir $@ && \
	umount $$tmpdir && \
	rm -rf $$tmpdir;

$(extra_bin): $(extra_srcdir)

install-local:
	mkdir -p $(DESTDIR)$(disklessdir)
	$(install_DATA) $(extra_bin) $(DESTDIR)$(disklessdir)/dban

clean-local:
	rm -rf $(extra_srcdir)
