#
# (c) 2009 Mandriva, http://www.mandriva.com
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

CC			:= gcc
SVNREV			:= $(shell echo $Rev: 4505 $ | tr -cd [[:digit:]])
BUILD_FOLDER		= $(shell pwd)/build
3RDPARTY_FOLDER		= $(shell pwd)/3rd_party
REVOBIN_BINARIES	= image_e2fs image_fat image_jfs image_lvm image_lvmreiserfs image_ntfs image_raw image_swap image_ufs image_xfs
REVORESTORE_BINARIES	= autorestore revosendlog revowait revodoneimage revogetname revogetuuid revosetdefault revoinc revoinv

include consts.mk

all: image save restore ui_newt bench ntblfix memtest deps
	cp -a scripts/mount.sh 		$(BUILD_FOLDER)/bin
	cp -a scripts/mount-nfs.sh 	$(BUILD_FOLDER)/bin

deps:
	# also copy the requiered stuff
	cp -a /usr/lib/libnewt.so* $(BUILD_FOLDER)/lib
	cp -a /lib/libslang.so* $(BUILD_FOLDER)/lib
	cp -a /usr/lib/libz.so* $(BUILD_FOLDER)/lib
	cp -a /usr/bin/whiptail $(BUILD_FOLDER)/usr/bin
	cp -a /lib/libext2fs.so.* /usr/lib/libext2fs.so $(BUILD_FOLDER)/lib
	cp -a /lib/libcom_err.so.2* $(BUILD_FOLDER)/lib

image: reiserfs
	$(MAKE) -C revosave CC=$(CC) RFS_DIR=$(BUILD_FOLDER)/$(REISER_FOLDER)
	(for i in $(REVOBIN_BINARIES); \
		do \
		cp -a "revosave/$$i" $(BUILD_FOLDER)/revobin; \
		done \
	)
	cp -a revosave/liblrs.so*		$(BUILD_FOLDER)/lib

save: image
	$(MAKE) -C autosave CC=$(CC) LRS_DIR=../revosave
	cp -a autosave/autosave 		$(BUILD_FOLDER)/bin
	cp -a autosave/floppysave 		$(BUILD_FOLDER)/bin

restore: image
	$(MAKE) -C autorestore CC=$(CC) LRS_DIR=../revosave
	(for i in $(REVORESTORE_BINARIES); \
		do \
		cp -a "autorestore/$$i" $(BUILD_FOLDER)/bin; \
		done \
	)
	cp -a autorestore/revoboot 		$(BUILD_FOLDER)/etc/init.d
	cp -a autorestore/revolib.sh 		$(BUILD_FOLDER)/usr/lib/revolib.sh

bench: image
	$(MAKE) -C bench CC=$(CC) LRS_DIR=../revosave
	cp -a bench/bench 	$(BUILD_FOLDER)/bin
	cp -a bench/bench.ping 	$(BUILD_FOLDER)/bin

# please see README.rebuild to learn why we do not d/l it
reiserfs:
	[ -d $(BUILD_FOLDER)/$(REISER_FOLDER) ] || (tar zxf $(3RDPARTY_FOLDER)/$(REISER_TARBALL) -C $(BUILD_FOLDER) && patch -N -p1 -d $(BUILD_FOLDER)/$(REISER_FOLDER) < $(3RDPARTY_FOLDER)/reiserfsprogs-3.x.0i-linbox01.diff)
	[ -f $(BUILD_FOLDER)/$(REISER_FOLDER)/Makefile ] || (cd $(BUILD_FOLDER)/$(REISER_FOLDER) && CC="$(CC)" ./configure)
	$(MAKE) -C $(BUILD_FOLDER)/$(REISER_FOLDER) CC="$(CC)"

ui_newt: save
	$(MAKE) -C ui_newt CC=$(CC) LRS_DIR=../revosave
	cp -a ui_newt/ui_newt $(BUILD_FOLDER)/bin/uinewt # changes name

ntblfix:
	$(MAKE) -C ntblfix CC=$(CC)
	cp -a ntblfix/ntblfix $(BUILD_FOLDER)/bin

memtest:
	[ -e $(3RDPARTY_FOLDER)/$(MEMTEST_TARBALL) ] || wget $(MEMTEST_URI)/$(MEMTEST_TARBALL) -P $(3RDPARTY_FOLDER)
	[ -d $(BUILD_FOLDER)/$(MEMTEST_FOLDER) ] || tar zxf $(3RDPARTY_FOLDER)/$(MEMTEST_TARBALL) -C $(BUILD_FOLDER)
	# see http://bugs.debian.org/cgi-bin/bugreport.cgi?bug=319837#33
	sed -i "s/0x10000;/0x100000;/" $(BUILD_FOLDER)/$(MEMTEST_FOLDER)/memtest.lds
	$(MAKE) -C $(BUILD_FOLDER)/$(MEMTEST_FOLDER) CC=$(CC)

clean:
	$(MAKE) clean -C revosave
	$(MAKE) clean -C autosave
	$(MAKE) clean -C autorestore
	$(MAKE) clean -C ui_newt
	$(MAKE) clean -C ntblfix
	$(MAKE) clean -C bench
	$(MAKE) clean -C $(BUILD_FOLDER)/$(REISER_FOLDER)
	$(MAKE) clean -C $(BUILD_FOLDER)/$(MEMTEST_FOLDER)
	rm -f $(BUILD_FOLDER)/revobin/*
	rm -f $(BUILD_FOLDER)/bin/*
	-rm -f $(BUILD_FOLDER)/lib/*
	rm -f $(BUILD_FOLDER)/lib/modules/*.ko
	rm -f $(BUILD_FOLDER)/usr/lib/revolib.sh

dist-clean:
	rm -fr $(BUILD_FOLDER)/$(REISER_FOLDER)

.PHONY: ntblfix
