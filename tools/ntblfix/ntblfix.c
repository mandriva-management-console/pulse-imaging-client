/*
 * (c) 2003-2007 Linbox FAS, http://linbox.com
 * (c) 2008-2009 Mandriva, http://www.mandriva.com
 *
 * $Id$
 *
 * This file is part of Pulse 2, http://pulse2.mandriva.org
 *
 * Pulse 2 is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * Pulse 2 is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Pulse 2; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 *
 * NTBootLoaderFIX. Change the disk geometry hardcoded in the BL after a restoration on a
 * different hard disk. Uses vm86.c which comes from kdrive.
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/io.h>
#include <sys/kd.h>
#include <sys/stat.h>
#include <asm/types.h>

#include "vm86.h"

/*
 * Print a message and exit
 */

void die(const char *format_str, ...)
{
    va_list ap;

    fprintf(stderr, "ntblfix: ");
    va_start(ap, format_str);
    vfprintf(stderr, format_str, ap);
    va_end(ap);
    exit(1);
}

/*
 *  Main program
 */

int main(int argc, char *argv[])
{
    Vm86InfoPtr vi;

    int head, cyl, sect, status, fo, drive;
    unsigned short buffer[16];

    printf("ntblfix $Rev$. Copyright (C) 2006 Linbox FAS\n"
           "Vm86 library Copyright (C) 2000 Keith Packard and Juliusz Chroboczek\n");

    if (argc != 3) {
        die("usage: ntblfix device drivenumber\n"
            "\tdrivenumber start at 1 = (BIOS drive 0x80)\n");
    }
    drive = atoi(argv[2]) + 0x80 - 1;

    vi = Vm86Setup(1);
    if (!vi) {
        die("Cannot run Vm86Setup\n");
    }


    vi->vms.regs.eax = 0x0800;
    vi->vms.regs.edx = drive;

    //memcpy(vbe.info->vbe_signature, "VBE2", 4);

    if (Vm86DoInterrupt(vi, 0x13)) {
        die("Can't get CHS info (Vm86DoInterrupt failure)\n");
    }

    status = vi->vms.regs.eax;
    head = (vi->vms.regs.edx >> 8) + 1;
    sect = vi->vms.regs.ecx & 63;
    cyl = ((vi->vms.regs.ecx >> 8) & 255) + ((vi->vms.regs.ecx & 192) << 2) + 2;

    if (status) {
        die("Bad int13 exit status: 0x%X\n", status);
    }
    printf("Drive 0x%X BIOS geometry: %d/%d/%d\n", drive, cyl, head, sect);
    Vm86Cleanup(vi);

    fo = open(argv[1], O_RDWR | O_LARGEFILE);
    if (fo == -1) {
        die("cannot open device\n");
    }
    /* seek to the 63rd sector */
    if (lseek64(fo, (__u64) 512 * 63, SEEK_SET) == -1) {
        die("seek error\n");
    }
    if (read(fo, buffer, 32) == 0) {
        die("read error\n");
    }
    // removed the test buffer[0] != 0x5BEB
    if (buffer[1] != 0x4E90 || buffer[2] != 0x4654
        || buffer[3] != 0x2053) {
        die("This disk does not contain a WinNT boot loader\n");
    }
    printf("Bootloader: offset %d, heads %d, sectors %d\n", buffer[14],
           buffer[13], buffer[12]);
    if (buffer[14] != 63 || buffer[13] > 255 || buffer[12] > 63) {
        die("Strange numbers. Will not update\n");
    }
    if (buffer[13] != head) {
        printf("Updating heads number\n");
        if (head > 255) head=255;
        buffer[13] = head;
        if (lseek64(fo, (__u64) 512 * 63, SEEK_SET) == -1) {
            die("seek error\n");
        }
        if (write(fo, buffer, 32) == 0) {
            die("write error\n");
        }
    }
    close(fo);

    return 0;
}
