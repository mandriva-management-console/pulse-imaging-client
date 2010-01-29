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

CC=gcc-3.4
SVNREV:=$(shell echo $Rev: 4505 $ | tr -cd [[:digit:]])

BUILD_FOLDER = $(shell pwd)/build
3RDPARTY_FOLDER = $(shell pwd)/3rd_party

include consts.mk

all: image save restore ui_newt bench ntblfix memtest
	cp -a scripts/mount.sh 		$(BUILD_FOLDER)/bin
	cp -a scripts/mount-nfs.sh 	$(BUILD_FOLDER)/bin

image: zlib e2fs reiserfs
	$(MAKE) -C revosave CC=$(CC) ZLIB_DIR=$(BUILD_FOLDER)/$(ZLIB_FOLDER) RFS_DIR=$(BUILD_FOLDER)/$(REISER_FOLDER) E2FS_DIR=$(BUILD_FOLDER)/$(E2FS_FOLDER)
	cp -a revosave/image_raw 		$(BUILD_FOLDER)/revobin
	cp -a revosave/image_swap 		$(BUILD_FOLDER)/revobin
	cp -a revosave/image_e2fs 		$(BUILD_FOLDER)/revobin
	cp -a revosave/image_fat 		$(BUILD_FOLDER)/revobin
	cp -a revosave/image_ntfs 		$(BUILD_FOLDER)/revobin
	cp -a revosave/image_jfs 		$(BUILD_FOLDER)/revobin
	cp -a revosave/image_ufs 		$(BUILD_FOLDER)/revobin
	cp -a revosave/image_lvm 		$(BUILD_FOLDER)/revobin
	cp -a revosave/image_lvmreiserfs	$(BUILD_FOLDER)/revobin
	cp -a revosave/image_xfs 		$(BUILD_FOLDER)/revobin
	cp -a revosave/liblrs.so*		$(BUILD_FOLDER)/lib

save: image
	$(MAKE) -C autosave CC=$(CC) LRS_DIR=../revosave
	cp -a autosave/autosave 		$(BUILD_FOLDER)/bin
	cp -a autosave/floppysave 		$(BUILD_FOLDER)/bin

restore: image
	$(MAKE) -C autorestore CC=$(CC) ZLIB_DIR=$(BUILD_FOLDER)/$(ZLIB_FOLDER) LRS_DIR=../revosave
	cp -a autorestore/autorestore 		$(BUILD_FOLDER)/bin
	cp -a autorestore/revosendlog 		$(BUILD_FOLDER)/bin
	cp -a autorestore/revowait 		$(BUILD_FOLDER)/bin
	cp -a autorestore/revogetname 		$(BUILD_FOLDER)/bin
	cp -a autorestore/revosetdefault	$(BUILD_FOLDER)/bin
	cp -a autorestore/revoinc 		$(BUILD_FOLDER)/bin
	cp -a autorestore/revoinv 		$(BUILD_FOLDER)/bin
	cp -a autorestore/revoboot 		$(BUILD_FOLDER)/etc/init.d

bench: image
	$(MAKE) -C bench CC=$(CC) LRS_DIR=../revosave ZLIB_DIR=$(BUILD_FOLDER)/$(ZLIB_FOLDER)
	cp -a bench/bench 	$(BUILD_FOLDER)/bin
	cp -a bench/bench.ping 	$(BUILD_FOLDER)/bin

zlib:
	[ -e $(3RDPARTY_FOLDER)/$(ZLIB_TARBALL) ] || wget $(ZLIB_URI)/$(ZLIB_TARBALL) -P $(3RDPARTY_FOLDER)
	[ -d $(BUILD_FOLDER)/$(ZLIB_FOLDER) ] || tar zxf $(3RDPARTY_FOLDER)/$(ZLIB_TARBALL) -C $(BUILD_FOLDER)
	-patch -N -p1 -d $(BUILD_FOLDER)/$(ZLIB_FOLDER) < $(3RDPARTY_FOLDER)/zlib-1.2.3.diff
	$(MAKE) -C $(BUILD_FOLDER)/$(ZLIB_FOLDER) CC=$(CC)

slang:
	[ -e $(3RDPARTY_FOLDER)/$(SLANG_TARBALL) ] || wget $(SLANG_URI)/$(SLANG_TARBALL) -P $(3RDPARTY_FOLDER)
	[ -d $(BUILD_FOLDER)/$(SLANG_FOLDER) ] || tar zxf $(3RDPARTY_FOLDER)/$(SLANG_TARBALL) -C $(BUILD_FOLDER)
	[ -f $(BUILD_FOLDER)/$(SLANG_FOLDER)/Makefile ] || (cd $(BUILD_FOLDER)/$(SLANG_FOLDER); ./configure)
	$(MAKE) -C $(BUILD_FOLDER)/$(SLANG_FOLDER) CC=$(CC)
	cp -a $(BUILD_FOLDER)/$(SLANG_FOLDER)/src/elfobjs/libslang.so* $(BUILD_FOLDER)/lib
	strip $(BUILD_FOLDER)/lib/libslang.so*

