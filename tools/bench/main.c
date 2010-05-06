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
#include <time.h>
#include <sys/time.h>

#include "main.h"

/* global vars */
unsigned char buf[80];
unsigned char command[120];
unsigned char servip[40] = "";
unsigned char servprefix[80] = "";
unsigned char storagedir[80] = "";
char hostname[32] = "";

/* return current time
 */
double now(void)
{
  struct timeval tv;

  gettimeofday(&tv, NULL);

  return ((double)tv.tv_sec + (double)(tv.tv_usec/1000000.0));

  // return (clock()/(double)CLOCKS_PER_SEC);
}


/*
 */
unsigned char *find(const char *str, const char *fname)
{
    FILE *f;

    f = fopen(fname, "r");
    if (f == NULL)
        return NULL;
    while (fgets(buf, 80, f)) {
        if (strstr(buf, str)) {
            fclose(f);
            return strstr(buf, str) + strlen(str);
        }
    }
    fclose(f);
    return NULL;
}

/*
 * Get NFS server informations
 */
void netinfo(void)
{
    unsigned char *ptr, *ptr2;

    if ((ptr = find("Next server: ", "/etc/netinfo.log"))) {
        ptr2 = ptr;
        while (*ptr2) {
            if (*ptr2 < ' ') {
                *ptr2 = 0;
                break;
            } else
                ptr2++;
        }
        //printf ("*%s*\n", ptr);
        strcpy(servip, ptr);
    }

    if ((ptr = find("Boot file: ", "/etc/netinfo.log"))) {
        ptr2 = strstr(ptr, "/bin");
        if (ptr2)
            *ptr2 = 0;
        //printf ("*%s*\n", ptr);
        strcpy(servprefix, ptr);
    }
}

/*
 * Get the LBS host name
 */
void gethost(void)
{
    FILE *f;

    hostname[0] = 0;

    f = fopen("/etc/lbxname", "r");
    if (f != NULL) {
        fscanf(f, "%31s", hostname);
        fclose(f);
    }

    f = fopen("/revoinfo/hostname", "r");
    if (f == NULL)
        return ;
    fscanf(f, "%31s", hostname);
    fclose(f);
}

void commandline(void)
{
    unsigned char *ptr, *ptr2;

    if ((ptr = find("revosavedir=", "/etc/cmdline"))) {
        ptr2 = ptr;
        while (*ptr2 != ' ')
            ptr2++;
        *ptr2 = 0;
        //printf ("*%s*\n", ptr);
        strcpy(storagedir, ptr);
    }
}


int main (int argc, char *argv[])
{
  int run;
  char command[256];

  /* mount storage dir */
  netinfo();
  commandline();

  if (argc < 2) {
    do {
      sprintf(command,
              "/bin/autosave.sh \"%s\" \"%s\" \"%s\" %d",
              servip, servprefix, storagedir, 8192);
      printf("Mounting Storage directory...%s\n", command);
    }
    while (system(command) != 0);
  }

  /* low lovel bench */
  for (run = 1; run < 4; run ++) {
    printf("=== Low level bench #%d\n", run);
    compress_bench(run);
    disk_bench(run);
    nfs_bench(run);
  }
  /* ping */
  printf("=== Ping test\n");
  system("/bin/bench.ping");

  return 0;
}
