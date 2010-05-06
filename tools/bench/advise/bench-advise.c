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
 *
 * Author: Yann Droneaud <ydroneaud@mandriva.com>
 *
 */

/*
 * Help to benchmark various posix_{f,m}advise() parameters
 *
 * Dependencies: librt
 *
 * In order to test some real case, the source file should be bigger 
 * than the available memory (or total memory installed in the system).
 *
 *
 */

#define _GNU_SOURCE
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <time.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sched.h>
#include <stdint.h>

#include "timespec.h"

#include <fcntl.h>

/*
 * Data buffer, used for I/O operation
 *
 * use uint32_t to optimise memory access when the content of the buffer 
 * is used.
 * 
 * In each test, the content of the buffer is read in memory to simulate
 * usage. If the memory is not read, mmap() test would be definetly faster
 * than read() test
 *
 */
static uint32_t buffer[(4096 * 64) / 4];

/*
 * test only fadvise()
 *
 */
static int
test_fadvise(const char *name, int advise)
{
  int ret;
  int fd;
  ssize_t len;

  struct timespec start;
  struct timespec end;
  struct timespec duration;

  int fail = 0;

  size_t i;

  static uint32_t d;

  clock_gettime(CLOCK_REALTIME, &start);

  fd = open(name, O_RDONLY);

  if (fd == -1) {
    fprintf(stderr, "open error: %d\n", errno);
    return -1;
  }

  ret = posix_fadvise(fd, 0, 0, advise);
  if (ret != 0) {
    fprintf(stderr, "advise %d error: %d : %d\n", advise, ret, errno);
    fail = 1;
    goto end;
  }

  do {

    len = read(fd, buffer, sizeof(buffer));
    if (len == 0) {
      break;
    } else if (len == -1) {
      fprintf(stderr, "read error: %d\n", errno);
      fail = 1;
      break;
    }

    for(i = 0; i < (size_t) (len / sizeof(buffer[0])); i ++) {
      d += buffer[i];
    }
  } while(1);


 end:

  /* no more needed, release from memory */
  ret = posix_fadvise(fd, 0, 0, POSIX_FADV_DONTNEED);
  if (ret != 0) {
    fprintf(stderr, "advise %d error: %d : %d\n", POSIX_FADV_DONTNEED, ret, errno);
  }

  close(fd);

  clock_gettime(CLOCK_REALTIME, &end);

  timespec_substract(&duration, &end, &start);

  printf("%ld.%09ld%s\n",
	 duration.tv_sec, duration.tv_nsec, (fail) ? " failed" : "");
    
  return 0;
}

#define PAGESIZE 4096

/*
 * Test fadvise() on file descriptor, mmap() the whole file
 * then issue a madvise() for the whole file
 *
 */
static int
test_madvise1(const char *name, int advise_f, int advise_m)
{
  int ret;
  int fd;
  ssize_t len;

  struct timespec start;
  struct timespec end;
  struct timespec duration;

  int fail = 0;

  struct stat st;
  off_t size;
  off_t mod;
  off_t i, j;

  uint32_t *base;
  uint32_t *ptr;;

  static uint32_t d = 0;
 
  clock_gettime(CLOCK_REALTIME, &start);

  fd = open(name, O_RDONLY);
  if (fd == -1) {
    fprintf(stderr, "open error: %d\n", errno);
    return -1;
  }

  ret = fstat(fd, &st);
  if (ret != 0) {
    fprintf(stderr, "get size error: %d : %d\n", ret, errno);
    fail = 1;
    goto end;
  }

  ret = posix_fadvise(fd, 0, 0, advise_f);
  if (ret != 0) {
    fprintf(stderr, "fadvise %d error: %d : %d\n", advise_f, ret, errno);
    fail = 1;
    goto end;
  }

  size = st.st_size;

  mod = size % PAGESIZE;
  if (mod != 0) {
    size += PAGESIZE - mod;
  }

  base = (uint32_t *) mmap(NULL, size, PROT_READ, MAP_SHARED, fd, (off_t) 0);
  if (base == MAP_FAILED) {
    fprintf(stderr, "mmap failed:  %d\n", errno);
    fail = 1;
    goto end;
  }

  ret = posix_madvise(base, size, advise_m);
  if (ret != 0) {
    fprintf(stderr, "madvise %d error: %d : %d\n", advise_m, ret, errno);
    fail = 1;
    goto end;
  }

  ptr = base;

  for(i = size; i != 0;) {
        
    if (i >= (off_t) sizeof(buffer)) {

      for(j = 0; j < (off_t) (sizeof(buffer) / sizeof(uint32_t)); j++) {
	d += ptr[j];
      }

      posix_madvise(ptr, sizeof(buffer), POSIX_MADV_DONTNEED);

      ptr += (sizeof(buffer) / 4);
      i -= sizeof(buffer);

    } else {

      for(j = 0; j < (off_t) (i / sizeof(uint32_t)); j++) {
	d += ptr[j];
      }

      posix_madvise(ptr, i, POSIX_MADV_DONTNEED);

      i = 0;
    }
  }

 end:

  /* no more needed, release from memory */

  ret = posix_madvise(base, size, POSIX_MADV_DONTNEED);
  if (ret != 0) {
    fprintf(stderr, "madvise %d error: %d : %d\n", POSIX_MADV_DONTNEED, ret, errno);
  }

  ret = munmap(base, size);
  if (ret != 0) {
    fprintf(stderr, "munmap() error: %d\n", errno);
  }

  ret = posix_fadvise(fd, 0, 0, POSIX_FADV_DONTNEED);
  if (ret != 0) {
    fprintf(stderr, "fadvise %d error: %d : %d\n", POSIX_FADV_DONTNEED, ret, errno);
  }

  close(fd);

  clock_gettime(CLOCK_REALTIME, &end);

  timespec_substract(&duration, &end, &start);

  printf("%ld.%09ld%s\n",
	 duration.tv_sec, duration.tv_nsec, (fail) ? " failed" : "");
    
  return 0;
}

