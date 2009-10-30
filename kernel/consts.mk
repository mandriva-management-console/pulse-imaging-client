ARCHITECTURE		= i386
CC			= gcc-3.4

VERSION_LINUXKERNEL		= 2.6.23.9
URI_LINUXKERNEL     	= http://www.kernel.org/pub/linux/kernel/v2.6
FOLDER_LINUXKERNEL  	= linux-$(VERSION_LINUXKERNEL)
TARBALL_LINUXKERNEL 	= linux-$(VERSION_LINUXKERNEL).tar.bz2
CONFIGS_LINUXKERNEL		= dot-configs

VERSION_E1000			= 8.0.16
URI_E1000				= http://downloadmirror.intel.com/9180/eng
FOLDER_E1000			= e1000-$(VERSION_E1000)/src
TARBALL_E1000			= e1000-$(VERSION_E1000).tar.gz

VERSION_E1000e			= 0.5.18.3
URI_E1000e				= http://downloadmirror.intel.com/17840/eng
FOLDER_E1000e			= e1000e-$(VERSION_E1000e)/src
TARBALL_E1000e			= e1000e-$(VERSION_E1000e).tar.gz

VERSION_IGB				= 2.0.6
URI_IGB					= http://downloadmirror.intel.com/13663/eng
FOLDER_IGB				= igb-$(VERSION_IGB)/src
TARBALL_IGB				= igb-$(VERSION_IGB).tar.gz

VERSION_R8168			= 8.014.00
URI_R8168				= ftp://WebUser:2mG8dBW@210.51.181.211/cn/nic/
FOLDER_R8168			= r8168-$(VERSION_R8168)/src
TARBALL_R8168			= r8168-$(VERSION_R8168).tar.bz2

VERSION_R8169			= 6.011.00
URI_R8169				= ftp://WebUser:2mG8dBW@210.51.181.211/cn/nic
FOLDER_R8169			= r8169-$(VERSION_R8169)/src
TARBALL_R8169			= r8169-$(VERSION_R8169).tar.bz2

VERSION_R8101			= 1.013.00
URI_R8101				= ftp://WebUser:pGL7E6v@152.104.238.19/cn/nic
FOLDER_R8101			= r8101-$(VERSION_R8101)/src
TARBALL_R8101			= r8101-$(VERSION_R8101).tar.bz2

VERSION_ATL1			= 2.1.3-linux-2.6.23
URI_ATL1				= ftp://hogchain.net/pub/linux/attansic/kernel_driver
FOLDER_ATL1				= atl1-$(VERSION_ATL1)
TARBALL_ATL1			= atl1-$(VERSION_ATL1)-standalone.tar.gz

VERSION_570X			= 3.99k
URI_570X				= http://www.broadcom.com/docs/driver_download/570x
FOLDER_570X				= tg3-$(VERSION_570X)
ARCHIVE_570X			= linux-$(VERSION_570X).zip
TARBALL_570X			= tg3-$(VERSION_570X).tar.gz

VERSION_NXII			= 1.9.20b_1.50.13
URI_NXII				= http://www.broadcom.com/docs/driver_download/NXII
FOLDER_NXII				= netxtreme2-5.0.17/bnx2/src
ARCHIVE_NXII			= linux-$(VERSION_NXII).zip
TARBALL_NXII			= netxtreme2-5.0.17.tar.gz

VERSION_SK98			= 10.81.3.3
URI_SK98				= http://www.marvell.com/drivers/files
FOLDER_SK98				= DriverInstall
ARCHIVE_SK98			= Linux_$(VERSION_SK98).zip
TARBALL_SK98			= install_v$(VERSION_SK98).tar.bz2


FOLDER_ATL1e := $(PWD)/3rd_party/atl1e
SRC_ATL1e := $(FOLDER_ATL1e)/459-AR813X-linux-v1.0.0.9/src
TARBALL_ATL1e := 459-AR813X-linux-v1.0.0.9.tar.gz

FOLDER_ATL2 := $(PWD)/3rd_party/atl2
SRC_ATL2 := $(FOLDER_ATL2)/atl2-2.0.5
TARBALL_ATL2 := atl2-2.0.5.tar.bz2


