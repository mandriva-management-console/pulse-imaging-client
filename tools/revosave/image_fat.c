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

#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "fat.h"
#include "compress.h"
#include "client.h"

#define FAT32_ROOT_OK 0x01
#define FAT32_SEC32   0x02
#define FAT32_RSVD    0x04
#define FAT32_SIZE    0x08

#define FAT32_TEST_OK 0x0F

typedef struct p
{
  FAT bootsect;

  unsigned char *fat;
  unsigned char fat32;

  unsigned short RootDirSectors;
  unsigned long FAT_size;
  unsigned long FirstDataSector;
}
CPARAMS;

char info1[32], info2[32];

void
check_bootsect (FILE * f, PARAMS * p, CPARAMS *cp )
{
  char *str, dum[512];
  long i;

  fread (&(cp->bootsect), 512, 1, f);

  debug ("BPB Information :\n");
  cp->fat32 = 0;

  debug ("- Media        : %x\n", cp->bootsect.BPB_Media);

  debug ("- Byte/Sect.   : %d\n", cp->bootsect.BPB_BytsPerSec);
  if (cp->bootsect.BPB_BytsPerSec != 512)
    {
      debug ("Error in BpS\n");
      exit (1);
    }

  debug ("- Sect./Clust. : %d\n", cp->bootsect.BPB_SecPerClus);
  if (cp->bootsect.BPB_SecPerClus == 0)
    {
      debug ("Error in SecPerClus==0\n");
      exit (1);
    }

  debug ("- Sect/Track   : %d\n", cp->bootsect.BPB_SecPerTrk);
  debug ("- Heads        : %d\n", cp->bootsect.BPB_NumHeads);

  debug ("- Hidden Sec.  : %ld\n", cp->bootsect.BPB_HiddSec);
  debug ("- Reserved Sec.: %d\n", cp->bootsect.BPB_RsvdSecCnt);
  if (cp->bootsect.BPB_RsvdSecCnt == 32)
    cp->fat32 |= FAT32_RSVD;
  if (cp->bootsect.BPB_RsvdSecCnt == 0)
    {
      debug ("Strange : RsvdSecCnt==0\n");
      exit (1);
    }

  if (cp->bootsect.BPB_TotSec16)
    {
      p->nb_sect = cp->bootsect.BPB_TotSec16;
      str = "16";
    }
  else
    {
      p->nb_sect = cp->bootsect.BPB_TotSec32;
      str = "32";
      cp->fat32 |= FAT32_SEC32;
    }
  debug ("- Nb Sect.(%s) : %llu\n", str, p->nb_sect);

  debug ("- Num. FATs    : %d\n", cp->bootsect.BPB_NumFATs);
  if (cp->bootsect.BPB_NumFATs != 2)
    {
      debug ("Strange : FATS!=2\n");
      exit (1);
    }

  debug ("- Root Ent Cnt : %d\n", cp->bootsect.BPB_RootEntCnt);
  if (!cp->bootsect.BPB_RootEntCnt)
    cp->fat32 |= FAT32_ROOT_OK;

  if (cp->bootsect.BPB_FATSz16)
    {
      cp->FAT_size = cp->bootsect.BPB_FATSz16;
    }
  else
    {
      cp->FAT_size = cp->bootsect.fat_type.fat32.BPB_FATSz32;
      cp->fat32 |= FAT32_SIZE;
    }
  debug ("- FAT Size     : %ld\n", cp->FAT_size);

  debug ("- FAT 32 test  : %x\t-> ", cp->fat32);
  if (cp->fat32 == FAT32_TEST_OK)
    debug ("FAT 32\n");
  else if (cp->fat32)
    debug ("Test 1 failed : continue with care!!!\n");
  else
    debug ("FAT12/16\n");

  cp->RootDirSectors =
    ((cp->bootsect.BPB_RootEntCnt * 32) +
     (cp->bootsect.BPB_BytsPerSec - 1)) / cp->bootsect.BPB_BytsPerSec;
  cp->FirstDataSector =
    cp->bootsect.BPB_RsvdSecCnt + (cp->bootsect.BPB_NumFATs * cp->FAT_size) +
    cp->RootDirSectors;

  debug ("- OEM Name     : %c%c%c%c%c%c%c%c\n",
     cp->bootsect.BS_OEMName[0], cp->bootsect.BS_OEMName[1],
     cp->bootsect.BS_OEMName[2], cp->bootsect.BS_OEMName[3],
     cp->bootsect.BS_OEMName[4], cp->bootsect.BS_OEMName[5],
     cp->bootsect.BS_OEMName[6], cp->bootsect.BS_OEMName[7]);

  debug ("Calculated Data :\n");
  debug ("- Root Dir Sectors  : %d\n", cp->RootDirSectors);
  debug ("- First Data Sector : %ld\n", cp->FirstDataSector);

  debug
    ("- Nb of Clusters    : %ld (x<4085=FAT12,x<65525=FAT16, else FAT32)\n",
     i = ((p->nb_sect - cp->FirstDataSector) / cp->bootsect.BPB_SecPerClus));

  if (i < 4085)
    {
      debug ("Cannot handle FAT12 type, exiting...\n");
      exit (1);
    }
  if (i < 65525)
    {
      cp->fat32 = 0;
      debug ("FAT 16, now reading FAT\n");
    }
  else
    {
      cp->fat32 = 1;
      debug ("FAT 32, now reading FAT\n");
    }

  if (!cp->fat32)
    {
      debug ("- BS_DrvNum   : %x\n", cp->bootsect.fat_type.fat16.BS_DrvNum);
      debug ("- BS_BootSig  : %x\n", cp->bootsect.fat_type.fat16.BS_BootSig);
      debug ("- BS_VolID    : %lx\n", cp->bootsect.fat_type.fat16.BS_VolID);
      debug ("- BS_VolLab   : ");
      for (i = 0; i < 11; i++)
    debug ("%c", cp->bootsect.fat_type.fat16.BS_VolLab[i]);
      debug ("\n");
      debug ("- BS_FilSysType : ");
      for (i = 0; i < 8; i++)
    debug ("%c", cp->bootsect.fat_type.fat16.BS_FilSysType[i]);
      debug ("\n");
    }
  else
    {
      debug ("+ BPB_ExtFlags  : %x\n",
         cp->bootsect.fat_type.fat32.BPB_ExtFlags);
      debug ("+ BPB_FSVer     : %d\n", cp->bootsect.fat_type.fat32.BPB_FSVer);
      debug ("+ BPB_RootClus  : %ld\n",
         cp->bootsect.fat_type.fat32.BPB_RootClus);
      debug ("+ BPB_FSInfo    : %d\n", cp->bootsect.fat_type.fat32.BPB_FSInfo);
      debug ("+ BPB_BkBootSec : %d\n",
         cp->bootsect.fat_type.fat32.BPB_BkBootSec);

      debug ("- BS_DrvNum   : %x\n", cp->bootsect.fat_type.fat32.BS_DrvNum);
      debug ("- BS_BootSig  : %x\n", cp->bootsect.fat_type.fat32.BS_BootSig);
      debug ("- BS_VolID    : %lx\n", cp->bootsect.fat_type.fat32.BS_VolID);
      debug ("- BS_VolLab   : ");
      for (i = 0; i < 11; i++)
    debug ("%c", cp->bootsect.fat_type.fat32.BS_VolLab[i]);
      debug ("\n");
      debug ("- BS_FilSysType : ");
      for (i = 0; i < 8; i++)
    debug ("%c", cp->bootsect.fat_type.fat32.BS_FilSysType[i]);
      debug ("\n");
    }

  cp->fat = (unsigned char *) malloc (cp->FAT_size * 512L);
  if (cp->fat == NULL)
    {
      debug("ERROR: cannot allocate %ld bytes\n", cp->FAT_size * 512L);
      exit(1);
    }

  // Read (RsvdSecCnt - 1) Sectors (Boot already READ !!!)
  //
  for (i = 1; i < cp->bootsect.BPB_RsvdSecCnt; i++)
    fread (dum, 512, 1, f);

  // Read FAT
  //
  fread (cp->fat, 512, cp->FAT_size, f);
}

