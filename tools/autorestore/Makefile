#
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
#
topdir = $(abspath ../..)
include $(topdir)/common.mk

bin_all = autorestore
script_all = revosendlog revowait revodoneimage revogetname \
	revogetuuid revosetdefault revoinc revoinv

CLEANFILES = $(bin_all)

CFLAGS = -DLBLIMAGEVER='1.4' -Os -D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64 -Wall
CPPFLAGS = -I$(lrs_topdir)
LDFLAGS = -L$(lrs_topdir) -llrs  -lz

all-local: $(bin_all)

%: %.c
	$(CC) $(CFLAGS) $(CPPFLAGS) $(LDFLAGS) -o $@ $^

install-local:
	mkdir -p $(DESTDIR)$(bindir)
	for i in $(bin_all); do \
	  $(install_BIN) $$i $(DESTDIR)$(bindir); \
	done
	for i in $(script_all); do \
	  $(install_BIN) $$i $(DESTDIR)$(bindir); \
	done
	mkdir -p $(DESTDIR)$(rcdir)
	$(install_BIN) revoboot $(DESTDIR)$(rcdir)/
	mkdir -p $(DESTDIR)$(usrlibdir)
	$(install_BIN) revolib.sh $(DESTDIR)$(usrlibdir)/revolib.sh

.PHONY += check-syntax
