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

#
# To use these rules, you must define:
# - module_path: path to module generated module in source tree
# - module_archive: archive containing sources 
#
# Optionally, you can define:
# - module_uri: to retrieve archive
# - module_builddir: dir containing the Kbuild file (or compatible Makefile), 
#   relative to source tree root. (default: source tree root)
# - EXTRA_MAKEFLAGS will be pass to make when building module
#
# If a `patches' dir is found, it is supposed to be a quilt like patches dir,
# with appropriate `series' files. Patches must apply on source tree root.
#
linux_srcdir = $(topdir)/kernel/$(extra_srcdir)
module_srcdir = $(extra_srcdir)
patch_srcdir = $(module_srcdir)
module_builddir ?=
EXTRA_MAKEFLAGS ?= 

include $(topdir)/common.mk
include $(topdir)/extra.mk
include $(topdir)/patch.mk

abs_builddir = $(abspath $(CURDIR))/$(extra_srcdir)/$(module_builddir)
MAKE_MODULE = $(MAKE) -C $(linux_srcdir) \
		ARCH=$(ARCHITECTURE) \
		SUBDIRS=$(abs_builddir) \
		KSRC=$(linux_srcdir) $(EXTRA_MAKEFLAGS)

all-local: $(module_path)

$(module_path): 
	$(MAKE) patched-srcdir
	$(MAKE_MODULE)

install-local:
	mkdir -p $(initramfsdir)/lib/modules
	$(install_DATA) $(module_path) $(initramfsdir)/lib/modules/
	mkdir -p $(initcdfsdir)/lib/modules
	$(install_DATA) $(module_path) $(initcdfsdir)/lib/modules/

clean-local:
	if test -d "$(abs_builddir)"; then \
	  $(MAKE_MODULE) clean; \
	fi