void
allocated_sectors (PARAMS * p, CPARAMS * cp)
{
  unsigned long i, used = 0;
  unsigned long bitmap_lg;
  int shift = 0;

  void setbit (unsigned char *base, unsigned long bit)
  {
    unsigned char mask[8] =
      { 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80 };

    base[bit >> 3] |= mask[bit & 7];
  }

  int checkFAT (unsigned long i)
  {
    unsigned long clustnb, ent;

    clustnb = 2 + ((i - cp->FirstDataSector) >> shift);

    if (cp->fat32)
      ent = *(((unsigned long *) cp->fat) + clustnb);   //&0x0FFFFFFF;
    else
      ent = *(((unsigned short *) cp->fat) + clustnb);

    if (ent)
      return 1;
    return 0;
  }

  if (cp->bootsect.BPB_SecPerClus == 1)
    shift = 0;
  else if (cp->bootsect.BPB_SecPerClus == 2)
    shift = 1;
  else if (cp->bootsect.BPB_SecPerClus == 4)
    shift = 2;
  else if (cp->bootsect.BPB_SecPerClus == 8)
    shift = 3;
  else if (cp->bootsect.BPB_SecPerClus == 16)
    shift = 4;
  else if (cp->bootsect.BPB_SecPerClus == 32)
    shift = 5;
  else if (cp->bootsect.BPB_SecPerClus == 64)
    shift = 6;
  else if (cp->bootsect.BPB_SecPerClus == 128)
    shift = 7;
  else
    {
      debug ("Problem with BPB_SecPerClus = %d\n",
         cp->bootsect.BPB_SecPerClus);
      exit (1);
    }

  p->bitmap = (unsigned char *) calloc (bitmap_lg = (p->nb_sect + 7) / 8, 1);
  p->bitmaplg = bitmap_lg;

  // Fisrt, consider that from 0 to FirstDataSect, everything is Allocated
  //

  for (i = 0; i < cp->FirstDataSector; i++)
    {
      setbit (p->bitmap, i);
      used++;
    }

  // Then, for each sector, check in the FAT entry
  //

  for (i = cp->FirstDataSector; i < p->nb_sect; i++)
    if (checkFAT (i))
      {
    setbit (p->bitmap, i);
    used++;
      }

  // Debug...
  //
#if 0
  for (i = 0; i < bitmap_lg; i++)
    debug ("%02x%c", p->bitmap[i], ((i & 15) == 15) ? '\n' : ' ');
#endif

  sprintf(info1, "%llu", p->nb_sect);
  sprintf(info2, "%lu", used);
  print_sect_info(p->nb_sect, used);

}


int
main (int argc, char *argv[])
{
  FILE *fi;
  PARAMS params;
  CPARAMS cp;
  int fd;

  // Just some verifications : sanity check
  //
  assert (sizeof (FAT) == 512);

  if (argc != 3)
    {
      fprintf (stderr, "Usage : image_fat [device] [image prefix name]\n");
      exit (1);
    }


  debug("image_fat: Copyright (C) 2000-2005 Linbox Free&ALter Soft\n");

  // OK : analyse BPB & FAT type
  //      then analyse FAT itself
  fi = fopen (argv[1], "rb");
  check_bootsect (fi, &params, &cp);
  allocated_sectors (&params, &cp);
  fclose (fi);

  if (argv[2][0] == '?')
    exit (0);

  // Compress now
  //

  ui_send("init_backup", 5, argv[1], argv[2], info1, info2, argv[0]);
  fd = open (argv[1], O_RDONLY);
  compress_volume (fd, argv[2], &params, cp.fat32 ? "FAT32=1" : "FAT32=0");
  close (fd);

  return 0;
}
