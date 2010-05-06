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
#include "config.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

#include <zlib.h>

#define INSIZE 2048

unsigned char BUFFER[24064];
unsigned char Bitmap[24064 - 2048];
unsigned char IN[INSIZE];
unsigned char zero[512];

typedef struct params_ {
    int bitindex;
    int fo;
    unsigned long long offset;
} PARAMS;

int eof(int fd) {
    __off64_t pos, end;
    pos = lseek64(fd, 0, SEEK_CUR);
    if (pos < 0) {
        fprintf(stderr, "Error LSEEK : eof,pos\n");
    }
    end = lseek64(fd, 0, SEEK_END);
    if (end < 0) {
        fprintf(stderr, "Error LSEEK : eof,end\n");
    }
    if (lseek64(fd, pos, SEEK_SET) < 0) {
        fprintf(stderr, "Error LSEEK, reseek\n");
    }
    if (end == pos)
        return 1;
    return 0;
}

void fill(int fd, int bytes, int dir) {
    int err = 0;
    int lg = bytes;

    if (eof(fd)) {
        while (bytes > 512) {
            if (write(fd, zero, 512) != 512) {
                if (!err) {
                    err = 1;
                    fprintf(stderr, "Error FILL : write 512!=512\n");
                }
            }
            bytes -= 512;
        }
        if (write(fd, zero, bytes) != bytes) {
            fprintf(stderr, "Error FILL : %d bytes write...\n", bytes);
            err = 1;
        }
    } else {
        if (lseek64(fd, bytes, dir) < 0) {
            fprintf(stderr, "Error FILL : skip error");
            err = 1;
        }
    }
    if (err)
        fprintf(stderr, "Error when filling %d bytes\n", lg);
}

void flushToDisk(unsigned char *buff, unsigned char *bit, PARAMS * cp, int lg) {
    unsigned char *ptr = buff;
    unsigned char mask[] = { 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80 };
    int indx = cp->bitindex;

// printf("Enter : bitindex -> %d\n",indx);
    while (lg > 0) {
        while (!(bit[indx >> 3] & mask[indx & 7])) {
            indx++;
            cp->offset += 512;
            if (cp->fo)
                fill(cp->fo, 512, SEEK_CUR);
        }
//      printf("Write @offset : %lld\t",cp->offset);
//      {int i; for(i=0;i<15;i++) printf("%02x ",ptr[i]); printf("\n");}
        if (cp->fo) {
            if (write(cp->fo, ptr, 512) != 512)
                fprintf(stderr, "Flush Error : write 512!=512\n");
        }
        cp->offset += 512;
        ptr += 512;
        indx++;
        lg -= 512;
    }
// printf("Exit  : bitindex -> %d\n",indx);
    cp->bitindex = indx;
}

