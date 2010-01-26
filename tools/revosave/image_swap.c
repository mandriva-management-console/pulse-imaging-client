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

#include "compress.h"
#include "client.h"

char info1[32], info2[32];

void check_signature(FILE * f, PARAMS * p) {
    char dum[4096];

    fread(dum, 4096, 1, f);

    if (strncmp(dum + 4096 - 10, "SWAP-SPACE", 10) == 0)
        return;
    if (strncmp(dum + 4096 - 10, "SWAPSPACE2", 10) == 0)
        return;

    exit(1);
}

void allocated_sectors(PARAMS * p) {
    int i, used;

    p->bitmap = (unsigned char *)calloc(8, 1);
    p->bitmaplg = 8;

    p->nb_sect = 8;
    used = 8;

    for (i = 0; i < 8; i++)
        p->bitmap[i] = 0xFF;

    sprintf(info1, "%u", (unsigned int)p->nb_sect);
    sprintf(info2, "%u", used);
    print_sect_info(p->nb_sect, used);
}

/*  */
int main(int argc, char *argv[]) {
    FILE *fi;
    PARAMS params;
    int fd;

    if (argc != 3) {
        fprintf(stderr, "Usage : image_swap [device] [image prefix name]\n");
        exit(1);
    }
    // Just check for SWAP-SPACE or SWAPSPACE2

    fi = fopen(argv[1], "rb");
    check_signature(fi, &params);
    allocated_sectors(&params);
    fclose(fi);

    if (argv[2][0] == '?')
        exit(0);

    // Compress now
    //

    ui_send("init_backup", 5, argv[1], argv[2], info1, info2, argv[0]);
    fd = open(argv[1], O_RDONLY);
    compress_volume(fd, argv[2], &params, "SWAP=1");
    close(fd);

    return 0;
}