newt: slang popt
	[ -e $(3RDPARTY_FOLDER)/$(NEWT_TARBALL) ] || wget --no-check-certificate $(NEWT_URI)/$(NEWT_TARBALL) -P $(3RDPARTY_FOLDER)
	[ -d $(BUILD_FOLDER)/$(NEWT_FOLDER) ] || tar zxf $(3RDPARTY_FOLDER)/$(NEWT_TARBALL) -C $(BUILD_FOLDER)
	[ -f $(BUILD_FOLDER)/$(NEWT_FOLDER)/Makefile ] || (cd $(BUILD_FOLDER)/$(NEWT_FOLDER); CPPFLAGS='-I../$(POPT_FOLDER) -I../$(SLANG_FOLDER)' LIBS='-L../$(POPT_FOLDER)/.libs -L../$(SLANG_FOLDER)' ./configure)
	$(MAKE) -C $(BUILD_FOLDER)/$(NEWT_FOLDER) CC="$(CC) -L../$(POPT_FOLDER)/.libs -L../$(SLANG_FOLDER)" sharedlib whiptail
	cp -a $(BUILD_FOLDER)/$(NEWT_FOLDER)/libnewt.so* $(BUILD_FOLDER)/lib
	strip $(BUILD_FOLDER)/lib/libnewt.so*
	cp -a $(BUILD_FOLDER)/$(NEWT_FOLDER)/whiptail $(BUILD_FOLDER)/usr/bin
	strip $(BUILD_FOLDER)/usr/bin/whiptail

popt:
	[ -e $(3RDPARTY_FOLDER)/$(POPT_TARBALL) ] || wget $(POPT_URI)/$(POPT_TARBALL) -P $(3RDPARTY_FOLDER)
	[ -d $(BUILD_FOLDER)/$(POPT_FOLDER) ] || tar zxf $(3RDPARTY_FOLDER)/$(POPT_TARBALL) -C $(BUILD_FOLDER)
	[ -f $(BUILD_FOLDER)/$(POPT_FOLDER)/Makefile ] || (cd $(BUILD_FOLDER)/$(POPT_FOLDER); CC=$(CC) ./configure)
	$(MAKE) -C $(BUILD_FOLDER)/$(POPT_FOLDER) CC=$(CC)
	cp -a $(BUILD_FOLDER)/$(POPT_FOLDER)/.libs/libpopt.so* $(BUILD_FOLDER)/lib
	strip $(BUILD_FOLDER)/lib/libpopt.so*

e2fs:
	[ -e $(3RDPARTY_FOLDER)/$(E2FS_TARBALL) ] || wget $(E2FS_URI)/$(E2FS_TARBALL) -P $(3RDPARTY_FOLDER)
	[ -d $(BUILD_FOLDER)/$(E2FS_FOLDER) ] || tar zxf $(3RDPARTY_FOLDER)/$(E2FS_TARBALL) -C $(BUILD_FOLDER)
	[ -f $(BUILD_FOLDER)/$(E2FS_FOLDER)/Makefile ] || (cd $(BUILD_FOLDER)/$(E2FS_FOLDER); ./configure)
	$(MAKE) -C $(BUILD_FOLDER)/$(E2FS_FOLDER) CC=$(CC) libs

# please see README.rebuild to learn :
# - why we do not d/l it
# - why we do not build it it
reiserfs:
	[ -d $(BUILD_FOLDER)/$(REISER_FOLDER) ] || tar zxf $(3RDPARTY_FOLDER)/$(REISER_TARBALL) -C $(BUILD_FOLDER)

ui_newt: save newt
	$(MAKE) -C ui_newt CC=$(CC) LRS_DIR=../revosave SLANG_DIR=../$(SLANG_FOLDER) NEWT_DIR=../$(NEWT_FOLDER)
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
	$(MAKE) clean -C $(BUILD_FOLDER)/$(ZLIB_FOLDER)
	$(MAKE) clean -C $(BUILD_FOLDER)/$(MEMTEST_FOLDER)
	-[ -f $(BUILD_FOLDER)/$(NEWT_FOLDER)/Makefile ] && $(MAKE) clean -C $(BUILD_FOLDER)/$(NEWT_FOLDER) && rm -f $(BUILD_FOLDER)/$(NEWT_FOLDER)/Makefile
	-[ -f $(BUILD_FOLDER)/$(SLANG_FOLDER)/Makefile ] && $(MAKE) clean -C $(BUILD_FOLDER)/$(SLANG_FOLDER) && rm -f $(BUILD_FOLDER)/$(SLANG_FOLDER)/Makefile
	-rm -rf $(BUILD_FOLDER)/$(REISER_FOLDER)
	$(MAKE) clean -C $(BUILD_FOLDER)/$(E2FS_FOLDER)
	rm -f $(BUILD_FOLDER)/revobin/*
	rm -f $(BUILD_FOLDER)/bin/*
	-rm -f $(BUILD_FOLDER)/lib/*
	rm -f $(BUILD_FOLDER)/lib/modules/*.ko

dist-clean:
	rm -fr $(BUILD_FOLDER)/$(ZLIB_FOLDER)
	rm -fr $(BUILD_FOLDER)/$(NEWT_FOLDER)
	rm -fr $(BUILD_FOLDER)/$(SLANG_FOLDER)
	rm -fr $(BUILD_FOLDER)/$(REISER_FOLDER)
	rm -fr $(BUILD_FOLDER)/$(E2FS_FOLDER)

.PHONY: ntblfix
