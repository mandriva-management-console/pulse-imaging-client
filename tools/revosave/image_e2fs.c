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

#include <ext2fs/ext2fs.h>

#include "compress.h"
#include "client.h"
#include <assert.h>

#define in_use(m, x)    (ext2fs_test_bit ((x), (m)))

#define _(a) a

char info1[32], info2[32];

static void setbit(unsigned char *p,
                   unsigned long long num, unsigned long limit) {

    static const unsigned char mask[8] =
        { 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80 };

    assert(num < ((unsigned long long)limit * 8));

    p[num >> 3] |= mask[num & 7];
}

static void list_desc(ext2_filsys fs, PARAMS * p) {

    unsigned long blocks_per_group;
    unsigned long blocks_bitmap_len;

    char *blocks_bitmap;

    unsigned long blocks_size;
    unsigned long sect_per_block;

    unsigned long long blocks_count;

    /* current sector */
    unsigned long long ptr;
    /* total number of used sectors */
    unsigned long long used;

    unsigned long i, k;
    blk_t j;

    blk_t blk_itr;

    blk_t first_block;
    blk_t last_block;

    errcode_t ret;

    assert(fs != NULL);
    assert(p != NULL);

    blocks_size = EXT2_BLOCK_SIZE(fs->super);
    assert(blocks_size != 0);
    assert(blocks_size % 512 == 0);

    blocks_per_group = EXT2_BLOCKS_PER_GROUP(fs->super);
    assert(blocks_per_group != 0);
    assert(blocks_per_group % 8 == 0);

    blocks_bitmap_len = blocks_per_group / 8;
    assert(blocks_bitmap_len > 0);

    /* bitmap of allocated blocks in a group */
    blocks_bitmap = malloc(blocks_bitmap_len);
    assert(blocks_bitmap != NULL);

    sect_per_block = blocks_size / 512;
    assert(sect_per_block > 0);

    /* number of block in the filesystem
       (counting reserved blocks) */
#ifdef HAVE_EXT2FS_BLOCK_COUNT
    blocks_count = ext2fs_blocks_count(fs);
#else
    /* Without ext2fs_blocks_count(),
       64bit filesystem are not supported */
    blocks_count = fs->super->s_blocks_count;
#endif
    p->bitmaplg = ((blocks_count * sect_per_block) + 7) / 8;
    assert(p->bitmaplg > 0);

    p->bitmap = (unsigned char *)calloc(p->bitmaplg, 1);
    assert(p->bitmap != NULL);

    ptr = 0;
    used = 0;

    /* add the first reserverd blocks (aka block 0 and next) */
    for (j = 0; j < fs->super->s_first_data_block; j++)
        for (k = 0; k < sect_per_block; k++)
            setbit(p->bitmap, ptr++, p->bitmaplg);

    blk_itr = fs->super->s_first_data_block;
    for (i = 0; i < fs->group_desc_count; i++) {

        /* get bitmap data of allocated blocks in current group */
        ret = ext2fs_get_block_bitmap_range(fs->block_map,
                                            blk_itr, blocks_bitmap_len * 8,
                                            blocks_bitmap);
        assert(ret == 0);

        /* get absolute block index for this group
         * first block will be 1
         * last block could be 8192 for the first group,
         *  if 8912 block per group
         */
        first_block = ext2fs_group_first_block(fs, i);
        last_block = ext2fs_group_last_block(fs, i);

        assert(blk_itr <= first_block);
        assert(blk_itr <= last_block);
        assert((1 + last_block - first_block) <= blocks_per_group);

        /* set them relative to this */
        first_block -= blk_itr;
        last_block -= blk_itr;

        /* up to first block, it's unused */
        ptr += sect_per_block * first_block;

        /* parse blocks bitmap, from first to last (included) */
        for (j = first_block; j <= last_block; j++) {
            if (in_use(blocks_bitmap, j)) {
                for (k = 0; k < sect_per_block; k++) {
                    used++;
                    setbit(p->bitmap, ptr + k, p->bitmaplg);
                }
            }
            ptr += sect_per_block;
        }

        /* round up: past last block, there's no data in the group */
        ptr += sect_per_block * (blocks_per_group - (1 + last_block));

        blk_itr += blocks_per_group;
    }

    sprintf(info1, "%llu", blocks_count * sect_per_block);
    sprintf(info2, "%llu", used);

    print_sect_info(blocks_count * sect_per_block, used);
}

/*  */
int main(int argc, char **argv) {
    errcode_t retval;
    ext2_filsys fs = NULL;
    char *device_name;
    PARAMS p;
    int big_endian;
    int fd;

    if (argc != 3) {
        fprintf(stderr, "Usage : image_e2fs [device] [image prefix name]\n");
        exit(1);
    }

    initialize_ext2_error_table();

    device_name = argv[1];

    retval = ext2fs_open(device_name, 0, 0, 0, unix_io_manager, &fs);

    if (retval) {
        debug(_("Couldn't find valid filesystem superblock.\n"));
        exit(1);
    }

    big_endian = ((fs->flags & EXT2_FLAG_SWAP_BYTES) != 0);

    if (big_endian)
        debug(_("Note: This is a byte-swapped filesystem\n"));
    //list_super (fs->super);

    retval = ext2fs_read_bitmaps(fs);
    if (retval) {
        debug(_("Couldn't read bitmaps.\n"));
        ext2fs_close(fs);
        exit(1);
    }

    memset(&p, 0, sizeof(PARAMS));

    list_desc(fs, &p);
    ext2fs_close(fs);

    if (argv[2][0] == '?')
        exit(0);

    ui_send("init_backup", 5, argv[1], argv[2], info1, info2, argv[0]);
    fd = open(argv[1], O_RDONLY);
    p.nb_sect = 0;
    compress_volume(fd, (unsigned char *)argv[2], &p, "E2FS=1|TYPE=131");
    close(fd);

    exit(0);
}
