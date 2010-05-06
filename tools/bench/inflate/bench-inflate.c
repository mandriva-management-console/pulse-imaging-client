/*
 * (c) 2010 Mandriva, http://www.mandriva.com
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
 * Help to benchmark various zlib inflate() parameters
 * - inflate() loop algorithms 
 * - buffer size
 *
 * The program will run 100 times:
 *  - read a compress data file, 
 *  - uncompress the data,
 *  - hash them with MD5
 *
 * Then the program will return a CSV compatible output
 * with time taken for each loop iteration
 * Operator will have to open the CSV file with a spreadsheet
 * and analyze the data (mainly remove invalid values: values too high
 * above others)
 *
 * Dependencies: librt and libgcrypt
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>

#include <assert.h>
#include <time.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sched.h>

#include "timespec.h"

#include <zlib.h>

#include <gcrypt.h>

/*
 * give advice to the kernel about the kind of readahead to perform
 * if nothing defined, default behavor
 *
 */
#define ADVISE_SEQ 0
#define ADVISE_RAND 0

#if defined(ADVISE_SEQ) && \
    defined(ADVISE_RAND) && \
    (ADVISE_SEQ > 0) && (ADVISE_RAND > 0)
#error "Only one kind of advise could be used"
#endif

/*
 * give advice to the kernel about the number of INSIZE chunk
 * to readahead, before or after a read() operation 
 *
 * the value is used as the number of INSIZE chunk
 * to request
 */
#define WILLNEED_BEFORE 0
#define WILLNEED_AFTER 0

#if defined(WILLNEED_BEFORE) && \
    defined(WILLNEED_AFTER) && \
    (WILLNEED_BEFORE > 0) && (WILLNEED_AFTER > 0)
#error "Only one kind of advise could be used"
#endif

