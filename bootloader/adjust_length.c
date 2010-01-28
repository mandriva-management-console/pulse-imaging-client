/*
 * (c) 2003-2007 Ludovic Drolez, Linbox FAS, http://linbox.com
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

#include <stdlib.h>
#include <stdio.h>

int main(int argc, char *argv[]) {
    unsigned char command[132];
    unsigned long lg;
    FILE *fi;

    system("ls -l revoboot.pxe | awk '{print $5}' > pxe.size");

    fi = fopen("pxe.size", "rt");
    fscanf(fi, "%ld", &lg);
    fclose(fi);

    system("rm pxe.size");

    printf("File length : %ld\n", lg);
    lg -= 512;

    fi = fopen("revoboot.pxe", "r+b");
    fseek(fi, 32, SEEK_SET);
    fwrite(&lg, 4, 1, fi);
    fwrite(&lg, 4, 1, fi);
    fclose(fi);
}
