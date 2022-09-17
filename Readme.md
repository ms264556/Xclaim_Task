# XCLAIM KERNEL BUILDING PROCEDURES
## PRE-REQUISITES

Install the following packages in an ubuntu - 14.04 machine

```bash
$ sudo apt-get update

$ sudo ln -s /usr/bin/env /bin/env

$ sudo apt-get -y install build-essential

$ sudo apt-get -y install g++ curl cvs bison gettext flex texinfo libtool patch automake gawk lzma module-init-tools libssl-dev

$ sudo apt-get -y install pkg-config unzip

$ sudo apt-get remove libnss-mdns
```

-If the build host happens to be 64-bit, the 32-bit compatible libraries must be installed.

```bash
$ sudo apt-get install ia32-libs libncurses5-dev
```

-If python error occurs for setuptools package, then install it by using the below command

```bash
$ sudo apt-get install python-setuptools
```

## Building steps:

1)	Change the directory to buildroot
		$ cd <path_to_Xclaim_task_directory>/buildroot

2) Give the build command in the buildroot directory
	```bash
	$ sudo make PROFILE=ap-11n-scorpion-xclaim BINBLD_DIR=<path_to_Xclaim_task_directory> BINBLD_IMPORT_TARBALL=depot-release-ap_11ax_x2_ap-arm-qca-solo_112.1.0.0.3.tzg
	```
3) While kernel is getting compiled, it asks for some configurations, press Enter for all
4) It will be built successfully and ends with providing a version number