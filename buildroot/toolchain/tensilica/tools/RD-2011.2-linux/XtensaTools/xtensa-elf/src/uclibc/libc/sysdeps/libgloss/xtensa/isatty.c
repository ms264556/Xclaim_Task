/* isatty.c (copied from libc/posix/isatty.c) */

/* Dumb implementation so programs will at least run.  */

#include <sys/stat.h>
#include <errno.h>

#define UCLIBC_ERRNO_PTR ((struct _reent *)__errno_location())

extern int _fstat_r(void *, int, struct stat *);

int
isatty (int fd)
{
  struct stat buf;

  if (_fstat_r (UCLIBC_ERRNO_PTR, fd, &buf) < 0)
    return 0;
  if (S_ISCHR (buf.st_mode))
    return 1;
  return 0;
}

