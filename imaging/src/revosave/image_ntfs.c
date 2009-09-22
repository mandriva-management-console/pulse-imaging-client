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
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <fcntl.h>

#include <sys/stat.h>
#include <sys/ioctl.h>
#include <linux/hdreg.h>
#include <linux/fs.h>


#include "compress.h"

#include "ntfs.h"
#include "client.h"

char info1[32], info2[32];

typedef struct p
{
  ntfsboot boot;
  int ClustSize;
  unsigned long long filelg;
  unsigned long long albitmaplg;                /* allocated length */
}
CPARAMS;

unsigned char fn[100];          /* file name */

unsigned char *
parse_attr (unsigned char *ptr)
{
  return ptr + (*(unsigned long *) (ptr + 4));
}

// error message
char *command_ntfs_error = "NTFS Partition Damaged...\n\nPlease check your disks under Windows before trying another backup";

// common fatal error message
#define FATALERROR(str) debug(str " error line %d\n", __LINE__);exit(1);


void
parse_filename (unsigned char *ptr, PARAMS * p, CPARAMS * cp)
{
  int i;

  ptr += 0x18;

  debug (" Real Length : %lld\t", cp->filelg =
          *(unsigned long long *) (ptr + 0x30));
  debug ("(%08llx)\n", cp->filelg);
  debug (" Allocated lg: %llu\t", p->bitmaplg =
          *(unsigned long long *) (ptr + 0x28));
  debug ("(%08llx)\n", p->bitmaplg);
  p->bitmaplg *= cp->boot.spc;
  p->bitmap = (unsigned char *) calloc (p->bitmaplg, 1);
  //debug ("Memory for bitmap : %lld\n", p->bitmaplg);
  if (p->bitmap == NULL)
    {
      debug ("Calloc error\n");
      exit (1);
    }
  debug (" Flags       : %x\n", *(unsigned short *) (ptr + 0x38));

  debug (" Filename : ");
  for (i = 0; i < ptr[0x40]; i++)
    {
      debug ("%c", ptr[0x42 + 2 * i]);
      fn[i] = ptr[0x42 + 2 * i];
    }
  fn[i] = 0;

  debug ("\n");
}

void
setbit (unsigned char *ptr, unsigned long nb)
{
  unsigned char mask[8] = { 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80 };

  ptr[nb >> 3] |= mask[nb & 7];
}

