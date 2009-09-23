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

SVNREV:=$(shell echo $Rev: 4491 $ | tr -cd [[:digit:]])

include consts.mk

all: kernel modules

kernel:
	[ -e $(TARGET_LINUXKERNEL)/$(TARBALL_LINUXKERNEL) ] || wget $(URI_LINUXKERNEL)/$(TARBALL_LINUXKERNEL) -P $(TARGET_LINUXKERNEL)
	[ -d $(FOLDER_LINUXKERNEL) ] || tar jxf $(TARGET_LINUXKERNEL)/$(TARBALL_LINUXKERNEL) -C $(TARGET_LINUXKERNEL)
	[ -f $(FOLDER_LINUXKERNEL)/.config ] || cp -a $(CONFIGS_LINUXKERNEL)/dot-config-$(VERSION_LINUXKERNEL) $(FOLDER_LINUXKERNEL)/.config
	$(MAKE) -C $(FOLDER_LINUXKERNEL) ARCH=$(ARCHITECTURE) silentoldconfig
	$(MAKE) -C $(FOLDER_LINUXKERNEL) ARCH=$(ARCHITECTURE) all
	@cp -f $(FOLDER_LINUXKERNEL)/arch/i386/boot/bzImage build/bzImage-$(VERSION_LINUXKERNEL)-r$(SVNREV)
	@/usr/sbin/rdev build/bzImage-$(VERSION_LINUXKERNEL)-r$(SVNREV) /dev/ram0
	@ln -sf build/bzImage-$(VERSION_LINUXKERNEL)-r$(SVNREV) build/bzImage

modules: module_e1000 module_e1000e
	# others disebaled for now : module_igb module_r8168 module_r8169 module_atl1 module_atl1e module_atl2 module_570x module_nxii module_sk98lin

module_e1000:
	# If build fails comment 'get_perm_addr' ifdef in e1000_ethtool.c
	[ -e $(TARGET_E1000)/$(TARBALL_E1000) ] || wget $(URI_E1000)/$(TARBALL_E1000) -P $(TARGET_E1000)
	[ -d $(FOLDER_E1000) ] || tar zxf $(TARGET_E1000)/$(TARBALL_E1000) -C $(TARGET_E1000)
	$(MAKE) -C $(FOLDER_LINUXKERNEL) CC=$(CC) ARCH=$(ARCHITECTURE) SUBDIRS=../../../$(FOLDER_E1000) KSRC=../../../$(FOLDER_LINUXKERNEL)
	mkdir -p build/modules && cp -f $(FOLDER_E1000)/e1000.ko build/modules

module_e1000e:
	[ -e $(TARGET_E1000e)/$(TARBALL_E1000e) ] || wget $(URI_E1000e)/$(TARBALL_E1000e) -P $(TARGET_E1000e)
	[ -d $(FOLDER_E1000e) ] || tar zxf $(TARGET_E1000e)/$(TARBALL_E1000e) -C $(TARGET_E1000e)
	patch -p1 -d $(FOLDER_E1000e) < $(TARGET_E1000e)/patch_for_2.6.23_kernel.diff
	$(MAKE) -C $(FOLDER_LINUXKERNEL) CC=$(CC) ARCH=$(ARCHITECTURE) SUBDIRS=../../../$(FOLDER_E1000e) KSRC=../../../$(FOLDER_LINUXKERNEL)
	mkdir -p build/modules && cp -f $(FOLDER_E1000e)/e1000e.ko build/modules

module_igb:
	[ -d $(SRC_IGB) ] || (cd $(FOLDER_IGB) && tar zxf $(TARBALL_IGB))
	patch -p1 -d $(SRC_IGB) < $(SRC_IGB)/../../patch_for_2.6.23_kernel.diff
	make CC=$(CC) -C $(KSRC) SUBDIRS=$(SRC_IGB)
	mkdir -p build/modules && cp -f $(SRC_IGB)/igb.ko build/modules

module_r8168:
	[ -d $(SRC_R8168) ] || (cd $(FOLDER_R8168) && tar jxf $(TARBALL_R8168))
	make CC=$(CC) -C $(KSRC) SUBDIRS=$(SRC_R8168) modules
	mkdir -p build/modules && cp -f $(SRC_R8168)/r8168.ko build/modules

