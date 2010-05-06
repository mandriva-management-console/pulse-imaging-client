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

OBJ			= compress.o
LIBOBJ		= compress.o easy_sock.o client.o
OBJS		= decompress-bitmap.o decompress.o decompress-test.o image_e2fs.o image_fat.o image_jfs.o image_lvm.o image_ntfs.o image_raw.o image_swap.o image_xfs.o lvm.o image_lvmreiserfs.o
IMGFILES	= image_swap image_fat image_ntfs image_e2fs image_xfs image_jfs image_lvm decompress decompress-test image_raw decompress-bitmap image_lvmreiserfs

CFLAGS	= -Wall -O2 -DLBLIMAGEVER='1.2'
LDFLAGS	= -s
LIBLRS	= -L. -llrs
LIBZ	= -lz
RFSLIBS	= -L$(RFS_DIR)/lib -lcore  -lmisc
LIBE2FS	= -L$(E2FS_DIR)/lib -lext2fs -le2p -luuid -lcom_err

all: $(OBJ) $(LIBOBJ) liblrs.so.1 liblrs.a $(IMGFILES)

depend:
	$(CC) -M $(CFLAGS) *.c >.depend

image_fat: image_fat.o  compress.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LIBLRS)

image_swap: image_swap.o compress.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LIBLRS)

image_raw: image_raw.o compress.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LIBLRS)

image_ntfs: image_ntfs.o compress.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LIBLRS)

image_e2fs: image_e2fs.o compress.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LIBLRS) $(LIBE2FS)

image_xfs: image_xfs.o compress.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LIBLRS)

image_jfs: image_jfs.o compress.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LIBLRS)

image_lvm: image_lvm.o lvm.o compress.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LIBLRS)

image_lvmreiserfs: lvm.o image_lvmreiserfs.o compress.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LIBLRS) $(RFSLIBS)

decompress: decompress.o $(OBJ)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LIBLRS) $(LIBZ)

decompress-test: decompress-test.o $(OBJ)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $< --static $(LIBZ)

decompress-bitmap: decompress-bitmap.o $(OBJ)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $< $(LIBZ)

image_lvmreiserfs.o: image_lvmreiserfs.c
	$(CC) $(CFLAGS) -I$(RFS_DIR) -o image_lvmreiserfs.o -c image_lvmreiserfs.c

image_e2fs.o: image_e2fs.c
	$(CC) $(CFLAGS) -I$(E2FS_DIR)/lib -o image_e2fs.o -c image_e2fs.c

liblrs.so.1: $(LIBOBJ)
	$(CC) -shared -Wl,-soname,$@ -o $@ $^ $(LIBZ)
	strip $@
	ln -sf $@ liblrs.so

liblrs.a: $(LIBOBJ)
	ar rc $@ $^ $(LIBOBJ)
	ranlib $@

include .depend

clean:
	rm -f liblrs.so.1 liblrs.so liblrs.a $(LIBOBJ) $(IMGFILES) $(OBJS)
