ARCHITECTURE		= i386
CC					= gcc-3.4

VERSION_LINUXKERNEL	= 2.6.23.9
URI_LINUXKERNEL     = http://www.kernel.org/pub/linux/kernel/v2.6
TARGET_LINUXKERNEL  = 3rd_party/linux
FOLDER_LINUXKERNEL  = $(TARGET_LINUXKERNEL)/linux-$(VERSION_LINUXKERNEL)
TARBALL_LINUXKERNEL = linux-$(VERSION_LINUXKERNEL).tar.bz2
CONFIGS_LINUXKERNEL	= dot-configs

VERSION_E1000		= 8.0.16
URI_E1000			= http://downloadmirror.intel.com/9180/eng
TARGET_E1000		= 3rd_party/e1000
FOLDER_E1000		= $(TARGET_E1000)/e1000-$(VERSION_E1000)/src
TARBALL_E1000		= e1000-$(VERSION_E1000).tar.gz

VERSION_E1000e		= 0.5.18.3
URI_E1000e			= http://downloadmirror.intel.com/17840/eng
TARGET_E1000e		= 3rd_party/e1000e
FOLDER_E1000e		= $(TARGET_E1000e)/e1000e-$(VERSION_E1000e)/src
TARBALL_E1000e		= e1000e-$(VERSION_E1000e).tar.gz

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

