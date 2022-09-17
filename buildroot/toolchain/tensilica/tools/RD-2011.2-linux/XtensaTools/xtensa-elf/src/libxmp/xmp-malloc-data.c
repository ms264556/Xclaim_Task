/* Copyright (c) 2008-2009 by Tensilica Inc.  ALL RIGHTS RESERVED.
   These coded instructions, statements, and computer programs are the
   copyrighted works and confidential proprietary information of Tensilica Inc.
   They may not be modified, copied, reproduced, distributed, or disclosed to
   third parties in any manner, medium, or form, in whole or in part, without
   the prior written consent of Tensilica Inc.  */

#include <xtensa/config/core-isa.h>

#if XCHAL_HAVE_S32C1I && XCHAL_HAVE_RELEASE_SYNC
#include "xmp-library.h"
#endif

/* WARNING: This file must not contain any code. */

/* sbrk related xmp variables.  */
char *shared_heap_ptr;
extern char _shared_heap_sentry;	/* Defined by the linker */
char *_shared_heap_sentry_ptr = &_shared_heap_sentry;

/* malloc related xmp variables.  */

#if XCHAL_HAVE_S32C1I && XCHAL_HAVE_RELEASE_SYNC
#if !XCHAL_DCACHE_IS_COHERENT 
xmp_atomic_int_t xmp_sbrk_lock;
#else
xmp_mutex_t xmp_malloc_lock = XMP_RECURSIVE_MUTEX_INITIALIZER(NULL);

/* WARNING: These are copied from xmp-malloc.c and must be kept in
   sync.  We are trying to keep xmp-malloc.c as close to newlib's
   mallocr.c as possible, so re-factoring is not an option.  */

#ifndef INTERNAL_SIZE_T
#define INTERNAL_SIZE_T size_t
#endif

#define SIZE_SZ                (sizeof(INTERNAL_SIZE_T))

struct malloc_chunk
{
  INTERNAL_SIZE_T prev_size; /* Size of previous chunk (if free). */
  INTERNAL_SIZE_T size;      /* Size in bytes, including overhead. */
  struct malloc_chunk* fd;   /* double links -- used only if free. */
  struct malloc_chunk* bk;
};

typedef struct malloc_chunk* mbinptr;

#define av_ malloc_av_
#define trim_threshold		malloc_trim_threshold
#define top_pad			malloc_top_pad
#define n_mmaps_max		malloc_n_mmaps_max
#define mmap_threshold		malloc_mmap_threshold
#define sbrk_base		malloc_sbrk_base
#define max_sbrked_mem		malloc_max_sbrked_mem
#define max_total_mem		malloc_max_total_mem
#define current_mallinfo	malloc_current_mallinfo
#define n_mmaps			malloc_n_mmaps
#define max_n_mmaps		malloc_max_n_mmaps
#define mmapped_mem		malloc_mmapped_mem
#define max_mmapped_mem		malloc_max_mmapped_mem

#define malloc_stats			_xmp_malloc_stats_r
#define malloc_trim			_xmp_malloc_trim_r
#define malloc_usable_size		_xmp_malloc_usable_size_r

#define malloc_update_mallinfo		__xmp_malloc_update_mallinfo

#define malloc_av_			__xmp_malloc_av_
#define malloc_current_mallinfo		__xmp_malloc_current_mallinfo
#define malloc_max_sbrked_mem		__xmp_malloc_max_sbrked_mem
#define malloc_max_total_mem		__xmp_malloc_max_total_mem
#define malloc_sbrk_base		__xmp_malloc_sbrk_base
#define malloc_top_pad			__xmp_malloc_top_pad
#define malloc_trim_threshold		__xmp_malloc_trim_threshold

#define NAV             128   /* number of bins */

#define bin_at(i)      ((mbinptr)((char*)&(av_[2*(i) + 2]) - 2*SIZE_SZ))
#define next_bin(b)    ((mbinptr)((char*)(b) + 2 * sizeof(mbinptr)))
#define prev_bin(b)    ((mbinptr)((char*)(b) - 2 * sizeof(mbinptr)))

