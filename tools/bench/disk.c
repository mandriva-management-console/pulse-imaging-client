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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <linux/fs.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include "main.h"


void
disk_bench (int run)
{
  char *disks[] = { "/dev/hda", "/dev/sda", NULL };
  char *dev = NULL;
  double t1, t2;
  int i, fd = -1, megs;
  char buf[2048];

  /* open our test device */
  for (i=0; disks[i] != NULL; i++) {
    if ((fd = open(disks[i], O_RDONLY)) != -1) {
      dev = disks[i];
      break;
    }
  }

  system("/bin/sync");
  sleep(2);

  t1 = now();
  megs = 128*run;
  for(i=0; i < 1024/2*megs; i++) {
    if (read(fd, buf, 2048) != 2048) {
      printf("Read error\n");
    }
  }
  close(fd);
  t2 = now();
  printf("%dMB read in %f s from %s = %f MB/s\n", megs, t2-t1, dev, (double)megs/(t2-t1));
}

