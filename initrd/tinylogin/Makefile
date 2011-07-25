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

links_bin = addgroup adduser delgroup deluser login su
links_usr_bin = passwd
links_sbin = getty sulogin

include $(topdir)/common.mk

install-local:
	for dir in sbin bin usr/bin; do \
	  mkdir -p $(initramfsdir)/$$dir; \
	done 
	for dest in $(links_bin); do \
	  ln -fs busybox $(initramfsdir)/bin/$$dest; \
	done
	for dest in $(links_usr_bin); do \
	  ln -fs ../../bin/busybox $(initramfsdir)/usr/bin/$$dest; \
	done
	for dest in $(links_sbin); do \
	  ln -fs ../bin/busybox $(initramfsdir)/sbin/$$dest; \
	done
