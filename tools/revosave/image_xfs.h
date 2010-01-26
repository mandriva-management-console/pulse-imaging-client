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
 * XFS structures taken from Linux kernel XFS includes
 */

#define MAX_XFSBLOCKSIZE 65536

#define XFS_SUPER_MAGIC 0x58465342

typedef enum {
    XFS_BTNUM_BNOi, XFS_BTNUM_CNTi, XFS_BTNUM_BMAPi, XFS_BTNUM_INOi,
    XFS_BTNUM_MAX
} xfs_btnum_t;

#define BBSIZE 512

#define XFS_BTNUM_BNO   ((xfs_btnum_t)XFS_BTNUM_BNOi)

#define XFS_SB_MAGIC        0x58465342  // 'XFSB'

typedef enum { B_FALSE, B_TRUE } boolean_t;

typedef __u32 xfs_agblock_t;    // blockno in alloc. group
typedef __u32 xfs_extlen_t;     // extent length in blocks
typedef __u32 xfs_agnumber_t;   // allocation group number
typedef __s32 xfs_extnum_t;     // # of extents in a file
typedef __s16 xfs_aextnum_t;    // # extents in an attribute fork
typedef __s64 xfs_fsize_t;      // bytes in a file
typedef __u64 xfs_ufsize_t;     // unsigned bytes in a file

typedef __s32 xfs_suminfo_t;    // type of bitmap summary info
typedef __s32 xfs_rtword_t;     // word type for bitmap manipulations

typedef __s64 xfs_lsn_t;        // log sequence number
typedef __s32 xfs_tid_t;        // transaction identifier

typedef __u32 xfs_dablk_t;      // dir/attr block number (in file)
typedef __u32 xfs_dahash_t;     // dir/attr hash value

typedef __u16 xfs_prid_t;       // prid_t truncated to 16bits in XFS

typedef struct {
    unsigned char __u_bits[16];
} uuid_t;

// These types are 64 bits on disk but are either 32 or 64 bits in memory.
// Disk based types:
typedef __u64 xfs_dfsbno_t;     // blockno in filesystem (agno|agbno)
typedef __u64 xfs_drfsbno_t;    // blockno in filesystem (raw)
typedef __u64 xfs_drtbno_t;     // extent (block) in realtime area
typedef __u64 xfs_dfiloff_t;    // block number in a file
typedef __u64 xfs_dfilblks_t;   // number of blocks in a file

typedef __u64 xfs_off_t;
typedef __u64 xfs_ino_t;        // <inode> type
typedef __s64 xfs_daddr_t;      // <disk address> type
typedef char *xfs_caddr_t;      // <core address> type
typedef off_t linux_off_t;
typedef __u32 xfs_dev_t;

#define XFS_AGF_DADDR           (1)
#define XFS_AGFL_DADDR      (3)

#define XFS_AGFL_SIZE       (BBSIZE / sizeof(xfs_agblock_t))

struct xfs_agfl {
    xfs_agblock_t agfl_bno[XFS_AGFL_SIZE];
};

struct xfs_superblock {
    __u32 sb_magicnum;          // magic number == XFS_SB_MAGIC
    __u32 sb_blocksize;         // logical block size, bytes
    xfs_drfsbno_t sb_dblocks;   // number of data blocks
    xfs_drfsbno_t sb_rblocks;   // number of realtime blocks
    xfs_drtbno_t sb_rextents;   // number of realtime extents
    uuid_t sb_uuid;             // file system unique id
    xfs_dfsbno_t sb_logstart;   // starting block of log if internal
    xfs_ino_t sb_rootino;       // root inode number
    xfs_ino_t sb_rbmino;        // bitmap inode for realtime extents

    xfs_ino_t sb_rsumino;       // summary inode for rt bitmap

    xfs_agblock_t sb_rextsize;  // realtime extent size, blocks

    xfs_agblock_t sb_agblocks;  // size of an allocation group
    xfs_agnumber_t sb_agcount;  // number of allocation groups
    xfs_extlen_t sb_rbmblocks;  // number of rt bitmap blocks

    xfs_extlen_t sb_logblocks;  // number of log blocks
    __u16 sb_versionnum;        // header version == XFS_SB_VERSION
    __u16 sb_sectsize;          // volume sector size, bytes
    __u16 sb_inodesize;         // inode size, bytes
    __u16 sb_inopblock;         // inodes per block
    char sb_fname[12];          // file system name
    __u8 sb_blocklog;           // log2 of sb_blocksize
    __u8 sb_sectlog;            // log2 of sb_sectsize
    __u8 sb_inodelog;           // log2 of sb_inodesize
    __u8 sb_inopblog;           // log2 of sb_inopblock
    __u8 sb_agblklog;           // log2 of sb_agblocks (rounded up)
    __u8 sb_rextslog;           // log2 of sb_rextents
    __u8 sb_inprogress;         // mkfs is in progress, don't mount
    __u8 sb_imax_pct;           // max % of fs for inode space
    // statistics
    // These fields must remain contiguous.  If you really
    // want to change their layout, make sure you fix the
    // code in xfs_trans_apply_sb_deltas().
    __u64 sb_icount;            // allocated inodes
    __u64 sb_ifree;             // free inodes
    __u64 sb_fdblocks;          // free data blocks
    __u64 sb_frextents;         // free realtime extents

