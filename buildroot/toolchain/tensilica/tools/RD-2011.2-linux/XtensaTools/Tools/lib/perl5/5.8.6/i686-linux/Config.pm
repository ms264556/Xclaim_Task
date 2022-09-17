# This file was created by configpm when Perl was built. Any changes
# made to this file will be lost the next time perl is built.

package Config;
@EXPORT = qw(%Config);
@EXPORT_OK = qw(myconfig config_sh config_vars config_re);

my %Export_Cache = map {($_ => 1)} (@EXPORT, @EXPORT_OK);

# Define our own import method to avoid pulling in the full Exporter:
sub import {
    my $pkg = shift;
    @_ = @EXPORT unless @_;

    my @funcs = grep $_ ne '%Config', @_;
    my $export_Config = @funcs < @_ ? 1 : 0;

    my $callpkg = caller(0);
    foreach my $func (@funcs) {
	die sprintf qq{"%s" is not exported by the %s module\n},
	    $func, __PACKAGE__ unless $Export_Cache{$func};
	*{$callpkg.'::'.$func} = \&{$func};
    }

    *{"$callpkg\::Config"} = \%Config if $export_Config;
    return;
}

die "Perl lib version (v5.8.6) doesn't match executable version ($])"
    unless $^V;

$^V eq v5.8.6
    or die "Perl lib version (v5.8.6) doesn't match executable version (" .
	sprintf("v%vd",$^V) . ")";

##
## This file was produced by running the Configure script. It holds all the
## definitions figured out by Configure. Should you modify one of these values,
## do not forget to propagate your changes by running "Configure -der". You may
## instead choose to run each of the .SH files by yourself, or "Configure -S".
##
#
## Package name      : perl5
## Source directory  : .
## Configuration time: Mon Sep 15 15:07:51 PDT 2008
## Configured by     : source
## Target system     : linux peacock 2.4.21-40.elsmp #1 smp thu feb 2 22:22:39 est 2006 i686 i686 i386 gnulinux 
#
## Configure command line arguments.
#PERL_PATCHLEVEL=

our $summary : unique = <<'!END!';
Summary of my $package (revision $revision $version_patchlevel_string) configuration:
  Platform:
    osname=$osname, osvers=$osvers, archname=$archname
    uname='$myuname'
    config_args='$config_args'
    hint=$hint, useposix=$useposix, d_sigaction=$d_sigaction
    usethreads=$usethreads use5005threads=$use5005threads useithreads=$useithreads usemultiplicity=$usemultiplicity
    useperlio=$useperlio d_sfio=$d_sfio uselargefiles=$uselargefiles usesocks=$usesocks
    use64bitint=$use64bitint use64bitall=$use64bitall uselongdouble=$uselongdouble
    usemymalloc=$usemymalloc, bincompat5005=undef
  Compiler:
    cc='$cc', ccflags ='$ccflags',
    optimize='$optimize',
    cppflags='$cppflags'
    ccversion='$ccversion', gccversion='$gccversion', gccosandvers='$gccosandvers'
    intsize=$intsize, longsize=$longsize, ptrsize=$ptrsize, doublesize=$doublesize, byteorder=$byteorder
    d_longlong=$d_longlong, longlongsize=$longlongsize, d_longdbl=$d_longdbl, longdblsize=$longdblsize
    ivtype='$ivtype', ivsize=$ivsize, nvtype='$nvtype', nvsize=$nvsize, Off_t='$lseektype', lseeksize=$lseeksize
    alignbytes=$alignbytes, prototype=$prototype
  Linker and Libraries:
    ld='$ld', ldflags ='$ldflags'
    libpth=$libpth
    libs=$libs
    perllibs=$perllibs
    libc=$libc, so=$so, useshrplib=$useshrplib, libperl=$libperl
    gnulibc_version='$gnulibc_version'
  Dynamic Linking:
    dlsrc=$dlsrc, dlext=$dlext, d_dlsymun=$d_dlsymun, ccdlflags='$ccdlflags'
    cccdlflags='$cccdlflags', lddlflags='$lddlflags'

!END!
my $i = 0;
foreach my $c (4,3,2) { $i |= ord($c); $i <<= 8 }
$i |= ord(1);
my $byteorder = join('', unpack('aaaa', pack('L!', $i)));
my $summary_expanded;

sub myconfig {
    return $summary_expanded if $summary_expanded;
    ($summary_expanded = $summary) =~ s{\$(\w+)}
		 { my $c = $Config{$1}; defined($c) ? $c : 'undef' }ge;
    $summary_expanded;
}

