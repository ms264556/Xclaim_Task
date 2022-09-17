deps_config := \
	target/vmdkimage/Config.in \
	target/isoinstaller/Config.in \
	target/restoretool/Config.in \
	target/usbtool/Config.in \
	target/nfs/Config.in \
	target/initramfs/Config.in \
	target/tar/Config.in \
	target/ext2/Config.in \
	target/squashfs/Config.in \
	target/reiserfs/Config.in \
	target/jffs2/Config.in \
	target/cramfs/Config.in \
	target/Config.in \
	package/usb_modeswitch/Config.in \
	package/usb_modemcmd/Config.in \
	package/usb_devmon/Config.in \
	package/usb_default/Config.in \
	package/libusb/Config.in \
	package/kexec/Config.in \
	package/lipe/Config.in \
	package/ltrace/Config.in \
	package/libelf/Config.in \
	package/strace/Config.in \
	package/codump/Config.in \
	package/testshape/Config.in \
	package/rpmkeytest/Config.in \
	package/chariot/Config.in \
	package/aussie-rescue/Config.in \
	package/reiserfsprogs/Config.in \
	package/e2fsprogs/Config.in \
	package/zip/Config.in \
	package/zing/Config.in \
	package/zlib/Config.in \
	package/xorg/Config.in \
	package/xfsprogs/Config.in \
	package/wsgclient/Config.in \
	package/wps/Config.in \
	package/wide-dhcpv6/Config.in \
	package/which/Config.in \
	package/wget/Config.in \
	package/vtun/Config.in \
	package/vsftpd/Config.in \
	package/valgrind/Config.in \
	package/util-linux/Config.in \
	package/udhcp/Config.in \
	package/udev/Config.in \
	package/tube/Config.in \
	package/tunnelmgr/Config.in \
	package/ttcp/Config.in \
	package/trousers/Config.in \
	package/tr069/Config.in \
	package/tpm-tools/Config.in \
	package/tool4tpm/Config.in \
	package/tn5250/Config.in \
	package/tinyx/Config.in \
	package/tinylogin/Config.in \
	package/tftpd/Config.in \
	package/tcpdump/Config.in \
	package/tcl/Config.in \
	package/tacplus/Config.in \
	package/sysklogd/Config.in \
	package/statd/Config.in \
	package/stainfod/Config.in \
	package/ssmtp/Config.in \
	package/socat/Config.in \
	package/sntp/Config.in \
	package/smtpclient/Config.in \
	package/smsd/Config.in \
	package/slang/Config.in \
	package/sfdisk/Config.in \
	package/sdl/Config.in \
	package/scand/Config.in \
	package/rxvt/Config.in \
	package/ruda/Config.in \
	package/rsync/Config.in \
	package/rsmd/Config.in \
	package/rp-pppoe/Config.in \
	package/rpkitpm/Config.in \
	package/rks_nbp/Config.in \
	package/rksblistd/Config.in \
	package/rfw/Config.in \
	package/rfmd/Config.in \
	package/registered-domain/Config.in \
	package/regdomain/Config.in \
	package/regdmn_chann/Config.in \
	package/readline/Config.in \
	package/raidtools/Config.in \
	package/qte/Config.in \
	package/qemu/Config.in \
	package/python/Config.in \
	package/procps/Config.in \
	package/pppd/Config.in \
	package/portmap/Config.in \
	package/portage/Config.in \
	package/pcmcia/Config.in \
	package/pciutils/Config.in \
	package/parprouted/Config.in \
	package/openvpn/Config.in \
	package/openssl/Config.in \
	package/openssh/Config.in \
	package/openntpd/Config.in \
	package/openldap/Config.in \
	package/openl2tp/Config.in \
	package/omac_aes/Config.in \
	package/ocsp/Config.in \
	package/nuttcp/Config.in \
	package/ntp/Config.in \
	package/newt/Config.in \
	package/netsnmp/Config.in \
	package/netkittelnet/Config.in \
	package/netkitbase/Config.in \
	package/netcat/Config.in \
	package/ndsend/Config.in \
	package/ndisc6/Config.in \
	package/ncurses/Config.in \
	package/nano/Config.in \
	package/mtd/Config.in \
	package/msgd/Config.in \
	package/mrouted/Config.in \
	package/mpg123/Config.in \
	package/mosquitto_lbs_sub/Config.in \
	package/mosquitto/Config.in \
	package/modutils/Config.in \
	package/module-init-tools/Config.in \
	package/mkdosfs/Config.in \
	package/miniupnpd/Config.in \
	package/microwin/Config.in \
	package/microperl/Config.in \
	package/microcom/Config.in \
	package/jardmeshd/Config.in \
	package/meshd/Config.in \
	package/memtester/Config.in \
	package/memcached/Config.in \
	package/mdnsproxy/Config.in \
	package/mdadm/Config.in \
	package/m4/Config.in \
	package/lzo/Config.in \
	package/lvm2/Config.in \
	package/ltt/Config.in \
	package/ltp-testsuite/Config.in \
	package/lrzsz/Config.in \
	package/generatelocalip/Config.in \
	package/lldpd/Config.in \
	package/links/Config.in \
	package/libtool/Config.in \
	package/libsysfs/Config.in \
	package/libRutil/Config.in \
	package/librkshash/Config.in \
	package/librkscrypto/Config.in \
	package/libsqlite3/Config.in \
	package/libpng/Config.in \
	package/libpcap/Config.in \
	package/libnl/Config.in \
	package/libmad/Config.in \
	package/libipc_sock/Config.in \
	package/libiconv/Config.in \
	package/libgmp/Config.in \
	package/libglib12/Config.in \
	package/libfloat/Config.in \
	package/libccl/Config.in \
	package/libcache_pool/Config.in \
	package/less/Config.in \
	package/jpeg/Config.in \
	package/irqbalance/Config.in \
	package/iptables/Config.in \
	package/ipsec-tools/Config.in \
	package/iproute2/Config.in \
	package/ipmitool/Config.in \
	package/ipmiutil/Config.in \
	package/iperf/Config.in \
	package/iostat/Config.in \
	package/i2c-tools/Config.in \
	package/hub_registrar/Config.in \
	package/hotspot/Config.in \
	package/hotplug/Config.in \
	package/hostap/Config.in \
	package/gzip/Config.in \
	package/gpsd/Config.in \
	package/gkermit/Config.in \
	package/gettext/Config.in \
	package/freetype/Config.in \
	package/file/Config.in \
	package/fakeroot/Config.in \
	package/ethtool/Config.in \
	package/ethreg-2/Config.in \
	package/ethreg/Config.in \
	package/ekahaud/Config.in \
	package/ebtables/Config.in \
	package/dropbear/Config.in \
	package/dnsmasq/Config.in \
	package/dmalloc/Config.in \
	package/dm/Config.in \
	package/dlrc/Config.in \
	package/distcc/Config.in \
	package/director/Config.in \
	package/directfb/Config.in \
	package/dhry/Config.in \
	package/dhcp-forwarder/Config.in \
	package/dhcp/Config.in \
	package/delta/Config.in \
	package/defaults/Config.in \
	package/debug-utils/Config.in \
	package/csm/Config.in \
	package/cvs/Config.in \
	package/customize/Config.in \
	package/custom/Config.in \
	package/curl/Config.in \
	package/coova-chilli/Config.in \
	package/cmm/Config.in \
	package/clusterD/Config.in \
	package/ckermit/Config.in \
	package/channelfly/Config.in \
	package/cares/Config.in \
	package/ca-certs/Config.in \
	package/c2lib/Config.in \
	package/bsp/Config.in \
	package/brownout/Config.in \
	package/bridge/Config.in \
	package/bonjour/Config.in \
	package/boa/Config.in \
	package/bison/Config.in \
	package/berkeleydb/Config.in \
	package/bc/Config.in \
	package/avpd/Config.in \
	package/automake/Config.in \
	package/autoconf/Config.in \
	package/agqmi/Config.in \
	package/aerosctd/Config.in \
	package/acpid/Config.in \
	package/wpa-supplicant/Config.in \
	package/wireless-tools/Config.in \
	package/webserver/Config.in \
	package/timer/Config.in \
	package/tar/Config.in \
	package/syslog/Config.in \
	package/sed/Config.in \
	package/rpm/Config.in \
	package/rpcapd/Config.in \
	package/rksmcast/Config.in \
	package/rksconfig/Config.in \
	package/rkscli/Config.in \
	package/rkschano/Config.in \
	package/rksaim/Config.in \
	package/patch/Config.in \
	package/mfr/Config.in \
	package/make/Config.in \
	package/librsm/Config.in \
	package/librsc/Config.in \
	package/hostapd/Config.in \
	package/grep/Config.in \
	package/gawk/Config.in \
	package/freeswan/Config.in \
	package/flex/Config.in \
	package/findutils/Config.in \
	package/eth1xd/Config.in \
	package/ed/Config.in \
	package/diffutils/Config.in \
	package/coreutils/Config.in \
	package/bzip2/Config.in \
	package/bash/Config.in \
	package/auto-prov/Config.in \
	package/auto-conf/Config.in \
	package/aruba-pluto/Config.in \
	package/aruba-l2tpd/Config.in \
	package/aruba-rap/Config.in \
	toolchain/gcc/Config.in.2 \
	toolchain/ccache/Config.in.2 \
	package/apmgr/Config.in \
	package/jardwifi/Config.in \
	package/madwifi/Config.in \
	package/linux-kernel/Config.in \
	package/grub/Config.in \
	package/uboot/Config.in \
	package/redboot/Config.in \
	package/busybox/Config.in \
	package/Config.in \
	toolchain/sstrip/Config.in \
	toolchain/binmd5/Config.in \
	toolchain/elf2flt/Config.in \
	toolchain/gdb/Config.in \
	toolchain/ccache/Config.in \
	toolchain/gcc/Config.in \
	toolchain/binutils/Config.in \
	toolchain/uClibc/Config.in \
	toolchain/kernel-headers/Config.in \
	toolchain/Config.in \
	package/release/Config.in \
	Config.in

.config include/config.h: $(deps_config)

$(deps_config):
