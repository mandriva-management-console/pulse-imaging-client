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
# The kernel source should be decompressed in /usr/src, with a 'linux' link
# pointing to the right directory, or be given using SRC on command line
#

SRC	:= /usr/src
HERE	:= `pwd`
CC	:= gcc-4.1
KSRC	:= $(SRC)/linux

FOLDER_E1000 := $(PWD)/3rd_party/e1000
SRC_E1000 := $(FOLDER_E1000)/e1000-8.0.9/src
TARBALL_E1000 := e1000-8.0.9.tar.gz

FOLDER_E1000e := $(PWD)/3rd_party/e1000e
SRC_E1000e := $(FOLDER_E1000e)/e1000e-0.5.18.3/src
TARBALL_E1000e := e1000e-0.5.18.3.tar.gz

FOLDER_IGB := $(PWD)/3rd_party/igb
SRC_IGB := $(FOLDER_IGB)/igb-1.3.19.3/src
TARBALL_IGB := igb-1.3.19.3.tar.gz

FOLDER_R8168 := $(PWD)/3rd_party/r8168
SRC_R8168 := $(FOLDER_R8168)/r8168-8.012.00/src
TARBALL_R8168 := r8168-8.012.00.tar.bz2

FOLDER_R8169 := $(PWD)/3rd_party/r8169
SRC_R8169 := $(FOLDER_R8169)/r8169-6.010.00/src
TARBALL_R8169 := r8169-6.010.00.tar.bz2

FOLDER_ATL1 := $(PWD)/3rd_party/atl1
SRC_ATL1 := $(FOLDER_ATL1)/atl1-2.1.3-linux-2.6.23
TARBALL_ATL1 := atl1-2.1.3-linux-2.6.23-standalone.tar.gz

FOLDER_ATL1e := $(PWD)/3rd_party/atl1e
SRC_ATL1e := $(FOLDER_ATL1e)/459-AR813X-linux-v1.0.0.9/src
TARBALL_ATL1e := 459-AR813X-linux-v1.0.0.9.tar.gz

FOLDER_ATL2 := $(PWD)/3rd_party/atl2
SRC_ATL2 := $(FOLDER_ATL2)/atl2-2.0.5
TARBALL_ATL2 := atl2-2.0.5.tar.bz2

FOLDER_570X := $(PWD)/3rd_party/570x
SRC_570X := $(FOLDER_570X)/tg3-3.92n
PKG_570X := linux-3.92n.zip
TARBALL_570X := tg3-3.92n.tar.gz

FOLDER_NXII := $(PWD)/3rd_party/nxii
SRC_NXII := $(FOLDER_NXII)/netxtreme2-4.8.10/bnx2/src
PKG_NXII := linux-1.8.5b_1.48.53.zip
TARBALL_NXII := Server/Linux/Driver/netxtreme2-4.8.10.tar.gz

FOLDER_SK98LIN := $(PWD)/3rd_party/sk98lin
SRC_SK98LIN := $(FOLDER_SK98LIN)/build
PKG_SK98LIN := install_v10.70.7.3.tar.bz2
TARBALL_SK98LIN := DriverInstall/sk98lin.tar.bz2

all: kernel modules

kernel:
	@echo -e "\n\nCurrent linux is:\n"
	@ls -l $(SRC)/linux
	@echo -e "\nContinue ?(y/n)"
	@(read ANS; [ "$$ANS" != "n" ])
# kernel compile
	[ -f $(SRC)/linux/.config ]
	# FB disabled, do not need to copy this file anymore
	# cp -f media/logo_linux_vga16.ppm $(SRC)/linux/drivers/video/logo/
	$(MAKE) -C $(SRC)/linux all CC=$(CC)
	mkdir -p build
	cp -f $(SRC)/linux/arch/i386/boot/bzImage build
	/usr/sbin/rdev build/bzImage /dev/ram0
	mkdir -p build/modules
	find $(SRC)/linux/drivers/net $(SRC)/linux/lib $(SRC)/linux/drivers/pcmcia -name "*.ko" -exec cp -a {} build/modules \;

modules: module_e1000 module_e1000e module_igb module_r8168 module_r8169 module_atl1 module_atl1e module_atl2 module_570x module_nxii module_sk98lin

module_e1000:
	# If build fails comment 'get_perm_addr' ifdef in e1000_ethtool.c
	[ -d $(SRC_E1000) ] || (cd $(FOLDER_E1000) && tar zxf $(TARBALL_E1000))
	make CC=$(CC) -C $(KSRC) SUBDIRS=$(SRC_E1000)
	mkdir -p build/modules && cp -f $(SRC_E1000)/e1000.ko build/modules

module_e1000e:
	[ -d $(SRC_E1000e) ] || (cd $(FOLDER_E1000e) && tar zxf $(TARBALL_E1000e))
	patch -p1 -d $(SRC_E1000e) < $(SRC_E1000e)/../../patch_for_2.6.23_kernel.diff
	make CC=$(CC) -C $(KSRC) SUBDIRS=$(SRC_E1000e)
	mkdir -p build/modules && cp -f $(SRC_E1000e)/e1000e.ko build/modules

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
	$(MAKE) -C $(KSRC) clean
	rm -fr build/bzImage build/modules/*

modules_clean:
	rm -fr $(SRC_E1000) $(SRC_SK98LIN) $(SRC_NXII) $(SRC_570X) $(SRC_ATL2) $(SRC_ATL1e) $(SRC_ATL1) $(SRC_R8169) $(SRC_R8168) $(SRC_IGB) $(SRC_E1000e)