void
parse_datarun (unsigned char *ptr, FILE * file, PARAMS * p, CPARAMS * cp)
{
  int n1, n2, i, j, k, l;
  unsigned long long loffset, length, rl, al, il;
  signed long long offset;
  unsigned char *curptr = p->bitmap;
  unsigned char *clust;
  unsigned long nb;

  debug ("Verify allocated & real length via DataAttributes\n");

  debug ("Allocated lg: %lld\t", al = *(unsigned long long *) (ptr + 0x28));
  debug ("Real lg: %lld\t", rl = *(unsigned long long *) (ptr + 0x30));
  debug ("Init lg: %lld\n", il = *(unsigned long long *) (ptr + 0x38));

  if (il != rl)
    {
      debug ("Error Real lg<>Init lg ???\n");
      exit (1);
    }
  if (p->bitmaplg != al * cp->boot.spc)
    {
      debug ("!!! FileSize was modified...\n");
      p->bitmaplg = al * cp->boot.spc;
      free (p->bitmap);
      p->bitmap = (unsigned char *) calloc (p->bitmaplg, 1);
      if (p->bitmap == NULL) {
          FATALERROR ("calloc");
        }
      curptr = p->bitmap;
      debug ("Real Length : %lld\n", cp->filelg = rl);

      // should we abort the backup !?
#if 0
      exit(1);
#endif

    }

  debug ("Bitmap size : %llu\n", p->bitmaplg);

  ptr += 0x40;

  loffset = 0;

  // Should loop here for multiple DataRun
  //
  do
    {
      n1 = *ptr;

      debug ("\n-Datarun %02x-", n1);

      n2 = n1 & 15;
      n1 >>= 4;
      ptr++;

      length = 0;
      offset = 0;

      if (n2)
        for (i = 0; i < n2; i++)
          length = 256 * length + ptr[n2 - 1 - i];
      ptr += n2;

      if (n1)
        {
          for (i = 0; i < n1; i++)
            offset = 256 * offset + ptr[n1 - 1 - i];
          if (ptr[n1 - 1] >= 0x80)
            {
              offset = offset - (1L << (8 * n1));
              debug ("Negative relative offset\n");
            }
        }
      ptr += n1;

      offset += loffset;
      loffset = offset;

      debug ("Offset : %lld , lg : %lld\t(%llx)\t", offset, length,
              (unsigned long long) offset * cp->ClustSize);
      fflush(stdout);

      if (fseeko (file, ((unsigned long long)offset)*cp->ClustSize, SEEK_SET)) {
        FATALERROR("datarun seek");
      }

      if (length)
        {
          clust = (unsigned char *) malloc (cp->ClustSize);
          for (i = 0; i < length; i++)
            {
              if (fread (clust, cp->ClustSize, 1, file) != 1) {
                FATALERROR("datarun read");
              }
              nb = 0;
              for (j = 0; j < cp->ClustSize; j++)
                for (k = 1; k < 256; k += k)
                  for (l = 0; l < cp->boot.spc; l++)
                    {
                      if (clust[j] & k) {
                        /* do not set bit if */
                        if ((((curptr - p->bitmap)*8) + nb) < cp->boot.sectors)
                            setbit (curptr, nb);
                        //else
                        //    debug("not set:%d ", ((curptr - p->bitmap)*8) + nb);
                      }
                      nb++;
                    }
              curptr += (cp->ClustSize * cp->boot.spc);
              offset++;
              //debug (".");
              fflush(stdout);
            }
          free (clust);
          debug ("\n");

        }

    }
  while (curptr - p->bitmap < (cp->filelg * cp->boot.spc));


  // End loop
  //
  if (*ptr)
    debug ("Got : %d byte (remaining not taken into account)\n",
            curptr - p->bitmap);

  for (nb = cp->filelg * cp->boot.spc; nb < p->bitmaplg; nb++)
    if (p->bitmap[nb] != 0)
      {
        p->bitmap[nb] = 0;
        debug ("Clearing end of bitmap file (%ld)\n", nb);
      }
  /*  */

  p->bitmaplg = cp->filelg * cp->boot.spc;
}

void
parse_mft (unsigned char *ptr, FILE * file, PARAMS * p, CPARAMS * cp)
{
  unsigned long at_type;
  unsigned char *nptr;
  unsigned char *origptr = ptr;

  if ((ptr[4] == 0x30) && (ptr[5] == 0))
    {
      int off;
      debug ("Oh,oh, UpdateInfo @0x30..., adding %d\t", off =
              2 * (ptr[6] + 256 * ptr[7]));
      off += 3;
      off &= 0xFFF8;
      debug ("(rounded to %d)\n", off);
      ptr += off;
    }

  ptr += 0x30;

  while ((ptr-origptr < 1024) && (at_type = (*(unsigned long *) ptr)) != 0xFFFFFFFF)
    {
      nptr = parse_attr (ptr);
      switch (at_type) {
      case 0x10:
        debug ("Standard Information :\n");
        ptr += 0x18;
        debug(" File perm: %04x\n", *(unsigned long *)(ptr+0x20));
        debug(" Version  : %04x\n", *(unsigned long *)(ptr+0x28));
        break;
      case 0x30:
        debug ("FileName : \n");
        parse_filename (ptr, p, cp);
        break;
      case 0x70:
        ptr += 0x18;
        debug (" Volume Information : v%d.%d flags:%04x\n", *(ptr+8), *(ptr+9), *(unsigned short *)(ptr+10));
        if (*(unsigned short *)(ptr+10) != 0) {
          if (*(unsigned short *)(ptr+10) == 0x4000) {
             debug(" Strange volume flags == 0x4000, so I continue...\n");
          } else {
             debug(" Strange volume flags... Damaged ?\n");
             exit(1);
          }
        }
        break;
      case 0x80:
        if (strcmp (fn, "$Bitmap") == 0) {
          debug ("Data RUN : \n");
          parse_datarun (ptr, file, p, cp);
        }
        break;
      }
      ptr = nptr;
    }
}

