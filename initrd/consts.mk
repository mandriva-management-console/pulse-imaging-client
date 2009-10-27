#
# (c) 2009 Mandriva, http://www.mandriva.com
#
# $Id: Makefile 4505 2009-09-23 13:42:56Z nrueff $
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

# PUMP
PUMP_VERSION		= 0.8.24
PUMP_URI			= http://www.linuxfocus.org/~guido
PUMP_TARGET			= 3rd_party
PUMP_FOLDER			= $(PUMP_TARGET)/pump-$(PUMP_VERSION)
PUMP_TARBALL		= pump-$(PUMP_VERSION)-patched-2009.tar.gz

# POPT
POPT_VERSION		= 1.15
POPT_URI			= http://rpm5.org/files/popt
POPT_TARGET			= 3rd_party
POPT_FOLDER			= $(POPT_TARGET)/popt-$(POPT_VERSION)
POPT_TARBALL		= popt-$(POPT_VERSION).tar.gz

# Busybox
BUSYBOX_VERSION		= 0.60.5
BUSYBOX_URI			= http://www.busybox.net/downloads/legacy
BUSYBOX_TARGET		= 3rd_party
BUSYBOX_FOLDER		= $(BUSYBOX_TARGET)/busybox-$(BUSYBOX_VERSION)
BUSYBOX_TARBALL		= busybox-$(BUSYBOX_VERSION).tar.gz

# SysVInit
SYSV_VERSION		= 2.84
SYSV_URI			= ??
SYSV_TARGET			= build
SYSV_FOLDER			= $(SYSV_TARGET)/sysvinit-$(SYSV_VERSION)/src
SYSV_TARBALL		= sysvinit-$(SYSV_VERSION).tar.gz

# ATFTP
ATFTP_VERSION		= 0.7.dfsg
ATFTP_URI			= http://ftp.debian.org/debian/pool/main/a/atftp
ATFTP_TARGET		= 3rd_party
ATFTP_FOLDER		= $(ATFTP_TARGET)/atftp-$(ATFTP_VERSION)
ATFTP_TARBALL		= atftp_$(ATFTP_VERSION).orig.tar.gz
ATFTP_PATCH			= atftp_$(ATFTP_VERSION)-7.diff.gz

# LVM
LVM_VERSION			= 2.02.54
LVM_URI				= ftp://sources.redhat.com/pub/lvm2
LVM_TARGET			= 3rd_party
LVM_FOLDER			= $(LVM_TARGET)/LVM2.$(LVM_VERSION)
LVM_TARBALL			= LVM2.$(LVM_VERSION).tgz
