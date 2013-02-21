#
# (c) 2009-2011 Mandriva, http://www.mandriva.com
#
# Author(s):
#  Nicolas Rueff
#  Jean Parpaillon
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
# along with Pulse 2. If not, see <http://www.gnu.org/licenses/>.
#
SUBDIRS = kernel initrd tools bootloader eltorito postinstall

topdir = $(abspath .)
include $(topdir)/common.mk

install-local:
	mkdir -p $(DESTDIR)$(bootloaderdir) $(DESTDIR)$(computersdir) $(DESTDIR)$(mastersdir)
	$(install_DATA) contrib/bootsplash/bootsplash.xpm \
	  $(DESTDIR)$(bootloaderdir)/bootsplash.xpm
	@echo "###"
	@echo "### WARNING: this is not finished !"
	@echo "###"
	@echo "### Run '$(imaginglibdir)/update-initrd all' as root to build initrd images."
	@echo "###"

install-nobuild:
	$(MAKE) install nobuild=1

# Some sanity check in initrd dirs
initcheck:
	# Check modules
	@cat $(initramfsdir)/etc/modules | grep ^[a-z1-9] | while read mod; do \
	  if test ! -e "$(initramfsdir)/lib/modules/$$mod.ko"; then \
	    echo "Missing module in $(initramfsdir)/lib/modules: $$mod.ko"; \
	    exit 1; \
	  fi; \
	done

	# Check binaries are correctly linked
	@rootinitrd='chroot "$(initramfsdir)"'; \
	find "$(initramfsdir)" -type f | while read bin; do \
	  relbin=`echo "$$bin" | sed -e "s|$(initramfsdir)||"`; \
	  if $(rootinitrd) /lib/ld-linux.so.2 --verify "$$relbin" >/dev/null; then \
	    if ! $(rootinitrd) /lib/ld-linux.so.2 --list "$$relbin" >/dev/null; then \
	      echo "Missing libraries for $$relbin"; \
	      exit 1; \
	    fi; \
	  fi; \
	done

dist: distdir
	rm -f $(archivebase).tar.gz
	tar cf - $(archivebase) | gzip -c > $(archivebase).tar.gz

# Test we can:
#  install from archivebase
#  create archive from archivebase
distcheck: dist
	mkdir -p $(archivebase)/_inst
	$(MAKE) -C $(archivebase) install DESTDIR=$(topdir)/$(archivebase)/_inst
	$(MAKE) -C $(archivebase) dist
	@echo "###"
	@echo "### Your archive is checked and ready at:"
	@echo "###   $(archivebase).tar.gz"
	@echo "###"

distdir:
	rm -rf $(archivebase)
	mkdir -p $(archivebase)
	tar -c \
	  --exclude=\.svn --exclude=\.git --exclude=_inst \
	  --exclude=$(project)-\* --exclude=sources \
	  . | tar xC $(archivebase)
	$(MAKE) -C $(archivebase) distclean

binary: binarydir
	tar -c -f - -C $(binarybase) . | gzip -c > $(binarybase).tar.gz
	rm -fr $(binarybase)

binarydir: check-root
	rm -rf $(binarybase)
	mkdir -p $(binarybase)
	$(MAKE) install DESTDIR=$(topdir)/$(binarybase)
	# Modules' dependencies checks
	$(topdir)/kernel/check_module_deps.sh \
	  $(topdir)/$(binarybase)/$(initramfsdir)/etc/modules \
	  $(topdir)/$(binarybase)/$(initramfsdir)/lib/modules
	$(binarybase)$(imaginglibdir)/update-initrd all DESTDIR=$(topdir)/$(binarybase)

PHONY += initcheck dist distcheck distdir binarydir binary
