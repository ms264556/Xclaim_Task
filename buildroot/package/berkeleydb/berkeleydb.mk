#############################################################
#
# berkeley db
#
#############################################################
DB_VER:=4.3.28
DB_SO_VER:=4.3
DB_SITE:=ftp://sleepycat1.inetu.net/releases/
DB_SOURCE:=db-$(DB_VER).NC.tar.gz
DB_DIR:=$(BUILD_DIR)/db-$(DB_VER).NC
DB_SHARLIB:=libdb-$(DB_SO_VER).so

ifneq ($(BR2_LARGEFILE),y)
DB_LARGEFILE="--disable-largefile"
endif

$(DL_DIR)/$(DB_SOURCE):
	$(WGET) -P $(DL_DIR) $(DB_SITE)/$(DB_SOURCE)

berkeleydb-source: $(DL_DIR)/$(DB_SOURCE)

$(DB_DIR)/.dist: $(DL_DIR)/$(DB_SOURCE)
	zcat $(DL_DIR)/$(DB_SOURCE) | tar -C $(BUILD_DIR) $(TAR_OPTIONS) -
	touch  $(DB_DIR)/.dist

$(DB_DIR)/.configured: $(DB_DIR)/.dist
	(cd $(DB_DIR)/build_unix; rm -rf config.cache; \
		$(TARGET_CONFIGURE_OPTS) \
		../dist/configure \
		--target=$(GNU_TARGET_NAME) \
		--host=$(GNU_TARGET_NAME) \
		--build=$(GNU_HOST_NAME) \
		--prefix=/usr \
		--exec-prefix=/usr \
		--bindir=/usr/bin \
		--sbindir=/usr/sbin \
		--libexecdir=/usr/lib \
		--sysconfdir=/etc \
		--datadir=/usr/share \
		--localstatedir=/var \
		--mandir=/usr/man \
		--infodir=/usr/info \
		--with-gnu-ld \
		--enable-shared \
		--disable-cxx \
		--disable-java \
		--disable-rpc \
		--disable-tcl \
		--disable-compat185 \
		--with-pic \
                $(DB_LARGEFILE) \
	);
	$(SED) 's/\.lo/.o/g' $(DB_DIR)/build_unix/Makefile
	touch  $(DB_DIR)/.configured

$(DB_DIR)/build_unix/.libs/$(DB_SHARLIB): $(DB_DIR)/.configured
	$(MAKE) CC=$(TARGET_CC) -C $(DB_DIR)/build_unix

$(STAGING_DIR)/lib/$(DB_SHARLIB): $(DB_DIR)/build_unix/.libs/$(DB_SHARLIB)
	$(MAKE) \
	    prefix=$(STAGING_DIR) \
	    exec_prefix=$(STAGING_DIR) \
	    bindir=$(STAGING_DIR)/bin \
	    sbindir=$(STAGING_DIR)/sbin \
	    libexecdir=$(STAGING_DIR)/lib \
	    datadir=$(STAGING_DIR)/share \
	    sysconfdir=$(STAGING_DIR)/etc \
	    localstatedir=$(STAGING_DIR)/var \
	    libdir=$(STAGING_DIR)/lib \
	    infodir=$(STAGING_DIR)/info \
	    mandir=$(STAGING_DIR)/man \
	    includedir=$(STAGING_DIR)/include \
	    -C $(DB_DIR)/build_unix install;
	chmod a-x $(STAGING_DIR)/lib/libdb*so*
	rm -f $(STAGING_DIR)/bin/db_*
	rm -rf $(STAGING_DIR)/share/locale $(STAGING_DIR)/info \
		$(STAGING_DIR)/man $(STAGING_DIR)/share/doc

$(TARGET_DIR)/lib/$(DB_SHARLIB): $(STAGING_DIR)/lib/$(DB_SHARLIB)
	rm -rf $(TARGET_DIR)/lib/libdb*
	cp -a $(STAGING_DIR)/lib/libdb*so*  $(TARGET_DIR)/lib/
	rm -f $(TARGET_DIR)/lib/libdb.so $(TARGET_DIR)/lib/libdb.la $(TARGET_DIR)/lib/libdb.a
	(cd $(TARGET_DIR)/usr/lib; ln -fs /lib/$(DB_SHARLIB) libdb.so)
	-$(STRIP) --strip-unneeded $(TARGET_DIR)/lib/libdb*so*

$(TARGET_DIR)/usr/lib/libdb.a: $(STAGING_DIR)/lib/libdb-$(DB_SO_VER).a
	cp -dpf $(STAGING_DIR)/include/db.h $(TARGET_DIR)/usr/include/
	cp -dpf $(STAGING_DIR)/lib/libdb*.a $(TARGET_DIR)/usr/lib/
	cp -dpf $(STAGING_DIR)/lib/libdb*.la $(TARGET_DIR)/usr/lib/
	touch -c $(TARGET_DIR)/usr/lib/libdb.a

berkeleydb-headers: $(TARGET_DIR)/usr/lib/libdb.a

berkeleydb-clean:
	$(MAKE) -C $(DB_DIR)/build_unix clean

berkeleydb-dirclean:
	rm -rf $(DB_DIR)

berkeleydb: uclibc $(TARGET_DIR)/lib/$(DB_SHARLIB)

#############################################################
#
# Toplevel Makefile options
#
#############################################################
ifeq ($(strip $(BR2_PACKAGE_BERKELEYDB)),y)
TARGETS+=berkeleydb
endif
