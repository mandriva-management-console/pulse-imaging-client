/*
 *  GRUB  --  GRand Unified Bootloader
 *  Copyright (C) 1996   Erich Boleyn  <erich@uruk.org>
 *  Copyright (C) 2000, 2001   Free Software Foundation, Inc.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifndef	_DEFFUNC_H_
#define	_DEFFUNC_H_

int translate_keycode(int c);
void smbios_get_biosinfo(char **p1, char **p2, char **p3);
void smbios_get_enclosure(char **p1, char **p2);
int smbios_get_memory(int *size, int *form, char **location, int *type,int *speed);
int smbios_get_numcpu(void);
void delay_func(int delay,const char *label );
int set_master_password_func(char *arg, int flags);
void nocursor ();
#endif

