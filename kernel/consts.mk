ARCHITECTURE			= i386
CC				= gcc

VERSION_LINUXKERNEL		= 2.6.32.27
URI_LINUXKERNEL     		= http://www.kernel.org/pub/linux/kernel/v2.6
FOLDER_LINUXKERNEL  		= linux-$(VERSION_LINUXKERNEL)
TARBALL_LINUXKERNEL		= linux-$(VERSION_LINUXKERNEL).tar.bz2
CONFIGS_LINUXKERNEL		= dot-configs

VERSION_E1000			= 8.0.25
URI_E1000			= http://downloadmirror.intel.com/9180/eng
FOLDER_E1000			= e1000-$(VERSION_E1000)/src
TARBALL_E1000			= e1000-$(VERSION_E1000).tar.gz

VERSION_E1000e			= 1.2.20
URI_E1000e			= http://downloadmirror.intel.com/15817/eng
FOLDER_E1000e			= e1000e-$(VERSION_E1000e)/src
TARBALL_E1000e			= e1000e-$(VERSION_E1000e).tar.gz

VERSION_IGB			= 2.4.12
URI_IGB				= http://downloadmirror.intel.com/13663/eng
FOLDER_IGB			= igb-$(VERSION_IGB)/src
TARBALL_IGB			= igb-$(VERSION_IGB).tar.gz

VERSION_R8168			= 8.024.00
URI_R8168			= ftp://WebUser:pGL7E6v@218.210.127.132/cn/nic/
FOLDER_R8168			= r8168-$(VERSION_R8168)/src
TARBALL_R8168			= r8168-$(VERSION_R8168).tar.bz2

VERSION_R8169			= 6.014.00
URI_R8169			= ftp://WebUser:pGL7E6v@218.210.127.132/cn/nic
FOLDER_R8169			= r8169-$(VERSION_R8169)/src
TARBALL_R8169			= r8169-$(VERSION_R8169).tar.bz2

VERSION_R8101			= 1.020.00
URI_R8101			= ftp://WebUser:pGL7E6v@218.210.127.132/cn/nic
FOLDER_R8101			= r8101-$(VERSION_R8101)/src
TARBALL_R8101			= r8101-$(VERSION_R8101).tar.bz2

VERSION_ATL1E			= 1.0.1.9
URI_ATL1E			= http://partner.atheros.com/Drivers.aspx
FOLDER_ATL1E			= AR81Family-Linux-v$(VERSION_ATL1E)/src
TARBALL_ATL1E			= AR81Family-Linux-v$(VERSION_ATL1E).tar.gz

VERSION_570X			= 3.105h
URI_570X			= http://www.broadcom.com/docs/driver_download/570x
FOLDER_570X			= tg3-$(VERSION_570X)
ARCHIVE_570X			= linux-$(VERSION_570X).zip
TARBALL_570X			= tg3-$(VERSION_570X).tar.gz

VERSION_NXII			= 2.0.8e
URI_NXII			= http://www.broadcom.com/docs/driver_download/NXII
FOLDER_NXII			= netxtreme2-5.2.55/bnx2/src
ARCHIVE_NXII			= linux-$(VERSION_NXII).zip
TARBALL_NXII			= netxtreme2-5.2.55.tar.gz
