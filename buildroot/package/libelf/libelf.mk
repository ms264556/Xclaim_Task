#############################################################
#
# libelf
#
#############################################################
LIBELF_VER=0.8.5
LIBELF_SOURCE=libelf-$(LIBELF_VER).tar.gz
LIBELF_SITE=http://www.mr511.de/software/
LIBELF_DIR=$(BUILD_DIR)/libelf-$(LIBELF_VER)

LIBELF_ARCH:=$(ARCH)
ifeq ("$(strip $(ARCH))","armeb")
LIBELF_ARCH:=arm
endif

$(DL_DIR)/$(LIBELF_SOURCE):
	$(WGET) -P $(DL_DIR) $(LIBELF_SITE)/$(LIBELF_SOURCE)

$(LIBELF_DIR)/.source: $(DL_DIR)/$(LIBELF_SOURCE)
	zcat $(DL_DIR)/$(LIBELF_SOURCE) | tar -C $(BUILD_DIR) $(TAR_OPTIONS) -
	toolchain/patch-kernel.sh $(LIBELF_DIR) package/libelf libelf\*.patch
	touch $(LIBELF_DIR)/.source

$(LIBELF_DIR)/.configured: $(LIBELF_DIR)/.source
	(cd $(LIBELF_DIR); \
		$(TARGET_CONFIGURE_OPTS) \
		./configure \
		--target=$(GNU_TARGET_NAME) \
		--host=$(GNU_TARGET_NAME) \
		--build=$(GNU_HOST_NAME) \
		--prefix=/usr \
		--sysconfdir=/etc \
		$(DISABLE_NLS) \
		--enable-shared \
	);
	touch $(LIBELF_DIR)/.configured;

$(LIBELF_DIR)/libelf.so.$(LIBELF_VER): $(LIBELF_DIR)/.configured
	$(MAKE) $(TARGET_CONFIGURE_OPTS) -C $(LIBELF_DIR)

$(STAGING_DIR)/usr/lib/libelf.a: $(LIBELF_DIR)/libelf.so.$(LIBELF_VER)
	$(MAKE1) $(TARGET_CONFIGURE_OPTS) ${INSTALL}="install -D" \
		instroot=$(STAGING_DIR) -C $(LIBELF_DIR) install

$(TARGET_DIR)/usr/lib/libelf.so.$(LIBELF_VER): $(STAGING_DIR)/usr/lib/libelf.a
	$(INSTALL) -m 0644 $(STAGING_DIR)/usr/lib/libelf.so.$(LIBELF_VER) $(TARGET_DIR)/usr/lib/
	$(INSTALL) -m 0644 $(STAGING_DIR)/usr/lib/libelf.so.0 $(TARGET_DIR)/usr/lib/

libelf: uclibc $(TARGET_DIR)/usr/lib/libelf.so.$(LIBELF_VER)

libelf-source: $(DL_DIR)/$(LIBELF_SOURCE)

libelf-clean:
	$(MAKE) prefix=$(TARGET_DIR)/usr -C $(LIBELF_DIR) uninstall
	-$(MAKE) -C $(LIBELF_DIR) clean

libelf-dirclean:
	rm -rf $(LIBELF_DIR)

#############################################################
#
# Toplevel Makefile options
#
#############################################################
ifeq ($(strip $(BR2_PACKAGE_LIBELF)),y)
TARGETS+=libelf
endif
