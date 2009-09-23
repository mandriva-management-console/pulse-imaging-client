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

#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <zlib.h>

#define OUTBUFF 8192

#define HEADERLG 2048L
#define TOTALLG 24064L
#define ALLOCLG (TOTALLG-HEADERLG)

#define BLKGETSIZE _IO(0x12,96) /* return device size /512 (long *arg) */

#define debug(...) fprintf(stderr, __VA_ARGS__)

typedef struct {
  unsigned char *bitmap;
  unsigned long bitmaplg;
  unsigned long long nb_sect;
} PARAMS;

typedef struct i
{
    unsigned char header[HEADERLG] __attribute__((packed));
    unsigned char bitmap[ALLOCLG]  __attribute__((packed));
} IMAGE_HEADER;

typedef struct c
{
        z_streamp zptr;
        unsigned char outbuff[OUTBUFF];
        int end,state,header;
        unsigned long crc;
        unsigned int compressed_blocks,block;
        unsigned long offset;
        unsigned long long cbytes;
        FILE *f;
} COMPRESS;

void compress_volume (int fi, unsigned char *nameprefix, PARAMS * p, char *info);
void compress_init(COMPRESS **c,int block,unsigned long long bytes,FILE *index);
void compress_data(COMPRESS *c,unsigned char *data,int lg,FILE *out,char end);
unsigned long long compress_end(COMPRESS *c,FILE *out);
void compress_write_error (void);
//void setblocksize(FILE *f);
void setblocksize(int f);
void print_sect_info(long long unsigned tot_sec, long long unsigned used_sec);
