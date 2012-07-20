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
topdir = $(abspath ../../..)

module = tg3.ko
module_path = $(extra_srcdir)/$(module)
module_version = 3.122n
extra_archive = tg3-$(module_version).tar.gz
extra_uri = http://www.broadcom.com/docs/driver_download/570x/$(extra_archive)

include $(topdir)/kernel/modules.mk

$(module_path): $(extra_srcdir)/tg3_flags.h

$(extra_srcdir)/tg3_flags.h:
	$(MAKE) patched-srcdir
	sed -i 's#^BCMCFGDIR=#BCMCFGDIR=$(linux_srcdir)#' $(extra_srcdir)/Makefile
	sed -i 's#^BCMSRCDIR=#BCMSRCDIR=$(linux_srcdir)#' $(extra_srcdir)/Makefile
	$(MAKE) -C $(extra_srcdir) CC=$(CC) ARCH=$(ARCHITECTURE) LINUXSRC=$(linux_srcdir) tg3_flags.h
