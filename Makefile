#
# (c) 2009 Mandriva, http://www.mandriva.com
#
# $Id$(MAKE)file 4598 2009-10-05 14:00:24Z nrueff $
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

SVNREV:=$(shell echo $Rev: 4657 $ | tr -cd [[:digit:]])

FOLDER_BOOTLOADER	= bootloader
FOLDER_KERNEL		= kernel
FOLDER_TOOLS		= tools
FOLDER_INITRD		= initrd

include $(FOLDER_KERNEL)/consts.mk
include $(FOLDER_TOOLS)/consts.mk
include $(FOLDER_INITRD)/consts.mk

BUILD_FOLDER		= build
INITRAMFS_FOLDER	= $(BUILD_FOLDER)/initramfs
INITRAMFS			= initrd-$(VERSION_LINUXKERNEL)-$(SVNREV).img.gz

help:
	@echo -e Available options:
	@echo -e \\t	+ bootloader \\t: PXE-related stuff
	@echo -e \\t	+ kernel \\t: kernel stuff
	@echo -e \\t	+ tools \\t: imaging tools
	@echo -e \\t	+ initrd \\t: the initrd itself
	@echo -e \\t	+ imaging \\t: the previous targets AND assembly

imaging:
 #MDV/NR kernel bootloader tools initrd
	# initial tree
	[ -d $(INITRAMFS_FOLDER) ] || cp -a $(FOLDER_INITRD)/tree $(INITRAMFS_FOLDER)

	# additionnal stuff under /bin
	cp -a $(FOLDER_TOOLS)/autorestore/autorestore $(INITRAMFS_FOLDER)/bin
	cp -a $(FOLDER_TOOLS)/autorestore/revosendlog $(INITRAMFS_FOLDER)/bin
	cp -a $(FOLDER_TOOLS)/autorestore/revowait $(INITRAMFS_FOLDER)/bin
	cp -a $(FOLDER_TOOLS)/autorestore/revogetname $(INITRAMFS_FOLDER)/bin
	cp -a $(FOLDER_TOOLS)/autorestore/revosetdefault $(INITRAMFS_FOLDER)/bin
	cp -a $(FOLDER_TOOLS)/autorestore/revoinc $(INITRAMFS_FOLDER)/bin
	cp -a $(FOLDER_TOOLS)/autorestore/revoinv $(INITRAMFS_FOLDER)/bin
	cp -a $(FOLDER_TOOLS)/autosave/autosave $(INITRAMFS_FOLDER)/bin
	cp -a $(FOLDER_TOOLS)/autosave/floppysave $(INITRAMFS_FOLDER)/bin
	cp -a $(FOLDER_TOOLS)/bench/bench $(INITRAMFS_FOLDER)/bin
	cp -a $(FOLDER_TOOLS)/bench/bench.ping $(INITRAMFS_FOLDER)/bin
	#MDV/NR cp -a $(FOLDER_TOOLS)/ntblfix/ntblfix $(INITRAMFS_FOLDER)/bin
	cp -a $(FOLDER_TOOLS)/scripts/mount.sh $(INITRAMFS_FOLDER)/bin
	cp -a $(FOLDER_TOOLS)/scripts/mount-nfs.sh $(INITRAMFS_FOLDER)/bin
	cp -a $(FOLDER_TOOLS)/ui_newt/ui_newt $(INITRAMFS_FOLDER)/bin/uinewt # changed name
	cp -a postinst/postmount $(INITRAMFS_FOLDER)/bin
	cp -a postinst/dopostinst $(INITRAMFS_FOLDER)/bin
	cp -a postinst/doinitinst $(INITRAMFS_FOLDER)/bin

	# additionnal stuff under /revobin
	#MDV/NR cp -a $(FOLDER_TOOLS)/revosave/image_lvmreiserfs $(BUILD_FOLDER)/revobin
	cp -a $(FOLDER_TOOLS)/revosave/image_raw $(INITRAMFS_FOLDER)/revobin
	cp -a $(FOLDER_TOOLS)/revosave/image_swap $(INITRAMFS_FOLDER)/revobin
	cp -a $(FOLDER_TOOLS)/revosave/image_e2fs $(INITRAMFS_FOLDER)/revobin
	cp -a $(FOLDER_TOOLS)/revosave/image_fat $(INITRAMFS_FOLDER)/revobin
	cp -a $(FOLDER_TOOLS)/revosave/image_ntfs $(INITRAMFS_FOLDER)/revobin
	cp -a $(FOLDER_TOOLS)/revosave/image_jfs $(INITRAMFS_FOLDER)/revobin
	cp -a $(FOLDER_TOOLS)/revosave/image_ufs $(INITRAMFS_FOLDER)/revobin
	cp -a $(FOLDER_TOOLS)/revosave/image_lvm $(INITRAMFS_FOLDER)/revobin

	# additionnal stuff under /lib
	cp -f $(FOLDER_TOOLS)/revosave/liblrs.so.1 $(INITRAMFS_FOLDER)/lib

	# additionnal stuff under /etc
	cp -a $(FOLDER_TOOLS)/autorestore/revoboot $(INITRAMFS_FOLDER)/etc/init.d

	# add modules
	cp -a $(FOLDER_KERNEL)/build/modules/*.ko $(INITRAMFS_FOLDER)/lib/modules

	(cd $(INITRAMFS_FOLDER); find . | cpio -o -H newc ) | gzip -c -9 > $(BUILD_FOLDER)/$(INITRAMFS)

kernel:
	$(MAKE) -C $(FOLDER_KERNEL) SVNREV=$(SVNREV)

bootloader:
	$(MAKE) -C $(FOLDER_BOOTLOADER) SVNREV=$(SVNREV)

tools:
	$(MAKE) -C $(FOLDER_TOOLS) SVNREV=$(SVNREV)

initrd:
	$(MAKE) -C $(FOLDER_INITRD) SVNREV=$(SVNREV)

clean:
	rm -fr $(INITRAMFS_FOLDER)
	$(MAKE) clean -C $(FOLDER_BOOTLOADER)
	$(MAKE) clean -C $(FOLDER_KERNEL)
	$(MAKE) clean -C $(FOLDER_TOOLS)
	$(MAKE) clean -C $(FOLDER_INITRD)

dist-clean:
	rm -fr $(INITRAMFS_FOLDER)
	$(MAKE) dist-clean -C $(FOLDER_BOOTLOADER)
	$(MAKE) dist-clean -C $(FOLDER_KERNEL)
	$(MAKE) dist-clean -C $(FOLDER_TOOLS)
	$(MAKE) dist-clean -C $(FOLDER_INITRD)

.PHONY: kernel bootloader tools initrd