int 
main(int argc, char *argv[])
{
    z_stream zptr;

    int zret;

    int fi;

    ssize_t size;

    struct timespec start;
    struct timespec end;
    struct timespec duration;

    uint8_t *IN;
    uint8_t *OUT;
    size_t INSIZE = 0;
    size_t OUTSIZE = 0;

    gcry_md_hd_t md;
    int md_md5;
    unsigned char *digest;

    gcry_error_t libgcrypt_err;

    int i;

    time_t t;

    char buf[64];

    int prio;
    int sched;
    struct sched_param param;
    pid_t pid;

    int ret;

    off_t offset;

    if (argc != 4) {
      fprintf(stderr, "need args: insize outsize file\n");
      return 1;
    }

    INSIZE = atoi(argv[1]);
    OUTSIZE = atoi(argv[2]);
    
    if (INSIZE == 0 || OUTSIZE == 0) {
      fprintf(stderr, "need args: insize outsize file\n");
      return 1;
    }

    IN = malloc(INSIZE);
    OUT = malloc(OUTSIZE);

    if (IN == NULL || OUT == NULL) {
      fprintf(stderr, "malloc() failed\n");
      return 1;
    }

    md_md5 = gcry_md_map_name("MD5");

    libgcrypt_err = gcry_md_open(&md, md_md5, 0);
    if (libgcrypt_err) {
      fprintf(stderr,
	      "ERROR: can't initialize MD: %s/%s\n",
	      gcry_strsource(libgcrypt_err),
	      gcry_strerror(libgcrypt_err));

      return 1;
    }


    /* open data file to flush cache */
    fi = open(argv[3], O_RDONLY);
    if (fi == -1) {
      fprintf(stderr, "can't open file: %s %s\n", argv[1], strerror(errno));
      return 1;
    }

#if defined(_POSIX_SYNCHRONIZED_IO) && _POSIX_SYNCHRONIZED_IO > 0
    ret = fdatasync(fi);
#else
    ret = fsync(fi);
#endif

    if (ret != 0) {
      fprintf(stderr, "sync failed: %s\n", strerror(errno));
      return 1;
    }

    ret = posix_fadvise(fi, 0, 0, POSIX_FADV_DONTNEED);
    if (ret != 0) {
      fprintf(stderr, "fadvise(POSIX_FADV_DONTNEED) error: %d : %d\n", ret, errno);
      return 1;
    }

    close(fi);

    /*
     * get high priority
     *
     */
    pid = getpid();

    errno = 0;
    prio = getpriority(PRIO_PROCESS, pid);
    if (errno != 0) {
      fprintf(stderr, "Can't get priority, harmless: %s\n", strerror(errno));
      prio = 0;
    }
  
    prio -= 20;
    if (setpriority(PRIO_PROCESS, pid, prio) == -1) {
      fprintf(stderr, "Can't set priority, harmless: %s\n", strerror(errno));
    }

    /*
     * get very high priority
     *
     * any other than SCHED_OTHER could be bad.
     * in fact anything different/higher than the scheduler/priority
     * used by the X server will probably give bad result
     */
  
    memset(&param, 0, sizeof(struct sched_param));
    sched = sched_getscheduler(pid);
    if (sched == -1) {
      fprintf(stderr, "Can't get scheduler, harmless: %s\n", strerror(errno));
    }

#if 0
# define BENCH_SCHED SCHED_FIFO
#else
# define BENCH_SCHED SCHED_RR 
#endif

    if (sched != BENCH_SCHED) {
      param.sched_priority = sched_get_priority_max(BENCH_SCHED);
      if (sched_setscheduler(pid, BENCH_SCHED, &param) == -1) {
	fprintf(stderr, "Can't set scheduler, harmless: %s\n", strerror(errno));
      }
    } else {
      if (sched_getparam(pid, &param) == -1 || 
	  param.sched_priority != sched_get_priority_max(BENCH_SCHED)) {
	param.sched_priority = sched_get_priority_max(BENCH_SCHED);
	if (sched_setparam(pid, &param) == -1) {
	  fprintf(stderr, "Can't set scheduler parameters, harmless: %s\n",
		  strerror(errno));
	}
      }
    }

    if (mlockall(MCL_CURRENT | MCL_FUTURE) == -1) {
      fprintf(stderr, "Can't lock memory, harmless: %s\n",
	      strerror(errno));
    }

    printf("\"%s\";", argv[0]);

#if defined(MY_WAY)
    printf("\"my way\";\n");
#elif defined(ZPIPE_WAY)
    printf("\"zpipe_way\";\n");
#else
#error "Need a way"
#endif

#if defined(ADVISE_SEQ) && ADVISE_SEQ > 0
    printf("\"sequential advise\";\n");
#elif defined(ADVISE_RAND) && ADVISE_RAND > 0
    printf("\"random advise\";\n");
#else
    printf("\"default advise\";\n");
#endif

#if defined(WILLNEED_BEFORE) && WILLNEED_BEFORE > 0
    printf("\"readahead before\";%d;\n", WILLNEED_BEFORE);
#elif defined(WILLNEED_AFTER) && WILLNEED_AFTER > 0
    printf("\"readahead after\";%d;\n", WILLNEED_AFTER);
#else
    printf("\"kernel readahead\";\n");
#endif

    printf("\"uid\";%d;\n", getuid());

    t = time(NULL);
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", localtime(&t));

    printf("\"Date:\";\n%s;\n", buf);

    printf("\"File\";\"%s\";\n", argv[3]);

    printf("\"IN size\";%d;\n", INSIZE);

    printf("\"OUT size\";%d;\n", OUTSIZE);
    printf("\n");

    for(i = 0; i < 100; i ++) {

      fflush(stdout);

      /* open data file */
      fi = open(argv[3], O_RDONLY);
      if (fi == -1) {
	fprintf(stderr, "can't open file: %s %s\n", argv[1], strerror(errno));
	return 1;
      }

#if defined(ADVISE_SEQ) && ADVISE_SEQ > 0
      /* will read sequentially */
      ret = posix_fadvise(fi, 0, 0, POSIX_FADV_SEQUENTIAL);
      if (ret != 0) {
	fprintf(stderr, "fadvise(POSIX_FADV_SEQUENTIAL) error: %d : %d\n", ret, errno);
	return 1;
      }
#elif defined(ADVISE_RAND) && ADVISE_RAND > 0
      ret = posix_fadvise(fi, 0, 0, POSIX_FADV_RANDOM);
      if (ret != 0) {
	fprintf(stderr, "fadvise(POSIX_FADV_RANDOM) error: %d : %d\n", ret, errno);
	return 1;
      }
#endif

      offset = 0;

      gcry_md_reset(md);

      clock_gettime(CLOCK_REALTIME, &start);

      zptr.total_in = 0;
      zptr.total_out = 0;

#if defined(MY_WAY)

#if defined(WILLNEED_BEFORE) && WILLNEED_BEFORE > 0
    ret = posix_fadvise(fi, offset, WILLNEED_BEFORE * INSIZE, POSIX_FADV_WILLNEED);
    if (ret != 0) {
      fprintf(stderr, "fadvise(POSIX_FADV_WILLNEED) error: %d : %d\n", ret, errno);
      return 1;
    }
    offset += INSIZE;
#endif

    /* read initial block of data */
    size = read(fi, IN, INSIZE);
    if (size != INSIZE) {
      if (size < 0) {
	fprintf(stderr, "can't read file: %s %s\n", argv[1], strerror(errno));
	return 1;
      }
      if (size == 0) {
	fprintf(stderr, "empty file: %s %s\n", argv[1], strerror(errno));
	return 1;
      }
    }

#if defined(WILLNEED_AFTER) && WILLNEED_AFTER > 0
    offset += size;

    ret = posix_fadvise(fi, offset, WILLNEED_AFTER * INSIZE, POSIX_FADV_WILLNEED);
    if (ret != 0) {
      fprintf(stderr, "fadvise(POSIX_FADV_WILLNEED) error: %d : %d\n", ret, errno);
      return 1;
    }
#endif

    zptr.zalloc = Z_NULL;
    zptr.zfree = Z_NULL;
    zptr.opaque = Z_NULL;
    
    zptr.next_in = (unsigned char *) IN;
    zptr.next_out = (unsigned char *) OUT;
    zptr.avail_in = size;
    zptr.avail_out = OUTSIZE;

    zret = inflateInit(&zptr);
    if (zret != Z_OK) {
      fprintf(stderr, "can't init zlib: %d %s\n", zret, strerror(errno));
      return 1;
    }

    while(1) {
      /* decompress
       * remarks: as of zlib-1.2.3 code
       * flush parameter only distinguish between three methods
       *    1) Z_FINISH,
       *    2) Z_BLOCK
       *    3) Z_NO_FLUSH, Z_FULL_SYNC, Z_SYNC_FLUSH
       */
      zret = inflate(&zptr, Z_SYNC_FLUSH);
      if (zret != Z_OK) {
	break;
      }

      if (zptr.avail_in == 0) {
#if defined(WILLNEED_BEFORE) && WILLNEED_BEFORE > 0
	ret = posix_fadvise(fi, offset, WILLNEED_BEFORE * INSIZE, POSIX_FADV_WILLNEED);
	if (ret != 0) {
	  fprintf(stderr, "fadvise(POSIX_FADV_WILLNEED) error: %d : %d\n", ret, errno);
	  return 1;
	}
	offset += INSIZE;
#endif

	size = read(fi, IN, INSIZE);
	if (size != INSIZE) {
	  if (size < 0) {
	    fprintf(stderr, "can't read file: %s %s\n", argv[1], strerror(errno));
	    return 1;
	  }

	  /* if size == 0 : EOF 
	   *
	   * no need to worry,
	   * - if inflate() need more data, it will return Z_BUF_ERROR
	   * and the loop will end
	   * - if inflate() do not need more data, but still have data
	   * to output, get them 
	   *
	   */
	}

#if defined(WILLNEED_AFTER) && WILLNEED_AFTER > 0
	if (size > 0) {
	  offset += size;

	  ret = posix_fadvise(fi, offset, WILLNEED_AFTER * INSIZE, POSIX_FADV_WILLNEED);
	  if (ret != 0) {
	    fprintf(stderr, "fadvise(POSIX_FADV_WILLNEED) error: %d : %d\n", ret, errno);
	    return 1;
	  }
	}
#endif        

	zptr.avail_in = size;
	zptr.next_in = (unsigned char *) IN;
      }

      /* buffer filled */
      if (zptr.avail_out == 0) {

	gcry_md_write(md, OUT, OUTSIZE);

	zptr.avail_out = OUTSIZE;
	zptr.next_out = OUT;
      }
    }

    /* flush any remaining data to disk */
    size = OUTSIZE - zptr.avail_out;
    if (size != 0) {

      gcry_md_write(md, OUT, size);

      zptr.avail_out = OUTSIZE;
      zptr.next_out = OUT;
    }

    if (zret != Z_STREAM_END) {
      fprintf(stderr, "not end of zlib stream !: %d %s\n", zret, strerror(errno));
      return 1;
    }

#elif defined(ZPIPE_WAY)  /* other way of doing, from zpipe.c */
    /* this portion was written in 2005 by Mark Adler */

    zptr.zalloc = Z_NULL;
    zptr.zfree = Z_NULL;
    zptr.opaque = Z_NULL;
    
    zptr.next_in = Z_NULL;
    zptr.next_out = Z_NULL;
    zptr.avail_in = 0;
    zptr.avail_out = 0;

    zret = inflateInit(&zptr);
    if (zret != Z_OK) {
      fprintf(stderr, "can't init zlib: %d %s\n", zret, strerror(errno));
      return 1;
    }

    do {

      assert(zptr.avail_in == 0);

#if defined(WILLNEED_BEFORE) && WILLNEED_BEFORE > 0
      ret = posix_fadvise(fi, offset, WILLNEED_BEFORE * INSIZE, POSIX_FADV_WILLNEED);
      if (ret != 0) {
	fprintf(stderr, "fadvise(POSIX_FADV_WILLNEED) error: %d : %d\n", ret, errno);
	return 1;
      }
      offset += INSIZE;
#endif

      /* read initial block of data */
      size = read(fi, IN, INSIZE);
      
      if (size != INSIZE) {
	if (size < 0) {
	  fprintf(stderr, "can't read file: %s %s\n", argv[1], strerror(errno));
	  return 1;
	}
	if (size == 0) {
	  break;
	}
      }

#if defined(WILLNEED_AFTER) && WILLNEED_AFTER > 0
      offset += size;

      ret = posix_fadvise(fi, offset, WILLNEED_AFTER * INSIZE, POSIX_FADV_WILLNEED);
      if (ret != 0) {
	fprintf(stderr, "fadvise(POSIX_FADV_WILLNEED) error: %d : %d\n", ret, errno);
	return 1;
      }
#endif

      zptr.avail_in = size;
      zptr.next_in = (unsigned char *) IN;

      /* consume all the input buffer */
      do {
	zptr.avail_out = OUTSIZE;
	zptr.next_out = OUT;

	zret = inflate(&zptr, Z_SYNC_FLUSH);
	switch(zret) {
	case Z_NEED_DICT:
	  zret = Z_DATA_ERROR;     /* and fall through */
	case Z_DATA_ERROR:
	case Z_MEM_ERROR:
	case Z_STREAM_ERROR:
	  fprintf(stderr, "zlib error !: %d %s\n", zret, strerror(errno));
	  (void)inflateEnd(&zptr);
	  return 1;
	}

	size = OUTSIZE - zptr.avail_out;
	if (size != 0) {
	  gcry_md_write(md, OUT, size);
	}
      } while (zptr.avail_out == 0);

    } while (zret != Z_STREAM_END);

    if (zret != Z_STREAM_END) {
      fprintf(stderr, "zlib error !, not the end: %d %s\n", zret, strerror(errno));
      return 1;
    }

#else
#warning "Need MY_WAY or ZPIPE_WAY"
#endif

    /* check for empty buffer */
    if (zptr.avail_in != 0) {
      fprintf(stderr, "WARNING data not used by zlib : %s %s\n", argv[1], strerror(errno));
    }

    /* try to read more data, just in case */
    size = read(fi, IN, INSIZE);
    if (size != 0) {
      fprintf(stderr, "WARNING more data at the end of zlib stream: %s %s\n", argv[1], strerror(errno));
    }

    zret = inflateEnd(&zptr);
    if (zret != Z_OK) {
      fprintf(stderr, "can't deinit zlib: %d %s\n", zret, strerror(errno));
    }

    gcry_md_final(md);

    digest = gcry_md_read(md, md_md5);

    clock_gettime(CLOCK_REALTIME, &end);

    timespec_substract(&duration, &end, &start);

    /* no more needed, release from memory */
    ret = posix_fadvise(fi, 0, 0, POSIX_FADV_DONTNEED);
    if (ret != 0) {
      fprintf(stderr, "fadvise(POSIX_FADV_DONTNEED) error: %d : %d\n", ret, errno);
      return 1;
    }

    close(fi);

    printf(";%ld,%09ld;\n",
	    duration.tv_sec, duration.tv_nsec);

    }

    printf("\n");

    printf("\"Nombre\";=NBVAL(B11:B110);\n");

    printf("\"Moyenne\";=MOYENNE(B11:B110);=ECARTYPE(B11:B110);\n");

    printf("\"Min\";=MIN(B11:B110);=B113 - 2 * C113;\n");

    printf("\"Max\";=Max(B11:B110);=B113 + 2 * C113;\n");

    printf("\n");

    printf("\"Data in\";%lu;\n", (unsigned long) zptr.total_in);

    printf("\"Data out\";%lu;\n", (unsigned long) zptr.total_out);

    printf("\"Ratio\";=B118/B117;\n");

    printf("\"Debit (in)\";=(B117 / B113) / (1024*1024);\n");
    printf("\"Debit (out)\";=(B118 / B113) / (1024*1024);\n");

    printf("\n");

    printf("\"MD5\";\"%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x\";\n", 
	   digest[0], 
	   digest[1], 
	   digest[2], 
	   digest[3], 
	   digest[4], 
	   digest[5], 
	   digest[6], 
	   digest[7],
	   digest[8],
	   digest[9],
	   digest[10],
	   digest[11],
	   digest[12],
	   digest[13],
	   digest[14],
	   digest[15]);

    /* release ressources */
    gcry_md_close(md);

    return 0;
}
