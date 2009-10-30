#
# (c) 2009 Mandriva, http://www.mandriva.com
#
# $Id: Makefile 4505 2009-09-23 13:42:56Z nrueff $
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

CC=gcc-3.4

BUILD_FOLDER = build
3RDPARTY_FOLDER = 3rd_party
include consts.mk

all: load

load: lvm atftp pump busybox sysvinit
	# dev summoning
	-(cd tree/dev; ../../MAKEDEV console ptyp std hda hdb hdc hdd hde hdf hdg hdh sda sdb sdc sdd sde sdf sdg sdh fd0 cciss.0 cciss.1 ida.0 scd0 scd1 input; mknod -m 600 initctl p; mknod -m 600 log p)
	# symlinks summoning
	(cd tree; ../symlinks)

atftp:
	[ -e $(3RDPARTY_FOLDER)/$(ATFTP_TARBALL) ] || wget $(ATFTP_URI)/$(ATFTP_TARBALL) -P $(3RDPARTY_FOLDER)
	[ -e $(3RDPARTY_FOLDER)/$(ATFTP_PATCH) ] || wget $(ATFTP_URI)/$(ATFTP_PATCH) -P $(3RDPARTY_FOLDER)
	[ -d $(BUILD_FOLDER)/$(ATFTP_FOLDER) ] || tar zxf $(3RDPARTY_FOLDER)/$(ATFTP_TARBALL) -C $(BUILD_FOLDER)
	-zcat $(3RDPARTY_FOLDER)/$(ATFTP_PATCH) | patch -N -p1 -d $(BUILD_FOLDER)/$(ATFTP_FOLDER)
	[ -f $(BUILD_FOLDER)/$(ATFTP_FOLDER)/Makefile ] || (cd $(BUILD_FOLDER)/$(ATFTP_FOLDER); CC=$(CC) ./configure --disable-libreadline)
	$(MAKE) -C $(BUILD_FOLDER)/$(ATFTP_FOLDER) CC=$(CC) atftp
	cp -a $(BUILD_FOLDER)/$(ATFTP_FOLDER)/atftp tree/bin
	strip tree/bin/atftp

lvm: dm
	[ -e $(3RDPARTY_FOLDER)/$(LVM_TARBALL) ] || wget $(LVM_URI)/$(LVM_TARBALL) -P $(3RDPARTY_FOLDER)
	[ -d $(BUILD_FOLDER)/$(LVM_FOLDER) ] || tar zxf $(3RDPARTY_FOLDER)/$(LVM_TARBALL) -C $(BUILD_FOLDER)
	[ -f $(BUILD_FOLDER)/$(LVM_FOLDER)/Makefile ] || (cd $(BUILD_FOLDER)/$(LVM_FOLDER); LDFLAGS='-L../$(DM_FOLDER)/lib/ioctl -L../../$(DM_FOLDER)/lib/ioctl' CFLAGS='-I../$(DM_FOLDER)/include -I../../$(DM_FOLDER)/include' CPPFLAGS='-I../$(DM_FOLDER)/include -I../../$(DM_FOLDER)/include' CC=$(CC) ./configure --disable-selinux --disable-readline)
	$(MAKE) -C $(BUILD_FOLDER)/$(LVM_FOLDER) CC=$(CC) tools
	cp -a $(BUILD_FOLDER)/$(LVM_FOLDER)/tools/lvm tree/bin
	strip tree/bin/lvm

dm:
	[ -e $(3RDPARTY_FOLDER)/$(DM_TARBALL) ] || wget $(DM_URI)/$(DM_TARBALL) -P $(3RDPARTY_FOLDER)
	[ -d $(BUILD_FOLDER)/$(DM_FOLDER) ] || tar zxf $(3RDPARTY_FOLDER)/$(DM_TARBALL) -C $(BUILD_FOLDER)
	[ -f $(BUILD_FOLDER)/$(DM_FOLDER)/Makefile ] || (cd $(BUILD_FOLDER)/$(DM_FOLDER); CC=$(CC) ./configure)
	$(MAKE) -C $(BUILD_FOLDER)/$(DM_FOLDER) CC=$(CC) lib
	cp -a $(BUILD_FOLDER)/$(DM_FOLDER)/lib/ioctl/libdevmapper.so tree/lib
	strip tree/lib/libdevmapper.so
	ln -sf libdevmapper.so tree/lib/libdevmapper.so.1.01

pump: popt
	[ -e $(3RDPARTY_FOLDER)/$(PUMP_TARBALL) ] || wget $(PUMP_URI)/$(PUMP_TARBALL) -P $(3RDPARTY_FOLDER)
	[ -d $(BUILD_FOLDER)/$(PUMP_FOLDER) ] || tar zxf $(3RDPARTY_FOLDER)/$(PUMP_TARBALL) -C $(BUILD_FOLDER)
	-cat $(3RDPARTY_FOLDER)/$(PUMP_PATCH) | patch -N -p1 -d $(BUILD_FOLDER)/$(PUMP_FOLDER)
	$(MAKE) -C $(BUILD_FOLDER)/$(PUMP_FOLDER) CC=$(CC) LDFLAGS='-L../$(POPT_FOLDER)/.libs' DEB_CFLAGS='-I../$(POPT_FOLDER)'
	cp -a $(BUILD_FOLDER)/$(PUMP_FOLDER)/pump tree/bin
	strip tree/bin/pump

