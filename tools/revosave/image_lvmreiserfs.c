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

#include <sys/mount.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <asm/types.h>
#include <errno.h>
#include <stdio.h>
#include <mntent.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/vfs.h>

#include "./reiserfsprogs/include/io.h"
#include "./reiserfsprogs/include/misc.h"
#include "./reiserfsprogs/include/reiserfs_lib.h"
#include "./reiserfsprogs/version.h"

//#include <linux/lvm.h>

#include "compress.h"
#include "client.h"
#include "lvm.h"

typedef struct p {
    reiserfs_bitmap_t bm;
    unsigned long blocks;
    unsigned long offset;       /* offset to real FS in bytes (LVM overhead) */
} CPARAMS;

char info1[32], info2[32];
unsigned long lvm_sect = 0;

/* BEGIN CODE TAKE FROM reiserfsprogs 3.x */

/* fixme: this assumes that journal start and journal size are set
   correctly */
static void check_first_bitmap(reiserfs_filsys_t fs, char *bitmap)
{
    int i;
    int bad;

    bad = 0;
    for (i = 0; i < rs_journal_start(fs->s_rs) +
         rs_journal_size(fs->s_rs) + 1; i++) {
        if (!test_bit(i, bitmap)) {
            bad = 1;
            /*reiserfs_warning ("block %d is marked free in the first bitmap, fixed\n", i); */
            /*set_bit (i, bitmap); */
        }
    }
    if (bad)
        reiserfs_warning(stderr,
                         "reiserfs_open: first bitmap looks corrupted\n");
}


/* read bitmap blocks */
void myreiserfs_read_bitmap_blocks(reiserfs_filsys_t fs, long offset)
{
    struct reiserfs_super_block *rs = fs->s_rs;
    struct buffer_head *bh = SB_BUFFER_WITH_SB(fs);
    int fd = fs->s_dev;
    unsigned long block;
    int i;
    /* LUDO: the 1st block has already the offset */
    bh->b_blocknr -= offset / 4096;
    /* read bitmaps, and correct a bit if necessary */
    SB_AP_BITMAP(fs) = getmem(sizeof(void *) * rs_bmap_nr(rs));
    for (i = 0, block = bh->b_blocknr + 1; i < rs_bmap_nr(rs); i++) {
        SB_AP_BITMAP(fs)[i] =
            bread(fd, block + (offset / 4096), fs->s_blocksize);
        if (!SB_AP_BITMAP(fs)[i]) {
            reiserfs_warning(stderr,
                             "reiserfs_open: bread failed reading bitmap #%d (%lu)\n",
                             i, block);
            SB_AP_BITMAP(fs)[i] = getblk(fd, block, fs->s_blocksize);
            memset(SB_AP_BITMAP(fs)[i]->b_data, 0xff, fs->s_blocksize);
            set_bit(BH_Uptodate, &SB_AP_BITMAP(fs)[i]->b_state);
        }

        /* all bitmaps have to have itself marked used on it */
        if (bh->b_blocknr == 16) {
            if (!test_bit
                (block % (fs->s_blocksize * 8),
                 SB_AP_BITMAP(fs)[i]->b_data)) {
                reiserfs_warning(stderr,
                                 "reiserfs_open: bitmap %d was marked free\n",
                                 i);
                /*set_bit (block % (fs->s_blocksize * 8), SB_AP_BITMAP (fs)[i]->b_data); */
            }
        } else {
            /* bitmap not spread over partition: fixme: does not
               work when number of bitmaps => 32768 */
            if (!test_bit(block, SB_AP_BITMAP(fs)[0]->b_data)) {
                reiserfs_warning(stderr,
                                 "reiserfs_open: bitmap %d was marked free\n",
                                 i);
                /*set_bit (block, SB_AP_BITMAP (fs)[0]->b_data); */
            }
        }

        if (i == 0) {
            /* first bitmap has to have marked used super block
               and journal areas */
            check_first_bitmap(fs, SB_AP_BITMAP(fs)[i]->b_data);
        }
        /* LUDO: remove the offset */
        block =
            ((bh->b_blocknr) ==
             16 ? ((i + 1) * fs->s_blocksize * 8) : (block + 1));

    }
}

