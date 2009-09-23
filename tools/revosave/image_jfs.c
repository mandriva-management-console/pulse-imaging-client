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
#include <linux/types.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include "compress.h"
#include "client.h"

char info1[32], info2[32];

void checkit(char *dev)
{
  int fd, bs;
  __u8 buf[512];

  fd = open (dev, O_RDONLY);
  if (fd == -1) exit(1);
  lseek(fd, 0x8000, SEEK_SET);
  if (read(fd, buf, 256) != 256) exit(1);

  if (strncmp (buf, "JFS1", 4) == 0)
    {
      debug("JFS1 magic found\n");
      bs = (buf[7]<<24) + (buf[6]<<16) + (buf[5]<<8) + (buf[4]);
      debug("version: %d\n", bs);
    }
  else
    {
      debug("magic not found\n");
      exit(1);
    }

  close (fd);
}

void
allocated_sectors (PARAMS * p, char *dev)
{
  int i, fd, bytes;
  int size;

  if ((fd = open (dev, O_RDONLY)) == -1) exit(1);

  if (ioctl(fd, BLKGETSIZE, &size) < 0) {
    /* it's a file not a block dev */
    struct stat st;

    fstat(fd, &st);
    size = (st.st_size/512);
    debug("Regular file: %d sectors\n",size);
  } else {
    debug("Block device: %d sectors\n",size);
  }
  close(fd);
  if (size == 0) exit(1);

  bytes = (size+7)/8;
  p->bitmap = (unsigned char *) calloc (1, bytes);
  p->bitmaplg = bytes;

  p->nb_sect = size;

  for (i = 0; i < bytes-1; i++)
    p->bitmap[i] = 0xFF;
  /* mark remaining sectors */
  if (size & 7) {
    p->bitmap[i] = (0xFF >> (8-(size & 7))) & 0xFF;
  }

  sprintf(info1, "%u", size);
  sprintf(info2, "%u", size);
  print_sect_info(size, size);
}

/*  */
int
main (int argc, char *argv[])
{
  int fd;
  PARAMS params;

  if (argc != 3)
    {
      fprintf (stderr, "Usage : image_jfs [device] [image prefix name]\n");
      exit (1);
    }

  checkit(argv[1]);

  if (argv[2][0] == '?')
      exit (0);

  allocated_sectors(&params, argv[1]);

  ui_send("init_backup", 5, argv[1], argv[2], info1, info2, argv[0]);
  fd = open (argv[1], O_RDONLY);
  compress_volume (fd, argv[2], &params, "");
  close (fd);

  return 0;
}
