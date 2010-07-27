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
SHAREDIR		:= /usr/local/share/pulse2/imaging
PULSE2_OWNER		:= root
PULSE2_GROUP		:= root

INSTALL 		= $(shell which install)

SVNREV			:=$(shell echo $Rev$ | tr -cd [[:digit:]])

FOLDER_BOOTLOADER	= bootloader
FOLDER_KERNEL		= kernel
FOLDER_TOOLS		= tools
FOLDER_INITRD		= initrd
FOLDER_ELTORITO		= eltorito
FOLDER_POSTINST		= postinstall

include $(FOLDER_KERNEL)/consts.mk
include $(FOLDER_TOOLS)/consts.mk
include $(FOLDER_INITRD)/consts.mk

BUILD_FOLDER		:= build
PREBUILD_FOLDER		= prebuild-binaries
PREBUILD_BINARIES	= revoboot.pxe-$(SVNREV) pxe_boot stage2_eltorito-$(SVNREV) cdrom_boot bzImage-$(VERSION_LINUXKERNEL)-$(SVNREV) kernel initrd-$(VERSION_LINUXKERNEL)-$(SVNREV).img.gz initrd  initrdcd-$(VERSION_LINUXKERNEL)-$(SVNREV).img.gz initrdcd memtest-$(SVNREV) memtest chntpw fusermount ntfs-3g parted ntfsresize dd_rescue libntfs-3g.so libntfs-3g.so.76 libntfs-3g.so.76.0.0 libntfs.so libntfs.so.10 libntfs.so.10.0.0 libparted.so libparted.so.0 libparted.so.0.0.1
INITRAMFS_FOLDER	= $(BUILD_FOLDER)/initramfs
INITCDFS_FOLDER		= $(BUILD_FOLDER)/initcdfs

all : imaging

install-prebuild:
	# calls "install" target, with the following vars set:
	# BUILD_FOLDER set to PREBUILD_FOLDER
	# and SVNREV set to $(PREBUILD_FOLDER)/REVISION
	$(MAKE) install BUILD_FOLDER=$(PREBUILD_FOLDER) SVNREV=`cat $(PREBUILD_FOLDER)/REVISION` VERSION_LINUXKERNEL=`cat $(PREBUILD_FOLDER)/KERNEL_VERSION`

