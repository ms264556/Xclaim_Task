#############################################################
#
# dm
#
#############################################################
# Copyright (C) 2005 by Richard Downer <rdowner@gmail.com>
# Derived from work
# Copyright (C) 2001-2005 by Erik Andersen <andersen@codepoet.org>
# Copyright (C) 2002 by Tim Riker <Tim@Rikers.org>
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU Library General Public License as
# published by the Free Software Foundation; either version 2 of the
# License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
# Library General Public License for more details.
#
# You should have received a copy of the GNU Library General Public
# License along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
# USA

DM_BASEVER=1.00
DM_PATCH=21
DM_VERSION=$(DM_BASEVER).$(DM_PATCH)
DM_SOURCE:=device-mapper.$(DM_VERSION).tgz
DM_SITE:=ftp://sources.redhat.com/pub/dm
DM_CAT:=zcat
DM_DIR:=$(BUILD_DIR)/device-mapper.$(DM_VERSION)
DM_STAGING_BINARY:=$(STAGING_DIR)/sbin/dmsetup
DM_TARGET_BINARY:=$(TARGET_DIR)/usr/sbin/dmsetup
DM_STAGING_LIBRARY:=$(STAGING_DIR)/lib/libdevmapper.so
DM_TARGET_LIBRARY:=$(TARGET_DIR)/usr/lib/libdevmapper.so

$(DL_DIR)/$(DM_SOURCE):
	 $(WGET) -P $(DL_DIR) $(DM_SITE)/$(DM_SOURCE)

$(DM_DIR)/.unpacked: $(DL_DIR)/$(DM_SOURCE)
	$(DM_CAT) $(DL_DIR)/$(DM_SOURCE) | tar -C $(BUILD_DIR) $(TAR_OPTIONS) -
	touch $(DM_DIR)/.unpacked

$(DM_DIR)/.configured: $(DM_DIR)/.unpacked
	(cd $(DM_DIR); rm -rf config.cache; \
		$(TARGET_CONFIGURE_OPTS) \
		CFLAGS="$(TARGET_CFLAGS)" \
		./configure \
		--target=$(GNU_TARGET_NAME) \
		--host=$(GNU_TARGET_NAME) \
		--build=$(GNU_HOST_NAME) \
		--prefix=/usr \
		$(DISABLE_NLS) \
		$(DISABLE_LARGEFILE) \
		--with-kernel-dir=/work/richard/xen/linux-2.6.10-xen0 \
		--with-user=$(shell id -un) --with-group=$(shell id -gn) \
	);
	touch  $(DM_DIR)/.configured

$(DM_DIR)/$(DM_BINARY): dm-build
$(DM_DIR)/$(DM_LIBRARY): dm-build

$(DM_STAGING_BINARY) $(DM_STAGING_LIBRARY): $(DM_DIR)/.configured
	$(MAKE) CC=$(TARGET_CC) -C $(DM_DIR)
	$(MAKE) -C $(DM_DIR) install prefix=$(STAGING_DIR)

# Install dmsetup from staging to target
$(DM_TARGET_BINARY): $(DM_STAGING_BINARY)
	$(INSTALL) -m 0755 $? $@

# Install libdevmapper.so.1.00 from staging to target
$(DM_TARGET_LIBRARY).$(DM_BASEVER): $(DM_STAGING_LIBRARY)
	$(INSTALL) -m 0644 $? $@

# Makes libdevmapper.so a symlink to libdevmapper.so.1.00
$(DM_TARGET_LIBRARY): $(DM_TARGET_LIBRARY).$(DM_BASEVER)
	rm $@
	ln -s $(<F) $@

dm: uclibc $(DM_TARGET_BINARY) $(DM_TARGET_LIBRARY)

dm-clean:
	rm $(DM_TARGET_BINARY) $(DM_TARGET_LIBRARY) $(DM_TARGET_LIBRARY).$(DM_BASEVER)
	$(MAKE) -C $(DM_DIR) clean

dm-dirclean:
	rm -rf $(DM_DIR)

#############################################################
#
# Toplevel Makefile options
#
#############################################################
ifeq ($(strip $(BR2_PACKAGE_DM)),y)
TARGETS+=dm
endif