/*
 * Same as previous, but unmap() the file as it is read,
 * so at the end, the file in fully unmapped.
 *
 */
static int
test_madvise2(const char *name, int advise_f, int advise_m)
{
  int ret;
  int fd;
  ssize_t len;

  struct timespec start;
  struct timespec end;
  struct timespec duration;

  int fail = 0;

  struct stat st;
  off_t size;
  off_t mod;
  off_t i, j;

  uint32_t *base;
  uint32_t *ptr;;

  static uint32_t d = 0;
 
  clock_gettime(CLOCK_REALTIME, &start);

  fd = open(name, O_RDONLY);
  if (fd == -1) {
    fprintf(stderr, "open error: %d\n", errno);
    return -1;
  }

  ret = fstat(fd, &st);
  if (ret != 0) {
    fprintf(stderr, "get size error: %d : %d\n", ret, errno);
    fail = 1;
    goto end;
  }

  ret = posix_fadvise(fd, 0, 0, advise_f);
  if (ret != 0) {
    fprintf(stderr, "fadvise %d error: %d : %d\n", advise_f, ret, errno);
    fail = 1;
    goto end;
  }

  size = st.st_size;

  mod = size % PAGESIZE;
  if (mod != 0) {
    size += PAGESIZE - mod;
  }

  base = (uint32_t *) mmap(NULL, size, PROT_READ, MAP_SHARED, fd, (off_t) 0);
  if (base == MAP_FAILED) {
    fprintf(stderr, "mmap failed:  %d\n", errno);
    fail = 1;
    goto end;
  }

  ret = posix_madvise(base, size, advise_m);
  if (ret != 0) {
    fprintf(stderr, "madvise %d error: %d : %d\n", advise_m, ret, errno);
    fail = 1;
    goto end;
  }

  ptr = base;

  for(i = size; i != 0;) {
        
    if (i >= (off_t) sizeof(buffer)) {

      for(j = 0; j < (off_t) (sizeof(buffer) / sizeof(uint32_t)); j++) {
	d += ptr[j];
      }

      posix_madvise(ptr, sizeof(buffer), POSIX_MADV_DONTNEED);
      munmap(ptr, sizeof(buffer));

      ptr += (sizeof(buffer) / 4);
      i -= sizeof(buffer);

    } else {

      for(j = 0; j < (off_t) (i / sizeof(uint32_t)); j++) {
	d += ptr[j];
      }

      posix_madvise(ptr, i, POSIX_MADV_DONTNEED);
      munmap(ptr, i);

      i = 0;
    }
  }

 end:

  /* no more needed, release from memory */

  ret = posix_fadvise(fd, 0, 0, POSIX_FADV_DONTNEED);
  if (ret != 0) {
    fprintf(stderr, "fadvise %d error: %d : %d\n", POSIX_FADV_DONTNEED, ret, errno);
  }

  close(fd);

  clock_gettime(CLOCK_REALTIME, &end);

  timespec_substract(&duration, &end, &start);

  printf("%ld.%09ld%s\n",
	 duration.tv_sec, duration.tv_nsec, (fail) ? " failed" : "");
    
  return 0;
}

/*
 * mmap() the file as it is read
 * never more than sizeof(buffer) is mmap()'ed in memory
 *
 */
