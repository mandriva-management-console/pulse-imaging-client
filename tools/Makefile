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

include consts.mk

all: image save restore ui_newt bench ntblfix

image: zlib reiserfs e2fs
	$(MAKE) -C revosave CC=$(CC) ZLIB_DIR=../$(ZLIB_FOLDER) RFS_DIR=../$(REISER_FOLDER) E2FS_DIR=../$(E2FS_FOLDER)

save: image
	$(MAKE) -C autosave CC=$(CC) LRS_DIR=../revosave

restore: image
	$(MAKE) -C autorestore CC=$(CC) ZLIB_DIR=../$(ZLIB_FOLDER) LRS_DIR=../revosave

bench: image
	$(MAKE) -C bench CC=$(CC) LRS_DIR=../revosave ZLIB_DIR=../$(ZLIB_FOLDER)

zlib:
	[ -e $(ZLIB_TARGET)/$(ZLIB_TARBALL) ] || wget $(ZLIB_URI)/$(ZLIB_TARBALL) -P $(ZLIB_TARGET)
	[ -d $(ZLIB_FOLDER) ] || tar zxf $(ZLIB_TARGET)/$(ZLIB_TARBALL) -C $(ZLIB_TARGET)
	-@patch -N -p0 -d $(ZLIB_TARGET) < 3rd_party/zlib-1.2.3.diff
	$(MAKE) -C $(ZLIB_FOLDER) CC=$(CC)

slang:
	[ -e $(SLANG_TARGET)/$(SLANG_TARBALL) ] || wget $(SLANG_URI)/$(SLANG_TARBALL) -P $(SLANG_TARGET)
	[ -d $(SLANG_FOLDER) ] || tar zxf $(SLANG_TARGET)/$(SLANG_TARBALL) -C $(SLANG_TARGET)
	[ -f $(SLANG_FOLDER)/Makefile ] || (cd $(SLANG_FOLDER); ./configure)
	$(MAKE) -C $(SLANG_FOLDER) CC=$(CC)

newt: slang
	[ -e $(NEWT_TARGET)/$(NEWT_TARBALL) ] || wget --no-check-certificate $(NEWT_URI)/$(NEWT_TARBALL) -P $(NEWT_TARGET)
	[ -d $(NEWT_FOLDER) ] || tar zxf $(NEWT_TARGET)/$(NEWT_TARBALL) -C $(NEWT_TARGET)
	[ -f $(NEWT_FOLDER)/Makefile ] || (cd $(NEWT_FOLDER); CPPFLAGS='-I../../$(SLANG_FOLDER)' LDFLAGS='-L../../$(SLANG_FOLDER)' ./configure)
	$(MAKE) sharedlib -C $(NEWT_FOLDER) CC=$(CC)

e2fs:
	[ -e $(E2FS_TARGET)/$(E2FS_TARBALL) ] || wget $(E2FS_URI)/$(E2FS_TARBALL) -P $(E2FS_TARGET)
	[ -d $(E2FS_FOLDER) ] || tar zxf $(E2FS_TARGET)/$(E2FS_TARBALL) -C $(E2FS_TARGET)
	[ -f $(E2FS_FOLDER)/Makefile ] || (cd $(E2FS_FOLDER); ./configure)
	$(MAKE) libs -C $(E2FS_FOLDER) CC=$(CC)

reiserfs:
	[ -e $(REISER_TARGET)/$(REISER_TARBALL) ] || wget $(REISER_URI)/$(REISER_TARBALL) -P $(REISER_TARGET)
	[ -d $(REISER_FOLDER) ] || tar zxf $(REISER_TARGET)/$(REISER_TARBALL) -C $(REISER_TARGET)
	[ -f $(REISER_FOLDER)/Makefile ] || (cd $(REISER_FOLDER); ./configure)
#	$(MAKE) -C $(REISER_FOLDER) CC=$(CC)

ui_newt: save
	$(MAKE) -C ui_newt CC=$(CC) LRS_DIR=../revosave SLANG_DIR=../$(SLANG_FOLDER) NEWT_DIR=../$(NEWT_FOLDER)

ntblfix:
	$(MAKE) -C ntblfix CC=$(CC)

clean:
	$(MAKE) clean -C revosave
	$(MAKE) clean -C autosave
	$(MAKE) clean -C autorestore
	$(MAKE) clean -C ui_newt
	$(MAKE) clean -C ntblfix
	$(MAKE) clean -C bench
	$(MAKE) clean -C $(ZLIB_FOLDER)
	-$(MAKE) clean -C $(NEWT_FOLDER)
	rm -f $(NEWT_FOLDER)/Makefile
	-$(MAKE) clean -C $(SLANG_FOLDER)
	rm -f $(SLANG_FOLDER)/Makefile
	-$(MAKE) clean -C $(REISER_FOLDER)
	rm -f $(REISER_FOLDER)/Makefile
	-$(MAKE) clean -C $(E2FS_FOLDER)

dist-clean:
	rm -fr $(ZLIB_FOLDER) $(NEWT_FOLDER) $(SLANG_FOLDER) $(REISER_FOLDER) $(E2FS_FOLDER)
	rm -f $(ZLIB_TARGET)/$(ZLIB_TARBALL) $(NEWT_TARGET)/$(NEWT_TARBALL) $(SLANG_TARGET)/$(SLANG_TARBALL) $(E2FS_TARGET)/$(E2FS_TARBALL) $(REISER_TARGET)/$(REISER_TARBALL)

.PHONY: ntblfix
