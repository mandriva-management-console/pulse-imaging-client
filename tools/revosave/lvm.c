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

#define _GNU_SOURCE
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

#include "compress.h"

#define FMTT_MAGIC "\040\114\126\115\062\040\170\133\065\101\045\162\060\116\052\076"


extern unsigned long lvm_sect;

/* compare two UUID strings
 *
 * return 1 if equal, else return 0
 * '-' characters are ignored in s2
 */
int uuid_compare(char *s1, char *s2)
{
    int i;

    for (i = 0; i < 32; i++) {
        if (*s2 == '-')
            s2++;
        if (*s1++ != *s2++)
            return 0;
    }
    return 1;
}


/* check if it's a LVM partition */
void lvm_check(char *device, long long *offset)
{
    unsigned long buf[256];
    FILE *fi;

    // check for LVM
    fi = fopen(device, "r");
    if (fi == NULL) {
        debug("Open failed\n");
        exit(1);
    }
    fread(&buf[0], 256 * 4, 1, fi);

    *offset = 0;
    /* lvm1 checks */
    debug("LVM1 signature check: %08lx\n", buf[0]);
    if (buf[0] == 0x00014d48) {
        debug("LVM1 found\n");

        *offset = buf[9] + buf[10];
        debug("LVM: Real part offset: %16llx\n", *offset);

        debug("LVM: VG name : '%32s'\n", (char *) &buf[11 + 32]);
        debug("LVM: PV Num  : %ld\n", buf[108]);
        debug("LVM: PE Size : %ld\n", buf[113] / 2);
        debug("LVM: PE Total: %ld\n", buf[114]);
        debug("LVM: PE Alloc: %ld\n", buf[115]);

        lvm_sect = buf[113] * buf[115];
        debug("LVM: Total sectors: %ld\n", lvm_sect);


        if (fseek(fi, buf[9], SEEK_SET) != 0) {
            debug("Seek error\n");
            return;
        }
    } else {
        __u64 off, mdh_offset;
        int state = 0, pe_count = 0, extent = 0, lev = 0;
        __u64 *pv;
        char *pvuuid;

        /* lvm2 */
        /* buf[128] = label_header */
        if (buf[128] != 0x4542414C || buf[129] != 0x454E4F4C) {
            debug("LVM2 LABELONE not found\n");
            return;
        }
        debug("LVM2 LABELONE found\n");
        if (buf[134] != 0x324D564C || buf[135] != 0x31303020) {
            debug("LVM2 001 not found\n");
            return;
        }
        /* buf[133] = label_header.offset_xl */
        /* pv_header */
        pv = (__u64 *) (((__u8 *) & buf[128]) + buf[133]);
        /* copy this PV uuid */
        pvuuid = (char *)pv;
        debug("LVM2 pv UUID: %32s\n", (char *) pvuuid);
        /* skip data */
        pv += 5;
        while (*pv) {
            pv += 2;
        }
        pv += 2;
        mdh_offset = *pv;
        debug("LVM2: Mdh offset: 0x%llX\n", mdh_offset);
        fseek(fi, mdh_offset, SEEK_SET);
        fread(buf, 128 * 4, 1, fi);
        if (strncmp((char *) &buf[1], (char *) FMTT_MAGIC, 16)) {
            debug("LVM: FMTT_MAGIC not found\n");
            return;
        }
        debug("LVM2: FMTT v%ld\n", buf[5]);
        off = buf[10] + ((__u64) buf[11] << 32);
        debug("LVM2: Meta offset: 0x%llX\n", off);
        if (off == 0)
            return;
        off += mdh_offset;
        fseek(fi, off, SEEK_SET);
        *offset = 0;
        /* Q&D parsing of meta data */
        state = 0;
        while (1) {
            char b[128], uuid[40];

            fgets(b, 127, fi);
            debug("LVM2:%d %s", lev, b);
            if (strstr(b, "{"))
                lev++;
            if (strstr(b, "}"))
                lev--;
            switch (state) {
            case 0:
                sscanf(b, "extent_size = %d", &extent);
                if (strstr(b, "physical_volumes"))
                    state = 1;
                break;
            case 1:
                if (sscanf(b, "id = \"%38s", uuid) == 1) {
                    if (uuid_compare(pvuuid, uuid))
                        state = 2;
                }
                break;
            case 2:
                if (sscanf(b, "pe_start = %lld", offset) == 1)
                    state = -1;
                break;
            case -1:
                sscanf(b, "pe_count = %d", &pe_count);
                break;
            }
            if (strstr(b, "}") && lev == 2 && state == -1)
                break;
        }
        if (state != -1) {
            debug("LVM2 pe_start not found!\n");
            *offset = 0;
            return;
        }
        debug("LVM: Saving %lld sectors\n", *offset);
        *offset *= 512;
        lvm_sect = extent * pe_count;
    }


    fclose(fi);

}
