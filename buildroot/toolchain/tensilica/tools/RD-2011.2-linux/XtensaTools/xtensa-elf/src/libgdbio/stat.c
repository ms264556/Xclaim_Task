/*
 * Copyright (c) 2006 by Tensilica Inc.  ALL RIGHTS RESERVED.
 * These coded instructions, statements, and computer programs are the
 * copyrighted works and confidential proprietary information of Tensilica Inc.
 * They may not be modified, copied, reproduced, distributed, or disclosed to
 * third parties in any manner, medium, or form, in whole or in part, without
 * the prior written consent of Tensilica Inc.
 */

#include "gdbio.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>


int _stat_r (struct _reent * reent, const char * file_name, struct stat * buf)
{
  gdbio_stat_struct gbuf;
  gdbio_ret_struct ret;
  int len;
  const char * end = file_name;
  while (*end++ != 0)
    ;
  len = end - file_name;

  ret = _gdbio_syscall(GDBIO_TARGET_SYSCALL_STAT, 
		       &gbuf, len, 0, file_name);

  reent->_errno = _gdbio_to_xtensa_errno(ret._errno);

  gbuf.st_dev = _fix_endian_4(gbuf.st_dev);
  gbuf.st_ino = _fix_endian_4(gbuf.st_ino);
  gbuf.st_mode = _fix_endian_4(gbuf.st_mode);
  gbuf.st_nlink = _fix_endian_4(gbuf.st_nlink);
  gbuf.st_uid = _fix_endian_4(gbuf.st_uid);
  gbuf.st_gid = _fix_endian_4(gbuf.st_gid);
  gbuf.st_rdev = _fix_endian_4(gbuf.st_rdev);
  gbuf.st_size = _fix_endian_8(gbuf.st_size);
  gbuf.st_blksize = _fix_endian_8(gbuf.st_blksize);
  gbuf.st_blocks = _fix_endian_8(gbuf.st_blocks);
  gbuf.st_atime = _fix_endian_4(gbuf.st_atime);
  gbuf.st_mtime = _fix_endian_4(gbuf.st_mtime);
  gbuf.st_ctime = _fix_endian_4(gbuf.st_ctime);

  memset(buf, 0, sizeof(struct stat));
  buf->st_dev = gbuf.st_dev;
  buf->st_ino = gbuf.st_ino;
  buf->st_mode = gbuf.st_mode;
  buf->st_nlink = gbuf.st_nlink;
  buf->st_uid = gbuf.st_uid;
  buf->st_gid = gbuf.st_gid;
  buf->st_rdev = gbuf.st_rdev;
  buf->st_size = gbuf.st_size;
  buf->st_blksize = gbuf.st_blksize;
  buf->st_blocks = gbuf.st_blocks;
  buf->st_atime = gbuf.st_atime;
  buf->st_mtime = gbuf.st_mtime;
  buf->st_ctime = gbuf.st_ctime;

  return ret.retval;
}
