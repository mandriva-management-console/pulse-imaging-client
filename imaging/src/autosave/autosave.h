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

#define swab64(x) \
({ \
        __u64 __x = (x); \
        ((__u64)( \
                (__u64)(((__u64)(__x) & (__u64)0x00000000000000ffULL) << 56) | \
                (__u64)(((__u64)(__x) & (__u64)0x000000000000ff00ULL) << 40) | \
                (__u64)(((__u64)(__x) & (__u64)0x0000000000ff0000ULL) << 24) | \
                (__u64)(((__u64)(__x) & (__u64)0x00000000ff000000ULL) <<  8) | \
                (__u64)(((__u64)(__x) & (__u64)0x000000ff00000000ULL) >>  8) | \
                (__u64)(((__u64)(__x) & (__u64)0x0000ff0000000000ULL) >> 24) | \
                (__u64)(((__u64)(__x) & (__u64)0x00ff000000000000ULL) >> 40) | \
                (__u64)(((__u64)(__x) & (__u64)0xff00000000000000ULL) >> 56) )); \
 \
})
