# (c) 2003-2007 Linbox FAS, http://linbox.com
# (c) 2008-2011 Mandriva, http://www.mandriva.com
#
# $Id$
#
# This file is part of Pulse 2, http://pulse2.mandriva.org
#
# Pulse 2 is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# Pulse 2 is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with Pulse 2; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
# MA 02110-1301, USA.
topdir = $(abspath ../..)
include $(topdir)/common.mk
include $(topdir)/extra.mk

bin_revo = image_e2fs image_fat image_jfs image_lvm image_lvmreiserfs image_ntfs image_raw image_swap image_xfs
script_revo = image_ufs
bin_all = $(bin_revo) decompress decompress-test decompress-bitmap
obj_all = $(patsubst %,%.o,$(bin_all))

liblrs_so = liblrs.so
liblrs_DEPS = compress.o easy_sock.o client.o

c_sources = $(wildcard *.c)

CFLAGS	= -Wall -O2 -DLBLIMAGEVER='1.2'
LDFLAGS	= 

lrs_LDFLAGS = -L. -llrs

z_LDFLAGS = -lz

reiserfsbasedir = $(topdir)/tools/reiserfsprogs
reiserfssrcdir = $(reiserfsbasedir)/$(extra_srcdir)
reiserfs_LDFLAGS = -L$(reiserfssrcdir)/reiserfscore -lcore -L$(reiserfssrcdir)/lib -lmisc
reiserfs_CPPFLAGS = -I$(reiserfssrcdir)

e2fs_LDFLAGS	= -lext2fs -lcom_err

CLEANFILES = $(bin_all) $(obj_all) lvm.o liblrs.so*

#
# XXX in order to build a shared library, 
# XXX flags -DPIC -fPIC should be used
#

all-local: $(liblrs_so) $(bin_all)

install-local:
	mkdir -p $(revobindir)
	for file in $(bin_revo) $(script_revo); do \
	  cp -a $$file $(revobindir)/; \
	done
	mkdir -p $(libdir)
	cp -a liblrs.so liblrs.so.1 $(libdir)/

image_e2fs: image_e2fs.o $(liblrs_so)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(lrs_LDFLAGS) $(e2fs_LDFLAGS)

image_fat: image_fat.o $(liblrs_so)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(lrs_LDFLAGS)

image_jfs: image_jfs.o $(liblrs_so)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(lrs_LDFLAGS)

image_lvm: image_lvm.o lvm.o $(liblrs_so)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(lrs_LDFLAGS)

image_lvmreiserfs: lvm.o image_lvmreiserfs.o $(reiserfssrcdir)/lib/misc.o $(liblrs_so)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(lrs_LDFLAGS) $(reiserfs_LDFLAGS)

image_ntfs: image_ntfs.o $(liblrs_so)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(lrs_LDFLAGS)

image_raw: image_raw.o $(liblrs_so)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(lrs_LDFLAGS)

image_swap: image_swap.o $(liblrs_so)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(lrs_LDFLAGS)

image_xfs: image_xfs.o $(liblrs_so)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(lrs_LDFLAGS)

decompress: decompress.o $(liblrs_so)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(lrs_LDFLAGS) $(z_LDFLAGS)

decompress-test: decompress-test.o compress.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $< $(z_LDFLAGS)

decompress-bitmap: decompress-bitmap.o compress.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $< $(z_LDFLAGS)

image_lvmreiserfs.o: image_lvmreiserfs.c $(reiserfssrcdir)/lib/misc.o
	$(CC) $(CFLAGS) $(reiserfs_CPPFLAGS) -o $@ -c $<

liblrs.so.1: $(liblrs_DEPS)
	$(CC) -shared -Wl,-soname,$@ -o $@ $^ $(z_LDFLAGS)

liblrs.so: liblrs.so.1
	ln -sf $< $@

liblrs.a: $(liblrs_DEPS)
	ar rc $@ $^ $(liblrs_DEPS)
	ranlib $@

$(reiserfssrcdir)/lib/misc.o:
	$(MAKE) -C $(reiserfsbasedir)

.depend: $(c_sources)
	$(CC) -M $(CFLAGS) *.c > .depend

include .depend

.PHONY += lib bin
