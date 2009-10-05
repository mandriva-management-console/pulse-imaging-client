#
# (c) 2009 Mandriva, http://www.mandriva.com
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

SVNREV:=$(shell echo $Rev$ | tr -cd [[:digit:]])

FOLDER_BOOTLOADER	= bootloader
FOLDER_KERNEL		= kernel
FOLDER_TOOLS		= tools
FOLDER_INITRD		= initrd

BUILD_FOLDER		= build
LOOP_FOLDER			= loop

include $(FOLDER_BOOTLOADER)/consts.mk
include $(FOLDER_KERNEL)/consts.mk
include $(FOLDER_TOOLS)/consts.mk
include $(FOLDER_INITRD)/consts.mk

help:
	@echo -e Available options:
	@echo -e \\t	+ bootloader \\t: PXE-related stuff
	@echo -e \\t	+ kernel \\t: kernel stuff
	@echo -e \\t	+ tools \\t: imaging tools
	@echo -e \\t	+ initrd \\t: the initrd itself
	@echo -e \\t	+ imagin \\t: the previous targets AND assembly

imaging: kernel bootloader tools initrd
	[ -d $(BUILD_FOLDER) ] || cp -a $(FOLDER_INITRD)/tree $(BUILD_FOLDER)

kernel:
	make -C $(FOLDER_KERNEL) SVNREV=$(SVNREV)

bootloader:
	make -C $(FOLDER_BOOTLOADER) SVNREV=$(SVNREV)

tools:
	make -C $(FOLDER_TOOLS) SVNREV=$(SVNREV)

initrd:
	make -C $(FOLDER_INITRD) SVNREV=$(SVNREV)

clean:
	make clean -C $(FOLDER_BOOTLOADER)
	make clean -C $(FOLDER_KERNEL)
	make clean -C $(FOLDER_TOOLS)
	make clean -C $(FOLDER_INITRD)

dist-clean:
	make dist-clean -C $(FOLDER_BOOTLOADER)
	make dist-clean -C $(FOLDER_KERNEL)
	make dist-clean -C $(FOLDER_TOOLS)
	make dist-clean -C $(FOLDER_INITRD)
