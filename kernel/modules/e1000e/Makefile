# (c) 2003-2007 Linbox FAS, http://linbox.com
# (c) 2008-2009 Mandriva, http://www.mandriva.com
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

module = e1000e.ko
module_path = $(extra_srcdir)/src/$(module)
module_version = 2.5.4
extra_archive = e1000e-$(module_version).tar.gz
# Don't update to 2.X until this bug is closed!
# https://sourceforge.net/p/e1000/bugs/1/
extra_uri = http://freefr.dl.sourceforge.net/project/e1000/e1000e%20stable/$(module_version)/$(extra_archive)
module_builddir = src

include $(topdir)/kernel/modules.mk
