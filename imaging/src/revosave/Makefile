# (c) 2003-2007 Linbox FAS, http://linbox.com
# (c) 2008-2009 Mandriva, http://www.mandriva.com
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

OBJ=compress.o
LIBOBJ=compress.o easy_sock.o client.o
IMGFILES=image_swap image_lvmreiserfs image_fat image_ntfs image_e2fs image_xfs image_jfs image_lvm decompress decompress-test image_raw decompress-bitmap

3RDPARTY=zlib newt slang

ZLIB_DIR=../zlib
ZLIB_ARC=../../3rd_party/zlib-1.2.3.tar.gz
ZLIB_OBJ=$(wildcard $(ZLIB)/*.o)

NEWT=3rd_party/newt/newt-0.52.10
SLANG=3rd_party/slang/slang-2.0.6
UINEWT=ui_newt

CC=gcc-3.4
CFLAGS=-Wall -O2 -DLBLIMAGEVER='1.2' -I$(ZLIB_DIR) -I./e2fsprogs/lib/ -m32
LDFLAGS=-s
LIBS= -L. -llrs
RFSLIBS=-L./reiserfsprogs/lib -lcore -lmisc

all: $(OBJ) $(LIBOBJ) liblrs.so.1 liblrs.a $(IMGFILES) $(3RDPARTY)

depend:
	$(CC) -M $(CFLAGS) *.c >.depend

image_fat: image_fat.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LIBS)

image_swap: image_swap.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LIBS)

image_raw: image_raw.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LIBS)

image_ntfs: image_ntfs.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LIBS)

image_e2fs: image_e2fs.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LIBS) -L./e2fsprogs/lib -lext2fs -le2p -luuid -lcom_err

image_xfs: image_xfs.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LIBS)

image_jfs: image_jfs.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LIBS)

image_lvm: image_lvm.o lvm.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LIBS)

image_lvmreiserfs: lvm.o image_lvmreiserfs.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LIBS) $(RFSLIBS)

decompress: decompress.o $(OBJ)
	$(CC) $(CFLAGS) $(LDFLAGS) -L. -llrs -lnewt -o $@ $^

decompress-test: decompress-test.o $(OBJ)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $< --static -L. -llrs

decompress-bitmap: decompress-bitmap.o $(OBJ)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $< -L. -llrs -lnewt

image_e2fs.o: image_e2fs.c compress.h compress-loop.h config.h e2fsprogs
	$(CC) $(CFLAGS) -o image_e2fs.o -c image_e2fs.c -I./e2fsprogs/lib

liblrs.so.1: $(LIBOBJ) $(ZLIB_DIR)
	$(CC) -shared -Wl,-soname,liblrs.so.1 -o liblrs.so.1 $^ $(ZLIB_OBJ)
	strip $@
	ln -sf $@ liblrs.so

liblrs.a: $(LIBOBJ)
	ar rc $@ $^ $(ZLIB_OBJ)
	ranlib $@

zlib:
	(cd $(ZLIB_DIR); tar zxf $(ZLIB_ARC) --strip 1; patch -p 0 < Makefile.diff)
	$(MAKE) -C $(ZLIB_DIR) clean
	$(MAKE) -C $(ZLIB_DIR)

newt: slang
	(cd $(NEWT); env CC=$(CC) ./configure -quiet)
	$(MAKE) -C $(NEWT) CPPFLAGS="-D_GNU_SOURCE -I../../$(SLANG)/src/elfobjs" whiptail

slang:
	(cd $(SLANG); env CC=$(CC) ./configure -quiet)
	$(MAKE) -C $(SLANG) elf

ui_newt: newt
	$(MAKE) -C $(UINEWT)

include .depend

clean:
	-$(MAKE) -C $(NEWT) clean
	-$(MAKE) -C $(SLANG) clean
	-$(MAKE) -C $(UINEWT) clean
	rm -f liblrs.so.1 liblrs.so liblrs.a $(IMGFILES)