#define IAV(i)  bin_at(i), bin_at(i)

mbinptr malloc_av_[NAV * 2 + 2] = {
 0, 0,
 IAV(0),   IAV(1),   IAV(2),   IAV(3),   IAV(4),   IAV(5),   IAV(6),   IAV(7),
 IAV(8),   IAV(9),   IAV(10),  IAV(11),  IAV(12),  IAV(13),  IAV(14),  IAV(15),
 IAV(16),  IAV(17),  IAV(18),  IAV(19),  IAV(20),  IAV(21),  IAV(22),  IAV(23),
 IAV(24),  IAV(25),  IAV(26),  IAV(27),  IAV(28),  IAV(29),  IAV(30),  IAV(31),
 IAV(32),  IAV(33),  IAV(34),  IAV(35),  IAV(36),  IAV(37),  IAV(38),  IAV(39),
 IAV(40),  IAV(41),  IAV(42),  IAV(43),  IAV(44),  IAV(45),  IAV(46),  IAV(47),
 IAV(48),  IAV(49),  IAV(50),  IAV(51),  IAV(52),  IAV(53),  IAV(54),  IAV(55),
 IAV(56),  IAV(57),  IAV(58),  IAV(59),  IAV(60),  IAV(61),  IAV(62),  IAV(63),
 IAV(64),  IAV(65),  IAV(66),  IAV(67),  IAV(68),  IAV(69),  IAV(70),  IAV(71),
 IAV(72),  IAV(73),  IAV(74),  IAV(75),  IAV(76),  IAV(77),  IAV(78),  IAV(79),
 IAV(80),  IAV(81),  IAV(82),  IAV(83),  IAV(84),  IAV(85),  IAV(86),  IAV(87),
 IAV(88),  IAV(89),  IAV(90),  IAV(91),  IAV(92),  IAV(93),  IAV(94),  IAV(95),
 IAV(96),  IAV(97),  IAV(98),  IAV(99),  IAV(100), IAV(101), IAV(102), IAV(103),
 IAV(104), IAV(105), IAV(106), IAV(107), IAV(108), IAV(109), IAV(110), IAV(111),
 IAV(112), IAV(113), IAV(114), IAV(115), IAV(116), IAV(117), IAV(118), IAV(119),
 IAV(120), IAV(121), IAV(122), IAV(123), IAV(124), IAV(125), IAV(126), IAV(127)
};


#ifndef DEFAULT_TRIM_THRESHOLD
#define DEFAULT_TRIM_THRESHOLD (128L * 1024L)
#endif

#ifndef DEFAULT_TOP_PAD
#define DEFAULT_TOP_PAD        (0)
#endif

unsigned long trim_threshold   = DEFAULT_TRIM_THRESHOLD;
unsigned long top_pad          = DEFAULT_TOP_PAD;

/* The first value returned from sbrk */
char* sbrk_base = (char*)(-1);

/* The maximum memory obtained from system via sbrk */
unsigned long max_sbrked_mem = 0; 

/* The maximum via either sbrk or mmap */
unsigned long max_total_mem = 0; 

struct mallinfo {
  int arena;    /* total space allocated from system */
  int ordblks;  /* number of non-inuse chunks */
  int smblks;   /* unused -- always zero */
  int hblks;    /* number of mmapped regions */
  int hblkhd;   /* total space in mmapped regions */
  int usmblks;  /* unused -- always zero */
  int fsmblks;  /* unused -- always zero */
  int uordblks; /* total allocated space */
  int fordblks; /* total non-inuse space */
  int keepcost; /* top-most, releasable (via malloc_trim) space */
};	

/* internal working copy of mallinfo */
struct mallinfo current_mallinfo = {  0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

#endif /* XCHAL_DCACHE_IS_COHERENT */
#endif /*  XCHAL_HAVE_S32C1I && XCHAL_HAVE_RELEASE_SYNC */