void
get_bitmap (FILE * fi, PARAMS * p, CPARAMS * cp)
{
  unsigned long long pos;
  char mft_entry[1024];

  // 6=sixth entry=should be bitmap
  debug ("Searching $Bitmap MFT (Record $6 : offset %lld)\n", pos =
          ((unsigned long long) cp->boot.mft * cp->ClustSize) + 6 * 1024);
  if (pos > 512L * cp->boot.sectors)
    {
      debug ("Error in search position\n");
      exit (1);
    }

  if (fseeko (fi, pos, SEEK_SET)) {
    debug ("Seek ERROR\n");
  };

  fread (mft_entry, 1, 1024, fi);
  parse_mft (mft_entry, fi, p, cp);
  if (strcmp (fn, "$Bitmap") != 0)
    {
      debug ("Error : $Bitmap not found\n");
      exit (1);
    }
}

void
get_volume (FILE * fi, PARAMS * p, CPARAMS * cp)
{
  unsigned long long pos;
  char mft_entry[1024];

  // 6=sixth entry=should be bitmap
  debug ("Searching $Volume MFT (Record $3 : offset %lld)\n", pos =
          ((unsigned long long) cp->boot.mft * cp->ClustSize) + 3 * 1024);
  if (pos > 512L * cp->boot.sectors) {
    FATALERROR("search position");
  }
  if (fseeko (fi, pos, SEEK_SET)) {
    FATALERROR("seek");
  }
  if (fread (mft_entry, 1024, 1, fi) != 1) {
    FATALERROR("read");
  }
  parse_mft (mft_entry, fi, p, cp);
}


/*
 * Compare the 1st four entries of MFT and MFTMirror
 */
void
comp_mft (FILE * fi, PARAMS * p, CPARAMS * cp)
{
  unsigned long long pos, pos2;
  char mft_entry[4096];
  char mft2_entry[4096];
  int i;

  pos = ((unsigned long long) cp->boot.mft * cp->ClustSize);
  pos2 = ((unsigned long long) cp->boot.mft2 * cp->ClustSize);

  if ((pos > 512L * cp->boot.sectors) || (pos2 > 512L * cp->boot.sectors)) {
    FATALERROR("search position");
  }
  if (fseeko (fi, pos, SEEK_SET)) {
    FATALERROR("seek");
  }
  if (fread (mft_entry, 4096, 1, fi) != 1) {
    FATALERROR("read");
  }
  if (fseeko (fi, pos2, SEEK_SET)) {
    FATALERROR("seek");
  }
  if (fread (mft2_entry, 4096, 1, fi) != 1) {
    FATALERROR("read");
  }
  for (i = 0*1024; i < 4096; i++) {
    char c1 = mft_entry[i];
    char c2 = mft2_entry[i];
    if (c1 != c2) {
      debug("MFT != MFT2. Offset %d, val: %c %c\n", i, c1, c2);
      exit(1);
    }
  }
  debug("MFT = MFTMirr\n");
}


