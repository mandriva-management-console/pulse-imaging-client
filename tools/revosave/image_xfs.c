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
#include <netinet/in.h>

#include "compress.h"
#include "client.h"
#include "image_xfs.h"

char info1[32], info2[32];

struct xfs_superblock sb;

/* proto */
void scanSbtree(int fd, PARAMS * p, struct xfs_agf *agf, __u32 root,
                __u32 levels);
void scanfuncBno(int fd, PARAMS * p, char *ablock, __u32 level,
                 struct xfs_agf *agf);

/*  */
inline __u64 ntohll(__u64 number) {
    return (htonl((number >> 32) & 0xFFFFFFFF) |
            ((__u64) (htonl(number & 0xFFFFFFFF)) << 32));
}

/* check for XFS */
void checkit(char *dev) {
    int fd, bs;
    __u8 buf[512];

    fd = open(dev, O_RDONLY);
    if (fd == -1)
        exit(1);
    if (read(fd, buf, 256) != 256)
        exit(1);

    if (strncmp(buf, "XFSB", 4) == 0) {
        debug("XFSB magic found\n");
        bs = (buf[4] << 24) + (buf[5] << 16) + (buf[6] << 8) + (buf[7]);
        debug("Block size : %d\n", bs);
        if (bs != 4096) {
            debug("Strange block size %d ! Only 4096 supported. \n", bs);
            exit(1);
        }
    } else {
        debug("XFSB magic not found\n");
        exit(1);
    }
    close(fd);
}

int readdata(int fd, void *buf, __u64 offset, __u32 size) {
    if (lseek64(fd, offset, SEEK_SET) == -1) {
        perror("lseek64 readdata()");
    }

    read(fd, buf, size);

    return 0;
}

/* AG to offset conversion */
__u64 convertAgbToDaddr(__u32 agno, __u32 Agblockno) {
    __u64 FSBlock;

    FSBlock =
        (((__u64) agno) * ((__u64) ntohl(sb.sb_agblocks))) +
        ((__u64) Agblockno);
    return (FSBlock * (__u64) 4096);
}

__u64 convertAgToDaddr(__u32 agno, __u32 Offset) {
    return (convertAgbToDaddr(agno, 0) + ((__u64) Offset) * ((__u64) BBSIZE));
}

void addtohist(PARAMS * p, __u32 agno, __u32 Agblockno, __u64 Len) {
    __u64 i;
    __u64 Base;

    Base =
        (((__u64) agno) * ((__u64) ntohl(sb.sb_agblocks))) +
        ((__u64) Agblockno);
    // debug("%lld , %lld\n", Base, Len);
    for (i = 0; i < Len; i++) {
        /* unset 8 bits since one block = 8 sectors */
        p->bitmap[Base + i] = 0;
    }
}

void scanFreelist(int fd, PARAMS * p, struct xfs_agf *agf) {
    __u32 agno = ntohl(agf->agf_seqno);
    struct xfs_agfl agfl;
    __u32 blockno, i;

    if (ntohl(agf->agf_flcount) == 0)
        return;

    readdata(fd, &agfl, convertAgToDaddr(agno, XFS_AGFL_DADDR), sizeof(agfl));

    // process list
    i = ntohl(agf->agf_flfirst);
    for (;;) {
        blockno = ntohl(agfl.agfl_bno[i]);
        addtohist(p, agno, blockno, 1);
        if (i == ntohl(agf->agf_fllast))
            break;
        if (++i == XFS_AGFL_SIZE)
            i = 0;
    }
}

void scanSbtree(int fd, PARAMS * p, struct xfs_agf *agf, __u32 root,
                __u32 levels) {
    char buf[MAX_XFSBLOCKSIZE];

    readdata(fd, buf, convertAgbToDaddr(ntohl(agf->agf_seqno), root), 4096);
    scanfuncBno(fd, p, buf, levels - 1, agf);
}

