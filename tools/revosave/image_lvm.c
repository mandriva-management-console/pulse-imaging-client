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

//#include <linux/lvm.h>

#include "compress.h"
#include "client.h"
#include "lvm.h"

char info1[32], info2[32];
unsigned long lvm_sect;

static inline void setbit(unsigned char *base, unsigned long bit) {
    unsigned char mask[8] = { 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80 };

    base[bit >> 3] |= mask[bit & 7];
}

void allocated_sectors(PARAMS * p) {
    unsigned long i;
    unsigned long bitmap_lg;
    int off = 0;

    off = p->nb_sect;

    p->bitmap = (unsigned char *)calloc(bitmap_lg = (off + 7) / 8, 1);
    p->bitmaplg = bitmap_lg;

    // backup LVM: everything
    for (i = 0; i < off; i++)
        setbit(p->bitmap, i);

    sprintf(info1, "%u", off);
    sprintf(info2, "%u", off);
    print_sect_info(off, off);

}

/* main */
int main(int argc, char *argv[]) {
    PARAMS params;
    long long offset;
    int fd;

    if (argc != 3) {
        fprintf(stderr, "Usage : image_lvm [device] [image prefix name]\n");
        exit(1);
    }
    // check for LVM
    lvm_check(argv[1], &offset);
    if (offset == 0)
        exit(1);

    params.nb_sect = offset / 512;
    allocated_sectors(&params);

    if (argv[2][0] == '?')
        exit(0);

    // Compress now

    ui_send("init_backup", 5, argv[1], argv[2], info1, info2, argv[0]);
    fd = open(argv[1], O_RDONLY);

    compress_volume(fd, argv[2], &params, "LVM");
    close(fd);

    return 0;
}
