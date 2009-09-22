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

/*
 * image_e2fs derived from :
 *
 * dumpe2fs.c       - List the control structures of a second
 *            extended filesystem
 *
 * Copyright (C) 1992, 1993, 1994  Remy Card <card@masi.ibp.fr>
 *                                 Laboratoire MASI, Institut Blaise Pascal
 *                                 Universite Pierre et Marie Curie (Paris VI)
 *
 * Copyright 1995, 1996, 1997 by Theodore Ts'o.
 *
 * %Begin-Header%
 * This file may be redistributed under the terms of the GNU Public
 * License.
 * %End-Header%
 */

/*
 * History:
 * 94/01/09 - Creation
 * 94/02/27 - Ported to use the ext2fs library
 */

#include "config.h"

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "ext2fs/ext2_fs.h"
#include "ext2fs/ext2fs.h"
#include "e2p/e2p.h"

#include "compress.h"
#include "client.h"

#define in_use(m, x)    (ext2fs_test_bit ((x), (m)))

#define _(a) a

char info1[32], info2[32];

static void
setbit (unsigned char *p, int num)
{
  unsigned char mask[8] = { 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80 };
  p[num >> 3] |= mask[num & 7];
}

static void
list_desc (ext2_filsys fs, PARAMS * p)
{
  unsigned long i, j, k, ptr;
  blk_t group_blk, next_blk;
  char *block_bitmap = fs->block_map->bitmap;
  char *inode_bitmap = fs->inode_map->bitmap;
  int inode_blocks_per_group;
  int group_desc_blocks;
  int nbsect;
  unsigned long used = 0;

  inode_blocks_per_group = ((fs->super->s_inodes_per_group *
                 EXT2_INODE_SIZE (fs->super)) +
                EXT2_BLOCK_SIZE (fs->super) - 1) /
    EXT2_BLOCK_SIZE (fs->super);
  group_desc_blocks = ((fs->super->s_blocks_count -
            fs->super->s_first_data_block +
            EXT2_BLOCKS_PER_GROUP (fs->super) - 1) /
               EXT2_BLOCKS_PER_GROUP (fs->super) +
               EXT2_DESC_PER_BLOCK (fs->super) - 1) /
    EXT2_DESC_PER_BLOCK (fs->super);

  nbsect = fs->blocksize / 512;
  p->bitmaplg = (fs->super->s_blocks_count * nbsect + 7) / 8;
  p->bitmap = (unsigned char *) calloc (p->bitmaplg, 1);

  ptr = 0;
  for (i = 0; i < fs->super->s_first_data_block; i++)
    for (j = 0; j < nbsect; j++)
      setbit (p->bitmap, ptr++);

  group_blk = fs->super->s_first_data_block;
  for (i = 0; i < fs->group_desc_count; i++)
    {
      next_blk = group_blk + fs->super->s_blocks_per_group;
      if (next_blk > fs->super->s_blocks_count)
    next_blk = fs->super->s_blocks_count;
      /*debug ( _("Group %lu: (Blocks %u -- %u)\t"), i,
         group_blk, next_blk -1 );

         if (ext2fs_bg_has_super (fs, i))
         debug ( _("  %s Superblock at %u,"
         "  Group Descriptors at %u-%u\n"),
         i == 0 ? _("Primary") : _("Backup"),
         group_blk, group_blk + 1,
         group_blk + group_desc_blocks);
         debug ( _("  Block bitmap at %u (+%d), "
         "Inode bitmap at %u (+%d)\n  "
         "Inode table at %u-%u (+%d)\n"),
         fs->group_desc[i].bg_block_bitmap,
         fs->group_desc[i].bg_block_bitmap - group_blk,
         fs->group_desc[i].bg_inode_bitmap,
         fs->group_desc[i].bg_inode_bitmap - group_blk,
         fs->group_desc[i].bg_inode_table,
         fs->group_desc[i].bg_inode_table +
         inode_blocks_per_group - 1,
         fs->group_desc[i].bg_inode_table - group_blk);

         debug (_("  %d free blocks, %d free inodes, %d directories\n"),
         fs->group_desc[i].bg_free_blocks_count,
         fs->group_desc[i].bg_free_inodes_count,
         fs->group_desc[i].bg_used_dirs_count);
       */
      for (j = 0; j < next_blk - group_blk; j++)
    {
      if (in_use (block_bitmap, j))
        for (k = 0; k < nbsect; k++)
          {
        used++;
        setbit (p->bitmap, ptr + k);
          }
      ptr += nbsect;
    }

      block_bitmap += fs->super->s_blocks_per_group / 8;
      inode_bitmap += fs->super->s_inodes_per_group / 8;

      group_blk = next_blk;
    }

  sprintf(info1, "%lu", ((long) fs->super->s_blocks_count * nbsect));
  sprintf(info2, "%lu", used);
  print_sect_info(((long) fs->super->s_blocks_count * nbsect), used);

}


/*  */
int
main (int argc, char **argv)
{
  errcode_t retval;
  ext2_filsys fs;
  char *device_name;
  PARAMS p;
  int big_endian;
  int fd;

  if (argc != 3)
    {
      fprintf (stderr, "Usage : image_e2fs [device] [image prefix name]\n");
      exit (1);
    }

  initialize_ext2_error_table ();

  device_name = argv[1];

  retval = ext2fs_open (device_name, 0, 0, 0, unix_io_manager, &fs);

  if (retval)
    {
      debug (_("Couldn't find valid filesystem superblock.\n"));
      exit (1);
    }

  big_endian = ((fs->flags & EXT2_FLAG_SWAP_BYTES) != 0);

  if (big_endian)
    debug (_("Note: This is a byte-swapped filesystem\n"));
  //list_super (fs->super);

  retval = ext2fs_read_bitmaps (fs);
  if (retval)
    {
      debug (_("Couldn't read bitmaps.\n"));
      ext2fs_close (fs);
      exit (1);
    }

  list_desc (fs, &p);
  ext2fs_close (fs);

  if (argv[2][0] == '?')
    exit (0);

  ui_send("init_backup", 5, argv[1], argv[2], info1, info2, argv[0]);
  fd = open (argv[1], O_RDONLY);
  p.nb_sect = 0;
  compress_volume (fd, argv[2], &p, "E2FS=1|TYPE=131");
  close (fd);

  exit (0);
}
