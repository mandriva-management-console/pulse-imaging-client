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

imaging: kernel bootloader tools initrd
	# initial tree
	rm -fr $(INITRAMFS_FOLDER) && mkdir -p $(INITRAMFS_FOLDER)
	tar c --exclude=\.svn -C initrd/tree . | tar x -C build/initramfs

	# gather tools arb
	cp -a $(FOLDER_TOOLS)/build/bin/* $(INITRAMFS_FOLDER)/bin
	cp -a $(FOLDER_TOOLS)/build/revobin/* $(INITRAMFS_FOLDER)/revobin
	cp -a $(FOLDER_TOOLS)/build/lib/* $(INITRAMFS_FOLDER)/lib
	cp -a $(FOLDER_TOOLS)/build/usr/bin/* $(INITRAMFS_FOLDER)/usr/bin
	cp -a $(FOLDER_TOOLS)/build/etc/init.d/* $(INITRAMFS_FOLDER)/etc/init.d

	# additionnal stuff under /bin
	cp -a postinst/postmount $(INITRAMFS_FOLDER)/bin
	cp -a postinst/dopostinst $(INITRAMFS_FOLDER)/bin
	cp -a postinst/doinitinst $(INITRAMFS_FOLDER)/bin

	# add modules
	cp -a $(FOLDER_KERNEL)/build/modules/*.ko $(INITRAMFS_FOLDER)/lib/modules
	cp -a $(FOLDER_KERNEL)/build/modules/cd $(INITRAMFS_FOLDER)/lib/modules

	(cd $(INITRAMFS_FOLDER); find . | cpio -o -H newc ) | gzip -c -9 > $(BUILD_FOLDER)/initrd-$(VERSION_LINUXKERNEL)-$(SVNREV).img.gz
	ln -sf initrd-$(VERSION_LINUXKERNEL)-$(SVNREV).img.gz $(BUILD_FOLDER)/initrd.img

kernel:
	$(MAKE) -C $(FOLDER_KERNEL) SVNREV=$(SVNREV)

bootloader:
	$(MAKE) -C $(FOLDER_BOOTLOADER) SVNREV=$(SVNREV)

tools:
	$(MAKE) -C $(FOLDER_TOOLS) SVNREV=$(SVNREV)

initrd:
	$(MAKE) -C $(FOLDER_INITRD) SVNREV=$(SVNREV)

target-clean:
	rm -fr $(INITRAMFS_FOLDER)

clean: target-clean
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
