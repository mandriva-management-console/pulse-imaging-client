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
PUMP_VERSION	= 0.8.24
PUMP_URI		= http://www.linuxfocus.org/~guido
PUMP_FOLDER		= pump-$(PUMP_VERSION)
PUMP_TARBALL	= pump-$(PUMP_VERSION)-patched-2009.tar.gz
PUMP_PATCH		= pump-$(PUMP_VERSION)-patched-2009.diff

# POPT
POPT_VERSION	= 1.15
POPT_URI		= http://rpm5.org/files/popt
POPT_FOLDER		= popt-$(POPT_VERSION)
POPT_TARBALL	= popt-$(POPT_VERSION).tar.gz

# Busybox
BUSYBOX_VERSION	= 1.1.3
BUSYBOX_URI		= http://www.busybox.net/downloads/legacy
BUSYBOX_FOLDER	= busybox-$(BUSYBOX_VERSION)
BUSYBOX_TARBALL	= busybox-$(BUSYBOX_VERSION).tar.gz
BUSYBOX_CONFIG	= busybox-$(BUSYBOX_VERSION).config

# SysVInit
SYSV_VERSION	= 2.84
SYSV_URI		= ftp://archive.debian.org/debian-archive/debian/pool/main/s/sysvinit
SYSV_FOLDER		= sysvinit-$(SYSV_VERSION).orig/src
SYSV_TARBALL	= sysvinit_$(SYSV_VERSION).orig.tar.gz
SYSV_PATCH		= sysvinit_$(SYSV_VERSION)-2woody1.diff.gz

# ATFTP
ATFTP_VERSION	= 0.7
ATFTP_URI		= ftp://archive.debian.org/debian-archive/debian/pool/main/a/atftp
ATFTP_FOLDER	= atftp-$(ATFTP_VERSION)
ATFTP_TARBALL	= atftp_$(ATFTP_VERSION).orig.tar.gz
ATFTP_PATCH		= atftp_$(ATFTP_VERSION)-7.diff.gz

# LVM
#MDV/NR LVM_VERSION		= 2.02.54
LVM_VERSION		= 2.01.04
#MDV/NR LVM_URI	= ftp://sources.redhat.com/pub/lvm2
LVM_URI			= ftp://sources.redhat.com/pub/lvm2/old
LVM_FOLDER		= LVM2.$(LVM_VERSION)
LVM_TARBALL		= LVM2.$(LVM_VERSION).tgz

# DM
DM_VERSION		= 1.01.00
DM_URI			= ftp://sources.redhat.com/pub/dm/old/
DM_FOLDER		= device-mapper.$(DM_VERSION)
DM_TARBALL		= device-mapper.$(DM_VERSION).tgz
