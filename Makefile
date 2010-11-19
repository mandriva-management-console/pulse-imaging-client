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
PREBUILD_BINARIES	= \
			revoboot.pxe-$(SVNREV) 					pxe_boot		\
			stage2_eltorito-$(SVNREV)				cdrom_boot		\
			bzImage-$(VERSION_LINUXKERNEL)-$(SVNREV) 		kernel			\
			initrd-$(VERSION_LINUXKERNEL)-$(SVNREV).img.gz		initrd			\
			initrdcd-$(VERSION_LINUXKERNEL)-$(SVNREV).img.gz	initrdcd		\
			memtest-$(SVNREV)					memtest			\
			chntpw-$(SVNREV)					chntpw			\
			fusermount-$(SVNREV)					fusermount		\
			ntfs-3g-$(SVNREV)					ntfs-3g			\
			parted-$(SVNREV)					parted			\
			ntfsresize-$(SVNREV)					ntfsresize		\
			dd_rescue-$(SVNREV)					dd_rescue		\
			libntfs-3g.so-$(SVNREV)					libntfs-3g.so		\
			libntfs-3g.so.76-$(SVNREV)				libntfs-3g.so.76	\
			libntfs-3g.so.76.0.0-$(SVNREV)				libntfs-3g.so.76.0.0	\
			libntfs.so-$(SVNREV)					libntfs.so		\
			libntfs.so.10-$(SVNREV)					libntfs.so.10		\
			libntfs.so.10.0.0-$(SVNREV)				libntfs.so.10.0.0	\
			libparted.so-$(SVNREV)					libparted.so		\
			libparted.so.0-$(SVNREV)				libparted.so.0		\
			libparted.so.0.0.1-$(SVNREV)				libparted.so.0.0.1	\
			exportfs.ko-$(SVNREV)					exportfs.ko		\
			ext2.ko-$(SVNREV)					ext2.ko			\
			ext3.ko-$(SVNREV)					ext3.ko			\
			ext4.ko-$(SVNREV)					ext4.ko			\
			fuse.ko-$(SVNREV)					fuse.ko			\
			jbd.ko-$(SVNREV)					jbd.ko			\
			jbd2.ko-$(SVNREV)					jbd2.ko			\
			mbcache.ko-$(SVNREV) 					mbcache.ko		\
			reiserfs.ko-$(SVNREV)					reiserfs.ko		\
			xfs.ko-$(SVNREV) 					xfs.ko

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
	$(INSTALL) -m 770 -o $(PULSE2_OWNER) -g $(PULSE2_GROUP) -d $(DESTDIR)/$(VARDIR)/bootloader
	$(INSTALL) -m 440 -o $(PULSE2_OWNER) -g $(PULSE2_GROUP) -t $(DESTDIR)/$(VARDIR)/bootloader \
		$(BUILD_FOLDER)/pxe_boot		\
		$(BUILD_FOLDER)/cdrom_boot		\
		contrib/bootsplash/bootsplash.xpm

	# diskless stuff (kernel, initramfs, memtest)
	# everything is set RO
	$(INSTALL) -m 770 -o $(PULSE2_OWNER) -g $(PULSE2_GROUP) -d $(DESTDIR)/$(VARDIR)/diskless
	$(INSTALL) -m 440 -o $(PULSE2_OWNER) -g $(PULSE2_GROUP) -t $(DESTDIR)/$(VARDIR)/diskless \
		$(BUILD_FOLDER)/kernel		\
		$(BUILD_FOLDER)/initrd		\
		$(BUILD_FOLDER)/initrdcd	\
		$(BUILD_FOLDER)/memtest

	# postinstall related stuff
	# everything is set RO
	$(INSTALL) -m 770 -o $(PULSE2_OWNER) -g $(PULSE2_GROUP) -d $(DESTDIR)/$(VARDIR)/postinst/bin
	$(INSTALL) -m 550 -o $(PULSE2_OWNER) -g $(PULSE2_GROUP) -t $(DESTDIR)/$(VARDIR)/postinst/bin \
		$(FOLDER_POSTINST)/bin/mountwin	\
		$(BUILD_FOLDER)/chntpw		\
		$(BUILD_FOLDER)/fusermount	\
		$(BUILD_FOLDER)/ntfs-3g		\
		$(BUILD_FOLDER)/parted		\
		$(BUILD_FOLDER)/ntfsresize	\
		$(BUILD_FOLDER)/dd_rescue
	$(INSTALL) -m 770 -o $(PULSE2_OWNER) -g $(PULSE2_GROUP) -d $(DESTDIR)/$(VARDIR)/postinst/lib
	$(INSTALL) -m 440 -o $(PULSE2_OWNER) -g $(PULSE2_GROUP) -t $(DESTDIR)/$(VARDIR)/postinst/lib \
		$(FOLDER_POSTINST)/lib/libpostinst.sh	\
		$(BUILD_FOLDER)/libntfs-3g.so		\
		$(BUILD_FOLDER)/libntfs-3g.so.76	\
		$(BUILD_FOLDER)/libntfs-3g.so.76.0.0	\
		$(BUILD_FOLDER)/libparted.so		\
		$(BUILD_FOLDER)/libparted.so.0		\
		$(BUILD_FOLDER)/libparted.so.0.0.1	\
		$(BUILD_FOLDER)/libntfs.so		\
		$(BUILD_FOLDER)/libntfs.so.10		\
		$(BUILD_FOLDER)/libntfs.so.10.0.0
	$(INSTALL) -m 770 -o $(PULSE2_OWNER) -g $(PULSE2_GROUP) -d $(DESTDIR)/$(VARDIR)/postinst/lib/modules
	$(INSTALL) -m 440 -o $(PULSE2_OWNER) -g $(PULSE2_GROUP) -t $(DESTDIR)/$(VARDIR)/postinst/lib/modules \
		$(BUILD_FOLDER)/exportfs.ko	\
		$(BUILD_FOLDER)/ext2.ko		\
		$(BUILD_FOLDER)/ext3.ko		\
		$(BUILD_FOLDER)/ext4.ko		\
		$(BUILD_FOLDER)/fuse.ko		\
		$(BUILD_FOLDER)/jbd.ko		\
		$(BUILD_FOLDER)/jbd2.ko		\
		$(BUILD_FOLDER)/mbcache.ko	\
		$(BUILD_FOLDER)/reiserfs.ko	\
		$(BUILD_FOLDER)/xfs.ko
	# Install win32 utilities
	$(INSTALL) -m 770 -o $(PULSE2_OWNER) -g $(PULSE2_GROUP) -d $(DESTDIR)/$(VARDIR)/postinst/winutils
	$(INSTALL) -m 440 -o $(PULSE2_OWNER) -g $(PULSE2_GROUP) -t $(DESTDIR)/$(VARDIR)/postinst/winutils \
		$(FOLDER_POSTINST)/winutils/newsid.exe

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

