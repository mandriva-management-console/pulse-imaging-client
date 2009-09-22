#
# $Id: Makefile,v 1.7 2003/06/25 12:09:00 ludo Exp $
#

VER=$(shell grep VERSION config.h|tr -dc [0-9])

OBJ=asm.o bios.o boot.o pci.o \
    char_io.o builtins.o \
    common.o stage2.o \
    cmdline.o disk_io.o graphics.o console.o \
    fsys_fat.o fsys_ext2fs.o builtins_lbs.o

OBJNET= pxe.o udp.o

OBJZLIB=adler32.o inflate.o infblock.o infcodes.o inftrees.o inffast.o infutil.o zfunc.o

DEFINES = -DNO_DECOMPRESSION -DSUPPORT_NETBOOT -DSUPPORT_DISKLESS -DFSYS_TFTP -DFSYS_EXT2FS -DFSYS_FAT -DNO_INT13_FALLBACK -DSUPPORT_GRAPHICS=1
#-DDEBUG

HELP=-DHELP_ON
#HELP=

ARCH=pentium
ARCH=i386

CFLAGS=-I. -Os -Wall $(HELP) $(DEFINES) -nostdinc -malign-jumps=1 -malign-loops=1 -malign-functions=1 -mcpu=$(ARCH) -march=$(ARCH) -m32

###################################

CC=gcc-3.4

###################################

CFLAGSZLIB=-I. -DMAX_WBITS=14 -DMAX_MEM_LEVEL=7 -DMY_ZCALLOC -DNO_ERRNO_H -O9 -mcpu=$(ARCH) -march=$(ARCH) -m32

revoboot.pxe:	$(OBJ) $(OBJZLIB) $(OBJNET) adjust_length
	$(CC) -o revoboot.pxe.exec -nostdlib -Wl,-N -Wl,-Ttext -Wl,7C00 $(OBJ) $(OBJZLIB) $(OBJNET) -m32
	objcopy -O binary revoboot.pxe.exec revoboot.pxe
	./adjust_length
	[ ! -f revoboot.pxe.$(VER) ] && cp revoboot.pxe revoboot.pxe.$(VER)

adjust_length: adjust_length.c
	$(CC) -o adjust_length adjust_length.c

clean:
	rm -r -f $(OBJ) $(OBJZLIB) $(OBJNET) revoboot.pxe revoboot.pxe.exec adjust_length revoboot.pxe.$(VER)

char_io.c: config.h
char_io.o: config.h
zfunc.c: zfunc.h
adler32.c: zlib.h
deflate.c: zlib.h deflate.h
trees.c: zlib.h trees.h
zutil.c: zlib.h zutil.h
builtins.c: builtins_lbs.h
common.c: builtins_lbs.h

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

