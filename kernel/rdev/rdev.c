/*
 * set root device, stolen from util-linux-ng 2.17.2
 *
 * (c) 2003-2007 Ludovic Drolez, Linbox FAS, http://linbox.com
 * (c) 2008-2009 Nicolas Rueff, Mandriva, http://www.mandriva.com
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
#include <fcntl.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>

#define DEFAULT_OFFSET 508

static void die(char *msg) {
    perror(msg);
    exit(1);
}

int main(int argc, char **argv) {
    int image, offset = DEFAULT_OFFSET;
    unsigned short val;
    struct stat s;

    if (stat(argv[2], &s) < 0)
        die(argv[2]);
    val = s.st_rdev;

    if ((image = open(argv[1],O_WRONLY)) < 0)
        die(argv[1]);
    if (lseek(image,offset,0) < 0)
        die("lseek");
    if (write(image,(char *)&val,2) != 2)
        die(argv[1]);
    if (close(image) < 0)
        die("close");

    return 0;
}