install:
	# bootloader stuff (revoboot + grub/eltorito)
	# everything is set RO
	$(INSTALL) -m 770 -o $(PULSE2_OWNER) -g $(PULSE2_GROUP) $(VARDIR)/bootloader -d
	$(INSTALL) -m 440 -o $(PULSE2_OWNER) -g $(PULSE2_GROUP) $(BUILD_FOLDER)/revoboot.pxe-$(SVNREV) $(VARDIR)/bootloader
	$(INSTALL) -m 440 -o $(PULSE2_OWNER) -g $(PULSE2_GROUP) $(BUILD_FOLDER)/pxe_boot $(VARDIR)/bootloader
	$(INSTALL) -m 440 -o $(PULSE2_OWNER) -g $(PULSE2_GROUP) $(BUILD_FOLDER)/stage2_eltorito-$(SVNREV) $(VARDIR)/bootloader
	$(INSTALL) -m 440 -o $(PULSE2_OWNER) -g $(PULSE2_GROUP) $(BUILD_FOLDER)/cdrom_boot $(VARDIR)/bootloader
	$(INSTALL) -m 440 -o $(PULSE2_OWNER) -g $(PULSE2_GROUP) contrib/bootsplash/bootsplash.xpm $(VARDIR)/bootloader

	# diskless stuff (kernel, initramfs, memtest)
	# everything is set RO
	$(INSTALL) -m 770 -o $(PULSE2_OWNER) -g $(PULSE2_GROUP) $(VARDIR)/diskless -d
	$(INSTALL) -m 440 -o $(PULSE2_OWNER) -g $(PULSE2_GROUP) $(BUILD_FOLDER)/bzImage-$(VERSION_LINUXKERNEL)-$(SVNREV) $(VARDIR)/diskless
	$(INSTALL) -m 440 -o $(PULSE2_OWNER) -g $(PULSE2_GROUP) $(BUILD_FOLDER)/kernel $(VARDIR)/diskless
	$(INSTALL) -m 440 -o $(PULSE2_OWNER) -g $(PULSE2_GROUP) $(BUILD_FOLDER)/initrd-$(VERSION_LINUXKERNEL)-$(SVNREV).img.gz $(VARDIR)/diskless
	$(INSTALL) -m 440 -o $(PULSE2_OWNER) -g $(PULSE2_GROUP) $(BUILD_FOLDER)/initrd $(VARDIR)/diskless
	$(INSTALL) -m 440 -o $(PULSE2_OWNER) -g $(PULSE2_GROUP) $(BUILD_FOLDER)/initrdcd-$(VERSION_LINUXKERNEL)-$(SVNREV).img.gz $(VARDIR)/diskless
	$(INSTALL) -m 440 -o $(PULSE2_OWNER) -g $(PULSE2_GROUP) $(BUILD_FOLDER)/initrdcd $(VARDIR)/diskless
	$(INSTALL) -m 440 -o $(PULSE2_OWNER) -g $(PULSE2_GROUP) $(BUILD_FOLDER)/memtest-$(SVNREV) $(VARDIR)/diskless
	$(INSTALL) -m 440 -o $(PULSE2_OWNER) -g $(PULSE2_GROUP) $(BUILD_FOLDER)/memtest $(VARDIR)/diskless

	# postinstall related stuff
	# everything is set RO
	$(INSTALL) -m 770 -o $(PULSE2_OWNER) -g $(PULSE2_GROUP) $(VARDIR)/postinst -d
	$(INSTALL) -m 770 -o $(PULSE2_OWNER) -g $(PULSE2_GROUP) $(VARDIR)/postinst/bin -d
	$(INSTALL) -m 555 -o $(PULSE2_OWNER) -g $(PULSE2_GROUP) $(FOLDER_POSTINST)/bin/mountwin $(VARDIR)/postinst/bin
	$(INSTALL) -m 555 -o $(PULSE2_OWNER) -g $(PULSE2_GROUP) $(BUILD_FOLDER)/chntpw $(VARDIR)/postinst/bin
	$(INSTALL) -m 555 -o $(PULSE2_OWNER) -g $(PULSE2_GROUP) $(BUILD_FOLDER)/fusermount $(VARDIR)/postinst/bin
	$(INSTALL) -m 555 -o $(PULSE2_OWNER) -g $(PULSE2_GROUP) $(BUILD_FOLDER)/ntfs-3g $(VARDIR)/postinst/bin
	$(INSTALL) -m 555 -o $(PULSE2_OWNER) -g $(PULSE2_GROUP) $(BUILD_FOLDER)/parted $(VARDIR)/postinst/bin
	$(INSTALL) -m 555 -o $(PULSE2_OWNER) -g $(PULSE2_GROUP) $(BUILD_FOLDER)/ntfsresize $(VARDIR)/postinst/bin
	$(INSTALL) -m 555 -o $(PULSE2_OWNER) -g $(PULSE2_GROUP) $(BUILD_FOLDER)/dd_rescue $(VARDIR)/postinst/bin
	$(INSTALL) -m 770 -o $(PULSE2_OWNER) -g $(PULSE2_GROUP) $(VARDIR)/postinst/lib -d
	$(INSTALL) -m 555 -o $(PULSE2_OWNER) -g $(PULSE2_GROUP) $(FOLDER_POSTINST)/lib/libpostinst.sh $(VARDIR)/postinst/lib
	$(INSTALL) -m 555 -o $(PULSE2_OWNER) -g $(PULSE2_GROUP) $(BUILD_FOLDER)/libntfs-3g.so* $(VARDIR)/postinst/lib
	$(INSTALL) -m 555 -o $(PULSE2_OWNER) -g $(PULSE2_GROUP) $(BUILD_FOLDER)/libparted.so* $(VARDIR)/postinst/lib
	$(INSTALL) -m 555 -o $(PULSE2_OWNER) -g $(PULSE2_GROUP) $(BUILD_FOLDER)/libntfs.so* $(VARDIR)/postinst/lib

