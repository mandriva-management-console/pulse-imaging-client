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
PUMP_TARGET			= build
PUMP_FOLDER			= $(PUMP_TARGET)/pump-$(PUMP_VERSION)
PUMP_TARBALL		= pump-$(PUMP_VERSION)-patched-2009.tar.gz

# POPT
POPT_VERSION		= 1.15
POPT_URI			= http://rpm5.org/files/popt
POPT_TARGET			= build
POPT_FOLDER			= $(POPT_TARGET)/popt-$(POPT_VERSION)
POPT_TARBALL		= popt-$(POPT_VERSION).tar.gz

# Busybox
http://www.busybox.net/downloads/legacy/busybox-0.60.5.tar.gz
