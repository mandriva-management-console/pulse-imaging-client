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

typedef struct __attribute__ ((packed)) {
    unsigned char BS_DrvNum;
    unsigned char BS_Reserved1;
    unsigned char BS_BootSig;

    unsigned long BS_VolID;
    unsigned char BS_VolLab[11];
    unsigned char BS_FilSysType[8];

    unsigned char BootCode[510 - 62];

    unsigned short BS_Signature;
} FAT16;

typedef struct __attribute__ ((packed)) {
    unsigned long BPB_FATSz32;
    unsigned short BPB_ExtFlags;
    unsigned short BPB_FSVer;
    unsigned long BPB_RootClus;
    unsigned short BPB_FSInfo;
    unsigned short BPB_BkBootSec;
    unsigned char BPB_Reserved[12];

    unsigned char BS_DrvNum;
    unsigned char BS_Reserved1;
    unsigned char BS_BootSig;

    unsigned long BS_VolID;
    unsigned char BS_VolLab[11];
    unsigned char BS_FilSysType[8];

    unsigned char BootCode[510 - 90];

    unsigned short BS_Signature;
} FAT32;

typedef struct __attribute__ ((packed)) {
    unsigned char BS_jmpBoot[3];
    unsigned char BS_OEMName[8];

    unsigned short BPB_BytsPerSec;
    unsigned char BPB_SecPerClus;
    unsigned short BPB_RsvdSecCnt;
    unsigned char BPB_NumFATs;
    unsigned short BPB_RootEntCnt;
    unsigned short BPB_TotSec16;
    unsigned char BPB_Media;
    unsigned short BPB_FATSz16;
    unsigned short BPB_SecPerTrk;
    unsigned short BPB_NumHeads;
    unsigned long BPB_HiddSec;
    unsigned long BPB_TotSec32;

    union __attribute__ ((packed)) {
        FAT16 fat16;
        FAT32 fat32;
    } fat_type;

} FAT;
