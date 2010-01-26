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
 */

typedef struct fat16_s {
    unsigned char BS_DrvNum __attribute__ ((packed));
    unsigned char BS_Reserved1 __attribute__ ((packed));
    unsigned char BS_BootSig __attribute__ ((packed));

    unsigned long BS_VolID __attribute__ ((packed));
    unsigned char BS_VolLab[11] __attribute__ ((packed));
    unsigned char BS_FilSysType[8] __attribute__ ((packed));

    unsigned char BootCode[510 - 62] __attribute__ ((packed));

    unsigned short BS_Signature __attribute__ ((packed));
} FAT16;

typedef struct fat32_s {
    unsigned long BPB_FATSz32 __attribute__ ((packed));
    unsigned short BPB_ExtFlags __attribute__ ((packed));
    unsigned short BPB_FSVer __attribute__ ((packed));
    unsigned long BPB_RootClus __attribute__ ((packed));
    unsigned short BPB_FSInfo __attribute__ ((packed));
    unsigned short BPB_BkBootSec __attribute__ ((packed));
    unsigned char BPB_Reserved[12] __attribute__ ((packed));

    unsigned char BS_DrvNum __attribute__ ((packed));
    unsigned char BS_Reserved1 __attribute__ ((packed));
    unsigned char BS_BootSig __attribute__ ((packed));

    unsigned long BS_VolID __attribute__ ((packed));
    unsigned char BS_VolLab[11] __attribute__ ((packed));
    unsigned char BS_FilSysType[8] __attribute__ ((packed));

    unsigned char BootCode[510 - 90] __attribute__ ((packed));

    unsigned short BS_Signature __attribute__ ((packed));
} FAT32;

typedef struct fat_s {
    unsigned char BS_jmpBoot[3] __attribute__ ((packed));
    unsigned char BS_OEMName[8] __attribute__ ((packed));

    unsigned short BPB_BytsPerSec __attribute__ ((packed));
    unsigned char BPB_SecPerClus __attribute__ ((packed));
    unsigned short BPB_RsvdSecCnt __attribute__ ((packed));
    unsigned char BPB_NumFATs __attribute__ ((packed));
    unsigned short BPB_RootEntCnt __attribute__ ((packed));
    unsigned short BPB_TotSec16 __attribute__ ((packed));
    unsigned char BPB_Media __attribute__ ((packed));
    unsigned short BPB_FATSz16 __attribute__ ((packed));
    unsigned short BPB_SecPerTrk __attribute__ ((packed));
    unsigned short BPB_NumHeads __attribute__ ((packed));
    unsigned long BPB_HiddSec __attribute__ ((packed));
    unsigned long BPB_TotSec32 __attribute__ ((packed));

    union {
        FAT16 fat16;
        FAT32 fat32;
    } fat_type;

} FAT;
