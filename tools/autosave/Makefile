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
topdir = $(abspath ../..)
include $(topdir)/common.mk

CLEANFILES = autosave

CFLAGS=-Os -Wall
LDFLAGS = -L$(lrs_topdir) -llrs
CPPFLAGS = -I$(lrs_topdir)

all-local: autosave

autosave: autosave.c $(lrs_topdir)
	$(CC) $(CFLAGS) $(LDFLAGS) $(CPPFLAGS) -o $@ $<

check-syntax:
	$(CC) $(CFLAGS) -pedantic -fsyntax-only autosave.c

$(lrs_topdir)/liblrs.so:
	$(MAKE) -C $(lrs_topdir)

install-local:
	mkdir -p $(DESTDIR)$(bindir)
	$(install_BIN) autosave $(DESTDIR)$(bindir)/
	$(install_BIN) floppysave $(DESTDIR)$(bindir)/
