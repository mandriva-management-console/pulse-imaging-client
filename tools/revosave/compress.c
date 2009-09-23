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
#include <unistd.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <linux/fs.h>
#include <string.h>
#include <time.h>

#include "compress.h"
#include "client.h"

#define LARGE_OFF_T (((off_t) 1 << 62) - 1 + ((off_t) 1 << 62))
#define UI_READ_ERROR ui_read_error(__FILE__,__LINE__, errno, fi)

/* Check that off_t can represent 2**63 - 1 correctly.
     We can't simply define LARGE_OFF_T to be 9223372036854775807,
     since some C++ compilers masquerading as C compilers
     incorrectly reject 9223372036854775807.  */
/*
int off_t_is_large[(LARGE_OFF_T % 2147483629 == 721
                && LARGE_OFF_T % 2147483647 == 1) ? 1 : -1];
*/

#define BLOCKFLUSH 10

unsigned long long done, todo;

#if 0
void setblocksize(FILE *f)
{
    int bs=0, fd;

    fd = fileno(f);
    ioctl(fd, BLKBSZGET, &bs);
    bs = 512;
    if (ioctl(fd, BLKBSZSET, &bs)) {
        perror("Could not set block size to 512");
    }
    /* ioctl(fd, BLKBSZGET, &bs);
    printf("Block size= %d\n", bs); */

}

#else
void setblocksize(int fd)
{
    int bs=0;

    ioctl(fd, BLKBSZGET, &bs);
    bs = 512;
    if (ioctl(fd, BLKBSZSET, &bs)) {
        perror("Could not set block size to 512");
    }
    /* bs = 0;
    ioctl(fd, BLKBSZGET, &bs);
    printf("Block size= %d\n", bs); */

}
#endif


/* Fatal error, notify the server about it */
void fatal(void)
{
    system("/bin/revosendlog 8");
}

/*
 * backup read error: disk dead ? out of bounds read ?
 */
void ui_read_error(char *file, int line, int err, int fd)
{
    char sline[32], serrno[32], soffset[32], tmp[256];
    off64_t offset;

    sprintf(sline, "%d", line);
    sprintf(serrno, "%d", err);
    offset = lseek64(fd, 0, SEEK_CUR);
    sprintf(soffset, "%llu", (unsigned long long)offset);

    snprintf(tmp, 255,
             "Hard Disk Read Error ! Bad hard disk or filesystem !\n"
             "errno %d (%s)\n" " file %s, line %d \n"
             "offset=%08lx%08lx ", err, strerror(err), file, line,
             (long) ((long long) offset >> 32), (long) offset);
    fprintf(stderr, "ERROR: %s\n", tmp);

    fatal();
    ui_send("misc_error", 2, "Hard Disc read error", tmp);
}

/*
 * prints the number of total/used sectors
 */
void print_sect_info(long long unsigned tot_sec, long long unsigned used_sec)
{
    debug("- Total sectors : %llu = %llu MiB\n", tot_sec,  (tot_sec / 2048));
    debug("- Used sectors  : %llu = %llu MiB (%3.2f%%)\n", used_sec,
            (used_sec / 2048), 100.0 * (float) used_sec / tot_sec);
}

/*
 * Compression initialization
 */
void
compress_init (COMPRESS ** c, int block, unsigned long long bytes,
               FILE * index)
{
  FILE *f;

  *c = (COMPRESS *) malloc (sizeof (COMPRESS));
  (*c)->zptr = (z_streamp) malloc (sizeof (z_stream));
  (*c)->zptr->zalloc = Z_NULL;
  (*c)->zptr->zfree = Z_NULL;

//      deflateInit((*c)->zptr,Z_BEST_SPEED);
  if ((f = fopen("/etc/complevel", "r")) != NULL) {
    int complevel = 3;

    fscanf(f, "%d", &complevel);
    fclose(f);
    deflateInit ((*c)->zptr, complevel);
  } else {
    deflateInit ((*c)->zptr, 3);
  }

  (*c)->crc = adler32 (0L, Z_NULL, 0);
  (*c)->end = 0;
  (*c)->state = Z_NO_FLUSH;

  (*c)->header = 1;
  (*c)->block = block;
  (*c)->offset = 0;
  (*c)->f = index;
  (*c)->cbytes = bytes;
  (*c)->compressed_blocks = BLOCKFLUSH;

  (*c)->zptr->avail_out = OUTBUFF;
  (*c)->zptr->next_out = (*c)->outbuff;
}

