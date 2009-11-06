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

SVNREV:=$(shell echo $Rev: 4491 $ | tr -cd [[:digit:]])

OBJ 		= asm.o bios.o boot.o pci.o char_io.o builtins.o common.o stage2.o cmdline.o disk_io.o graphics.o console.o fsys_fat.o fsys_ext2fs.o builtins_pulse2.o
OBJNET		= pxe.o udp.o
# add -DDEBUG to add debug strings
# remove -DQUIET to recover the original Grub behavior
DEFINES 	= -DNO_DECOMPRESSION -DSUPPORT_NETBOOT -DSUPPORT_DISKLESS -DFSYS_TFTP -DFSYS_EXT2FS -DFSYS_FAT -DNO_INT13_FALLBACK -DSUPPORT_GRAPHICS=1
HELP		= -DHELP_ON
ARCH		= i386
CFLAGS		= -I. -Os -Wall $(HELP) $(DEFINES) -nostdinc -falign-jumps=1 -falign-loops=1 -falign-functions=1 -march=$(ARCH) -m32
CC			= gcc-3.4

OBJZLIB		= adler32.o inflate.o infblock.o infcodes.o inftrees.o inffast.o infutil.o zfunc.o
CFLAGSZLIB	= -I. -DMAX_WBITS=14 -DMAX_MEM_LEVEL=7 -DMY_ZCALLOC -DNO_ERRNO_H -O9 -march=$(ARCH) -m32


all: revoboot.pxe
	mv revoboot.pxe revoboot.pxe-$(SVNREV)
	ln -sf revoboot.pxe-$(SVNREV) revoboot.pxe

revoboot.pxe:	$(OBJ) $(OBJZLIB) $(OBJNET) adjust_length
	rm -f revoboot.pxe
	$(CC) -o revoboot.pxe.exec -nostdlib -Wl,-N -Wl,-Ttext -Wl,7C00 $(OBJ) $(OBJZLIB) $(OBJNET) -m32
	objcopy -O binary revoboot.pxe.exec revoboot.pxe
	./adjust_length

adjust_length: adjust_length.c
	$(CC) -o adjust_length adjust_length.c

dist-clean: clean

clean:
	rm -r -f $(OBJ) $(OBJZLIB) $(OBJNET) revoboot.pxe revoboot.pxe.* adjust_length

char_io.c: config.h
char_io.o: config.h
zfunc.c: zfunc.h
adler32.c: zlib.h
deflate.c: zlib.h deflate.h
trees.c: zlib.h trees.h
zutil.c: zlib.h zutil.h
builtins.c: builtins_pulse2.h
common.c: builtins_pulse2.h

adler32.o:
	$(CC) $(CFLAGSZLIB) -c -o $@ $*.c

inflate.o:
	$(CC) $(CFLAGSZLIB) -c -o $@ $*.c

inftrees.o:
	$(CC) $(CFLAGSZLIB) -c -o $@ $*.c

infutil.o:
	$(CC) $(CFLAGSZLIB) -c -o $@ $*.c

infblock.o:
	$(CC) $(CFLAGSZLIB) -c -o $@ $*.c

infcodes.o:
	$(CC) $(CFLAGSZLIB) -c -o $@ $*.c

inffast.o:
	$(CC) $(CFLAGSZLIB) -c -o $@ $*.c

asm.o: asm.S
	$(CC) -c -I. -O9 $(DEFINES) -Wall -fno-builtin -nostdinc -o asm.o asm.S -m32

.PHONY: revoboot.pxe
