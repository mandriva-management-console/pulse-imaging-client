#
# (c) 2010 Mandriva, http://www.mandriva.com
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

include consts.mk

all: chntpw fuse ntfs3g parted ntfsprogs

chntpw:
	[ -e $(3RDPARTY_FOLDER)/$(CHNTPW_TARBALL) ] || wget $(CHNTPW_URI)/$(CHNTPW_TARBALL) -P $(3RDPARTY_FOLDER)
	[ -d $(BUILD_FOLDER)/$(CHNTPW_FOLDER) ] || unzip $(3RDPARTY_FOLDER)/$(CHNTPW_TARBALL) -d $(BUILD_FOLDER)
	cp -a $(BUILD_FOLDER)/$(CHNTPW_FOLDER)/chntpw.static $(BUILD_FOLDER)/
	chmod +x $(BUILD_FOLDER)/chntpw.static

fuse:
	[ -e $(3RDPARTY_FOLDER)/$(FUSE_TARBALL) ] || wget $(FUSE_URI)/$(FUSE_TARBALL) -P $(3RDPARTY_FOLDER)
	[ -d $(BUILD_FOLDER)/$(FUSE_FOLDER) ] || tar zxf $(3RDPARTY_FOLDER)/$(FUSE_TARBALL) -C $(BUILD_FOLDER)
	[ -f $(BUILD_FOLDER)/$(FUSE_FOLDER)/Makefile ] || (cd $(BUILD_FOLDER)/$(FUSE_FOLDER) && CC="$(CC)" ./configure)
	$(MAKE) -C $(BUILD_FOLDER)/$(FUSE_FOLDER) CC="$(CC)"
	cp -a $(BUILD_FOLDER)/$(FUSE_FOLDER)/util/fusermount $(BUILD_FOLDER)/

ntfs3g:
	[ -e $(3RDPARTY_FOLDER)/$(NTFS3G_TARBALL) ] || wget $(NTFS3G_URI)/$(NTFS3G_TARBALL) -P $(3RDPARTY_FOLDER)
	[ -d $(BUILD_FOLDER)/$(NTFS3G_FOLDER) ] || tar zxf $(3RDPARTY_FOLDER)/$(NTFS3G_TARBALL) -C $(BUILD_FOLDER)
	[ -f $(BUILD_FOLDER)/$(NTFS3G_FOLDER)/Makefile ] || (cd $(BUILD_FOLDER)/$(NTFS3G_FOLDER) && CC="$(CC)" ./configure)
	$(MAKE) -C $(BUILD_FOLDER)/$(NTFS3G_FOLDER) CC="$(CC)"
	cp -a $(BUILD_FOLDER)/$(NTFS3G_FOLDER)/src/.libs/ntfs-3g $(BUILD_FOLDER)/
	cp -a $(BUILD_FOLDER)/$(NTFS3G_FOLDER)/libntfs-3g/.libs/libntfs-3g.so* $(BUILD_FOLDER)/

parted:
	[ -e $(3RDPARTY_FOLDER)/$(PARTED_TARBALL) ] || wget $(PARTED_URI)/$(PARTED_TARBALL) -P $(3RDPARTY_FOLDER)
	[ -d $(BUILD_FOLDER)/$(PARTED_FOLDER) ] || tar zxf $(3RDPARTY_FOLDER)/$(PARTED_TARBALL) -C $(BUILD_FOLDER)
	[ -f $(BUILD_FOLDER)/$(PARTED_FOLDER)/Makefile ] || (cd $(BUILD_FOLDER)/$(PARTED_FOLDER) && CC="$(CC)" ./configure --disable-device-mapper --without-readline)
	$(MAKE) -C $(BUILD_FOLDER)/$(PARTED_FOLDER) CC="$(CC)"
	cp -a $(BUILD_FOLDER)/$(PARTED_FOLDER)/parted/.libs/parted $(BUILD_FOLDER)/
	cp -a $(BUILD_FOLDER)/$(PARTED_FOLDER)/libparted/.libs/libparted.so* $(BUILD_FOLDER)/

ntfsprogs:
	[ -e $(3RDPARTY_FOLDER)/$(NTFSPROGS_TARBALL) ] || wget $(NTFSPROGS_URI)/$(NTFSPROGS_TARBALL) -P $(3RDPARTY_FOLDER)
	[ -d $(BUILD_FOLDER)/$(NTFSPROGS_FOLDER) ] || tar jxf $(3RDPARTY_FOLDER)/$(NTFSPROGS_TARBALL) -C $(BUILD_FOLDER)
	[ -f $(BUILD_FOLDER)/$(NTFSPROGS_FOLDER)/Makefile ] || (cd $(BUILD_FOLDER)/$(NTFSPROGS_FOLDER) && CC="$(CC)" ./configure)
	$(MAKE) -C $(BUILD_FOLDER)/$(NTFSPROGS_FOLDER) CC="$(CC)"
	cp -a $(BUILD_FOLDER)/$(NTFSPROGS_FOLDER)/ntfsprogs/.libs/ntfsresize $(BUILD_FOLDER)/
	cp -a $(BUILD_FOLDER)/$(NTFSPROGS_FOLDER)/libntfs/.libs/libntfs.so* $(BUILD_FOLDER)/

clean:
	$(MAKE) clean -C $(BUILD_FOLDER)/$(FUSE_FOLDER)
	$(MAKE) clean -C $(BUILD_FOLDER)/$(NTFS3G_FOLDER)
	$(MAKE) clean -C $(BUILD_FOLDER)/$(PARTED_FOLDER)
	$(MAKE) clean -C $(BUILD_FOLDER)/$(NTFSPROGS_FOLDER)
