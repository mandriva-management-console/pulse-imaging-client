/* shared.h - definitions used in all GRUB-specific code */
/*
 *  GRUB  --  GRand Unified Bootloader
 *  Copyright (C) 1996  Erich Boleyn  <erich@uruk.org>
 *  Copyright (C) 1999, 2000 Free Software Foundation, Inc.
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

/*
 *  Generic defines to use anywhere
 */

#ifndef ASM_FILE

/* Sets text mode character attribute at the cursor position.  See A_*
   constants defined above. */
void set_attrib (int attr);

extern char basedir[];

extern char *linux_data_real_addr;

/* This variable specifies which console should be used.  */
extern int terminal;

/* Color settings */
extern int normal_color, highlight_color;

extern unsigned long boot_part_offset;

extern unsigned short pxe_pointer_offset;
extern unsigned short pxe_pointer_segment;
extern unsigned short pxe_stack_offset;
extern unsigned short pxe_stack_segment;
extern unsigned short pxe_int1a_offset;
extern unsigned short pxe_int1a_segment;

/* Control the auto fill mode.  */
extern int auto_fill;

// Moved to zfunc.c
static inline void *zmemcpy(void *dest,const void *src,int len)
{
    int d0,d1,d2;
     
    asm volatile ("cld\n\t"
                  "rep\n\t"
                  "movsb"
                : "=&c" (d0), "=&S" (d1), "=&D" (d2)
                : "0" (len),"1" (src),"2" (dest)
                : "memory");
    return dest;
}
#define fast_memmove(a,b,c) zmemcpy(a,b,c)

#endif

#define BUILTIN_DESC            0x10    /* Description Tag */

/* The size of the key map.  */
#undef KEY_MAP_SIZE
#define KEY_MAP_SIZE		256

#define TERMINAL_CONSOLE	(1 << 0)	/* keyboard and screen */
#define TERMINAL_SERIAL		(1 << 1)	/* serial console */
#define TERMINAL_DUMB		(1 << 16)	/* dumb terminal */

