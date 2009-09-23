# (c) 2003-2007 Linbox FAS, http://linbox.com
# (c) 2008-2009 Mandriva, http://www.mandriva.com
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

CFLAGS=-DLBLIMAGEVER=\"2.0\" -O2 -D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64 -Wall -I.. -I../3rd_party/newt/newt-0.52.10 -I../3rd_party/slang/slang-2.0.6/src/elfobjs
#LIBS=./libnewt.a -lslang
LIBS=-s -lnewt -lslang
CC=gcc-3.3

UI_OBJS=../server.o ui_newt.o ../easy_sock.o
#ZLIBOBJ=$(wildcard ../zlib/*.o)

all: uinewt

uinewt: $(UI_OBJS) Makefile
	$(CC) $(CFLAGS) -o $@ $(UI_OBJS) $(LIBS)

clean:
	rm ui_newt.o
#image_raw: image_raw.o client.o easy_sock.o compress.o $(ZLIBOBJ)
#	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ -L..-I../zlib