void
parse_boot (FILE * fi, PARAMS * p, CPARAMS * cp)
{
  int i;
  unsigned char *ptr;
  __u32 size;

  fread (&cp->boot, 1, 512, fi);

  ptr = (unsigned char *)(&cp->boot);
  if (memcmp(ptr+3, "NTFS    ", 8) != 0)
    {
      debug("Not an NTFS file system partition (bad signature)\n");
      exit(1);
    }

  debug("NTFS signature found\n");
  debug ("BOOT structure : %d\n", sizeof (ntfsboot));
  debug ("Sect/Cluster   : %d (size=%d)\n", cp->boot.spc, cp->ClustSize =
          cp->boot.spc * cp->boot.Bps);
  if (cp->boot.Bps != 512)
    {
      debug ("Oh oh : BPS!=512 \n");
      exit (1);
    }
  i = cp->boot.spc;
  if ((i != 1) && (i != 2) && (i != 4) && (i != 8) && (i != 16) && (i != 32)
      && (i != 64) && (i != 128))
    {
      debug ("Error with boot.spc\n");
      exit (1);
    }

  debug ("Clust/FileSegm.: %ld\n", cp->boot.cps);
  debug ("Clust/IndxBlk  : %ld\n", cp->boot.cpi);
  debug ("Sectors        : %lld\n", cp->boot.sectors);
  debug ("MFT            : %lld\n", cp->boot.mft);
  debug ("MFT 2          : %lld\n", cp->boot.mft2);

  /* check the device size */
  if (ioctl(fileno(fi), BLKGETSIZE, &size) < 0) {
    /* it's a file not a block dev */
    debug("Regular file not a block device.\n");
  } else {
    debug("Block device: %d sectors\n",size);
    if (cp->boot.sectors > size)
      {
        debug ("Filesystem size > Block dev size: probably part of a RAID volume.\nERROR: Cannot backup NTFS RAID volumes.\n");
        exit (1);
      }
  }

  if (cp->boot.mft > cp->boot.sectors)
    {
      debug ("Error in integrity of boot.mft\n");
      exit (1);
    }
  if (cp->boot.mft2 > cp->boot.sectors)
    {
      debug ("Error in integrity of boot.mft2\n");
      exit (1);
    }
  if (cp->boot.mft > cp->boot.mft2)
    {
      debug ("Oh oh : MFT > MFT2... ?!?!?\n");
      /* exit (1); */
    }

  if (cp->boot.cps < 0x80)
    {
      if (cp->boot.cps * cp->ClustSize != 1024)
        {
          debug ("Oh oh : MFT size != 1024 !!!!\n");
          exit (1);
        }
    }
  else
    {
      if (cp->boot.cps != 0xF6)
        {
          debug ("Oh oh : MFT size (neg) != 0xF6 !!!!\n");
          exit (1);
        }
    }
}

void
count_used (PARAMS * p, CPARAMS *cp)
{
  unsigned long used = 0;
  int bits[16] = { 0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4 };
  unsigned long i, lgend;

  debug ("Counting...");
  lgend = cp->boot.sectors >> 3;

  for (i = 0; i < lgend; i++)
    used += (bits[p->bitmap[i] >> 4] + bits[p->bitmap[i] & 15]);

  debug ("Used : %ld (total : %lld)\n", used, cp->boot.sectors);

  sprintf(info1, "%lld", cp->boot.sectors);
  sprintf(info2, "%ld", used);
  print_sect_info(cp->boot.sectors, used);
}


int
main (int argc, char *argv[])
{
  FILE *fi;
  PARAMS p;
  CPARAMS cp;
  int fd;
  char tmp[255];

  if (argc != 3)
    {
      fprintf (stderr, "Usage : image_ntfs [device] [image prefix name]\n");
      exit (1);
    }

  debug("image_ntfs: Copyright (C) 2000-2005 Linbox Free&ALter Soft\n");

  fi = fopen (argv[1], "rb");
  parse_boot (fi, &p, &cp);
  get_volume (fi, &p, &cp);
  get_bitmap (fi, &p, &cp);
  count_used (&p, &cp);

  // Compare the 2 MFTs
  comp_mft(fi, &p, &cp);
  if (argv[2][0] == '?') {
    exit (0);
  }
  fclose (fi);

  ui_send("init_backup", 5, argv[1], argv[2], info1, info2, argv[0]);
  fd = open (argv[1], O_RDONLY);
  compress_volume (fd, argv[2], &p, "NTFS=1");
  close (fd);

  return 0;
}
