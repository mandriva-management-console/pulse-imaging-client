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
void drive_info (unsigned char *buffer);
void cpuinfo (void);
int cpuspeed (void);

int smbios_init (void);
void smbios_get_sysinfo(char **p1, char **p2, char **p3, char **p4, char **p5);

int inc_func (char *arg, int flags);
int setdefault_func (char *arg, int flags);
int nosecurity_func (char *arg, int flags);
int partcopy_func (char *arg, int flags);
int ptabs_func (char *arg, int flags);
int test_func (char *arg, int flags);
int identify_func (char *arg, int flags);
int identifyauto_func (char *arg, int flags);
int kbdfr_func (char *arg, int flags);
int nop_func (char *arg, int flags);

/* macros */
#define MIN(X,Y) ((X) < (Y) ? (X) : (Y))

/* variables */

/* grub builtins */

static struct builtin builtin_inc = {
  "inc",
  inc_func,
  BUILTIN_CMDLINE,
#ifdef HELP_ON
  "inc [MESSAGE ...]",
  "increment a Pulse 2 backup number."
#endif
};

static struct builtin builtin_setdefault = {
  "setdefault",
  setdefault_func,
  BUILTIN_CMDLINE,
#ifdef HELP_ON
  "setdefault number",
  "set the default grub menu entry for the next reboot."
#endif
};

static struct builtin builtin_nosecurity = {
  "nosecurity",
  nosecurity_func,
  BUILTIN_CMDLINE | BUILTIN_MENU,
#ifdef HELP_ON
  "nosecurity",
  "allow access to grub cmdline."
#endif
};

static struct builtin builtin_partcopy = {
  "partcopy",
  partcopy_func,
  BUILTIN_CMDLINE | BUILTIN_MENU,
#ifdef HELP_ON
  "partcopy START PREFIXNAME [TYPE]",
  "Create a primary partition at the starting address START with the"
    " compressed files beginning with PREFIXNAME. Update partition table "
    "with the partition TYPE type."
#endif
};

static struct builtin builtin_ptabs = {
  "ptabs",
  ptabs_func,
  BUILTIN_CMDLINE,
#ifdef HELP_ON
  "ptabs DISK FILE",
  "Copy uncompressed sectors from FILE (LBA,DATA) to disk DISK."
#endif
};

static struct builtin builtin_identify = {
  "identify",
  identify_func,
  BUILTIN_CMDLINE,
#ifdef HELP_ON
  "identify",
  "Ask for Name/ID and send UDP packet to server."
#endif
};

static struct builtin builtin_identifyauto = {
  "identifyauto",
  identifyauto_func,
  BUILTIN_CMDLINE,
#ifdef HELP_ON
  "identifyauto",
  "Send an ID packet to the server with Name/ID=+/+"
#endif
};

static struct builtin builtin_kbdfr = {
  "kbdfr",
  kbdfr_func,
  BUILTIN_CMDLINE | BUILTIN_MENU,
#ifdef HELP_ON
  "kbdfr",
  "azerty keymap"
#endif
};

static struct builtin builtin_nop = {
  "nop",
  nop_func,
  BUILTIN_MENU,
#ifdef HELP_ON
  "nop",
  "Do nothing"
#endif
};