void scanfuncBno(int fd, PARAMS * p, char *ablock, __u32 level,
                 struct xfs_agf *agf) {
    xfs_alloc_block *block = (xfs_alloc_block *) ablock;
    xfs_alloc_ptr *pp;
    xfs_alloc_rec *rp;
    int i, mxr;

    if (level == 0) {
        rp = (xfs_alloc_rec *) (ablock + sizeof(xfs_alloc_block));

        for (i = 0; i < ntohs(block->bb_numrecs); i++) {
            addtohist(p, ntohl(agf->agf_seqno), ntohl(rp[i].ar_startblock),
                      ntohl(rp[i].ar_blockcount));
        }
        return;
    }

    mxr =
        (int)((4096 -
               (uint) sizeof(xfs_alloc_block)) / ((uint) sizeof(xfs_alloc_key) +
                                                  (uint)
                                                  sizeof(xfs_alloc_ptr)));
    pp = (xfs_alloc_ptr *) ((char *)(ablock) + sizeof(xfs_alloc_block) +
                            (mxr * sizeof(xfs_alloc_key)));
    for (i = 0; i < ntohs(block->bb_numrecs); i++) {
        scanSbtree(fd, p, agf, ntohl(pp[i]), level);
    }
}

/*  */
void readsb(PARAMS * p, char *dev) {
    int fd;
    __u32 agcount, agblocks, i;
    __u64 blocks, fblocks, used = 0;
    int bits[16] = { 0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4 };

    fd = open(dev, O_RDONLY);
    if (fd == -1)
        exit(1);
    if (read(fd, &sb, sizeof(xfs_superblock)) != sizeof(xfs_superblock))
        exit(1);

    blocks = ntohll(sb.sb_dblocks);
    fblocks = ntohll(sb.sb_fdblocks);
    agcount = ntohl(sb.sb_agcount);
    agblocks = ntohl(sb.sb_agblocks);

    debug("Total Blocks  : %lld\n", blocks);
    debug("Used Blocks   : %lld\n", blocks - fblocks);
    debug("Alloc groups  : %d\n", agcount);
    debug("Alloc blocks  : %d\n", agblocks);
    debug("Volume name   : %12s\n", sb.sb_fname);

    /* alloc bitmap */
    /* 1 block = 8 sectors */
    p->bitmap = (unsigned char *)calloc(1, blocks);
    p->bitmaplg = blocks;
    /* fill bitmap */
    memset(p->bitmap, 255, blocks);
    p->nb_sect = blocks * 8;

    /* rewind */
    lseek(fd, 0, SEEK_SET);

    /* AG loop */
    for (i = 0; i < agcount; i++) {
        struct xfs_agf agf;

        if (lseek64(fd, convertAgToDaddr(i, XFS_AGF_DADDR), SEEK_SET) == -1) {
            perror("=lseek64");
        }

        read(fd, &agf, sizeof(struct xfs_agf));

        if ((ntohl(agf.agf_magicnum)) != XFS_AGF_MAGIC) {
            debug("No magic found in AG %d\n", i);
            exit(1);
        }

        debug("Alloc %d: %u free\n", i, ntohl(agf.agf_freeblks));

        scanFreelist(fd, p, &agf);
        scanSbtree(fd, p, &agf, ntohl(agf.agf_roots[XFS_BTNUM_BNO]),
                   ntohl(agf.agf_levels[XFS_BTNUM_BNO]));

    }

    /* count used (verification) */
    for (i = 0; i < p->bitmaplg; i++)
        used += (bits[p->bitmap[i] >> 4] + bits[p->bitmap[i] & 15]);

    debug("Used sectors : %lld (total : %lld)\n", used, p->nb_sect);

    if (used == (blocks - fblocks) * 8) {
        debug("Verified bitmap count OK\n");
    } else {
        debug("Superblock used block count and bitmap count do not match !\n");
        debug("%lld != %lld\n", used, (blocks - fblocks) * 8);
        if (used > (blocks - fblocks) * 8) {
            debug("Since my count is greater, let's continue !\n");
        } else
            exit(1);
    }

    sprintf(info1, "%llu", p->nb_sect);
    sprintf(info2, "%llu", used);
    print_sect_info(p->nb_sect, used);
}

int main(int argc, char *argv[]) {
    int fd;
    PARAMS params;

    if (argc != 3) {
        fprintf(stderr, "Usage : image_xfs [device] [image prefix name]\n");
        exit(1);
    }

    checkit(argv[1]);
    readsb(&params, argv[1]);

    if (argv[2][0] == '?')
        exit(0);

    ui_send("init_backup", 5, argv[1], argv[2], info1, info2, argv[0]);
    fd = open(argv[1], O_RDONLY);
    compress_volume(fd, argv[2], &params, "XFS");
    close(fd);

    return 0;
}