module_r8169:
	[ -d $(SRC_R8169) ] || (cd $(FOLDER_R8169) && tar jxf $(TARBALL_R8169))
	make CC=$(CC) -C $(KSRC) SUBDIRS=$(SRC_R8169) modules
	mkdir -p build/modules && cp -f $(SRC_R8169)/r8169.ko build/modules

module_atl1:
	[ -d $(SRC_ATL1) ] || (cd $(FOLDER_ATL1) && tar zxf $(TARBALL_ATL1))
	make CC=$(CC) -C $(KSRC) SUBDIRS=$(SRC_ATL1) modules
	mkdir -p build/modules && cp -f $(SRC_ATL1)/atl1.ko build/modules

module_atl1e:
	[ -d $(SRC_ATL1e) ] || (cd $(FOLDER_ATL1e) && tar zxf $(TARBALL_ATL1e))
	make CC=$(CC) -C $(KSRC) SUBDIRS=$(SRC_ATL1e) modules
	mkdir -p build/modules && cp -f $(SRC_ATL1e)/atl1e.ko build/modules

module_atl2:
	[ -d $(SRC_ATL2) ] || (cd $(FOLDER_ATL2) && tar jxf $(TARBALL_ATL2))
	make CC=$(CC) -C $(KSRC) SUBDIRS=$(SRC_ATL2) modules
	mkdir -p build/modules && cp -f $(SRC_ATL2)/atl2.ko build/modules

module_570x:
	[ -d $(SRC_570X) ] || (cd $(FOLDER_570X) && unzip $(PKG_570X) && tar zxf $(TARBALL_570X) && rm -fr $(FOLDER_570X)/Server)
	sed -i 's!^#define TG3_VLAN_TAG_USED 0$$!#define TG3_VLAN_TAG_USED 1!' $(SRC_570X)/tg3.c
	make CC=$(CC) -C $(SRC_570X) LINUXSRC=$(KSRC) tg3_flags.h
	make CC=$(CC) -C $(KSRC) SUBDIRS=$(SRC_570X) modules
	mkdir -p build/modules && cp -f $(SRC_570X)/tg3.ko build/modules

module_nxii:
	[ -d $(SRC_NXII) ] || (cd $(FOLDER_NXII) && unzip $(PKG_NXII) && tar zxf $(TARBALL_NXII) && rm -fr $(FOLDER_NXII)/Server)
	make CC=$(CC) LINUXSRC=$(KSRC) -C $(KSRC) SUBDIRS=$(SRC_NXII) modules
	mkdir -p build/modules && cp -f $(SRC_NXII)/bnx2.ko build/modules

module_sk98lin:
	[ -d $(SRC_SK98LIN) ] || (cd $(FOLDER_SK98LIN) && tar xvjf $(PKG_SK98LIN) && tar xvjf $(TARBALL_SK98LIN) && rm -fr $(FOLDER_SK98LIN)/DriverInstall)
	mkdir -p $(SRC_SK98LIN) && cp -a $(SRC_SK98LIN)/../common/* $(SRC_SK98LIN) && cp -a $(SRC_SK98LIN)/../2.6/* $(SRC_SK98LIN)
	sed -i "s|\-Idrivers/net/sk98lin|-I$(SRC_SK98LIN)|" $(SRC_SK98LIN)/Makefile
	CONFIG_SK98LIN=m make CC=$(CC) LINUXSRC=$(KSRC) -C $(KSRC) SUBDIRS=$(SRC_SK98LIN) modules
	mkdir -p build/modules && cp -f $(SRC_SK98LIN)/sk98lin.ko build/modules

clean: modules_clean
	$(MAKE) -C $(FOLDER_LINUXKERNEL) clean
	$(MAKE) -C $(FOLDER_LINUXKERNEL) clean ARCH=$(ARCHITECTURE) SUBDIRS=../../../$(FOLDER_E1000) KSRC=../../../$(FOLDER_LINUXKERNEL)
	$(MAKE) -C $(FOLDER_LINUXKERNEL) clean ARCH=$(ARCHITECTURE) SUBDIRS=../../../$(FOLDER_E1000e) KSRC=../../../$(FOLDER_LINUXKERNEL)
	rm -fr build/bzImage build/modules/*

dist-clean:

modules_clean:
	rm -fr $(SRC_E1000) $(SRC_SK98LIN) $(SRC_NXII) $(SRC_570X) $(SRC_ATL2) $(SRC_ATL1e) $(SRC_ATL1) $(SRC_R8169) $(SRC_R8168) $(SRC_IGB) $(SRC_E1000e)
