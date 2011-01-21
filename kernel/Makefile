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

SVNREV:=$(shell echo $Rev$ | tr -cd [[:digit:]])

BUILD_FOLDER =  $(shell pwd)/build
3RDPARTY_FOLDER =  $(shell pwd)/3rd_party

include consts.mk

all: kernel modules

kernel: rdev
	# preparation
	[ -e $(3RDPARTY_FOLDER)/$(TARBALL_LINUXKERNEL) ] || wget $(URI_LINUXKERNEL)/$(TARBALL_LINUXKERNEL) -P $(3RDPARTY_FOLDER)
	[ -d $(BUILD_FOLDER)/$(FOLDER_LINUXKERNEL) ] || tar jxf $(3RDPARTY_FOLDER)/$(TARBALL_LINUXKERNEL) -C $(BUILD_FOLDER)
	-zcat $(3RDPARTY_FOLDER)/001-megaraid_sas_v00.00.04.31-for_2.6.32.24.diff.gz | patch -N -p1 -d $(BUILD_FOLDER)/$(FOLDER_LINUXKERNEL)/drivers/scsi/megaraid
	[ -f $(BUILD_FOLDER)/$(FOLDER_LINUXKERNEL)/.config ] || cp -a $(CONFIGS_LINUXKERNEL)/dot-config-$(VERSION_LINUXKERNEL) $(BUILD_FOLDER)/$(FOLDER_LINUXKERNEL)/.config && $(MAKE) -C $(BUILD_FOLDER)/$(FOLDER_LINUXKERNEL) ARCH=$(ARCHITECTURE) silentoldconfig

	# kernel build
	$(MAKE) -C $(BUILD_FOLDER)/$(FOLDER_LINUXKERNEL) ARCH=$(ARCHITECTURE) all
	cp -aL $(BUILD_FOLDER)/$(FOLDER_LINUXKERNEL)/arch/i386/boot/bzImage $(BUILD_FOLDER)/bzImage-$(VERSION_LINUXKERNEL)-$(SVNREV)
	./rdev $(BUILD_FOLDER)/bzImage-$(VERSION_LINUXKERNEL)-$(SVNREV) /dev/ram0

	# gather mandatory modules
	rm -f $(BUILD_FOLDER)/modules/*.ko
	find $(BUILD_FOLDER)/$(FOLDER_LINUXKERNEL)/drivers/net -iname \*.ko -exec cp -a {} $(BUILD_FOLDER)/modules \;
	find $(BUILD_FOLDER)/$(FOLDER_LINUXKERNEL)/drivers/pcmcia -iname \*.ko -exec cp -a {} $(BUILD_FOLDER)/modules \;
	find $(BUILD_FOLDER)/$(FOLDER_LINUXKERNEL)/drivers/ssb -iname \*.ko -exec cp -a {} $(BUILD_FOLDER)/modules \;
	find $(BUILD_FOLDER)/$(FOLDER_LINUXKERNEL)/drivers/i2c -iname \*.ko -exec cp -a {} $(BUILD_FOLDER)/modules \;
	find $(BUILD_FOLDER)/$(FOLDER_LINUXKERNEL)/drivers/uio -iname \*.ko -exec cp -a {} $(BUILD_FOLDER)/modules \;
	find $(BUILD_FOLDER)/$(FOLDER_LINUXKERNEL)/crypto -iname \*.ko -exec cp -a {} $(BUILD_FOLDER)/modules \;
	find $(BUILD_FOLDER)/$(FOLDER_LINUXKERNEL)/lib -iname \*.ko -exec cp -a {} $(BUILD_FOLDER)/modules \;

	# CD modules
	cp -a $(BUILD_FOLDER)/$(FOLDER_LINUXKERNEL)/drivers/ide/ide-cd_mod.ko		$(BUILD_FOLDER)/modules/cd/ide-cd.ko
	cp -a $(BUILD_FOLDER)/$(FOLDER_LINUXKERNEL)/drivers/cdrom/cdrom.ko		$(BUILD_FOLDER)/modules/cd
	cp -a $(BUILD_FOLDER)/$(FOLDER_LINUXKERNEL)/fs/isofs/isofs.ko			$(BUILD_FOLDER)/modules/cd
	cp -a $(BUILD_FOLDER)/$(FOLDER_LINUXKERNEL)/drivers/scsi/sr_mod.ko		$(BUILD_FOLDER)/modules/cd
	cp -a $(BUILD_FOLDER)/$(FOLDER_LINUXKERNEL)/drivers/usb/storage/usb-storage.ko 	$(BUILD_FOLDER)/modules/cd

	# Filesystems
	cp -a $(BUILD_FOLDER)/$(FOLDER_LINUXKERNEL)/fs/mbcache.ko			$(BUILD_FOLDER)/modules/fs
	cp -a $(BUILD_FOLDER)/$(FOLDER_LINUXKERNEL)/fs/fuse/fuse.ko			$(BUILD_FOLDER)/modules/fs
	cp -a $(BUILD_FOLDER)/$(FOLDER_LINUXKERNEL)/fs/ext2/ext2.ko			$(BUILD_FOLDER)/modules/fs
	cp -a $(BUILD_FOLDER)/$(FOLDER_LINUXKERNEL)/fs/ext3/ext3.ko			$(BUILD_FOLDER)/modules/fs
	cp -a $(BUILD_FOLDER)/$(FOLDER_LINUXKERNEL)/fs/ext4/ext4.ko			$(BUILD_FOLDER)/modules/fs
	cp -a $(BUILD_FOLDER)/$(FOLDER_LINUXKERNEL)/fs/xfs/xfs.ko			$(BUILD_FOLDER)/modules/fs
	cp -a $(BUILD_FOLDER)/$(FOLDER_LINUXKERNEL)/fs/reiserfs/reiserfs.ko		$(BUILD_FOLDER)/modules/fs
	cp -a $(BUILD_FOLDER)/$(FOLDER_LINUXKERNEL)/fs/jbd/jbd.ko			$(BUILD_FOLDER)/modules/fs
	cp -a $(BUILD_FOLDER)/$(FOLDER_LINUXKERNEL)/fs/jbd2/jbd2.ko			$(BUILD_FOLDER)/modules/fs
	cp -a $(BUILD_FOLDER)/$(FOLDER_LINUXKERNEL)/fs/exportfs/exportfs.ko		$(BUILD_FOLDER)/modules/fs

modules: module_r8101 module_r8168 module_r8169 module_atl1e module_e1000 module_e1000e module_igb module_nxii module_570x

module_e1000:
	# If build fails comment 'get_perm_addr' ifdef in e1000_ethtool.c
	[ -e $(3RDPARTY_FOLDER)/$(TARBALL_E1000) ] || wget $(URI_E1000)/$(TARBALL_E1000) -P $(3RDPARTY_FOLDER)
	[ -d $(BUILD_FOLDER)/$(FOLDER_E1000) ] || tar zxf $(3RDPARTY_FOLDER)/$(TARBALL_E1000) -C $(BUILD_FOLDER)
	$(MAKE) -C $(BUILD_FOLDER)/$(FOLDER_LINUXKERNEL) CC=$(CC) ARCH=$(ARCHITECTURE) SUBDIRS=../$(FOLDER_E1000) KSRC=../$(FOLDER_LINUXKERNEL)
	cp -f $(BUILD_FOLDER)/$(FOLDER_E1000)/e1000.ko $(BUILD_FOLDER)/modules

module_e1000e:
	[ -e $(3RDPARTY_FOLDER)/$(TARBALL_E1000e) ] || wget $(URI_E1000e)/$(TARBALL_E1000e) -P $(3RDPARTY_FOLDER)
	[ -d $(BUILD_FOLDER)/$(FOLDER_E1000e) ] || tar zxf $(3RDPARTY_FOLDER)/$(TARBALL_E1000e) -C $(BUILD_FOLDER)
	$(MAKE) -C $(BUILD_FOLDER)/$(FOLDER_LINUXKERNEL) CC=$(CC) ARCH=$(ARCHITECTURE) SUBDIRS=../$(FOLDER_E1000e) KSRC=../$(FOLDER_LINUXKERNEL)
	cp -f $(BUILD_FOLDER)/$(FOLDER_E1000e)/e1000e.ko $(BUILD_FOLDER)/modules

module_igb:
	[ -e $(3RDPARTY_FOLDER)/$(TARBALL_IGB) ] || wget $(URI_IGB)/$(TARBALL_IGB) -P $(3RDPARTY_FOLDER)
	[ -d $(BUILD_FOLDER)/$(FOLDER_IGB) ] || tar zxf $(3RDPARTY_FOLDER)/$(TARBALL_IGB) -C $(BUILD_FOLDER)
	$(MAKE) -C $(BUILD_FOLDER)/$(FOLDER_LINUXKERNEL) CC=$(CC) ARCH=$(ARCHITECTURE) SUBDIRS=../$(FOLDER_IGB) KSRC=../$(FOLDER_LINUXKERNEL)
	cp -f $(BUILD_FOLDER)/$(FOLDER_IGB)/igb.ko $(BUILD_FOLDER)/modules

module_r8168:
	[ -e $(3RDPARTY_FOLDER)/$(TARBALL_R8168) ] || wget $(URI_R8168)/$(TARBALL_R8168) -P $(3RDPARTY_FOLDER)
	[ -d $(BUILD_FOLDER)/$(FOLDER_R8168) ] || tar jxf $(3RDPARTY_FOLDER)/$(TARBALL_R8168) -C $(BUILD_FOLDER)
	-zcat $(3RDPARTY_FOLDER)/002-r8168_8.020.00-for_2.6.32.27.diff.gz | patch -N -p2 -d $(BUILD_FOLDER)/$(FOLDER_R8168)
	$(MAKE) -C $(BUILD_FOLDER)/$(FOLDER_LINUXKERNEL) CC=$(CC) ARCH=$(ARCHITECTURE) SUBDIRS=../$(FOLDER_R8168) KSRC=../$(FOLDER_LINUXKERNEL)
	cp -f $(BUILD_FOLDER)/$(FOLDER_R8168)/r8168.ko $(BUILD_FOLDER)/modules

module_r8169:
	[ -e $(3RDPARTY_FOLDER)/$(TARBALL_R8169) ] || wget $(URI_R8169)/$(TARBALL_R8169) -P $(3RDPARTY_FOLDER)
	[ -d $(BUILD_FOLDER)/$(FOLDER_R8169) ] || tar jxf $(3RDPARTY_FOLDER)/$(TARBALL_R8169) -C $(BUILD_FOLDER)
	$(MAKE) -C $(BUILD_FOLDER)/$(FOLDER_LINUXKERNEL) CC=$(CC) ARCH=$(ARCHITECTURE) SUBDIRS=../$(FOLDER_R8169) KSRC=../$(FOLDER_LINUXKERNEL)
	cp -f $(BUILD_FOLDER)/$(FOLDER_R8169)/r8169.ko $(BUILD_FOLDER)/modules

module_r8101:
	[ -e $(3RDPARTY_FOLDER)/$(TARBALL_R8101) ] || wget $(URI_R8101)/$(TARBALL_R8101) -P $(3RDPARTY_FOLDER)
	[ -d $(BUILD_FOLDER)/$(FOLDER_R8101) ] || tar jxf $(3RDPARTY_FOLDER)/$(TARBALL_R8101) -C $(BUILD_FOLDER)
	-zcat $(3RDPARTY_FOLDER)/003-r8101_1.019.00-for_2.6.32.27.diff.gz | patch -N -p2 -d $(BUILD_FOLDER)/$(FOLDER_R8101)
	$(MAKE) -C $(BUILD_FOLDER)/$(FOLDER_LINUXKERNEL) CC=$(CC) ARCH=$(ARCHITECTURE) SUBDIRS=../$(FOLDER_R8101) KSRC=../$(FOLDER_LINUXKERNEL)
	cp -f $(BUILD_FOLDER)/$(FOLDER_R8101)/r8101.ko $(BUILD_FOLDER)/modules

module_atl1e:
	[ -e $(3RDPARTY_FOLDER)/$(TARBALL_ATL1E) ] || wget $(URI_ATL1E)/$(TARBALL_ATL1E) -P $(3RDPARTY_FOLDER)
	[ -d $(BUILD_FOLDER)/$(FOLDER_ATL1E) ] || mkdir -p $(BUILD_FOLDER)/$(FOLDER_ATL1E) && tar zxf $(3RDPARTY_FOLDER)/$(TARBALL_ATL1E) -C $(BUILD_FOLDER)/$(FOLDER_ATL1E)/../ || true
	$(MAKE) -C $(BUILD_FOLDER)/$(FOLDER_LINUXKERNEL) CC=$(CC) ARCH=$(ARCHITECTURE) SUBDIRS=../$(FOLDER_ATL1E) KSRC=../$(FOLDER_LINUXKERNEL)
	cp -f $(BUILD_FOLDER)/$(FOLDER_ATL1E)/atl1e.ko $(BUILD_FOLDER)/modules

module_570x:
	[ -e $(3RDPARTY_FOLDER)/$(ARCHIVE_570X) ] || [ -e $(3RDPARTY_FOLDER)/$(TARBALL_570X) ] || wget $(URI_570X)/$(ARCHIVE_570X) -P $(3RDPARTY_FOLDER)
	[ -e $(3RDPARTY_FOLDER)/$(TARBALL_570X) ] || unzip -j $(3RDPARTY_FOLDER)/$(ARCHIVE_570X) \*$(TARBALL_570X) -d $(3RDPARTY_FOLDER)
	[ -d $(BUILD_FOLDER)/$(FOLDER_570X) ] || tar zxf $(3RDPARTY_FOLDER)/$(TARBALL_570X) -C $(BUILD_FOLDER)
	sed -i 's!^#define TG3_VLAN_TAG_USED 0$$!#define TG3_VLAN_TAG_USED 1!' $(BUILD_FOLDER)/$(FOLDER_570X)/tg3.c
	sed -i 's!^BCMCFGDIR=$!!BCMCFGDIR=../$(FOLDER_LINUXKERNEL)!' $(BUILD_FOLDER)/$(FOLDER_570X)/Makefile
	sed -i 's!^BCMSRCDIR=$!!BCMSRCDIR=../$(FOLDER_LINUXKERNEL)!' $(BUILD_FOLDER)/$(FOLDER_570X)/Makefile
	$(MAKE) -C $(BUILD_FOLDER)/$(FOLDER_570X) CC=$(CC) ARCH=$(ARCHITECTURE) LINUXSRC=../$(FOLDER_LINUXKERNEL) tg3_flags.h
	$(MAKE) -C $(BUILD_FOLDER)/$(FOLDER_LINUXKERNEL) CC=$(CC) ARCH=$(ARCHITECTURE) SUBDIRS=../$(FOLDER_570X) KSRC=../$(FOLDER_LINUXKERNEL)
	cp -f $(BUILD_FOLDER)/$(FOLDER_570X)/tg3.ko $(BUILD_FOLDER)/modules

module_nxii:
	[ -e $(3RDPARTY_FOLDER)/$(ARCHIVE_NXII) ] || [ -e $(3RDPARTY_FOLDER)/$(TARBALL_NXII) ] || wget $(URI_NXII)/$(ARCHIVE_NXII) -P $(3RDPARTY_FOLDER)
	[ -e $(3RDPARTY_FOLDER)/$(TARBALL_NXII) ] || unzip -j $(3RDPARTY_FOLDER)/$(ARCHIVE_NXII) \*$(TARBALL_NXII) -d $(3RDPARTY_FOLDER)
	[ -d $(BUILD_FOLDER)/$(FOLDER_NXII) ] || tar zxf $(3RDPARTY_FOLDER)/$(TARBALL_NXII) -C $(BUILD_FOLDER)
	$(MAKE) -C $(BUILD_FOLDER)/$(FOLDER_LINUXKERNEL) CC=$(CC) ARCH=$(ARCHITECTURE) SUBDIRS=../$(FOLDER_NXII) LINUXSRC=../$(FOLDER_LINUXKERNEL)
	cp -f $(BUILD_FOLDER)/$(FOLDER_NXII)/bnx2.ko build/modules

clean: modules_clean rdev_clean
	$(MAKE) -C $(BUILD_FOLDER)/$(FOLDER_LINUXKERNEL) clean

dist-clean:
	rm -f $(BUILD_FOLDER)/bzImage-*
	rm -fr $(BUILD_FOLDER)/atl1-*
	rm -fr $(BUILD_FOLDER)/DriverInstall
	rm -fr $(BUILD_FOLDER)/e1000-*
	rm -fr $(BUILD_FOLDER)/e1000e-*
	rm -fr $(BUILD_FOLDER)/igb-*
	rm -fr $(BUILD_FOLDER)/linux-*
	rm -fr $(BUILD_FOLDER)/netxtreme2-*
	rm -fr $(BUILD_FOLDER)/r8168-*
	rm -fr $(BUILD_FOLDER)/r8169-*
	rm -fr $(BUILD_FOLDER)/modules/*.ko

modules_clean:
	$(MAKE) -C $(BUILD_FOLDER)/$(FOLDER_LINUXKERNEL) clean ARCH=$(ARCHITECTURE) SUBDIRS=../$(FOLDER_E1000)	KSRC=../$(FOLDER_LINUXKERNEL)
	$(MAKE) -C $(BUILD_FOLDER)/$(FOLDER_LINUXKERNEL) clean ARCH=$(ARCHITECTURE) SUBDIRS=../$(FOLDER_E1000e) KSRC=../$(FOLDER_LINUXKERNEL)
	$(MAKE) -C $(BUILD_FOLDER)/$(FOLDER_LINUXKERNEL) clean ARCH=$(ARCHITECTURE) SUBDIRS=../$(FOLDER_IGB)	KSRC=../$(FOLDER_LINUXKERNEL)
	$(MAKE) -C $(BUILD_FOLDER)/$(FOLDER_LINUXKERNEL) clean ARCH=$(ARCHITECTURE) SUBDIRS=../$(FOLDER_R8168)	KSRC=../$(FOLDER_LINUXKERNEL)
	$(MAKE) -C $(BUILD_FOLDER)/$(FOLDER_LINUXKERNEL) clean ARCH=$(ARCHITECTURE) SUBDIRS=../$(FOLDER_R8169)	KSRC=../$(FOLDER_LINUXKERNEL)
	$(MAKE) -C $(BUILD_FOLDER)/$(FOLDER_LINUXKERNEL) clean ARCH=$(ARCHITECTURE) SUBDIRS=../$(FOLDER_R8101)	KSRC=../$(FOLDER_LINUXKERNEL)
	$(MAKE) -C $(BUILD_FOLDER)/$(FOLDER_LINUXKERNEL) clean ARCH=$(ARCHITECTURE) SUBDIRS=../$(FOLDER_ATL1E)	KSRC=../$(FOLDER_LINUXKERNEL)
	$(MAKE) -C $(BUILD_FOLDER)/$(FOLDER_LINUXKERNEL) clean ARCH=$(ARCHITECTURE) SUBDIRS=../$(FOLDER_NXII)	KSRC=../$(FOLDER_LINUXKERNEL)
	$(MAKE) -C $(BUILD_FOLDER)/$(FOLDER_LINUXKERNEL) clean ARCH=$(ARCHITECTURE) SUBDIRS=../$(FOLDER_570X)	KSRC=../$(FOLDER_LINUXKERNEL)

rdev: rdev.c
	$(CC) -Wall -o rdev rdev.c

rdev_clean:
	rm -f rdev


