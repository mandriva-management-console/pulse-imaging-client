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
topdir = $(abspath ..)

# Detect number of CPUs
NPROCS := 1
OS := $(shell uname -s)
ifeq ($(OS),Linux)
	NPROCS = $(shell grep -c ^processor /proc/cpuinfo)
endif

SUBDIRS = rdev . modules firmware

include $(topdir)/version.mk

extra_archive = linux-$(linux_version).tar.bz2
extra_uri = http://ftp.free.fr/mirrors/ftp.kernel.org/linux/kernel/v3.x/$(extra_archive)
linux_configdir = dot-configs
linux_config = $(extra_srcdir)/.config
linux_image = $(extra_srcdir)/arch/$(ARCHITECTURE)/boot/bzImage
rdev = $(CURDIR)/rdev/rdev
patch_srcdir = $(extra_srcdir)

include $(topdir)/common.mk
include $(topdir)/extra.mk
include $(topdir)/patch.mk

all-local: $(linux_image)

linux-image: $(linux_image)

$(linux_image): $(linux_config)
	# Temporary fix for lpfc_scsi
	# See: http://comments.gmane.org/gmane.linux.kernel/1301034
	cat $(extra_srcdir)/drivers/scsi/lpfc/Makefile | grep -v "ccflags-y += -Werror" > $(extra_srcdir)/drivers/scsi/lpfc/Makefile.new
	mv $(extra_srcdir)/drivers/scsi/lpfc/Makefile.new $(extra_srcdir)/drivers/scsi/lpfc/Makefile
	$(MAKE) -j$(NPROCS) -C $(extra_srcdir) ARCH=$(ARCHITECTURE) bzImage
	$(MAKE) -j$(NPROCS) -C $(extra_srcdir) ARCH=$(ARCHITECTURE) modules

$(linux_config): $(linux_configdir)/dot-config-$(linux_version)
	$(MAKE) patched-srcdir
	cp -a $< $@
	$(MAKE) -C $(@D) ARCH=$(ARCHITECTURE) silentoldconfig

install-local: /dev/ram0
	mkdir -p $(DESTDIR)$(disklessdir)
	$(install_DATA) $(extra_srcdir)/arch/$(ARCHITECTURE)/boot/bzImage \
	  $(DESTDIR)$(disklessdir)/kernel
	$(rdev) $(DESTDIR)$(disklessdir)/kernel /dev/ram0

	mkdir -p $(DESTDIR)$(initcdfsdir)/lib/modules/cd
	mkdir -p $(DESTDIR)$(initramfsdir)/lib/modules
	mkdir -p $(DESTDIR)$(postinstdir)/lib/modules

	# initrd common modules
	for mod in `find $(extra_srcdir) \
	     -path '$(extra_srcdir)/drivers/net/*.ko' \
	     -o -path '$(extra_srcdir)/drivers/pcmcia/*.ko' \
	     -o -path '$(extra_srcdir)/drivers/ssb/*.ko' \
	     -o -path '$(extra_srcdir)/drivers/i2c/*.ko' \
	     -o -path '$(extra_srcdir)/drivers/uio/*.ko' \
	     -o -path '$(extra_srcdir)/drivers/hwmon/hwmon.ko' \
	     -o -path '$(extra_srcdir)/drivers/ptp/*.ko' \
	     -o -path '$(extra_srcdir)/drivers/pps/*.ko' \
	     -o -path '$(extra_srcdir)/crypto/*.ko' \
	     -o -path '$(extra_srcdir)/lib/*.ko'`; do \
	       $(install_DATA) $$mod $(DESTDIR)$(initramfsdir)/lib/modules/; \
	done

	# initrdcd only modules
	for mod in `find $(extra_srcdir) \
	  -path $(extra_srcdir)/drivers/ide/ide-cd_mod.ko \
	  -o -path $(extra_srcdir)/drivers/cdrom/cdrom.ko \
	  -o -path $(extra_srcdir)/fs/isofs/isofs.ko \
	  -o -path $(extra_srcdir)/drivers/scsi/sr_mod.ko \
	  -o -path $(extra_srcdir)/drivers/usb/storage/usb-storage.ko`; do \
	  $(install_DATA) $$mod $(DESTDIR)$(initcdfsdir)/lib/modules/cd; \
	done
	mv $(DESTDIR)$(initcdfsdir)/lib/modules/cd/ide-cd_mod.ko \
	  $(DESTDIR)$(initcdfsdir)/lib/modules/cd/ide-cd.ko 

	# postinst modules
	for mod in `find $(extra_srcdir) \
	  -path $(extra_srcdir)/fs/mbcache.ko \
	  -o -path $(extra_srcdir)/fs/fuse/fuse.ko \
	  -o -path $(extra_srcdir)/fs/ext2/ext2.ko \
	  -o -path $(extra_srcdir)/fs/ext3/ext3.ko \
	  -o -path $(extra_srcdir)/fs/ext4/ext4.ko \
	  -o -path $(extra_srcdir)/fs/xfs/xfs.ko \
	  -o -path $(extra_srcdir)/fs/reiserfs/reiserfs.ko \
	  -o -path $(extra_srcdir)/fs/jbd/jbd.ko \
	  -o -path $(extra_srcdir)/fs/jbd2/jbd2.ko \
	  -o -path $(extra_srcdir)/fs/exportfs/exportfs.ko`; do \
	  $(install_DATA) $$mod $(DESTDIR)$(postinstdir)/lib/modules/; \
	done

clean-local:
	if test -d "$(extra_srcdir)"; then \
	  $(MAKE) -C $(extra_srcdir) clean; \
	fi
	rm -f $(linux_image)

distclean-local:
	test -L $(extra_archive) && rm -f $(extra_archive) || true

/dev/ram0:
	@if test `id -u` -eq 0; then \
	  mknod /dev/ram0 b 1 0; \
	else \
	  echo "###"; \
	  echo "### /dev/ram0 is needed. Please create it with the following command, as root:"; \
	  echo "### mknod /dev/ram0 b 1 0"; \
	  echo "###"; \
	  false; \
	fi