popt:
	[ -e $(3RDPARTY_FOLDER)/$(POPT_TARBALL) ] || wget $(POPT_URI)/$(POPT_TARBALL) -P $(3RDPARTY_FOLDER)
	[ -d $(BUILD_FOLDER)/$(POPT_FOLDER) ] || tar zxf $(3RDPARTY_FOLDER)/$(POPT_TARBALL) -C $(BUILD_FOLDER)
	[ -f $(BUILD_FOLDER)/$(POPT_FOLDER)/Makefile ] || (cd $(BUILD_FOLDER)/$(POPT_FOLDER); CC=$(CC) ./configure)
	$(MAKE) -C $(BUILD_FOLDER)/$(POPT_FOLDER) CC=$(CC)

busybox:
	[ -e $(3RDPARTY_FOLDER)/$(BUSYBOX_TARBALL) ] || wget $(BUSYBOX_URI)/$(BUSYBOX_TARBALL) -P $(3RDPARTY_FOLDER)
	[ -d $(BUILD_FOLDER)/$(BUSYBOX_FOLDER) ] || tar zxf $(3RDPARTY_FOLDER)/$(BUSYBOX_TARBALL) -C $(BUILD_FOLDER)
	[ -e $(BUILD_FOLDER)/$(BUSYBOX_FOLDER)/.config ] || cp -a $(3RDPARTY_FOLDER)/$(BUSYBOX_CONFIG) $(BUILD_FOLDER)/$(BUSYBOX_FOLDER)/.config
	$(MAKE) -C $(BUILD_FOLDER)/$(BUSYBOX_FOLDER) CC=$(CC)
	cp -a $(BUILD_FOLDER)/$(BUSYBOX_FOLDER)/busybox tree/bin

sysvinit:
	[ -e $(3RDPARTY_FOLDER)/$(SYSV_TARBALL) ] || wget $(SYSV_URI)/$(SYSV_TARBALL) -P $(3RDPARTY_FOLDER)
	[ -e $(3RDPARTY_FOLDER)/$(SYSV_PATCH) ] || wget $(SYSV_URI)/$(SYSV_PATCH) -P $(3RDPARTY_FOLDER)
	[ -d $(BUILD_FOLDER)/$(SYSV_FOLDER) ] || tar zxf $(3RDPARTY_FOLDER)/$(SYSV_TARBALL) -C $(BUILD_FOLDER)
	-zcat $(3RDPARTY_FOLDER)/$(SYSV_PATCH) | patch -N -p1 -d $(BUILD_FOLDER)/$(SYSV_FOLDER)/..
	$(MAKE) -C $(BUILD_FOLDER)/$(SYSV_FOLDER) CC=$(CC)
	for n in halt init killall5 runlevel shutdown; do cp -a $(BUILD_FOLDER)/$(SYSV_FOLDER)/$$n tree/sbin; done

clean:
	-$(MAKE) clean -C $(BUILD_FOLDER)/$(POPT_FOLDER)
	rm -f $(BUILD_FOLDER)/$(POPT_FOLDER)/Makefile
	-$(MAKE) clean -C $(BUILD_FOLDER)/$(BUSYBOX_FOLDER)
	-$(MAKE) clean -C $(BUILD_FOLDER)/$(SYSV_FOLDER)
	-$(MAKE) clean -C $(BUILD_FOLDER)/$(PUMP_FOLDER)
	-$(MAKE) clean -C $(BUILD_FOLDER)/$(ATFTP_FOLDER)
	rm -f $(BUILD_FOLDER)/$(ATFTP_FOLDER)/Makefile
	-$(MAKE) clean -C $(BUILD_FOLDER)/$(BUSYBOX_FOLDER)
	-$(MAKE) clean -C $(BUILD_FOLDER)/$(SYSV_FOLDER)
	find tree/dev -type b -delete
	find tree/dev -type c -delete
	find tree/dev -type l -delete
	find tree/dev -type p -delete
	rm -fr tree/dev/cciss
	rm -fr tree/dev/ida
	rm -fr tree/dev/input
	find tree/ -type l -delete
	rm -f tree/bin/atftp
	rm -f tree/bin/lvm
	rm -f tree/bin/pump
	rm -f tree/bin/busybox
	for n in halt init killall5 runlevel shutdown; do rm -f tree/sbin/$$n; done

dist-clean:
	rm -fr $(BUILD_FOLDER)/$(SYSV_FOLDER)
	rm -fr $(BUILD_FOLDER)/$(BUSYBOX_FOLDER)
	rm -fr $(BUILD_FOLDER)/$(POPT_FOLDER)
	rm -fr $(BUILD_FOLDER)/$(PUMP_FOLDER)
	rm -fr $(BUILD_FOLDER)/$(LVM_FOLDER)
	rm -fr $(BUILD_FOLDER)/$(ATFTP_FOLDER)

