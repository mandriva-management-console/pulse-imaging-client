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
/*  0 */ unsigned char      jump[3];
/*  3 */ unsigned char      name[8];
/*  B */ unsigned short     Bps;
/*  D */ unsigned char      spc;
/*  E */ unsigned short     reserved;
/* 10 */ unsigned char      fats;
//Not used NTFS
/* 11 */ unsigned short     entries;
//Not used NTFS
/* 13 */ unsigned short     sect_in_vol;
//Not used NTFS
/* 15 */ unsigned char      descriptor;
/* 16 */ unsigned short     sect_in_fat;
//Not used NTFS
/* 18 */ unsigned short     spt;
/* 1A */ unsigned short     heads;
/* 1C */ unsigned long      offset;
/* 20 */ unsigned long      sect_in_vol2;
//Not Used NTFS (offset 0x20)
/* 24 */ unsigned long      nu;
//Not Used NTFS (offset 0x24)
/* 28 */ unsigned long long sectors;
/* 30 */ unsigned long long mft;
/* 38 */ unsigned long long mft2;
/* 40 */ unsigned long      cps;
/* 44 */ unsigned long      cpi;
/* 48 */ unsigned long long serial;
/* 50 */ unsigned long      chksum;
/* 54 */ unsigned char      code[426];
/* 1FE*/ unsigned short     id;
} ntfsboot;