prebuild:
	@echo This will updated binaries in $(PREBUILD_FOLDER), based on binaries in $(BUILD_FOLDER), at revision $(SVNREV), kernel at version $(VERSION_LINUXKERNEL)
	[ -d $(PREBUILD_FOLDER) ] || mkdir $(PREBUILD_FOLDER)
	(for i in $(PREBUILD_BINARIES); \
		do \
		cp -a "$(BUILD_FOLDER)/$$i" $(PREBUILD_FOLDER); \
	done)
	rm -f $(PREBUILD_FOLDER)/BUILDENV
	echo $(SVNREV) > $(PREBUILD_FOLDER)/REVISION
	echo $(VERSION_LINUXKERNEL) > $(PREBUILD_FOLDER)/KERNEL_VERSION
	echo '$$Rev$$' >> $(PREBUILD_FOLDER)/BUILDENV
	uname -a      >> $(PREBUILD_FOLDER)/BUILDENV

imaging: kernel bootloader tools initrd eltorito postinst
	# initial tree
	rm -fr $(INITRAMFS_FOLDER) && mkdir -p $(INITRAMFS_FOLDER)
	tar c --exclude=\.svn -C initrd/tree . | tar x -C build/initramfs

	# gather tools arb
	cp -a $(FOLDER_TOOLS)/build/bin/* $(INITRAMFS_FOLDER)/bin/
	cp -a $(FOLDER_TOOLS)/build/revobin/* $(INITRAMFS_FOLDER)/revobin/
	cp -a $(FOLDER_TOOLS)/build/lib/* $(INITRAMFS_FOLDER)/lib/
	cp -a $(FOLDER_TOOLS)/build/usr/bin/* $(INITRAMFS_FOLDER)/usr/bin/
	cp -a $(FOLDER_TOOLS)/build/etc/init.d/* $(INITRAMFS_FOLDER)/etc/init.d/
	cp -a $(FOLDER_TOOLS)/build/usr/lib/* $(INITRAMFS_FOLDER)/usr/lib/

	# add modules
	rm -f $(INITRAMFS_FOLDER)/lib/modules/*.ko
	cp -a $(FOLDER_KERNEL)/build/modules/*.ko $(INITRAMFS_FOLDER)/lib/modules

	# check all modules are there
	(cat "$(INITRAMFS_FOLDER)/etc/modules" | grep ^[a-z1-9] | while read i; do if [ ! -e "$(INITRAMFS_FOLDER)/lib/modules/$$i.ko" ]; then echo "Missing probed module : $$i"; exit 1; fi; done)
	(find "$(INITRAMFS_FOLDER)/lib/modules" -maxdepth 1 -type f | while read i; do j=`basename $$i .ko` ; grep -q ^$$j$$ "$(INITRAMFS_FOLDER)/etc/modules"; if [ $$? -ne 0 ]; then echo "Missing loaded module : $$i"; exit 1; fi; done)

	# check all libs are there
	(find "$(INITRAMFS_FOLDER)" -type f | while read i; do j=`echo "$$i" | sed "s|$(INITRAMFS_FOLDER)||"`; chroot "$(INITRAMFS_FOLDER)" /lib/ld-linux.so.2 --verify "$$j" >/dev/null || continue && chroot "$(INITRAMFS_FOLDER)" /lib/ld-linux.so.2 --list "$$j" >/dev/null || exit 1; done)

	# initial CD tree
	rm -fr $(INITCDFS_FOLDER) && mkdir -p $(INITCDFS_FOLDER)
	mkdir -p $(INITCDFS_FOLDER)/lib/modules
	cp -a $(FOLDER_KERNEL)/build/modules/cd $(INITCDFS_FOLDER)/lib/modules

	(cd $(INITRAMFS_FOLDER); find . | cpio -o -H newc ) | gzip -c -9 > $(BUILD_FOLDER)/initrd-$(VERSION_LINUXKERNEL)-$(SVNREV).img.gz
	ln -sf initrd-$(VERSION_LINUXKERNEL)-$(SVNREV).img.gz $(BUILD_FOLDER)/initrd
	(cd $(INITCDFS_FOLDER); find . | cpio -o -H newc ) | gzip -c -9 > $(BUILD_FOLDER)/initrdcd-$(VERSION_LINUXKERNEL)-$(SVNREV).img.gz
	ln -sf initrdcd-$(VERSION_LINUXKERNEL)-$(SVNREV).img.gz $(BUILD_FOLDER)/initrdcd

kernel:
	$(MAKE) -C $(FOLDER_KERNEL) SVNREV=$(SVNREV)
	cp -a $(FOLDER_KERNEL)/$(BUILD_FOLDER)/bzImage-$(VERSION_LINUXKERNEL)-$(SVNREV) $(BUILD_FOLDER)/bzImage-$(VERSION_LINUXKERNEL)-$(SVNREV)
	ln -sf bzImage-$(VERSION_LINUXKERNEL)-$(SVNREV) $(BUILD_FOLDER)/kernel

bootloader:
	$(MAKE) -C $(FOLDER_BOOTLOADER) SVNREV=$(SVNREV)
	cp -a $(FOLDER_BOOTLOADER)/revoboot.pxe-$(SVNREV) $(BUILD_FOLDER)/revoboot.pxe-$(SVNREV)
	ln -sf revoboot.pxe-$(SVNREV) $(BUILD_FOLDER)/pxe_boot

tools:
	$(MAKE) -C $(FOLDER_TOOLS) SVNREV=$(SVNREV)
	# see http://bugs.debian.org/cgi-bin/bugreport.cgi?bug=319837#33
	cp -a $(FOLDER_TOOLS)/$(BUILD_FOLDER)/$(MEMTEST_FOLDER)/memtest $(BUILD_FOLDER)/memtest-$(SVNREV)
	ln -sf memtest-$(SVNREV) $(BUILD_FOLDER)/memtest

initrd:
	$(MAKE) -C $(FOLDER_INITRD) SVNREV=$(SVNREV)

eltorito:
	$(MAKE) -C $(FOLDER_ELTORITO) SVNREV=$(SVNREV)
	cp -a $(FOLDER_ELTORITO)/stage2_eltorito-$(SVNREV) $(BUILD_FOLDER)/stage2_eltorito-$(SVNREV)
	ln -sf stage2_eltorito-$(SVNREV) $(BUILD_FOLDER)/cdrom_boot

postinst:
	$(MAKE) -C $(FOLDER_POSTINST) SVNREV=$(SVNREV)
	cp -a $(FOLDER_POSTINST)/build/chntpw.static $(BUILD_FOLDER)/chntpw
	cp -a $(FOLDER_POSTINST)/build/fusermount $(BUILD_FOLDER)/fusermount
	cp -a $(FOLDER_POSTINST)/build/ntfs-3g $(BUILD_FOLDER)/ntfs-3g
	cp -a $(FOLDER_POSTINST)/build/parted $(BUILD_FOLDER)/parted
	cp -a $(FOLDER_POSTINST)/build/ntfsresize $(BUILD_FOLDER)/ntfsresize
	cp -a $(FOLDER_POSTINST)/build/dd_rescue.bin $(BUILD_FOLDER)/dd_rescue
	cp -a $(FOLDER_POSTINST)/build/libntfs-3g.so* $(BUILD_FOLDER)/
	cp -a $(FOLDER_POSTINST)/build/libparted.so* $(BUILD_FOLDER)/
	cp -a $(FOLDER_POSTINST)/build/libntfs.so* $(BUILD_FOLDER)/

target-clean:
	rm -fr $(INITRAMFS_FOLDER)

clean: target-clean
	$(MAKE) clean -C $(FOLDER_BOOTLOADER)
	$(MAKE) clean -C $(FOLDER_KERNEL)
	$(MAKE) clean -C $(FOLDER_TOOLS)
	$(MAKE) clean -C $(FOLDER_INITRD)
	$(MAKE) clean -C $(FOLDER_ELTORITO)
	$(MAKE) clean -C $(FOLDER_POSTINST)

dist-clean:
	rm -fr $(INITRAMFS_FOLDER)
	$(MAKE) dist-clean -C $(FOLDER_BOOTLOADER)
	$(MAKE) dist-clean -C $(FOLDER_KERNEL)
	$(MAKE) dist-clean -C $(FOLDER_TOOLS)
	$(MAKE) dist-clean -C $(FOLDER_INITRD)
	$(MAKE) dist-clean -C $(FOLDER_ELTORITO)

.PHONY: kernel bootloader tools initrd eltorito prebuild