int main(int argc, char *argv[]) {
    z_stream zptr;
    int state;
    int ret, firstpass, bitmaplg;
    FILE *fi;
    PARAMS currentparams;

 start:
    state = Z_SYNC_FLUSH;
    firstpass = 1;
    bitmaplg = 0;

    zptr.zalloc = NULL;
    zptr.zfree = NULL;

    fi = fopen(argv[1], "rb");
    if (fi == NULL) {
        printf("Cannot open input file\n");
        exit(1);
    }
    zptr.avail_in = fread(IN, 1, INSIZE, fi);

    memset(zero, 0, 512);

    currentparams.fo = 0;
    currentparams.offset = 0;
    currentparams.bitindex = 0;
    if (argc > 2) {
        currentparams.fo = open(argv[2], O_RDWR | O_CREAT | O_LARGEFILE, 0666);
        sscanf(argv[3], "%lld", &currentparams.offset);
        if (currentparams.fo) {
            unsigned long long i;
            printf("Seeking to : %lld\n", currentparams.offset);
            i = lseek64(currentparams.fo, currentparams.offset, SEEK_SET);
            if (i != currentparams.offset) {
                fprintf(stderr, "Seek error : %lld\n", i);
            }
        }
    }

    zptr.next_in = (char *)IN;
    zptr.next_out = (char *)BUFFER;     // was dbuf.data;
    zptr.avail_out = 24064;

    inflateInit(&zptr);

    do {
//  if (inflateSyncPoint(&zptr)) printf("#");

        ret = inflate(&zptr, state);

//  printf("-> %d : %d / %d\n",ret ,zptr.avail_in ,zptr.avail_out );

        if ((ret == Z_OK) && (zptr.avail_out == 0)) {
            if (firstpass) {
                printf("Params : *%s*\n", BUFFER);
                if (strstr(BUFFER, "BLOCKS=")) {
                    FILE *fo;
                    int i = 0;
                    sscanf(strstr(BUFFER, "BLOCKS=") + 7, "%d", &i);
                    fo = fopen("/tmp/blocks", "w");
                    fprintf(fo, "%d\n", i);
                    fclose(fo);
                }
                if (strstr(BUFFER, "ALLOCTABLELG="))
                    sscanf(strstr(BUFFER, "ALLOCTABLELG=") + 13, "%d",
                           &bitmaplg);
                memcpy(Bitmap, BUFFER + 2048, 24064 - 2048);
#if 0
                {
                    int i;
                    for (i = 0; i < 256; i++) {
                        if ((i & 15) == 0)
                            printf("\n%04x : ", i);
                        printf("%02X ", Bitmap[i]);
                    }
                    printf("\n");
                }
#endif
                currentparams.bitindex = 0;
                firstpass = 0;
            } else {
                flushToDisk(BUFFER, Bitmap, &currentparams, 24064);
            }

            zptr.next_out = (char *)BUFFER;
            zptr.avail_out = 24064;
        }

        if ((ret == Z_OK) && (zptr.avail_in == 0)) {
            zptr.avail_in = fread(IN, 1, INSIZE, fi);
            zptr.next_in = (char *)IN;
        }
    }
    while (ret == Z_OK);

    if (ret == Z_STREAM_END) {
        {
            if (firstpass) {
                printf("Params : *%s*\n", BUFFER);
                if (strstr(BUFFER, "BLOCKS=")) {
                    FILE *fo;
                    int i = 0;
                    sscanf(strstr(BUFFER, "BLOCKS=") + 7, "%d", &i);
                    fo = fopen("/tmp/blocks", "w");
                    fprintf(fo, "%d\n", i);
                    fclose(fo);
                }
                if (strstr(BUFFER, "ALLOCTABLELG="))
                    sscanf(strstr(BUFFER, "ALLOCTABLELG=") + 13, "%d",
                           &bitmaplg);
                memcpy(Bitmap, BUFFER + 2048, 24064 - 2048);
                zptr.next_out = (char *)BUFFER;
                zptr.avail_out = 24064;
            }
        }

        printf("Flushing to EOF ... (%d bytes)\n", 24064 - zptr.avail_out);
        flushToDisk(BUFFER, Bitmap, &currentparams, 24064 - zptr.avail_out);
        zptr.next_out = (char *)BUFFER;
        zptr.avail_out = 24064;
    }

    ret = inflate(&zptr, Z_FINISH);
    inflateEnd(&zptr);

    printf("Returned : %d\t", ret);
    printf("(AvailIn : %d / ", zptr.avail_in);
    printf("AvailOut: %d)\n", zptr.avail_out);

    if (ret < 0) {
        fprintf(stderr, "ZLIB error, restarting...\n");
        goto start;
    }

    printf("Offset : %lld\n", currentparams.offset);
    printf("Bitmap index : %d\n", currentparams.bitindex);

    if (bitmaplg) {
        if (bitmaplg * 8 > currentparams.bitindex) {
            printf("Remaining bitmap : %d\n",
                   bitmaplg * 8 - currentparams.bitindex);
            currentparams.offset +=
                512 * (bitmaplg * 8 - currentparams.bitindex);
            if (currentparams.fo) {
                // why fill ?
                fill(currentparams.fo,
                     512 * (bitmaplg * 8 - currentparams.bitindex), SEEK_CUR);
            }
        }
    }

    if (currentparams.fo)
        close(currentparams.fo);

    {
        FILE *fo;
        fo = fopen("/tmp/offset", "w");
        fprintf(fo, "%lld\n", currentparams.offset);
    }
    fclose(fi);
    return 0;
}
