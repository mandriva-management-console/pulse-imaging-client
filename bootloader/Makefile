#
# (c) 2009-2011 Mandriva, http://www.mandriva.com
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
include $(topdir)/common.mk

CFLAGS = -march=$(ARCHITECTURE) -m32
CPPFLAGS = -I.

revoboot_SRCS = bios.c boot.c pci.c char_io.c builtins.c common.c stage2.c \
	cmdline.c disk_io.c graphics.c console.c fsys_fat.c fsys_ext2fs.c \
	builtins_pulse2.c pxe.c udp.c
revoboot_OBJS = $(patsubst %.c,%.o,$(revoboot_SRCS))
# add -DDEBUG to add debug strings
# remove -DQUIET to recover the original Grub behavior
revoboot_DEFINES = -DHELP_ON -DNO_DECOMPRESSION -DSUPPORT_NETBOOT -DSUPPORT_DISKLESS -DFSYS_TFTP -DFSYS_EXT2FS -DFSYS_FAT -DNO_INT13_FALLBACK -DSUPPORT_GRAPHICS=1 -DQUIET 
revoboot_CFLAGS = -O2 -Wall $(revoboot_DEFINES) -falign-jumps -falign-loops -falign-functions $(CFLAGS)
revoboot_CPPFLAGS = $(CPPFLAGS) -nostdinc

zlib_SRCS = adler32.c inflate.c infblock.c infcodes.c inftrees.c inffast.c infutil.c \
	zfunc.c
zlib_OBJS = $(patsubst %.c,%.o,$(zlib_SRCS))
zlib_CFLAGS = -DMY_ZCALLOC -DNO_ERRNO_H -DMAX_WBITS=15 -DMAX_MEM_LEVEL=8 -O9 $(CFLAGS)
zlib_CPPFLAGS = $(CPPFLAGS)

CLEANFILES = $(zlib_OBJS) $(revoboot_OBJS) \
	revoboot.pxe revoboot.pxe.exec adjust_length asm.o

all-local: revoboot.pxe

install-local:
	mkdir -p $(DESTDIR)$(bootloaderdir)
	$(install_DATA) revoboot.pxe $(DESTDIR)$(bootloaderdir)/pxe_boot

revoboot.pxe.exec: adjust_length asm.o $(revoboot_OBJS) $(zlib_OBJS)
	$(CC) $(CFLAGS) -o $@ -nostdlib -Wl,-N -Wl,-Ttext -Wl,7C00 -Wl,--build-id=none \
	  asm.o $(revoboot_OBJS) $(zlib_OBJS)

revoboot.pxe: revoboot.pxe.exec
	objcopy -O binary $< $@
	./adjust_length

adjust_length: adjust_length.c
	$(CC) -o $@ $<

zfunc.c: zfunc.h

adler32.c: zlib.h

asm.o: asm.S
	$(CC) $(revoboot_CPPFLAGS) $(CFLAGS) -O9 -Wall $(revoboot_DEFINES) -fno-builtin -c -o $@ $^

$(revoboot_OBJS): %.o: %.c
	$(CC) $(revoboot_CFLAGS) $(revoboot_CPPFLAGS) -c -o $@ $<

$(zlib_OBJS): %.o: %.c
	$(CC) $(zlib_CFLAGS) $(zlib_CPPFLAGS) -c -o $@ $<