postinst: kernel
	$(MAKE) -C $(FOLDER_POSTINST) SVNREV=$(SVNREV)
	cp -a $(FOLDER_POSTINST)/$(BUILD_FOLDER)/chntpw.static $(BUILD_FOLDER)/chntpw-$(SVNREV)
	ln -sf chntpw-$(SVNREV) $(BUILD_FOLDER)/chntpw
	cp -a $(FOLDER_POSTINST)/$(BUILD_FOLDER)/fusermount $(BUILD_FOLDER)/fusermount-$(SVNREV)
	ln -sf fusermount-$(SVNREV) $(BUILD_FOLDER)/fusermount
	cp -a $(FOLDER_POSTINST)/$(BUILD_FOLDER)/ntfs-3g $(BUILD_FOLDER)/ntfs-3g-$(SVNREV)
	ln -sf ntfs-3g-$(SVNREV) $(BUILD_FOLDER)/ntfs-3g
	cp -a $(FOLDER_POSTINST)/$(BUILD_FOLDER)/parted $(BUILD_FOLDER)/parted-$(SVNREV)
	ln -sf parted-$(SVNREV) $(BUILD_FOLDER)/parted
	cp -a $(FOLDER_POSTINST)/$(BUILD_FOLDER)/ntfsresize $(BUILD_FOLDER)/ntfsresize-$(SVNREV)
	ln -sf ntfsresize-$(SVNREV) $(BUILD_FOLDER)/ntfsresize
	cp -a $(FOLDER_POSTINST)/$(BUILD_FOLDER)/dd_rescue.bin $(BUILD_FOLDER)/dd_rescue-$(SVNREV)
	ln -sf dd_rescue-$(SVNREV) $(BUILD_FOLDER)/dd_rescue
	cp -a $(FOLDER_POSTINST)/$(BUILD_FOLDER)/libntfs-3g.so $(BUILD_FOLDER)/libntfs-3g.so-$(SVNREV)
	ln -sf libntfs-3g.so-$(SVNREV) $(BUILD_FOLDER)/libntfs-3g.so
	cp -a $(FOLDER_POSTINST)/$(BUILD_FOLDER)/libntfs-3g.so.76 $(BUILD_FOLDER)/libntfs-3g.so.76-$(SVNREV)
	ln -sf libntfs-3g.so.76-$(SVNREV) $(BUILD_FOLDER)/libntfs-3g.so.76
	cp -a $(FOLDER_POSTINST)/$(BUILD_FOLDER)/libntfs-3g.so.76.0.0 $(BUILD_FOLDER)/libntfs-3g.so.76.0.0-$(SVNREV)
	ln -sf libntfs-3g.so.76.0.0-$(SVNREV) $(BUILD_FOLDER)/libntfs-3g.so.76.0.0
	cp -a $(FOLDER_POSTINST)/$(BUILD_FOLDER)/libparted.so $(BUILD_FOLDER)/libparted.so-$(SVNREV)
	ln -sf libparted.so-$(SVNREV) $(BUILD_FOLDER)/libparted.so
	cp -a $(FOLDER_POSTINST)/$(BUILD_FOLDER)/libparted.so.0 $(BUILD_FOLDER)/libparted.so.0-$(SVNREV)
	ln -sf libparted.so.0-$(SVNREV) $(BUILD_FOLDER)/libparted.so.0
	cp -a $(FOLDER_POSTINST)/$(BUILD_FOLDER)/libparted.so.0.0.1 $(BUILD_FOLDER)/libparted.so.0.0.1-$(SVNREV)
	ln -sf libparted.so.0.0.1-$(SVNREV) $(BUILD_FOLDER)/libparted.so.0.0.1
	cp -a $(FOLDER_POSTINST)/$(BUILD_FOLDER)/libntfs.so $(BUILD_FOLDER)/libntfs.so-$(SVNREV)
	ln -sf libntfs.so-$(SVNREV) $(BUILD_FOLDER)/libntfs.so
	cp -a $(FOLDER_POSTINST)/$(BUILD_FOLDER)/libntfs.so.10 $(BUILD_FOLDER)/libntfs.so.10-$(SVNREV)
	ln -sf libntfs.so.10-$(SVNREV) $(BUILD_FOLDER)/libntfs.so.10
	cp -a $(FOLDER_POSTINST)/$(BUILD_FOLDER)/libntfs.so.10.0.0 $(BUILD_FOLDER)/libntfs.so.10.0.0-$(SVNREV)
	ln -sf libntfs.so.10.0.0-$(SVNREV) $(BUILD_FOLDER)/libntfs.so.10.0.0
	cp -a $(FOLDER_KERNEL)/$(BUILD_FOLDER)/$(FOLDER_LINUXKERNEL)/fs/mbcache.ko $(BUILD_FOLDER)/mbcache.ko-$(SVNREV)
	ln -sf mbcache.ko-$(SVNREV) $(BUILD_FOLDER)/mbcache.ko
	cp -a $(FOLDER_KERNEL)/$(BUILD_FOLDER)/$(FOLDER_LINUXKERNEL)/fs/fuse/fuse.ko $(BUILD_FOLDER)/fuse.ko-$(SVNREV)
	ln -sf fuse.ko-$(SVNREV) $(BUILD_FOLDER)/fuse.ko
	cp -a $(FOLDER_KERNEL)/$(BUILD_FOLDER)/$(FOLDER_LINUXKERNEL)/fs/ext2/ext2.ko $(BUILD_FOLDER)/ext2.ko-$(SVNREV)
	ln -sf ext2.ko-$(SVNREV) $(BUILD_FOLDER)/ext2.ko
	cp -a $(FOLDER_KERNEL)/$(BUILD_FOLDER)/$(FOLDER_LINUXKERNEL)/fs/ext3/ext3.ko $(BUILD_FOLDER)/ext3.ko-$(SVNREV)
	ln -sf ext3.ko-$(SVNREV) $(BUILD_FOLDER)/ext3.ko
	cp -a $(FOLDER_KERNEL)/$(BUILD_FOLDER)/$(FOLDER_LINUXKERNEL)/fs/ext4/ext4.ko $(BUILD_FOLDER)/ext4.ko-$(SVNREV)
	ln -sf ext4.ko-$(SVNREV) $(BUILD_FOLDER)/ext4.ko
	cp -a $(FOLDER_KERNEL)/$(BUILD_FOLDER)/$(FOLDER_LINUXKERNEL)/fs/xfs/xfs.ko $(BUILD_FOLDER)/xfs.ko-$(SVNREV)
	ln -sf xfs.ko-$(SVNREV) $(BUILD_FOLDER)/xfs.ko
	cp -a $(FOLDER_KERNEL)/$(BUILD_FOLDER)/$(FOLDER_LINUXKERNEL)/fs/reiserfs/reiserfs.ko $(BUILD_FOLDER)/reiserfs.ko-$(SVNREV)
	ln -sf reiserfs.ko-$(SVNREV) $(BUILD_FOLDER)/reiserfs.ko
	cp -a $(FOLDER_KERNEL)/$(BUILD_FOLDER)/$(FOLDER_LINUXKERNEL)/fs/jbd/jbd.ko $(BUILD_FOLDER)/jbd.ko-$(SVNREV)
	ln -sf jbd.ko-$(SVNREV) $(BUILD_FOLDER)/jbd.ko
	cp -a $(FOLDER_KERNEL)/$(BUILD_FOLDER)/$(FOLDER_LINUXKERNEL)/fs/jbd2/jbd2.ko $(BUILD_FOLDER)/jbd2.ko-$(SVNREV)
	ln -sf jbd2.ko-$(SVNREV) $(BUILD_FOLDER)/jbd2.ko
	cp -a $(FOLDER_KERNEL)/$(BUILD_FOLDER)/$(FOLDER_LINUXKERNEL)/fs/exportfs/exportfs.ko $(BUILD_FOLDER)/exportfs.ko-$(SVNREV)
	ln -sf exportfs.ko-$(SVNREV) $(BUILD_FOLDER)/exportfs.ko

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

export PROJECT_NAME = pulse2-imaging-server
export VERSION = 1.3.0

export TARBALL = $(PROJECT_NAME)-$(VERSION)
export RELEASES_DIR = releases
export TARBALL_GZ = $(TARBALL).tar.gz
export EXCLUDE_FILES = --exclude .svn
export CPA = cp -af

tarball: $(RELEASES_DIR)/$(TARBALL_GZ):
$(RELEASES_DIR)/$(TARBALL_GZ):
	mkdir -p $(RELEASES_DIR)/$(TARBALL)
	$(CPA) bootloader build BUILD contrib eltorito initrd INSTALL kernel Makefile postinstall prebuild-binaries tests tools $(RELEASES_DIR)/$(TARBALL)
	cd $(RELEASES_DIR) && tar -czf $(TARBALL_GZ) $(EXCLUDE_FILES) $(TARBALL); rm -rf $(TARBALL);


.PHONY: kernel bootloader tools initrd eltorito prebuild
