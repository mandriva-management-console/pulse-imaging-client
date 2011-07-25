# (c) 2003-2007 Linbox FAS, http://linbox.com
# (c) 2008-2011 Mandriva, http://www.mandriva.com
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
SUBDIRS = advise inflate

topdir = $(abspath ../..)
include $(topdir)/common.mk

bin_all = bench
obj_all = main.o compress.o disk.o nfs.o

CFLAGS	= -O2 -Wall -DBENCH
LDFLAGS = -lz

CLEANFILES += $(bin_all) $(obj_all)

all-local: $(bin_all)

bench: $(obj_all)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

install-local:
	mkdir -p $(DESTDIR)$(bindir)
	$(install_BIN) bench $(DESTDIR)$(bindir)
	$(install_BIN) bench.ping $(DESTDIR)$(bindir)