/* read super block and bitmaps. fixme: only 4k blocks, pre-journaled format
   is refused */
reiserfs_filsys_t myreiserfs_open(char *filename, int flags, int *error,
                                  void *vp, long offset)
{
    reiserfs_filsys_t fs;
    struct buffer_head *bh;
    struct reiserfs_super_block *rs;
    int fd, i;

    fd = open(filename, flags | O_LARGEFILE);
    if (fd == -1) {
        if (error)
            *error = errno;
        return 0;
    }

    fs = getmem(sizeof(*fs));
    fs->s_dev = fd;
    fs->s_vp = vp;
    asprintf(&fs->file_name, "%s", filename);

    /* reiserfs super block is either in 16-th or in 2-nd 4k block of the
       device */
    for (i = 16; i > 0; i -= 14) {
        bh = bread(fd, i + (offset / 4096), 4096);
        if (!bh) {
            reiserfs_warning(stderr,
                             "reiserfs_open: bread failed reading block %d\n",
                             i);
        } else {
            rs = (struct reiserfs_super_block *) bh->b_data;

            if (is_reiser2fs_magic_string(rs)
                || is_reiserfs_magic_string(rs))
                goto found;

            /* reiserfs signature is not found at the i-th 4k block */
            brelse(bh);
        }
    }

    reiserfs_warning(stderr,
                     "reiserfs_open: neither new nor old reiserfs format "
                     "found on %s\n", filename);
    if (error)
        *error = 666;
    return fs;

  found:

    /* fixme: we could make some check to make sure that super block looks
       correctly */
    fs->s_version = is_reiser2fs_magic_string(rs) ? REISERFS_VERSION_2 :
        REISERFS_VERSION_1;
    fs->s_blocksize = rs_blocksize(rs);
    fs->s_hash_function = code2func(rs_hash(rs));
    SB_BUFFER_WITH_SB(fs) = bh;
    fs->s_rs = rs;
    fs->s_flags = flags;        /* O_RDONLY or O_RDWR */
    fs->s_vp = vp;


    myreiserfs_read_bitmap_blocks(fs, offset);

    return fs;

}



/* copy reiserfs filesystem bitmap into memory bitmap */
int myreiserfs_fetch_disk_bitmap(reiserfs_bitmap_t bm,
                                 reiserfs_filsys_t fs)
{
    int i;
    int bytes;
    char *p;
    int unused_bits;

    //reiserfs_warning (stderr, "Fetching on-disk bitmap..");
    assert(bm->bm_bit_size == SB_BLOCK_COUNT(fs));

    bytes = fs->s_blocksize;
    p = bm->bm_map;
    for (i = 0; i < SB_BMAP_NR(fs); i++) {
        if ((i == (SB_BMAP_NR(fs) - 1))
            && bm->bm_byte_size % fs->s_blocksize)
            bytes = bm->bm_byte_size % fs->s_blocksize;

        memcpy(p, SB_AP_BITMAP(fs)[i]->b_data, bytes);
        p += bytes;
    }

    /* on disk bitmap has bits out of SB_BLOCK_COUNT set to 1, where as
       reiserfs_bitmap_t has those bits set to 0 */
    unused_bits = bm->bm_byte_size * 8 - bm->bm_bit_size;
    for (i = 0; i < unused_bits; i++)
        clear_bit(bm->bm_bit_size + i, bm->bm_map);

    bm->bm_set_bits = 0;
    /* FIXME: optimize that */
    for (i = 0; i < bm->bm_bit_size; i++)
        if (reiserfs_bitmap_test_bit(bm, i))
            bm->bm_set_bits++;
    /* unused part of last bitmap block is filled with 0s */
    if (bm->bm_bit_size % (fs->s_blocksize * 8))
        for (i = SB_BLOCK_COUNT(fs) % (fs->s_blocksize * 8);
             i < fs->s_blocksize * 8; i++)
            if (!test_bit(i, SB_AP_BITMAP(fs)[SB_BMAP_NR(fs) - 1]->b_data)) {
                reiserfs_warning(stderr,
                                 "fetch_bitmap: on-disk bitmap is not padded properly\n");
                break;
            }
    //reiserfs_warning (stderr, "done\n");
    return 0;
}


