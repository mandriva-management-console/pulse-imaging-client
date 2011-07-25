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

extra_bin = $(extra_srcdir)/busybox
extra_version = 1.1.3
extra_archive = busybox-$(extra_version).tar.bz2
extra_uri = http://www.busybox.net/downloads/$(extra_archive)
patch_srcdir = $(extra_srcdir)

links_bin = [ ash basename bash cat chgrp chmod chown cp cut date dd df dirname \
	dmesg dos2unix du dutmp echo false grep gunzip gzip head hostname kill \
	ln ls mkdir mknod more mount mv pidof ping ps pwd reset rm rmdir run-parts \
	sed sh sleep sort stty sync tar tee telnet test touch tr true umount \
	uname uniq unix2dos wget zcat

links_usr_bin = chvt clear env find free id killall logger date tail touch tty \
	wc which whoami yes

links_sbin = chvt clear env find free id killall logger rdate tail touch tty \
	wc which whoami yes

include $(topdir)/common.mk
include $(topdir)/extra.mk
include $(topdir)/patch.mk

busybox_config = busybox-$(extra_version).config

all-local: $(extra_bin)

$(extra_bin): 
	$(MAKE) $(extra_srcdir)/.config
	# Force sequential as parallel fails
	$(MAKE) -j1 -C $(extra_srcdir)
	strip $@

$(extra_srcdir)/.config: $(busybox_config) patched-srcdir
	cp $< $@

install-local:
	for dir in sbin bin usr/bin; do \
	  mkdir -p $(initramfsdir)/$$dir; \
	done 
	cp $(extra_srcdir)/busybox $(initramfsdir)/bin/
	for dest in $(links_bin); do \
	  ln -fs busybox $(initramfsdir)/bin/$$dest; \
	done
	for dest in $(links_usr_bin); do \
	  ln -fs ../../bin/busybox $(initramfsdir)/usr/bin/$$dest; \
	done
	for dest in $(links_sbin); do \
	  ln -fs ../bin/busybox $(initramfsdir)/sbin/$$dest; \
	done

CLEANFILES += $(extra_bin)
