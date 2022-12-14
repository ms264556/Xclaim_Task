#############################################################
#
# build binutils for use on the host system
#
#############################################################
BINUTILS_VERSION:=$(strip $(subst ",, $(BR2_BINUTILS_VERSION)))

EXTRA_BINUTILS_CONFIG_OPTIONS:=$(strip $(subst ",, $(BR2_EXTRA_BINUTILS_CONFIG_OPTIONS)))
#"
BINUTILS_SITE:=ftp://ftp.kernel.org/pub/linux/devel/binutils
ifeq ($(BINUTILS_VERSION),2.16)
BINUTILS_SITE:=http://ftp.gnu.org/gnu/binutils/
endif
ifeq ($(BINUTILS_VERSION),2.16.1)
BINUTILS_SITE:=http://ftp.gnu.org/gnu/binutils/
endif
ifeq ($(BINUTILS_VERSION),2.15)
BINUTILS_SITE:=http://ftp.gnu.org/gnu/binutils/
endif
ifeq ($(BINUTILS_VERSION),2.14)
BINUTILS_SITE:=http://ftp.gnu.org/gnu/binutils/
endif
ifeq ($(BINUTILS_VERSION),2.13)
BINUTILS_SITE:=http://ftp.gnu.org/gnu/binutils/
endif
ifeq ($(BINUTILS_VERSION),2.15.97)
BINUTILS_SITE:=ftp://sources.redhat.com/pub/binutils/snapshots/
endif
ifeq ($(BINUTILS_VERSION),2.17)
BINUTILS_SITE:=http://ftp.gnu.org/gnu/binutils/
endif
ifeq ($(BINUTILS_VERSION),2.19)
BINUTILS_SITE:=http://ftp.gnu.org/gnu/binutils/
endif
ifeq ($(BINUTILS_VERSION),2.19.1)
BINUTILS_SITE:=http://ftp.gnu.org/gnu/binutils/
endif
ifeq ($(BINUTILS_VERSION),2.20)
BINUTILS_SITE:=http://ftp.gnu.org/gnu/binutils/
endif
ifeq ($(BINUTILS_VERSION),2.20.1)
BINUTILS_SITE:=http://ftp.gnu.org/gnu/binutils/
endif

BINUTILS_SOURCE:=binutils-$(BINUTILS_VERSION).tar.bz2
BINUTILS_DIR:=$(TOOL_BUILD_DIR)/binutils-$(BINUTILS_VERSION)
BINUTILS_CAT:=bzcat

BINUTILS_DIR1:=$(TOOL_BUILD_DIR)/binutils-$(BINUTILS_VERSION)-build

$(DL_DIR)/$(BINUTILS_SOURCE):
	mkdir -p $(DL_DIR)
	$(WGET) -P $(DL_DIR) $(BINUTILS_SITE)/$(BINUTILS_SOURCE)

binutils-unpacked: $(BINUTILS_DIR)/.unpacked
$(BINUTILS_DIR)/.unpacked: $(DL_DIR)/$(BINUTILS_SOURCE)
	mkdir -p $(TOOL_BUILD_DIR)
	$(BINUTILS_CAT) $(DL_DIR)/$(BINUTILS_SOURCE) | tar -C $(TOOL_BUILD_DIR) $(TAR_OPTIONS) -
	$(CONFIG_UPDATE) $(BINUTILS_DIR)
	touch $(BINUTILS_DIR)/.unpacked

$(BINUTILS_DIR)/.patched: $(BINUTILS_DIR)/.unpacked
	# Apply appropriate binutils patches.
	toolchain/patch-kernel.sh $(BINUTILS_DIR) toolchain/binutils/$(BINUTILS_VERSION) \*.patch
	# Fix to compile with newer makeinfo v5+
	sed -i -e 's/something like @colophon/something like @@colophon/' \
		-e 's/doc@cygnus.com/doc@@cygnus.com/' \
		$(BINUTILS_DIR)/bfd/doc/bfd.texinfo
ifeq ($(BINUTILS_VERSION),2.18)
	sed -i -e 's/@subsubsection/@@subsubsection/' \
		$(BINUTILS_DIR)/bfd/doc/elf.texi
endif
	touch $(BINUTILS_DIR)/.patched

$(BINUTILS_DIR1)/.configured: $(BINUTILS_DIR)/.patched
	mkdir -p $(BINUTILS_DIR1)
	(cd $(BINUTILS_DIR1); \
		CC="$(HOSTCC)" \
		$(BINUTILS_DIR)/configure \
		--prefix=$(STAGING_DIR) \
		--build=$(GNU_HOST_NAME) \
		--host=$(GNU_HOST_NAME) \
		--target=$(REAL_GNU_TARGET_NAME) \
		$(DISABLE_NLS) \
		$(MULTILIB) \
		$(OPENMP) \
		--disable-werror \
		$(BR2_CONFIGURE_STAGING_SYSROOT) \
		$(SOFT_FLOAT_CONFIG_OPTION) \
		$(EXTRA_BINUTILS_CONFIG_OPTIONS));
	touch $(BINUTILS_DIR1)/.configured

$(BINUTILS_DIR1)/binutils/objdump: $(BINUTILS_DIR1)/.configured
	$(MAKE) -C $(BINUTILS_DIR1) all

# Make install will put gettext data in staging_dir/share/locale.
# Unfortunatey, it isn't configureable.
$(STAGING_DIR)/$(REAL_GNU_TARGET_NAME)/bin/ld: $(BINUTILS_DIR1)/binutils/objdump
	$(MAKE) -C $(BINUTILS_DIR1) install

binutils-dependancies:
	@if ! which which > /dev/null ; then \
		echo ;\
		echo "You must install 'which' on your build machine"; \
		exit 1; \
	fi;
	@if ! which bison > /dev/null ; then \
		echo ;\
		echo "You must install 'bison' on your build machine"; \
		exit 1; \
	fi;
	@if ! which flex > /dev/null ; then \
		echo ;\
		echo "You must install 'flex' on your build machine"; \
		exit 1; \
	fi;
	@if ! which msgfmt > /dev/null ; then \
		echo ;\
		echo "You must install 'gettext' on your build machine"; \
		exit 1; \
	fi;

binutils: binutils-dependancies $(STAGING_DIR)/$(REAL_GNU_TARGET_NAME)/bin/ld

binutils-source: $(DL_DIR)/$(BINUTILS_SOURCE)

binutils-clean:
	rm -f $(STAGING_DIR)/bin/$(REAL_GNU_TARGET_NAME)*
	-$(MAKE) -C $(BINUTILS_DIR1) clean

binutils-dirclean:
	rm -rf $(BINUTILS_DIR1)



#############################################################
#
# build binutils for use on the target system
#
#############################################################
BINUTILS_DIR2:=$(BUILD_DIR)/binutils-$(BINUTILS_VERSION)-target
$(BINUTILS_DIR2)/.configured: $(BINUTILS_DIR)/.patched
	mkdir -p $(BINUTILS_DIR2)
	(cd $(BINUTILS_DIR2); \
		PATH=$(TARGET_PATH) \
		CFLAGS="$(TARGET_CFLAGS)" \
		CFLAGS_FOR_BUILD="-O2 -g" \
		$(BINUTILS_DIR)/configure \
		--prefix=/usr \
		--exec-prefix=/usr \
		--build=$(GNU_HOST_NAME) \
		--host=$(REAL_GNU_TARGET_NAME) \
		--target=$(REAL_GNU_TARGET_NAME) \
		$(DISABLE_NLS) \
		$(MULTILIB) \
		--disable-werror \
		$(BR2_CONFIGURE_STAGING_SYSROOT) \
		$(SOFT_FLOAT_CONFIG_OPTION) );
	touch $(BINUTILS_DIR2)/.configured

$(BINUTILS_DIR2)/binutils/objdump: $(BINUTILS_DIR2)/.configured
	PATH=$(TARGET_PATH) \
	$(MAKE) -C $(BINUTILS_DIR2) all

$(TARGET_DIR)/usr/bin/ld: $(BINUTILS_DIR2)/binutils/objdump
	PATH=$(TARGET_PATH) \
	$(MAKE) DESTDIR=$(TARGET_DIR) \
		tooldir=/usr build_tooldir=/usr \
		-C $(BINUTILS_DIR2) install
	#rm -rf $(TARGET_DIR)/share/locale $(TARGET_DIR)/usr/info \
	#	$(TARGET_DIR)/usr/man $(TARGET_DIR)/usr/share/doc
	-$(STRIP) $(TARGET_DIR)/usr/$(REAL_GNU_TARGET_NAME)/bin/* > /dev/null 2>&1
	-$(STRIP) $(TARGET_DIR)/usr/bin/* > /dev/null 2>&1

binutils_target: $(GCC_DEPENDANCY) $(TARGET_DIR)/usr/bin/ld

binutils_target-clean:
	(cd $(TARGET_DIR)/usr/bin; \
		rm -f addr2line ar as gprof ld nm objcopy \
		      objdump ranlib readelf size strings strip)
	rm -f $(TARGET_DIR)/bin/$(REAL_GNU_TARGET_NAME)*
	-$(MAKE) -C $(BINUTILS_DIR2) clean

binutils_target-dirclean:
	rm -rf $(BINUTILS_DIR2)