void allocated_sectors(PARAMS * p, CPARAMS * cp)
{
    unsigned long i, used = 0;
    unsigned long bitmap_lg;
    int off = 0;

    void setbit(unsigned char *base, unsigned long bit) {
        unsigned char mask[8] =
            { 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80 };

        base[bit >> 3] |= mask[bit & 7];
    }

    // TODO : check if block is really 4096 byte long...
    // ...it seems to be
    p->nb_sect = cp->blocks * 8;
    off = cp->offset / 512;

    p->bitmap = (unsigned char *) calloc(bitmap_lg =
                                         (p->nb_sect + off + 7) / 8, 1);
    p->bitmaplg = bitmap_lg;

    // backup LVM: everything
    for (i = 0; i < off; i++)
        setbit(p->bitmap, i);
    // backup Reiserfs
    for (i = 0; i < p->nb_sect; i++)
        if (reiserfs_bitmap_test_bit(cp->bm, i / 8)) {
            setbit(p->bitmap, i + off);
            used++;
        }

    sprintf(info1, "%lu", p->nb_sect + off);
    sprintf(info2, "%lu", used + off);
    print_sect_info(p->nb_sect + off, used + off);

    if (p->nb_sect != lvm_sect && lvm_sect != 0) {
      debug("Cannot backup this LVM/Reiserfs volume in optimized mode\n"
            "because data may span multiple volumes.\n");
      exit(1);

    }
    p->nb_sect += off;

}


int main(int argc, char *argv[])
{
    reiserfs_bitmap_t bm;
    reiserfs_filsys_t fs;
    PARAMS params;
    CPARAMS cp;
    int err = 0;
    long long offset;
    int fd;

    if (argc != 3) {
        fprintf(stderr,
                "Usage : image_reiserfs [device] [image prefix name]\n");
        exit(1);
    }
    // check for LVM
    lvm_check(argv[1], &offset);

    fs = myreiserfs_open(argv[1], O_RDONLY, &err, 0, offset);
    if ((!(fs)) || err) {
        debug("Reiserfs_open failed\n");
        exit(1);
    }
    if (fs->s_blocksize != 4096) {
        debug("Bad blocksize (!= 4096)\n");
        exit(1);
    }
    debug("Bitmap  : %d bits\n", SB_BLOCK_COUNT(fs));
    bm = reiserfs_create_bitmap(SB_BLOCK_COUNT(fs));
    myreiserfs_fetch_disk_bitmap(bm, fs);
    cp.bm = bm;
    cp.offset = offset;
    cp.blocks = rs_block_count(fs->s_rs);
    assert(fs->s_blocksize == 4096);

    /* for (i=0;i<SB_BLOCK_COUNT(fs)/8;i++) {
       debug("%02x", bm->bm_map[i]);
       }
       debug("\n"); */

    allocated_sectors(&params, &cp);

    reiserfs_close(fs);

    if (argv[2][0] == '?')
        exit(0);

    // Compress now
    //

    ui_send("init_backup", 5, argv[1], argv[2], info1, info2, argv[0]);
    fd = open(argv[1], O_RDONLY);
    compress_volume(fd, argv[2], &params, "RFS3");
    close(fd);

    return 0;
}
