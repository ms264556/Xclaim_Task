/* syscalls.c

   Provide all the glue from uclibc's references to _foo to
   libgloss's _foo_r. If you build libgloss without reentrant
   system calls, then this is probably unnecessary. See reent.h
   for more details.
*/

#include <sys/reent.h>
#include <errno.h>
#include <unistd.h>

#define UCLIBC_ERRNO_PTR ((struct _reent *)__errno_location())

/*=======================================================================*/
#ifdef L_close_r
extern int _close_r (struct _reent *, int);

int close (int __fd) 
{
  return _close_r (UCLIBC_ERRNO_PTR, __fd);
}
#endif

/*=======================================================================*/
#ifdef L_execve_r
/* currently unused */
extern int _execve_r (struct _reent *, char *, char **, char **);

int execve(char * __path, char * __argv[], char * const __envp[])
{
  return _execve_r(UCLIBC_ERRNO_PTR, __path, __argv, __envp);
}
#endif

/*=======================================================================*/
#ifdef L_fcntl

int fcntl (int __fd, int __cmd, ...)
{
  errno = ENOSYS;
  return -1;
}
#endif

/*=======================================================================*/
#ifdef L_fork_r
extern int _fork_r (struct _reent *);

__pid_t fork(void)
{
  return _fork_r(UCLIBC_ERRNO_PTR);
}
#endif

/*=======================================================================*/
#ifdef L_fstat_r
extern int _fstat_r (struct _reent *, int, struct stat *);

int fstat (int __fd, struct stat *__buf)
{
  return _fstat_r(UCLIBC_ERRNO_PTR, __fd, __buf);
}
#endif

/*=======================================================================*/
#ifdef L_getpid_r
__pid_t getpid (void)
{
  /* onlye one process, so just return 1 */
  return 1;
}
#endif

/*=======================================================================*/
#ifdef L_gettimeofday_r
#include <sys/time.h>
extern int _gettimeofday_r (struct _reent *, struct timeval *, struct timezone *);

int gettimeofday (struct timeval * __tv, struct timezone * __tz)
{
  return _gettimeofday_r(UCLIBC_ERRNO_PTR, __tv, __tz);
}
#endif

/*=======================================================================*/
#ifdef L_kill_r
extern int _kill_r (struct _reent *, int, int);

int kill (__pid_t __pid, int __sig)
{
  return _kill_r(UCLIBC_ERRNO_PTR, __pid, __sig);
}
#endif

/*=======================================================================*/
#ifdef L_link_r
extern int _link_r (struct _reent *, const char *, const char *);

int link (__const char *__from, __const char *__to)
{
  return _link_r (UCLIBC_ERRNO_PTR, __from, __to);
}
#endif

/*=======================================================================*/
#ifdef L_lseek_r
#include <sys/types.h>
extern __off_t _lseek_r (struct _reent *, int, __off_t, int);

__off_t lseek (int __fd, __off_t __offset, int __whence)
{
  return _lseek_r(UCLIBC_ERRNO_PTR, __fd, __offset, __whence);
}
#endif

/*=======================================================================*/
#ifdef L_mkdir
#include <sys/types.h>
int mkdir (const char * __pathname, mode_t __mode)
{
  errno = EPERM;
  return -1;
}
#endif

/*=======================================================================*/
#ifdef L_open_r
extern int _open_r (struct _reent *, const char *, int, int);

int open (const char * __path, int __flags, int __mode)
{
  return _open_r (UCLIBC_ERRNO_PTR, __path, __flags, __mode);
}
#endif

/*=======================================================================*/
#ifdef L_read_r
#include <sys/types.h>
extern __ssize_t _read_r (struct _reent *, int, void *, __ssize_t);

__ssize_t read (int __fd, void * __buf, size_t __n)
{
  return _read_r(UCLIBC_ERRNO_PTR, __fd, __buf, __n);
}
#endif

/*=======================================================================*/
#ifdef L_rename_r
#include <sys/types.h>
extern int _rename_r(struct _reent *, const char *, const char *);

int rename(const char * __old, const char * __new)
{
  return _rename_r(UCLIBC_ERRNO_PTR, __old, __new);
}
#endif

/*=======================================================================*/
#ifdef L_rmdir

int rmdir (const char * __pathname)
{
  errno = EPERM;
  return -1;
}
#endif

/*=======================================================================*/
#ifdef L_sbrk_r
extern void *_sbrk_r (struct _reent *, int);
#include <sys/types.h>

void * sbrk(intptr_t __delta)
{
  return _sbrk_r(UCLIBC_ERRNO_PTR, __delta);
}
#endif

/*=======================================================================*/
#ifdef L_stat_r
struct stat;
extern int _stat_r (struct _reent *, const char *, struct stat *);

int stat (const char * __filename, struct stat * __buf)
{
  return _stat_r(UCLIBC_ERRNO_PTR, __filename, __buf);
}
#endif

/*=======================================================================*/
#ifdef L_sigprocmask
/* libgloss doesn't have signals */
int sigprocmask(int how, void *set, void *oldset)
{
  errno = EINVAL;
  return -1;
}
#endif

/*=======================================================================*/
#ifdef L___syscall_sigaction
struct sigaction;
/* libgloss doesn't have signals */
int __syscall_sigaction(int signum, const struct sigaction * act,
			struct sigaction * oldact)
{
  errno = EINVAL;
  return -1;
}
#endif

/*=======================================================================*/
#ifdef L_times_r
/* this is currently unused */
#include <sys/times.h>
extern clock_t _times_r (struct _reent *, struct tms *);

clock_t times (struct tms * __buf)
{
  return _times_r (UCLIBC_ERRNO_PTR, __buf);
}
#endif

/*=======================================================================*/
#ifdef L_unlink_r
extern int _unlink_r (struct _reent *, const char *);

int unlink (const char * __name)
{
  return _unlink_r (UCLIBC_ERRNO_PTR, __name);
}
#endif

/*=======================================================================*/
#ifdef L_wait_r
#include <stdlib.h>
extern int _wait_r (struct _reent *, int *);

__pid_t wait (__WAIT_STATUS __stat_loc)
{
  return _wait_r (UCLIBC_ERRNO_PTR, __stat_loc)
}
#endif

/*=======================================================================*/
#ifdef L_write_r
#include <sys/types.h>
extern __ssize_t _write_r (struct _reent *, int, const void *, size_t);

__ssize_t write (int __fd, __const void *__buf, size_t __n)
{
  return _write_r (UCLIBC_ERRNO_PTR, __fd, __buf, __n);
}
#endif