    // End contiguous fields.
    xfs_ino_t sb_uquotino;      // user quota inode
    xfs_ino_t sb_gquotino;      // group quota inode
    __u16 sb_qflags;            // quota flags
    __u8 sb_flags;              // misc. flags
    __u8 sb_shared_vn;          // shared version number
    xfs_extlen_t sb_inoalignmt; // inode chunk alignment, fsblocks
    __u32 sb_unit;              // stripe or raid unit
    __u32 sb_width;             // stripe or raid width
    __u8 sb_dirblklog;          // log2 of dir block size (fsbs)
    __u8 sb_dummy[7];           // padding
} xfs_superblock;

#define NULLAGBLOCK ((xfs_agblock_t)-1)

struct xfs_btree_sblock {
    __u32 bb_magic;             // magic number for block type
    __u16 bb_level;             // 0 is a leaf
    __u16 bb_numrecs;           // current # of data records
    xfs_agblock_t bb_leftsib;   // left sibling block or NULLAGBLOCK
    xfs_agblock_t bb_rightsib;  // right sibling block or NULLAGBLOCK
};

typedef struct xfs_btree_sblock xfs_alloc_block;

typedef struct xfs_btree_hdr {
    __u32 bb_magic;             // magic number for block type
    __u16 bb_level;             // 0 is a leaf
    __u16 bb_numrecs;           // current # of data records
} xfs_btree_hdr_t;

typedef struct xfs_btree_block {
    xfs_btree_hdr_t bb_h;       // header
    union {
        struct {
            xfs_agblock_t bb_leftsib;
            xfs_agblock_t bb_rightsib;
        } s;                    // short form pointers
        struct {
            xfs_dfsbno_t bb_leftsib;
            xfs_dfsbno_t bb_rightsib;
        } l;                    // long form pointers
    } bb_u;                     // rest
} xfs_btree_block_t;

typedef xfs_agblock_t xfs_alloc_ptr;    // btree pointer type

struct xfs_alloc_rec_ {
    xfs_agblock_t ar_startblock;        // starting block number
    xfs_extlen_t ar_blockcount; // count of free blocks
};                              // xfs_alloc_rec_t, xfs_alloc_key_t;

typedef struct xfs_alloc_rec_ xfs_alloc_rec;

typedef struct xfs_alloc_rec_ xfs_alloc_key;

#define XFS_AGF_MAGIC   0x58414746
#define XFS_AGI_MAGIC   0x58414749

#define XFS_BTNUM_AGF   ((int)XFS_BTNUM_CNTi + 1)

struct xfs_agf {
    // Common allocation group header information
    __u32 agf_magicnum;         // magic number == XFS_AGF_MAGIC
    __u32 agf_versionnum;       // header version == XFS_AGF_VERSION
    xfs_agnumber_t agf_seqno;   // sequence # starting from 0
    xfs_agblock_t agf_length;   // size in blocks of a.g.

    // Freespace information
    xfs_agblock_t agf_roots[XFS_BTNUM_AGF];     // root blocks
    __u32 agf_spare0;           // spare field
    __u32 agf_levels[XFS_BTNUM_AGF];    // btree levels
    __u32 agf_spare1;           // spare field
    __u32 agf_flfirst;          // first freelist block's index
    __u32 agf_fllast;           // last freelist block's index
    __u32 agf_flcount;          // count of blocks in freelist
    xfs_extlen_t agf_freeblks;  // total free blocks
    xfs_extlen_t agf_longest;   // longest free space
};

#define XFS_AGF_MAGICNUM    0x00000001
#define XFS_AGF_VERSIONNUM  0x00000002
#define XFS_AGF_SEQNO       0x00000004
#define XFS_AGF_LENGTH      0x00000008
#define XFS_AGF_ROOTS       0x00000010
#define XFS_AGF_LEVELS      0x00000020
#define XFS_AGF_FLFIRST     0x00000040
#define XFS_AGF_FLLAST      0x00000080
#define XFS_AGF_FLCOUNT     0x00000100
#define XFS_AGF_FREEBLKS    0x00000200
#define XFS_AGF_LONGEST     0x00000400
#define XFS_AGF_NUM_BITS    11
#define XFS_AGF_ALL_BITS    ((1 << XFS_AGF_NUM_BITS) - 1)
