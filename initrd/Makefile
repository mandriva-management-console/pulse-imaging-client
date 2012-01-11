#
# (c) 2011 Mandriva, http://www.mandriva.com
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
SUBDIRS = atftp busybox dm lvm popt pump sysvinit tinylogin

topdir = $(abspath ..)
include $(topdir)/common.mk

install-local:
	mkdir -p $(DESTDIR)$(initramfsdir)
	tar c --exclude=\.svn -C tree . | tar xC $(DESTDIR)$(initramfsdir)
	$(MAKE) symlinks
	mkdir -p $(DESTDIR)$(imaginglibdir)
	$(install_BIN) update-initrd $(DESTDIR)$(imaginglibdir)/

symlinks:
	# Create directories
	for dir in sbin bin lib var usr/bin usr/lib \
	  etc/rc0.d etc/rc2.d etc/rc6.d etc/rcS.d; do \
	  mkdir -p $(DESTDIR)$(initramfsdir)/$$dir; \
	done 

	# Misc symlinks
	ln -sf sbin/init $(DESTDIR)$(initramfsdir)/init
	ln -sf halt $(DESTDIR)$(initramfsdir)/sbin/reboot
	ln -sf ../sbin/route $(DESTDIR)$(initramfsdir)/bin/route

	# Init tree
	ln -sf ../init.d/sendsigs	$(DESTDIR)$(initramfsdir)/etc/rc0.d/S20sendsigs
	ln -sf ../init.d/urandom	$(DESTDIR)$(initramfsdir)/etc/rc0.d/S30urandom
	ln -sf ../init.d/umountfs	$(DESTDIR)$(initramfsdir)/etc/rc0.d/S40umountfs
	ln -sf ../init.d/halt		$(DESTDIR)$(initramfsdir)/etc/rc0.d/S99halt
	ln -sf ../init.d/netbase	$(DESTDIR)$(initramfsdir)/etc/rc0.d/K04netbase
	ln -sf ../init.d/sysklogd	$(DESTDIR)$(initramfsdir)/etc/rc0.d/K16sysklogd

	ln -sf ../init.d/sysklogd	$(DESTDIR)$(initramfsdir)/etc/rc2.d/S00sysklogd
	ln -sf ../init.d/netbase	$(DESTDIR)$(initramfsdir)/etc/rc2.d/S03netbase
	ln -sf ../init.d/rmnologin	$(DESTDIR)$(initramfsdir)/etc/rc2.d/S16rmnologin
	ln -sf ../init.d/modutils	$(DESTDIR)$(initramfsdir)/etc/rc2.d/S20modutils
	ln -sf ../init.d/network	$(DESTDIR)$(initramfsdir)/etc/rc2.d/S40network
	ln -sf ../init.d/revoboot	$(DESTDIR)$(initramfsdir)/etc/rc2.d/S99revoboot

	ln -sf ../init.d/sendsigs	$(DESTDIR)$(initramfsdir)/etc/rc6.d/S20sendsigs
	ln -sf ../init.d/urandom	$(DESTDIR)$(initramfsdir)/etc/rc6.d/S30urandom
	ln -sf ../init.d/umountfs	$(DESTDIR)$(initramfsdir)/etc/rc6.d/S40umountfs
	ln -sf ../init.d/reboot		$(DESTDIR)$(initramfsdir)/etc/rc6.d/S99reboot
	ln -sf ../init.d/netbase	$(DESTDIR)$(initramfsdir)/etc/rc6.d/K04netbase
	ln -sf ../init.d/sysklogd	$(DESTDIR)$(initramfsdir)/etc/rc6.d/K16sysklogd

	ln -sf ../init.d/checkroot	$(DESTDIR)$(initramfsdir)/etc/rcS.d/S10checkroot
	ln -sf ../init.d/mountall	$(DESTDIR)$(initramfsdir)/etc/rcS.d/S35mountall
	ln -sf ../init.d/hostname	$(DESTDIR)$(initramfsdir)/etc/rcS.d/S40hostname
	ln -sf ../init.d/tz		$(DESTDIR)$(initramfsdir)/etc/rcS.d/S50tz
	ln -sf ../init.d/bootmisc.sh	$(DESTDIR)$(initramfsdir)/etc/rcS.d/S55bootmisc
	ln -sf ../init.d/dm		$(DESTDIR)$(initramfsdir)/etc/rcS.d/S60dm

	# LibC files
	ln -sf libm-2.11.2.so $(DESTDIR)$(initramfsdir)/lib/libm.so.6
	ln -sf libnss_files-2.11.2.so $(DESTDIR)$(initramfsdir)/lib/libnss_files.so.2
	ln -sf libresolv-2.11.2.so $(DESTDIR)$(initramfsdir)/lib/libresolv.so.2
	ln -sf libnsl-2.11.2.so $(DESTDIR)$(initramfsdir)/lib/libnsl.so.1
	ln -sf libutil-2.11.2.so $(DESTDIR)$(initramfsdir)/lib/libutil.so.1
	ln -sf libcrypt-2.11.2.so $(DESTDIR)$(initramfsdir)/lib/libcrypt.so.1
	ln -sf ld-2.11.2.so $(DESTDIR)$(initramfsdir)/lib/ld-linux.so.2
	ln -sf libdl-2.11.2.so $(DESTDIR)$(initramfsdir)/lib/libdl.so.2
	ln -sf libnss_dns-2.11.2.so $(DESTDIR)$(initramfsdir)/lib/libnss_dns.so.2
	ln -sf libc-2.11.2.so $(DESTDIR)$(initramfsdir)/lib/libc.so.6
	ln -sf libpthread-2.11.2.so $(DESTDIR)$(initramfsdir)/lib/libpthread.so.0
	# Other libs
	ln -sf libblkid.so.1.1.0 $(DESTDIR)$(initramfsdir)/lib/libblkid.so.1
	ln -sf libuuid.so.1.3.0 $(DESTDIR)$(initramfsdir)/lib/libuuid.so.1
	ln -sf libwrap.so.0.7.6 $(DESTDIR)$(initramfsdir)/lib/libwrap.so.0

	# Other stuff
	ln -sf /tmp $(DESTDIR)$(initramfsdir)/var/

	ln -sf nc $(DESTDIR)$(initramfsdir)/usr/bin/netcat
	ln -sf /usr/share/terminfo $(DESTDIR)$(initramfsdir)/usr/lib/

.PHONY += symlinks