void
compress_data (COMPRESS * c, unsigned char *data, int lg, FILE * out,
               char end)
{
  int ret, lout = c->outbuff - c->zptr->next_out;
  size_t w;
  //static int nocomp = 0, slg = 0, sout = 0;

  c->zptr->next_in = data;
  c->zptr->avail_in = lg;

  c->compressed_blocks++;

  if (c->compressed_blocks >= BLOCKFLUSH)
    {
      c->compressed_blocks = 0;
      c->state = Z_FULL_FLUSH;
    }

  if (end)
    c->end = 1;

  if (!c->header)
    {
      c->cbytes += lg;
      done += lg;
    }
  c->header = 0;

  do
    {
    deflate_again:
      //printf("Debug : IN : %ld / OUT : %ld\n",c->zptr->avail_in,c->zptr->avail_out);
      ret = deflate (c->zptr, (end == 0) ? c->state : Z_FINISH);

      if (c->zptr->avail_out == 0)
        {
          w = fwrite (c->outbuff, OUTBUFF, 1, out);
          lout += OUTBUFF;
          if (w != 1)
            compress_write_error ();
          c->crc = adler32 (c->crc, c->outbuff, OUTBUFF);
          c->zptr->avail_out = OUTBUFF;
          c->zptr->next_out = c->outbuff;
          c->offset += OUTBUFF;
          goto deflate_again;
        }

      if (c->state != Z_NO_FLUSH)
        {                       //printf("Debug : %d %d (%d)\n",c->zptr->avail_in,c->zptr->avail_out,ret);
          if ((c->zptr->avail_in == 0) && (c->zptr->avail_out != 0))
            {
              // DEBUG code
              //fprintf (c->f, "%lld:%d-%ld\n", c->cbytes, c->block,
              //       c->offset + OUTBUFF - c->zptr->avail_out);
              c->state = Z_NO_FLUSH;
            }
        }

      if (ret == Z_STREAM_END)
        {
          w = fwrite (c->outbuff, OUTBUFF - c->zptr->avail_out, 1, out);
          lout += OUTBUFF - c->zptr->avail_out;
          if ((w != 1) && (OUTBUFF - c->zptr->avail_out != 0))
            compress_write_error ();
          c->crc = adler32 (c->crc, c->outbuff, OUTBUFF - c->zptr->avail_out);
          c->zptr->avail_out = OUTBUFF;
          c->zptr->next_out = c->outbuff;
          //printf("e");
          break;
        }

      if (ret != Z_OK)
        {
          printf ("ZLIB error in deflate : %d\n", ret);
          exit (1);
        }

    }
  while ((c->zptr->avail_in) || (end));
#if 0
  lout += c->zptr->next_out - c->outbuff;
  debug("%d:%d %d %d\n", lg, lout, nocomp, c->zptr->next_out - c->outbuff);
  if (nocomp <= 0) {
        debug("comp\n");
        deflateParams(c->zptr, 3, Z_DEFAULT_STRATEGY);
        nocomp = 6;
        slg = sout = 0;
  } else {
        if (sout > slg && nocomp != 5) {
                debug("no comp\n");
                deflateParams(c->zptr, 0, Z_DEFAULT_STRATEGY);
        }
  }
  nocomp--;
  slg += lg;
  sout += lout;
#endif
}

unsigned long long
compress_end (COMPRESS * c, FILE * out)
{
  unsigned long long ret;

  if ((c->zptr->avail_out != OUTBUFF) || (!c->end))
    {                           //printf("-");
      compress_data (c, NULL, 0, out, 1);
    }

  deflateEnd (c->zptr);
  free (c->zptr);
//  {
//    FILE *flog;
//    flog = fopen ("/tmp/crc", "a+");
//    fprintf (flog, "%d:%lx", c->block, c->crc);
//    fclose (flog);
//  }
  ret = c->cbytes;
  free (c);

  return ret;
}

/* Write error */
void
compress_write_error (void)
{
  fatal();
  fprintf(stderr, "ERROR: Write error ! Server's disk might be full.\n");
  ui_send("backup_write_error", 0);
  exit(EXIT_FAILURE);
}

/*
 * main compression loop
 */
void
compress_volume (int fi, unsigned char *nameprefix, PARAMS * p, char *info)
{
  int i, j, k, nb;
  IMAGE_HEADER header;
  COMPRESS *c;
  unsigned char buffer[TOTALLG], *ptr, *dataptr;
  unsigned long remaining, used, skip;
  unsigned long long bytes = 0;
  unsigned short lg, datalg;
  FILE *fo, *fs, *index;
  char filename[128], firststring[200], *filestring, *sec,
    line[400], empty[] = "", numline[8];
  time_t start, now;

  setblocksize(fi);

  //debug("- Bitmap lg    : %ld\n",p->bitmaplg);
  nb = ((p->bitmaplg + ALLOCLG - 1) / ALLOCLG);
  //debug("- Nb of blocks : %d\n",nb);

  remaining = p->bitmaplg;
  ptr = p->bitmap;

  skip = 0;

  sprintf (firststring, "SECTORS=%llu|BLOCKS=%d|%s", p->nb_sect, nb, info);

  sprintf (filename, "%sidx", nameprefix);
  index = fopen (filename, "w");

  start = time(NULL);

#include "compress-loop.h"

  now = time(NULL);
  fclose (index);

  sec = ui_send("close", 0);
  debug("- saved in %d seconds\n", (int)difftime(now, start));
}
