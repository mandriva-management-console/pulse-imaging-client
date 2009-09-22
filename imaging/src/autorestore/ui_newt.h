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

#include <errno.h>

#define TOSTRING(x) #x
#define debug(...) fprintf(stderr, __VA_ARGS__)
#define UI_READ_ERROR ui_read_error(__FILE__,__LINE__, errno, fi)
#define UI_READ_ERROR2 ui_read_error(__FILE__,__LINE__, errno, 0)

void init_newt(char *, char *, char *, char *, int);
void close_newt(void);

void update_progress(int);
void update_file(char *f, int, int, char *dev, int);
void stats(void);
void update_head(char *msg);
void read_update_head(void);

void ui_write_error(char *s, int l, int err, int fd);
void ui_zlib_error(int err);
