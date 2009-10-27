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

include consts.mk

all: load

load: atftp pump busybox sysvinit
	# dev summoning
	-(cd tree/dev; ../../MAKEDEV console ptyp std hda hdb hdc hdd hde hdf hdg hdh sda sdb sdc sdd sde sdf sdg sdh fd0 cciss.0 cciss.1 ida.0 scd0 scd1 input; mknod -m 600 initctl p; mknod -m 600 log p)
	# symlinks summoning
	(cd tree; ../symlinks)
	# pump loading
	cp -a $(PUMP_FOLDER)/pump tree/bin
	# atftp loading
	cp -a $(ATFTP_FOLDER)/atftp $(LOOP)/bin
	# lvm tools loading
	cp -a $(LVM_FOLDER)/tools/lvm $(LOOP)/bin
	# sysvinit loading
	for n in halt init killall5 runlevel shutdown ;do \
	    cp -a $(SYSV_FOLDER)/$$n $(LOOP)/sbin || exit 1; \
	done

atftp:
	[ -e $(ATFTP_TARGET)/$(ATFTP_TARBALL) ] || wget $(ATFTP_URI)/$(ATFTP_TARBALL) -P $(ATFTP_TARGET)
	[ -e $(ATFTP_TARGET)/$(ATFTP_PATCH) ] || wget $(ATFTP_URI)/$(ATFTP_PATCH) -P $(ATFTP_TARGET)
	[ -d $(ATFTP_FOLDER) ] || tar zxf $(ATFTP_TARGET)/$(ATFTP_TARBALL) -C $(ATFTP_TARGET)
	-zcat $(ATFTP_TARGET)/$(ATFTP_PATCH) | patch -N -p1 -d $(ATFTP_FOLDER)
	[ -f $(ATFTP_FOLDER)/Makefile ] || (cd $(ATFTP_FOLDER); CC=$(CC) ./configure)
	$(MAKE) -C $(ATFTP_FOLDER) CC=$(CC) atftp

lvm:
	[ -e $(LVM_TARGET)/$(LVM_TARBALL) ] || wget $(LVM_URI)/$(LVM_TARBALL) -P $(LVM_TARGET)
	[ -d $(LVM_FOLDER) ] || tar zxf $(LVM_TARGET)/$(LVM_TARBALL) -C $(LVM_TARGET)
	[ -f $(LVM_FOLDER)/Makefile ] || (cd $(LVM_FOLDER); CC=$(CC) ./configure)
	$(MAKE) tools -C $(LVM_FOLDER) CC=$(CC)

pump: popt
	[ -e $(PUMP_TARGET)/$(PUMP_TARBALL) ] || wget $(PUMP_URI)/$(PUMP_TARBALL) -P $(PUMP_TARGET)
	[ -d $(PUMP_FOLDER) ] || tar zxf $(PUMP_TARGET)/$(PUMP_TARBALL) -C $(PUMP_TARGET)
	$(MAKE) -C $(PUMP_FOLDER) CC=$(CC) LDFLAGS='-L../../$(POPT_FOLDER)/.libs' DEB_CFLAGS='-I../../$(POPT_FOLDER)'
	strip $(PUMP_FOLDER)/pump

popt:
	[ -e $(POPT_TARGET)/$(POPT_TARBALL) ] || wget $(POPT_URI)/$(POPT_TARBALL) -P $(POPT_TARGET)
	[ -d $(POPT_FOLDER) ] || tar zxf $(POPT_TARGET)/$(POPT_TARBALL) -C $(POPT_TARGET)
	[ -f $(POPT_FOLDER)/Makefile ] || (cd $(POPT_FOLDER); CC=$(CC) ./configure)
	$(MAKE) -C $(POPT_FOLDER) CC=$(CC)

busybox:
	[ -e $(BUSYBOX_TARGET)/$(BUSYBOX_TARBALL) ] || wget $(BUSYBOX_URI)/$(BUSYBOX_TARBALL) -P $(BUSYBOX_TARGET)
	[ -d $(BUSYBOX_FOLDER) ] || tar zxf $(BUSYBOX_TARGET)/$(BUSYBOX_TARBALL) -C $(BUSYBOX_TARGET)
	#MDV/NR [ -f $(BUSYBOX_FOLDER)/Makefile ] || (cd $(BUSYBOX_FOLDER); CC=$(CC) ./configure)
	#MDV/NR $(MAKE) -C $(BUSYBOX_FOLDER) CC=$(CC)

sysvinit:
	[ -e $(SYSV_TARGET)/$(SYSV_TARBALL) ] || wget $(SYSV_URI)/$(SYSV_TARBALL) -P $(SYSV_TARGET)
	[ -d $(SYSV_FOLDER) ] || tar zxf $(SYSV_TARGET)/$(SYSV_TARBALL) -C $(SYSV_TARGET)
	$(MAKE) -C $(SYSV_FOLDER) CC=$(CC)

clean:
	-$(MAKE) clean -C $(POPT_FOLDER)
	rm -f $(POPT_FOLDER)/Makefile
	-$(MAKE) clean -C $(BUSYBOX_FOLDER)
	-$(MAKE) clean -C $(SYSV_FOLDER)
	-$(MAKE) clean -C $(PUMP_FOLDER)
	-$(MAKE) clean -C $(ATFTP_FOLDER)
	rm -f $(ATFTP_FOLDER)/Makefile
	-$(MAKE) clean -C $(LVM_FOLDER)
	find tree/dev -type b -delete
	find tree/dev -type c -delete
	find tree/dev -type l -delete
	find tree/ -type l -delete
