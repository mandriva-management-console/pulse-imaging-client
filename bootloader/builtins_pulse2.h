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
#ifndef __BUILTINS_PULSE__
#define __BUILTINS_PULSE__
/* functions */
void drive_info(unsigned char *buffer);
void cpuinfo(void);
int cpuspeed(void);


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
int kbdfr_func1();
unsigned char* smbios_get(int rtype, unsigned char ** rnext);
/* macros */
#define MIN(X,Y) ((X) < (Y) ? (X) : (Y))

  #ifdef DEBUG
      typedef struct smbios_struct
      {
	unsigned char  type                           ;
	unsigned char  length                         ;
	unsigned short handle                        ; 
	unsigned char  subtype                       ;
	      /* ... other fields are structure dependend ... */
      } __attribute__ ((packed)) smbios_struct ;
      int             smbios_get_struct_length (smbios_struct * struct_ptr);
      smbios_struct * smbios_next_struct (smbios_struct * struct_ptr);
      void typestructure(smbios_struct *base,int nbmaxstructure);
      void affiche_table_smbios_hexa(unsigned char type, char* buffer);
  #endif
#endif