local *_ = \my $a;
$_ = <<'!END!';
archlibexp='/usr/xtensa/tools-6.1/lib/perl5/5.8.6/i686-linux'
archname='i686-linux'
cc='gcc'
ccflags='-fno-strict-aliasing -pipe -I/usr/xtensa/tools-6.1/include -D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64 -I/usr/include/gdbm'
cppflags='-fno-strict-aliasing -pipe -I/usr/xtensa/tools-6.1/include -I/usr/include/gdbm'
dlsrc='dl_dlopen.xs'
dynamic_ext='attrs B ByteLoader Cwd Data/Dumper DB_File Devel/DProf Devel/Peek Devel/PPPort Digest/MD5 Encode Fcntl File/Glob Filter/Util/Call GDBM_File I18N/Langinfo IO IPC/SysV List/Util MIME/Base64 Opcode PerlIO/encoding PerlIO/scalar PerlIO/via POSIX re SDBM_File Socket Storable Sys/Hostname Sys/Syslog threads Time/HiRes Unicode/Normalize XS/APItest XS/Typemap threads/shared'
installarchlib='/usr/xtensa/tools-6.1/lib/perl5/5.8.6/i686-linux'
installprivlib='/usr/xtensa/tools-6.1/lib/perl5/5.8.6'
libpth='/usr/xtensa/tools-6.1/lib /lib /usr/lib /usr/local/lib'
libs='-lnsl -lgdbm -ldb -ldl -lm -lcrypt -lutil -lc'
osname='linux'
osvers='2.4.21-40.elsmp'
prefix='/usr/xtensa/tools-6.1'
privlibexp='/usr/xtensa/tools-6.1/lib/perl5/5.8.6'
sharpbang='#!'
shsharp='true'
so='so'
startsh='#!/bin/sh'
static_ext=' '
Author=''
CONFIG='true'
Date='$Date'
Header=''
Id='$Id'
Locker=''
Log='$Log'
Mcc='Mcc'
PATCHLEVEL='8'
PERL_API_REVISION='5'
PERL_API_SUBVERSION='0'
PERL_API_VERSION='8'
PERL_CONFIG_SH='true'
PERL_REVISION='5'
PERL_SUBVERSION='6'
PERL_VERSION='8'
RCSfile='$RCSfile'
Revision='$Revision'
SUBVERSION='6'
Source=''
State=''
_a='.a'
_exe=''
_o='.o'
afs='false'
afsroot='/afs'
alignbytes='4'
ansi2knr=''
aphostname='/bin/hostname'
api_revision='5'
api_subversion='0'
api_version='8'
api_versionstring='5.8.0'
ar='ar'
archlib='/usr/xtensa/tools-6.1/lib/perl5/5.8.6/i686-linux'
archname64=''
archobjs=''
asctime_r_proto='0'
awk='awk'
baserev='5.0'
bash=''
bin='/usr/xtensa/tools-6.1/bin'
binexp='/usr/xtensa/tools-6.1/bin'
bison='bison'
byacc='byacc'
byteorder='1234'
c=''
castflags='0'
cat='cat'
cccdlflags='-fpic'
ccdlflags='-Wl,-E'
ccflags_uselargefiles='-D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64'
ccname='gcc'
ccsymbols=''
ccversion=''
cf_by='source'
cf_email='source@peacock.hq.tensilica.com'
cf_time='Mon Sep 15 15:07:51 PDT 2008'
charsize='1'
chgrp=''
chmod='chmod'
chown=''
clocktype='clock_t'
comm='comm'
compress=''
config_arg0='./Configure'
config_arg10='-e'
config_arg1='-Dcc=gcc'
config_arg2='-Dprefix=/usr/xtensa/tools-6.1'
config_arg3='-Ubincompat5005'
config_arg4='-Uinstallusrbinperl'
config_arg5='-Dlocincpth=/usr/xtensa/tools-6.1/include'
config_arg6='-Dloclibpth=/usr/xtensa/tools-6.1/lib'
config_arg7='-Di_gdbm'
config_arg8='-d'
config_arg9='-s'
config_argc='10'
config_args='-Dcc=gcc -Dprefix=/usr/xtensa/tools-6.1 -Ubincompat5005 -Uinstallusrbinperl -Dlocincpth=/usr/xtensa/tools-6.1/include -Dloclibpth=/usr/xtensa/tools-6.1/lib -Di_gdbm -d -s -e'
contains='grep'
cp='cp'
cpio=''
cpp='cpp'
cpp_stuff='42'
cppccsymbols=''
cpplast='-'
cppminus='-'
cpprun='gcc -E'
cppstdin='gcc -E'
cppsymbols='__ELF__=1 _FILE_OFFSET_BITS=64 __GLIBC__=2 __GLIBC_MINOR__=3 __GNUC__=3 __GNUC_MINOR__=4 __GNU_LIBRARY__=6 i386=1 __i386=1 __i386__=1 _LARGEFILE_SOURCE=1 linux=1 __linux=1 __linux__=1 _POSIX_C_SOURCE=199506 _POSIX_SOURCE=1 __STDC__=1 unix=1 __unix=1 __unix__=1 __USE_BSD=1 __USE_FILE_OFFSET64=1 __USE_LARGEFILE=1 __USE_MISC=1 __USE_POSIX=1 __USE_POSIX199309=1 __USE_POSIX199506=1 __USE_POSIX2=1 __USE_SVID=1'
crypt_r_proto='0'
cryptlib=''
csh='csh'
ctermid_r_proto='0'
ctime_r_proto='0'
d_Gconvert='gcvt((x),(n),(b))'
d_PRIEUldbl='define'
d_PRIFUldbl='define'
d_PRIGUldbl='define'
d_PRIXU64='define'
d_PRId64='define'
d_PRIeldbl='define'
d_PRIfldbl='define'
d_PRIgldbl='define'
d_PRIi64='define'
d_PRIo64='define'
d_PRIu64='define'
d_PRIx64='define'
d_SCNfldbl='define'
d__fwalk='undef'
d_access='define'
d_accessx='undef'
d_aintl='undef'
d_alarm='define'
d_archlib='define'
d_asctime_r='undef'
d_atolf='undef'
d_atoll='define'
d_attribut='define'
d_bcmp='define'
d_bcopy='define'
d_bsd='undef'
d_bsdgetpgrp='undef'
d_bsdsetpgrp='undef'
d_bzero='define'
d_casti32='undef'
d_castneg='define'
d_charvspr='define'
d_chown='define'
d_chroot='define'
d_chsize='undef'
d_class='undef'
d_closedir='define'
d_cmsghdr_s='define'
d_const='define'
d_copysignl='define'
d_crypt='define'
d_crypt_r='undef'
d_csh='define'
d_ctermid_r='undef'
d_ctime_r='undef'
d_cuserid='define'
d_dbl_dig='define'
d_dbminitproto='undef'
d_difftime='define'
d_dirfd='define'
d_dirnamlen='undef'
d_dlerror='define'
d_dlopen='define'
d_dlsymun='undef'
d_dosuid='undef'
d_drand48_r='undef'
d_drand48proto='define'
d_dup2='define'
d_eaccess='undef'
d_endgrent='define'
d_endgrent_r='undef'
d_endhent='define'
d_endhostent_r='undef'
d_endnent='define'
d_endnetent_r='undef'
d_endpent='define'
d_endprotoent_r='undef'
d_endpwent='define'
d_endpwent_r='undef'
d_endsent='define'
d_endservent_r='undef'
d_eofnblk='define'
d_eunice='undef'
d_faststdio='undef'
d_fchdir='define'
d_fchmod='define'
d_fchown='define'
d_fcntl='define'
d_fcntl_can_lock='define'
d_fd_macros='define'
d_fd_set='define'
d_fds_bits='undef'
d_fgetpos='define'
d_finite='define'
d_finitel='define'
d_flexfnam='define'
d_flock='define'
d_flockproto='define'
d_fork='define'
d_fp_class='undef'
d_fpathconf='define'
d_fpclass='undef'
d_fpclassify='undef'
d_fpclassl='undef'
d_fpos64_t='undef'
d_frexpl='define'
d_fs_data_s='undef'
d_fseeko='define'
d_fsetpos='define'
d_fstatfs='define'
d_fstatvfs='define'
d_fsync='define'
d_ftello='define'
d_ftime='undef'
d_getcwd='define'
d_getespwnam='undef'
d_getfsstat='undef'
d_getgrent='define'
d_getgrent_r='undef'
d_getgrgid_r='undef'
d_getgrnam_r='undef'
d_getgrps='define'
d_gethbyaddr='define'
d_gethbyname='define'
d_gethent='define'
d_gethname='define'
d_gethostbyaddr_r='undef'
d_gethostbyname_r='undef'
d_gethostent_r='undef'
d_gethostprotos='define'
d_getitimer='define'
d_getlogin='define'
d_getlogin_r='undef'
d_getmnt='undef'
d_getmntent='define'
d_getnbyaddr='define'
d_getnbyname='define'
d_getnent='define'
d_getnetbyaddr_r='undef'
d_getnetbyname_r='undef'
d_getnetent_r='undef'
d_getnetprotos='define'
d_getpagsz='define'
d_getpbyname='define'
d_getpbynumber='define'
d_getpent='define'
d_getpgid='define'
d_getpgrp2='undef'
d_getpgrp='define'
d_getppid='define'
d_getprior='define'
d_getprotobyname_r='undef'
d_getprotobynumber_r='undef'
d_getprotoent_r='undef'
d_getprotoprotos='define'
d_getprpwnam='undef'
d_getpwent='define'
d_getpwent_r='undef'
d_getpwnam_r='undef'
d_getpwuid_r='undef'
d_getsbyname='define'
d_getsbyport='define'
d_getsent='define'
d_getservbyname_r='undef'
d_getservbyport_r='undef'
d_getservent_r='undef'
d_getservprotos='define'
d_getspnam='define'
d_getspnam_r='undef'
d_gettimeod='define'
d_gmtime_r='undef'
d_gnulibc='define'
d_grpasswd='define'
d_hasmntopt='define'
d_htonl='define'
d_ilogbl='define'
d_index='undef'
d_inetaton='define'
d_int64_t='define'
d_isascii='define'
d_isfinite='undef'
d_isinf='define'
d_isnan='define'
d_isnanl='define'
d_killpg='define'
d_lchown='define'
d_ldbl_dig='define'
d_link='define'
d_localtime_r='undef'
d_locconv='define'
d_lockf='define'
d_longdbl='define'
d_longlong='define'
d_lseekproto='define'
d_lstat='define'
d_madvise='define'
d_mblen='define'
d_mbstowcs='define'
d_mbtowc='define'
d_memchr='define'
d_memcmp='define'
d_memcpy='define'
d_memmove='define'
d_memset='define'
d_mkdir='define'
d_mkdtemp='define'
d_mkfifo='define'
d_mkstemp='define'
d_mkstemps='undef'
d_mktime='define'
d_mmap='define'
d_modfl='define'
d_modfl_pow32_bug='undef'
d_modflproto='undef'
d_mprotect='define'
d_msg='define'
d_msg_ctrunc='define'
d_msg_dontroute='define'
d_msg_oob='define'
d_msg_peek='define'
d_msg_proxy='define'
d_msgctl='define'
d_msgget='define'
d_msghdr_s='define'
d_msgrcv='define'
d_msgsnd='define'
d_msync='define'
d_munmap='define'
d_mymalloc='undef'
d_nice='define'
d_nl_langinfo='define'
d_nv_preserves_uv='define'
d_off64_t='undef'
d_old_pthread_create_joinable='undef'
d_oldpthreads='undef'
d_oldsock='undef'
d_open3='define'
d_pathconf='define'
d_pause='define'
d_perl_otherlibdirs='undef'
d_phostname='undef'
d_pipe='define'
d_poll='define'
d_portable='define'
d_procselfexe='define'
d_pthread_atfork='undef'
d_pthread_attr_setscope='define'
d_pthread_yield='define'
d_pwage='undef'
d_pwchange='undef'
d_pwclass='undef'
d_pwcomment='undef'
d_pwexpire='undef'
d_pwgecos='define'
d_pwpasswd='define'
d_pwquota='undef'
d_qgcvt='define'
d_quad='define'
d_random_r='undef'
d_readdir64_r='undef'
d_readdir='define'
d_readdir_r='undef'
d_readlink='define'
d_readv='define'
d_recvmsg='define'
d_rename='define'
d_rewinddir='define'
d_rmdir='define'
d_safebcpy='undef'
d_safemcpy='undef'
d_sanemcmp='define'
d_sbrkproto='define'
d_scalbnl='define'
d_sched_yield='define'
d_scm_rights='define'
d_seekdir='define'
d_select='define'
d_sem='define'
d_semctl='define'
d_semctl_semid_ds='define'
d_semctl_semun='define'
d_semget='define'
d_semop='define'
d_sendmsg='define'
d_setegid='define'
d_seteuid='define'
d_setgrent='define'
d_setgrent_r='undef'
d_setgrps='define'
d_sethent='define'
d_sethostent_r='undef'
d_setitimer='define'
d_setlinebuf='define'
d_setlocale='define'
d_setlocale_r='undef'
d_setnent='define'
d_setnetent_r='undef'
d_setpent='define'
d_setpgid='define'
d_setpgrp2='undef'
d_setpgrp='define'
d_setprior='define'
d_setproctitle='undef'
d_setprotoent_r='undef'
d_setpwent='define'
d_setpwent_r='undef'
d_setregid='define'
d_setresgid='define'
d_setresuid='define'
d_setreuid='define'
d_setrgid='undef'
d_setruid='undef'
d_setsent='define'
d_setservent_r='undef'
d_setsid='define'
d_setvbuf='define'
d_sfio='undef'
d_shm='define'
d_shmat='define'
d_shmatprototype='define'
d_shmctl='define'
d_shmdt='define'
d_shmget='define'
d_sigaction='define'
d_sigprocmask='define'
d_sigsetjmp='define'
d_sockatmark='define'
d_sockatmarkproto='undef'
d_socket='define'
d_socklen_t='define'
d_sockpair='define'
d_socks5_init='undef'
d_sqrtl='define'
d_srand48_r='undef'
d_srandom_r='undef'
d_sresgproto='undef'
d_sresuproto='undef'
d_statblks='define'
d_statfs_f_flags='undef'
d_statfs_s='define'
d_statvfs='define'
d_stdio_cnt_lval='undef'
d_stdio_ptr_lval='undef'
d_stdio_ptr_lval_nochange_cnt='undef'
d_stdio_ptr_lval_sets_cnt='undef'
d_stdio_stream_array='undef'
d_stdiobase='undef'
d_stdstdio='undef'
d_strchr='define'
d_strcoll='define'
d_strctcpy='define'
d_strerrm='strerror(e)'
d_strerror='define'
d_strerror_r='undef'
d_strftime='define'
d_strtod='define'
d_strtol='define'
d_strtold='define'
d_strtoll='define'
d_strtoq='define'
d_strtoul='define'
d_strtoull='define'
d_strtouq='define'
d_strxfrm='define'
d_suidsafe='undef'
d_symlink='define'
d_syscall='define'
d_syscallproto='define'
d_sysconf='define'
d_sysernlst=''
d_syserrlst='define'
d_system='define'
d_tcgetpgrp='define'
d_tcsetpgrp='define'
d_telldir='define'
d_telldirproto='define'
d_time='define'
d_times='define'
d_tm_tm_gmtoff='define'
d_tm_tm_zone='define'
d_tmpnam_r='undef'
d_truncate='define'
d_ttyname_r='undef'
d_tzname='define'
d_u32align='undef'
d_ualarm='define'
d_umask='define'
d_uname='define'
d_union_semun='undef'
d_unordered='undef'
d_usleep='define'
d_usleepproto='define'
d_ustat='define'
d_vendorarch='undef'
d_vendorbin='undef'
d_vendorlib='undef'
d_vendorscript='undef'
d_vfork='undef'
d_void_closedir='undef'
d_voidsig='define'
d_voidtty=''
d_volatile='define'
d_vprintf='define'
d_wait4='define'
d_waitpid='define'
d_wcstombs='define'
d_wctomb='define'
d_writev='define'
d_xenix='undef'
date='date'
db_hashtype='u_int32_t'
db_prefixtype='size_t'
db_version_major='4'
db_version_minor='1'
db_version_patch='25'
defvoidused='15'
direntrytype='struct dirent'
dlext='so'
doublesize='8'
drand01='drand48()'
drand48_r_proto='0'
eagain='EAGAIN'
ebcdic='undef'
echo='echo'
egrep='egrep'
emacs=''
endgrent_r_proto='0'
endhostent_r_proto='0'
endnetent_r_proto='0'
endprotoent_r_proto='0'
endpwent_r_proto='0'
endservent_r_proto='0'
eunicefix=':'
exe_ext=''
expr='expr'
extensions='attrs B ByteLoader Cwd Data/Dumper DB_File Devel/DProf Devel/Peek Devel/PPPort Digest/MD5 Encode Fcntl File/Glob Filter/Util/Call GDBM_File I18N/Langinfo IO IPC/SysV List/Util MIME/Base64 Opcode PerlIO/encoding PerlIO/scalar PerlIO/via POSIX re SDBM_File Socket Storable Sys/Hostname Sys/Syslog threads Time/HiRes Unicode/Normalize XS/APItest XS/Typemap threads/shared Errno'
extras=''
fflushNULL='define'
fflushall='undef'
find=''
firstmakefile='makefile'
flex=''
fpossize='16'
fpostype='fpos_t'
freetype='void'
from=':'
full_ar='/usr/xtensa/stools-6.1/bin/ar'
full_csh='/bin/csh'
full_sed='/bin/sed'
gccansipedantic=''
gccosandvers=''
gccversion='3.4.3 (tensilica-special)'
getgrent_r_proto='0'
getgrgid_r_proto='0'
getgrnam_r_proto='0'
gethostbyaddr_r_proto='0'
gethostbyname_r_proto='0'
gethostent_r_proto='0'
getlogin_r_proto='0'
getnetbyaddr_r_proto='0'
getnetbyname_r_proto='0'
getnetent_r_proto='0'
getprotobyname_r_proto='0'
getprotobynumber_r_proto='0'
getprotoent_r_proto='0'
getpwent_r_proto='0'
getpwnam_r_proto='0'
getpwuid_r_proto='0'
getservbyname_r_proto='0'
getservbyport_r_proto='0'
getservent_r_proto='0'
getspnam_r_proto='0'
gidformat='"lu"'
gidsign='1'
gidsize='4'
gidtype='gid_t'
glibpth='/usr/shlib  /lib /usr/lib /usr/lib/386 /lib/386 /usr/ccs/lib /usr/ucblib /usr/local/lib '
gmake='gmake'
gmtime_r_proto='0'
gnulibc_version='2.3.2'
grep='grep'
groupcat='cat /etc/group'
groupstype='gid_t'
gzip='gzip'
h_fcntl='false'
h_sysfile='true'
hint='recommended'
hostcat='cat /etc/hosts'
html1dir=' '
html1direxp=''
html3dir=' '
html3direxp=''
i16size='2'
i16type='short'
i32size='4'
i32type='long'
i64size='8'
i64type='long long'
i8size='1'
i8type='char'
i_arpainet='define'
i_bsdioctl=''
i_crypt='define'
i_db='define'
i_dbm='undef'
i_dirent='define'
i_dld='undef'
i_dlfcn='define'
i_fcntl='undef'
i_float='define'
i_fp='undef'
i_fp_class='undef'
i_gdbm='define'
i_grp='define'
i_ieeefp='undef'
i_inttypes='define'
i_langinfo='define'
i_libutil='undef'
i_limits='define'
i_locale='define'
i_machcthr='undef'
i_malloc='define'
i_math='define'
i_memory='undef'
i_mntent='define'
i_ndbm='undef'
i_netdb='define'
i_neterrno='undef'
i_netinettcp='define'
i_niin='define'
i_poll='define'
i_prot='undef'
i_pthread='define'
i_pwd='define'
i_rpcsvcdbm='undef'
i_sfio='undef'
i_sgtty='undef'
i_shadow='define'
i_socks='undef'
i_stdarg='define'
i_stddef='define'
i_stdlib='define'
i_string='define'
i_sunmath='undef'
i_sysaccess='undef'
i_sysdir='define'
i_sysfile='define'
i_sysfilio='undef'
i_sysin='undef'
i_sysioctl='define'
i_syslog='define'
i_sysmman='define'
i_sysmode='undef'
i_sysmount='define'
i_sysndir='undef'
i_sysparam='define'
i_sysresrc='define'
i_syssecrt='undef'
i_sysselct='define'
i_syssockio='undef'
i_sysstat='define'
i_sysstatfs='define'
i_sysstatvfs='define'
i_systime='define'
i_systimek='undef'
i_systimes='define'
i_systypes='define'
i_sysuio='define'
i_sysun='define'
i_sysutsname='define'
i_sysvfs='define'
i_syswait='define'
i_termio='undef'
i_termios='define'
i_time='define'
i_unistd='define'
i_ustat='define'
i_utime='define'
i_values='define'
i_varargs='undef'
i_varhdr='stdarg.h'
i_vfork='undef'
ignore_versioned_solibs='y'
inc_version_list=' '
inc_version_list_init='0'
incpath=''
inews=''
installbin='/usr/xtensa/tools-6.1/bin'
installhtml1dir=''
installhtml3dir=''
installman1dir='/usr/xtensa/tools-6.1/man/man1'
installman3dir='/usr/xtensa/tools-6.1/man/man3'
installprefix='/usr/xtensa/tools-6.1'
installprefixexp='/usr/xtensa/tools-6.1'
installscript='/usr/xtensa/tools-6.1/bin'
installsitearch='/usr/xtensa/tools-6.1/lib/perl5/site_perl/5.8.6/i686-linux'
installsitebin='/usr/xtensa/tools-6.1/bin'
installsitehtml1dir=''
installsitehtml3dir=''
installsitelib='/usr/xtensa/tools-6.1/lib/perl5/site_perl/5.8.6'
installsiteman1dir='/usr/xtensa/tools-6.1/man/man1'
installsiteman3dir='/usr/xtensa/tools-6.1/man/man3'
installsitescript='/usr/xtensa/tools-6.1/bin'
installstyle='lib/perl5'
installusrbinperl='undef'
installvendorarch=''
installvendorbin=''
installvendorhtml1dir=''
installvendorhtml3dir=''
installvendorlib=''
installvendorman1dir=''
installvendorman3dir=''
installvendorscript=''
intsize='4'
issymlink='test -h'
ivdformat='"ld"'
ivsize='4'
ivtype='long'
known_extensions='attrs B ByteLoader Cwd Data/Dumper DB_File Devel/DProf Devel/Peek Devel/PPPort Digest/MD5 Encode Fcntl File/Glob Filter/Util/Call GDBM_File I18N/Langinfo IO IPC/SysV List/Util MIME/Base64 NDBM_File ODBM_File Opcode PerlIO/encoding PerlIO/scalar PerlIO/via POSIX re SDBM_File Socket Storable Sys/Hostname Sys/Syslog Thread threads Time/HiRes Unicode/Normalize XS/APItest XS/Typemap threads/shared'
ksh=''
ld='gcc'
lddlflags='-shared -L/usr/xtensa/tools-6.1/lib'
ldflags=' -L/usr/xtensa/tools-6.1/lib'
ldflags_uselargefiles=''
ldlibpthname='LD_LIBRARY_PATH'
less='less'
lib_ext='.a'
libc='/lib/libc-2.3.2.so'
libperl='libperl.a'
libsdirs=' /usr/lib /usr/xtensa/tools-6.1/lib'
libsfiles=' libnsl.so libgdbm.a libdb.so libdl.so libm.so libcrypt.so libutil.so libc.so'
libsfound=' /usr/lib/libnsl.so /usr/xtensa/tools-6.1/lib/libgdbm.a /usr/lib/libdb.so /usr/lib/libdl.so /usr/lib/libm.so /usr/lib/libcrypt.so /usr/lib/libutil.so /usr/lib/libc.so'
libspath=' /usr/xtensa/tools-6.1/lib /lib /usr/lib /usr/local/lib'
libswanted='sfio socket bind inet nsl nm gdbm dbm db malloc dl dld ld sun m crypt sec util c cposix posix ucb BSD'
libswanted_uselargefiles=''
line=''
lint=''
lkflags=''
ln='ln'
lns='/bin/ln -s'
localtime_r_proto='0'
locincpth='/usr/xtensa/tools-6.1/include'
loclibpth='/usr/xtensa/tools-6.1/lib'
longdblsize='12'
longlongsize='8'
longsize='4'
lp=''
lpr=''
ls='ls'
lseeksize='8'
lseektype='off_t'
mail=''
mailx=''
make='make'
make_set_make='#'
mallocobj=''
mallocsrc=''
malloctype='void *'
man1dir='/usr/xtensa/tools-6.1/man/man1'
man1direxp='/usr/xtensa/tools-6.1/man/man1'
man1ext='1'
man3dir='/usr/xtensa/tools-6.1/man/man3'
man3direxp='/usr/xtensa/tools-6.1/man/man3'
man3ext='3'
mips_type=''
mistrustnm=''
mkdir='mkdir'
mmaptype='void *'
modetype='mode_t'
more='more'
multiarch='undef'
mv=''
myarchname='i686-linux'
mydomain='.hq.tensilica.com'
myhostname='peacock'
myuname='linux peacock 2.4.21-40.elsmp #1 smp thu feb 2 22:22:39 est 2006 i686 i686 i386 gnulinux '
n='-n'
need_va_copy='undef'
netdb_hlen_type='size_t'
netdb_host_type='const void *'
netdb_name_type='const char *'
netdb_net_type='in_addr_t'
nm='nm'
nm_opt=''
nm_so_opt='--dynamic'
nonxs_ext='Errno'
nroff='nroff'
nvEUformat='"E"'
nvFUformat='"F"'
nvGUformat='"G"'
nv_preserves_uv_bits='32'
nveformat='"e"'
nvfformat='"f"'
nvgformat='"g"'
nvsize='8'
nvtype='double'
o_nonblock='O_NONBLOCK'
obj_ext='.o'
old_pthread_create_joinable=''
optimize='-O2'
orderlib='false'
otherlibdirs=' '
package='perl5'
pager='/usr/bin/less'
passcat='cat /etc/passwd'
patchlevel='8'
path_sep=':'
perl5='/usr/xtensa/stools-6.1/bin/perl'
perl=''
perl_patchlevel=''
perladmin='source@peacock.hq.tensilica.com'
perllibs='-lnsl -ldl -lm -lcrypt -lutil -lc'
perlpath='/usr/xtensa/tools-6.1/bin/perl'
pg='pg'
phostname='hostname'
pidtype='pid_t'
plibpth=''
pmake=''
pr=''
prefixexp='/usr/xtensa/tools-6.1'
privlib='/usr/xtensa/tools-6.1/lib/perl5/5.8.6'
procselfexe='"/proc/self/exe"'
prototype='define'
ptrsize='4'
quadkind='3'
quadtype='long long'
randbits='48'
randfunc='drand48'
random_r_proto='0'
randseedtype='long'
ranlib=':'
rd_nodata='-1'
readdir64_r_proto='0'
readdir_r_proto='0'
revision='5'
rm='rm'
rmail=''
run=''
runnm='false'
sPRIEUldbl='"LE"'
sPRIFUldbl='"LF"'
sPRIGUldbl='"LG"'
sPRIXU64='"LX"'
sPRId64='"Ld"'
sPRIeldbl='"Le"'
sPRIfldbl='"Lf"'
sPRIgldbl='"Lg"'
sPRIi64='"Li"'
sPRIo64='"Lo"'
sPRIu64='"Lu"'
sPRIx64='"Lx"'
sSCNfldbl='"Lf"'
sched_yield='sched_yield()'
scriptdir='/usr/xtensa/tools-6.1/bin'
scriptdirexp='/usr/xtensa/tools-6.1/bin'
sed='sed'
seedfunc='srand48'
selectminbits='32'
selecttype='fd_set *'
sendmail=''
setgrent_r_proto='0'
sethostent_r_proto='0'
setlocale_r_proto='0'
setnetent_r_proto='0'
setprotoent_r_proto='0'
setpwent_r_proto='0'
setservent_r_proto='0'
sh='/bin/sh'
shar=''
shmattype='void *'
shortsize='2'
shrpenv=''
sig_count='65'
sig_name='ZERO HUP INT QUIT ILL TRAP ABRT BUS FPE KILL USR1 SEGV USR2 PIPE ALRM TERM STKFLT CHLD CONT STOP TSTP TTIN TTOU URG XCPU XFSZ VTALRM PROF WINCH IO PWR SYS NUM32 RTMIN NUM34 NUM35 NUM36 NUM37 NUM38 NUM39 NUM40 NUM41 NUM42 NUM43 NUM44 NUM45 NUM46 NUM47 NUM48 NUM49 NUM50 NUM51 NUM52 NUM53 NUM54 NUM55 NUM56 NUM57 NUM58 NUM59 NUM60 NUM61 NUM62 NUM63 RTMAX IOT CLD POLL UNUSED '
sig_name_init='"ZERO", "HUP", "INT", "QUIT", "ILL", "TRAP", "ABRT", "BUS", "FPE", "KILL", "USR1", "SEGV", "USR2", "PIPE", "ALRM", "TERM", "STKFLT", "CHLD", "CONT", "STOP", "TSTP", "TTIN", "TTOU", "URG", "XCPU", "XFSZ", "VTALRM", "PROF", "WINCH", "IO", "PWR", "SYS", "NUM32", "RTMIN", "NUM34", "NUM35", "NUM36", "NUM37", "NUM38", "NUM39", "NUM40", "NUM41", "NUM42", "NUM43", "NUM44", "NUM45", "NUM46", "NUM47", "NUM48", "NUM49", "NUM50", "NUM51", "NUM52", "NUM53", "NUM54", "NUM55", "NUM56", "NUM57", "NUM58", "NUM59", "NUM60", "NUM61", "NUM62", "NUM63", "RTMAX", "IOT", "CLD", "POLL", "UNUSED", 0'
sig_num='0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36 37 38 39 40 41 42 43 44 45 46 47 48 49 50 51 52 53 54 55 56 57 58 59 60 61 62 63 64 6 17 29 31 '
sig_num_init='0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 6, 17, 29, 31, 0'
sig_size='69'
signal_t='void'
sitearch='/usr/xtensa/tools-6.1/lib/perl5/site_perl/5.8.6/i686-linux'
sitearchexp='/usr/xtensa/tools-6.1/lib/perl5/site_perl/5.8.6/i686-linux'
sitebin='/usr/xtensa/tools-6.1/bin'
sitebinexp='/usr/xtensa/tools-6.1/bin'
sitehtml1dir=''
sitehtml1direxp=''
sitehtml3dir=''
sitehtml3direxp=''
sitelib='/usr/xtensa/tools-6.1/lib/perl5/site_perl/5.8.6'
sitelib_stem='/usr/xtensa/tools-6.1/lib/perl5/site_perl'
sitelibexp='/usr/xtensa/tools-6.1/lib/perl5/site_perl/5.8.6'
siteman1dir='/usr/xtensa/tools-6.1/man/man1'
siteman1direxp='/usr/xtensa/tools-6.1/man/man1'
siteman3dir='/usr/xtensa/tools-6.1/man/man3'
siteman3direxp='/usr/xtensa/tools-6.1/man/man3'
siteprefix='/usr/xtensa/tools-6.1'
siteprefixexp='/usr/xtensa/tools-6.1'
sitescript='/usr/xtensa/tools-6.1/bin'
sitescriptexp='/usr/xtensa/tools-6.1/bin'
sizesize='4'
sizetype='size_t'
sleep=''
smail=''
sockethdr=''
socketlib=''
socksizetype='socklen_t'
sort='sort'
spackage='Perl5'
spitshell='cat'
srand48_r_proto='0'
srandom_r_proto='0'
src='.'
ssizetype='ssize_t'
startperl='#!/usr/xtensa/tools-6.1/bin/perl'
stdchar='char'
stdio_base='((fp)->_IO_read_base)'
stdio_bufsiz='((fp)->_IO_read_end - (fp)->_IO_read_base)'
stdio_cnt='((fp)->_IO_read_end - (fp)->_IO_read_ptr)'
stdio_filbuf=''
stdio_ptr='((fp)->_IO_read_ptr)'
stdio_stream_array=''
strerror_r_proto='0'
strings='/usr/include/string.h'
submit=''
subversion='6'
sysman='/usr/share/man/man1'
tail=''
tar=''
targetarch=''
tbl=''
tee=''
test='test'
timeincl='/usr/include/sys/time.h /usr/include/time.h '
timetype='time_t'
tmpnam_r_proto='0'
to=':'
touch='touch'
tr='tr'
trnl='\n'
troff=''
ttyname_r_proto='0'
u16size='2'
u16type='unsigned short'
u32size='4'
u32type='unsigned long'
u64size='8'
u64type='unsigned long long'
u8size='1'
u8type='unsigned char'
uidformat='"lu"'
uidsign='1'
uidsize='4'
uidtype='uid_t'
uname='uname'
uniq='uniq'
uquadtype='unsigned long long'
use5005threads='undef'
use64bitall='undef'
use64bitint='undef'
usecrosscompile='undef'
usedl='define'
usefaststdio='define'
useithreads='undef'
uselargefiles='define'
uselongdouble='undef'
usemallocwrap='define'
usemorebits='undef'
usemultiplicity='undef'
usemymalloc='n'
usenm='false'
useopcode='true'
useperlio='define'
useposix='true'
usereentrant='undef'
usesfio='false'
useshrplib='false'
usesocks='undef'
usethreads='undef'
usevendorprefix='undef'
usevfork='false'
usrinc='/usr/include'
uuname=''
uvXUformat='"lX"'
uvoformat='"lo"'
uvsize='4'
uvtype='unsigned long'
uvuformat='"lu"'
uvxformat='"lx"'
vendorarch=''
vendorarchexp=''
vendorbin=''
vendorbinexp=''
vendorhtml1dir=' '
vendorhtml1direxp=''
vendorhtml3dir=' '
vendorhtml3direxp=''
vendorlib=''
vendorlib_stem=''
vendorlibexp=''
vendorman1dir=' '
vendorman1direxp=''
vendorman3dir=' '
vendorman3direxp=''
vendorprefix=''
vendorprefixexp=''
vendorscript=''
vendorscriptexp=''
version='5.8.6'
version_patchlevel_string='version 8 subversion 6'
versiononly='undef'
vi=''
voidflags='15'
xlibpth='/usr/lib/386 /lib/386'
yacc='/usr/bin/byacc'
yaccflags=''
zcat=''
zip='zip'
!END!
s/(byteorder=)(['"]).*?\2/$1$2$byteorder$2/m;
our $Config_SH : unique = $_;

# Search for it in the big string 
sub fetch_string {
    my($self, $key) = @_;

    my $quote_type = "'";
    my $marker = "$key=";

    # Check for the common case, ' delimited
    my $start = index($Config_SH, "\n$marker$quote_type");
    # If that failed, check for " delimited
    if ($start == -1) {
        $quote_type = '"';
        $start = index($Config_SH, "\n$marker$quote_type");
    }
    return undef if ( ($start == -1) &&  # in case it's first 
                      (substr($Config_SH, 0, length($marker)) ne $marker) );
    if ($start == -1) { 
        # It's the very first thing we found. Skip $start forward
        # and figure out the quote mark after the =.
        $start = length($marker) + 1;
        $quote_type = substr($Config_SH, $start - 1, 1);
    } 
    else { 
        $start += length($marker) + 2;
    }

    my $value = substr($Config_SH, $start, 
                       index($Config_SH, "$quote_type\n", $start) - $start);

    # If we had a double-quote, we'd better eval it so escape
    # sequences and such can be interpolated. Since the incoming
    # value is supposed to follow shell rules and not perl rules,
    # we escape any perl variable markers
    if ($quote_type eq '"') {
	$value =~ s/\$/\\\$/g;
	$value =~ s/\@/\\\@/g;
	eval "\$value = \"$value\"";
    }

    # So we can say "if $Config{'foo'}".
    $value = undef if $value eq 'undef';
    $self->{$key} = $value; # cache it
}

sub fetch_virtual {
    my($self, $key) = @_;

    my $value;

    if ($key =~ /^((?:cc|ld)flags|libs(?:wanted)?)_nolargefiles/) {
	# These are purely virtual, they do not exist, but need to
	# be computed on demand for largefile-incapable extensions.
	my $new_key = "${1}_uselargefiles";
	$value = $Config{$1};
	my $withlargefiles = $Config{$new_key};
	if ($new_key =~ /^(?:cc|ld)flags_/) {
	    $value =~ s/\Q$withlargefiles\E\b//;
	} elsif ($new_key =~ /^libs/) {
	    my @lflibswanted = split(' ', $Config{libswanted_uselargefiles});
	    if (@lflibswanted) {
		my %lflibswanted;
		@lflibswanted{@lflibswanted} = ();
		if ($new_key =~ /^libs_/) {
		    my @libs = grep { /^-l(.+)/ &&
                                      not exists $lflibswanted{$1} }
		                    split(' ', $Config{libs});
		    $Config{libs} = join(' ', @libs);
		} elsif ($new_key =~ /^libswanted_/) {
		    my @libswanted = grep { not exists $lflibswanted{$_} }
		                          split(' ', $Config{libswanted});
		    $Config{libswanted} = join(' ', @libswanted);
		}
	    }
	}
    }

    $self->{$key} = $value;
}

sub FETCH { 
    my($self, $key) = @_;

    # check for cached value (which may be undef so we use exists not defined)
    return $self->{$key} if exists $self->{$key};

    $self->fetch_string($key);
    return $self->{$key} if exists $self->{$key};
    $self->fetch_virtual($key);

    # Might not exist, in which undef is correct.
    return $self->{$key};
}
 
my $prevpos = 0;

sub FIRSTKEY {
    $prevpos = 0;
    substr($Config_SH, 0, index($Config_SH, '=') );
}

sub NEXTKEY {
    # Find out how the current key's quoted so we can skip to its end.
    my $quote = substr($Config_SH, index($Config_SH, "=", $prevpos)+1, 1);
    my $pos = index($Config_SH, qq($quote\n), $prevpos) + 2;
    my $len = index($Config_SH, "=", $pos) - $pos;
    $prevpos = $pos;
    $len > 0 ? substr($Config_SH, $pos, $len) : undef;
}

sub EXISTS { 
    return 1 if exists($_[0]->{$_[1]});

    return(index($Config_SH, "\n$_[1]='") != -1 or
           substr($Config_SH, 0, length($_[1])+2) eq "$_[1]='" or
           index($Config_SH, "\n$_[1]=\"") != -1 or
           substr($Config_SH, 0, length($_[1])+2) eq "$_[1]=\"" or
           $_[1] =~ /^(?:(?:cc|ld)flags|libs(?:wanted)?)_nolargefiles$/
          );
}

sub STORE  { die "\%Config::Config is read-only\n" }
*DELETE = \&STORE;
*CLEAR  = \&STORE;


sub config_sh {
    $Config_SH
}

sub config_re {
    my $re = shift;
    return map { chomp; $_ } grep eval{ /^(?:$re)=/ }, split /^/, $Config_SH;
}

sub config_vars {
    # implements -V:cfgvar option (see perlrun -V:)
    foreach (@_) {
	# find optional leading, trailing colons; and query-spec
	my ($notag,$qry,$lncont) = m/^(:)?(.*?)(:)?$/;	# flags fore and aft, 
	# map colon-flags to print decorations
	my $prfx = $notag ? '': "$qry=";		# tag-prefix for print
	my $lnend = $lncont ? ' ' : ";\n";		# line ending for print

	# all config-vars are by definition \w only, any \W means regex
	if ($qry =~ /\W/) {
	    my @matches = config_re($qry);
	    print map "$_$lnend", @matches ? @matches : "$qry: not found"		if !$notag;
	    print map { s/\w+=//; "$_$lnend" } @matches ? @matches : "$qry: not found"	if  $notag;
	} else {
	    my $v = (exists $Config{$qry}) ? $Config{$qry} : 'UNKNOWN';
	    $v = 'undef' unless defined $v;
	    print "${prfx}'${v}'$lnend";
	}
    }
}

sub TIEHASH {
    bless $_[1], $_[0];
}

# avoid Config..Exporter..UNIVERSAL search for DESTROY then AUTOLOAD
sub DESTROY { }

tie %Config, 'Config', {
    'archlibexp' => '/usr/xtensa/tools-6.1/lib/perl5/5.8.6/i686-linux',
    'archname' => 'i686-linux',
    'cc' => 'gcc',
    'ccflags' => '-fno-strict-aliasing -pipe -I/usr/xtensa/tools-6.1/include -D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64 -I/usr/include/gdbm',
    'cppflags' => '-fno-strict-aliasing -pipe -I/usr/xtensa/tools-6.1/include -I/usr/include/gdbm',
    'dlsrc' => 'dl_dlopen.xs',
    'dynamic_ext' => 'attrs B ByteLoader Cwd Data/Dumper DB_File Devel/DProf Devel/Peek Devel/PPPort Digest/MD5 Encode Fcntl File/Glob Filter/Util/Call GDBM_File I18N/Langinfo IO IPC/SysV List/Util MIME/Base64 Opcode PerlIO/encoding PerlIO/scalar PerlIO/via POSIX re SDBM_File Socket Storable Sys/Hostname Sys/Syslog threads Time/HiRes Unicode/Normalize XS/APItest XS/Typemap threads/shared',
    'installarchlib' => '/usr/xtensa/tools-6.1/lib/perl5/5.8.6/i686-linux',
    'installprivlib' => '/usr/xtensa/tools-6.1/lib/perl5/5.8.6',
    'libpth' => '/usr/xtensa/tools-6.1/lib /lib /usr/lib /usr/local/lib',
    'libs' => '-lnsl -lgdbm -ldb -ldl -lm -lcrypt -lutil -lc',
    'osname' => 'linux',
    'osvers' => '2.4.21-40.elsmp',
    'prefix' => '/usr/xtensa/tools-6.1',
    'privlibexp' => '/usr/xtensa/tools-6.1/lib/perl5/5.8.6',
    'sharpbang' => '#!',
    'shsharp' => 'true',
    'so' => 'so',
    'startsh' => '#!/bin/sh',
    'static_ext' => ' ',
    byteorder => $byteorder,

};

1;
