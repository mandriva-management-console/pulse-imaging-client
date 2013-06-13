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

/* functions */
void drive_info(unsigned char *buffer);
void cpuinfo(void);
int cpuspeed(void);
void delay_func(int delay,const char *label );
int smbios_init(void);
void smbios_get_sysinfo(char **p1, char **p2, char **p3, char **p4, char **p5);

int inc_func(char *arg, int flags);
int setdefault_func(char *arg, int flags);
int nosecurity_func(char *arg, int flags);
int partcopy_func(char *arg, int flags);
int ptabs_func(char *arg, int flags);
int test_func(char *arg, int flags);
int identify_func(char *arg, int flags);
int identifyauto_func(char *arg, int flags);
int kbdfr_func(char *arg, int flags);

/* macros */
#define MIN(X,Y) ((X) < (Y) ? (X) : (Y))


