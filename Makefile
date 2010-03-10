#
# (c) 2009-2010 Nicolas Rueff / Mandriva, http://www.mandriva.com
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
# along with Pulse 2. If not, see <http://www.gnu.org/licenses/>.
#

VARDIR 			:= /var/lib/pulse2/imaging
PULSE2_OWNER	:= root
PULSE2_GROUP	:= root

INSTALL = $(shell which install)

SVNREV:=$(shell echo $Rev$ | tr -cd [[:digit:]])

FOLDER_BOOTLOADER	= bootloader
FOLDER_KERNEL		= kernel
FOLDER_TOOLS		= tools
FOLDER_INITRD		= initrd
FOLDER_ELTORITO		= eltorito

include $(FOLDER_KERNEL)/consts.mk
include $(FOLDER_TOOLS)/consts.mk
include $(FOLDER_INITRD)/consts.mk

BUILD_FOLDER		= build
INITRAMFS_FOLDER	= $(BUILD_FOLDER)/initramfs

all : imaging

install:
	# bootloader stuff (revoboot)
	$(INSTALL) -m 550 -o $(PULSE2_OWNER) -g $(PULSE2_GROUP) $(VARDIR)/bootloader -d
	$(INSTALL) -m 440 -o $(PULSE2_OWNER) -g $(PULSE2_GROUP) $(BUILD_FOLDER)/revoboot.pxe-$(SVNREV) $(VARDIR)/bootloader
	$(INSTALL) -m 440 -o $(PULSE2_OWNER) -g $(PULSE2_GROUP) $(BUILD_FOLDER)/bootloader $(VARDIR)/bootloader
	$(INSTALL) -m 440 -o $(PULSE2_OWNER) -g $(PULSE2_GROUP) contrib/bootsplash/bootsplash.xpm $(VARDIR)/bootloader

	# diskless stuff (kernel and initramfs)
	$(INSTALL) -m 550 -o $(PULSE2_OWNER) -g $(PULSE2_GROUP) $(VARDIR)/diskless -d
	$(INSTALL) -m 440 -o $(PULSE2_OWNER) -g $(PULSE2_GROUP) $(BUILD_FOLDER)/bzImage-$(VERSION_LINUXKERNEL)-$(SVNREV) $(VARDIR)/diskless
	$(INSTALL) -m 440 -o $(PULSE2_OWNER) -g $(PULSE2_GROUP) $(BUILD_FOLDER)/kernel $(VARDIR)/diskless
	$(INSTALL) -m 440 -o $(PULSE2_OWNER) -g $(PULSE2_GROUP) $(BUILD_FOLDER)/initrd-$(VERSION_LINUXKERNEL)-$(SVNREV).img.gz $(VARDIR)/diskless
	$(INSTALL) -m 440 -o $(PULSE2_OWNER) -g $(PULSE2_GROUP) $(BUILD_FOLDER)/initrd $(VARDIR)/diskless

	# additionnal tools (memtest)
	$(INSTALL) -m 440 -o $(PULSE2_OWNER) -g $(PULSE2_GROUP) $(BUILD_FOLDER)/memtest-$(SVNREV) $(VARDIR)/diskless
	$(INSTALL) -m 440 -o $(PULSE2_OWNER) -g $(PULSE2_GROUP) $(BUILD_FOLDER)/memtest $(VARDIR)/diskless

imaging: kernel bootloader tools initrd eltorito
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
	ln -sf initrd-$(VERSION_LINUXKERNEL)-$(SVNREV).img.gz $(BUILD_FOLDER)/initrd

kernel:
	$(MAKE) -C $(FOLDER_KERNEL) SVNREV=$(SVNREV)
	cp -a $(FOLDER_KERNEL)/$(BUILD_FOLDER)/bzImage-$(VERSION_LINUXKERNEL)-$(SVNREV) $(BUILD_FOLDER)/bzImage-$(VERSION_LINUXKERNEL)-$(SVNREV)
	ln -sf bzImage-$(VERSION_LINUXKERNEL)-$(SVNREV) $(BUILD_FOLDER)/kernel

bootloader:
	$(MAKE) -C $(FOLDER_BOOTLOADER) SVNREV=$(SVNREV)
	cp -a $(FOLDER_BOOTLOADER)/revoboot.pxe-$(SVNREV) $(BUILD_FOLDER)/revoboot.pxe-$(SVNREV)
	ln -sf revoboot.pxe-$(SVNREV) $(BUILD_FOLDER)/bootloader

tools:
	$(MAKE) -C $(FOLDER_TOOLS) SVNREV=$(SVNREV)
	# see http://bugs.debian.org/cgi-bin/bugreport.cgi?bug=319837#33
	cp -a $(FOLDER_TOOLS)/$(BUILD_FOLDER)/$(MEMTEST_FOLDER)/memtest $(BUILD_FOLDER)/memtest-$(SVNREV)
	ln -sf memtest-$(SVNREV) $(BUILD_FOLDER)/memtest

initrd:
	$(MAKE) -C $(FOLDER_INITRD) SVNREV=$(SVNREV)

eltorito:
	$(MAKE) -C $(FOLDER_ELTORITO) SVNREV=$(SVNREV)
	cp -a $(FOLDER_ELTORITO)/eltorito-$(SVNREV) $(BUILD_FOLDER)/eltorito-$(SVNREV)
	ln -sf eltorito-$(SVNREV) $(BUILD_FOLDER)/eltorito

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