static int
test_madvise3(const char *name, int advise_f, int advise_m)
{
  int ret;
  int fd;
  ssize_t len;

  struct timespec start;
  struct timespec end;
  struct timespec duration;

  int fail = 0;

  struct stat st;
  off_t size;
  off_t mod;
  off_t offset;
  off_t mapsize;
  off_t i, j;

  uint32_t *ptr;;

  static uint32_t d = 0;
 
  clock_gettime(CLOCK_REALTIME, &start);

  fd = open(name, O_RDONLY);
  if (fd == -1) {
    fprintf(stderr, "open error: %d\n", errno);
    return -1;
  }

  ret = fstat(fd, &st);
  if (ret != 0) {
    fprintf(stderr, "get size error: %d : %d\n", ret, errno);
    fail = 1;
    goto end;
  }

  ret = posix_fadvise(fd, 0, 0, advise_f);
  if (ret != 0) {
    fprintf(stderr, "fadvise %d error: %d : %d\n", advise_f, ret, errno);
    fail = 1;
    goto end;
  }

  size = st.st_size;

  mod = size % PAGESIZE;
  if (mod != 0) {
    size += PAGESIZE - mod;
  }

  offset = 0;

  for(i = size; i != 0;) {

    if (i >= (off_t) sizeof(buffer)) {
      mapsize = (off_t) sizeof(buffer);
    } else {
      mapsize = i;
    }

    ptr = (uint32_t *) mmap(NULL, mapsize, PROT_READ, MAP_SHARED, fd, (off_t) offset);
    if (ptr == MAP_FAILED) {
      fprintf(stderr, "mmap failed:  %d\n", errno);
      fail = 1;
      goto end;
    }

    ret = posix_madvise(ptr, mapsize, advise_m);
    if (ret != 0) {
      fprintf(stderr, "madvise %d error: %d : %d\n", advise_m, ret, errno);
      fail = 1;
      goto end;
    }

    for(j = 0; j < (off_t) (mapsize / sizeof(uint32_t)); j++) {
      d += ptr[j];
    }

    posix_madvise(ptr, mapsize, POSIX_MADV_DONTNEED);
    munmap(ptr, mapsize);

    offset += mapsize;
    i -= mapsize;

  }

 end:

  /* no more needed, release from memory */

  ret = posix_fadvise(fd, 0, 0, POSIX_FADV_DONTNEED);
  if (ret != 0) {
    fprintf(stderr, "fadvise %d error: %d : %d\n", POSIX_FADV_DONTNEED, ret, errno);
  }

  close(fd);

  clock_gettime(CLOCK_REALTIME, &end);

  timespec_substract(&duration, &end, &start);

  printf("%ld.%09ld%s\n",
	 duration.tv_sec, duration.tv_nsec, (fail) ? " failed" : "");
    
  return 0;
}


int
main(int argc, char **argv)
{
#if 0

  /*
99.671760544
82.305618923
139.639477956
66.608155738
  */

  test_fadvise(argv[1], POSIX_FADV_NORMAL);
  test_fadvise(argv[1], POSIX_FADV_SEQUENTIAL);
  test_fadvise(argv[1], POSIX_FADV_RANDOM);
  test_fadvise(argv[1], POSIX_FADV_NOREUSE);

  printf("----\n");

  /*
108.582024465
82.907224872
221.087658511
  */
  test_madvise1(argv[1], POSIX_FADV_NORMAL, POSIX_MADV_NORMAL);
  test_madvise1(argv[1], POSIX_FADV_SEQUENTIAL, POSIX_MADV_SEQUENTIAL);
  test_madvise1(argv[1], POSIX_FADV_RANDOM, POSIX_MADV_RANDOM);

  printf("----\n");

  /*
173.483344462
127.504240578
  */
  test_madvise2(argv[1], POSIX_FADV_NORMAL, POSIX_MADV_NORMAL);
  test_madvise2(argv[1], POSIX_FADV_SEQUENTIAL, POSIX_MADV_SEQUENTIAL);

  printf("----\n");

  /*advise 7 error: 22 : 0
0.027454009 failed
  */

  /* will fail */
  test_fadvise(argv[1], POSIX_FADV_SEQUENTIAL | POSIX_FADV_NOREUSE);




#else

  /* do it twice, just to check for caching issues */
  test_fadvise(argv[1], POSIX_FADV_SEQUENTIAL);
  test_fadvise(argv[1], POSIX_FADV_SEQUENTIAL);

  test_madvise2(argv[1], POSIX_FADV_SEQUENTIAL, POSIX_MADV_SEQUENTIAL);

  test_madvise3(argv[1], POSIX_FADV_SEQUENTIAL, POSIX_MADV_SEQUENTIAL);

#endif

  return 0;
}
