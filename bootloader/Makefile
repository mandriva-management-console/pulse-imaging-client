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

OBJ 		= asm.o bios.o boot.o pci.o char_io.o builtins.o common.o stage2.o cmdline.o disk_io.o graphics.o console.o fsys_fat.o fsys_ext2fs.o builtins_pulse2.o pxe.o udp.o
# add -DDEBUG to add debug strings
# remove -DQUIET to recover the original Grub behavior
DEFINES 	= -DHELP_ON -DNO_DECOMPRESSION -DSUPPORT_NETBOOT -DSUPPORT_DISKLESS -DFSYS_TFTP -DFSYS_EXT2FS -DFSYS_FAT -DNO_INT13_FALLBACK -DSUPPORT_GRAPHICS=1 -DQUIET
ARCH		= i386
# don't f****g know why, but with -Os (legacy) instalead of -O2 I got a really strange behavior
# note : -Os perfectly works under gcc 3.3.x and earlier
# TODO : FIND OUT WHY !
CFLAGS		= -I. -O2 -Wall $(DEFINES) -nostdinc -falign-jumps -falign-loops -falign-functions -march=$(ARCH) -m32
CC			= gcc-3.4

OBJZLIB		= adler32.o inflate.o infblock.o infcodes.o inftrees.o inffast.o infutil.o zfunc.o

all: revoboot.pxe
	mv revoboot.pxe revoboot.pxe-$(SVNREV)
	ln -sf revoboot.pxe-$(SVNREV) revoboot.pxe

revoboot.pxe: $(OBJ) adjust_length
	rm -f revoboot.pxe
	$(MAKE) -f Makefile.zlib CC=$(CC)
	$(CC) -o revoboot.pxe.exec -nostdlib -Wl,-N -Wl,-Ttext -Wl,7C00 $(OBJ) $(OBJZLIB) -m32
	objcopy -O binary revoboot.pxe.exec revoboot.pxe
	./adjust_length

adjust_length: adjust_length.c
	$(CC) -o adjust_length adjust_length.c

asm.o: asm.S
	$(CC) -I. -O9 -Wall $(DEFINES) -nostdinc -fno-builtin -c -o $@ $^ -m32

dist-clean: clean

clean:
	$(MAKE) -f Makefile.zlib clean
	rm -r -f $(OBJ) revoboot.pxe revoboot.pxe.* adjust_length

.PHONY: revoboot.pxe
