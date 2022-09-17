/* high-level portions of hardware profiling  */

/*
 * Copyright (c) 2005-2006 by Tensilica Inc.  ALL RIGHTS RESERVED.
 * These coded instructions, statements, and computer programs are the
 * copyrighted works and confidential proprietary information of Tensilica Inc.
 * They may not be modified, copied, reproduced, distributed, or disclosed to
 * third parties in any manner, medium, or form, in whole or in part, without
 * the prior written consent of Tensilica Inc.
 */
	
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "xt_profiling.h"
#include "hw_profiler_internal.h"

#define BUILD_FOR_HARDWARE
#ifdef BUILD_FOR_HARDWARE
#define REPLACE_FS_WITH_DBFS
#include <xtensa/debugfs.h>
#endif

extern int xt_profile_errors;

static char * my_strcpy(char * dst, const char * src)
{
  while ((*dst++ = *src++))
    ;
  return dst;
}

static int
my_mkstemp(char *path)
{
  char *start, *p;
  int fd, i;

  for (p = path; *p; ++p) ; // go to the end of path
  
  while (*--p == 'X')       // replace X's with 0's
    *p = '0';

  start = p + 1;

  for (i = 0; i < 1000; i++) {
    fd = open(path, O_CREAT | O_EXCL | O_RDWR | O_BINARY, 0666);
    if (fd >= 0)
      return fd;
    
    for (p = start;;) {
      if (!*p)
	return -1;
      if (*p == 'z')
	*p++ = 'a';
      else {
	// isdigit
	if (*p >= '0' && *p <= '9')
	  *p = 'a';
	else
	  ++ * p;
	break;
      }
    }
  }
  return -1;
}


const char histogram_tag = 5; /* GMON_TAG_TIME_HIST_16 */
const char call_graph_arc_tag = 1; /* GMON_TAG_CG_ARC */

/***********************************************************
 Histogram related functions
***********************************************************/

typedef struct histogram_record
{
  unsigned int lo_pc;
  unsigned int hi_pc;
  unsigned int buckets;
  unsigned int profrate;
  char dimension[15];
  char abbrev;
} histogram_record_t;

typedef struct bins
{
  struct bins * next;
  unsigned short bin[BUCKETS_PER_RECORD];
} buckets_t;


typedef struct profiling_record {
  unsigned int base_pc;
  struct profiling_record * left;
  struct profiling_record * right;
  buckets_t * bins;
} profiling_record_t;


static int 
write_record(int fd, profiling_record_t * rec)
{
  histogram_record_t hrec;
  buckets_t * buckets;
  if (rec == NULL) 
    return 0;

  memset(&hrec, 0, sizeof(hrec));
  
  hrec.lo_pc = rec->base_pc;
  hrec.hi_pc = rec->base_pc + ADDRESSES_PER_RECORD;
  hrec.buckets = BUCKETS_PER_RECORD;
  /* Due to strangeness in gprof. We use a negative frequency
     so that it can detect the period.  */
  hrec.profrate = 0 - xt_profile_get_frequency();
  my_strcpy(hrec.dimension, "cycles");
  hrec.abbrev = 'c';
  
  buckets = rec->bins;

  while (buckets != NULL) {
    if (write(fd, &histogram_tag, sizeof(histogram_tag)) 
	!= sizeof(histogram_tag)) {
      return -1;
    }
  
    if (write(fd, &hrec, sizeof(hrec)) != sizeof(hrec)) {
      return -1;
    }
    
    if (write(fd, &buckets->bin, BUCKETS_PER_RECORD * sizeof(unsigned short)) 
	!= BUCKETS_PER_RECORD * sizeof(unsigned short)) {
      return -1;
    }
    buckets = buckets->next;
  }

  if (write_record (fd, rec->left) != 0)
    return -1;
  if (write_record (fd, rec->right) != 0)
    return -1;
  
  return 0;
}


static void 
clear_histogram_record(profiling_record_t * rec)
{
  if (rec) {
    buckets_t * buckets = rec->bins;
    while (buckets) {
      memset(&buckets->bin, 0, sizeof(unsigned short) * BUCKETS_PER_RECORD);
      buckets = buckets->next;
    }
    clear_histogram_record(rec->left);
    clear_histogram_record(rec->right);
  }
}


extern profiling_record_t * xt_profile_base_record;

int write_hist_records(int fd) 
{
  profiling_record_t * rec = xt_profile_base_record;
  return write_record(fd, rec);
}

#if 0
static void
print_record(profiling_record_t * rec)
{
  int i;
  buckets_t * buckets;
  if (rec == NULL) 
    return;
  
  buckets = rec->bins;

  while (buckets != NULL) {
    unsigned long cur_addr = rec->base_pc;
    printf("\nfor record at 0x%x (left: 0x%x right: 0x%x\n", 
	   (unsigned) rec, (unsigned) rec->left, (unsigned) rec->right);
    
    for (i = 0; i < BUCKETS_PER_RECORD; i++) {
      if (!(i % 32)) printf("\n0x%x: ", (unsigned) cur_addr);
      cur_addr += 2;
      printf("%x ", buckets->bin[i]);
    }
    printf("\n");
    buckets = buckets->next;
  }

  print_record (rec->left);
  print_record (rec->right);
}


void print_hist_records(void) 
{
  profiling_record_t * rec = xt_profile_base_record;
  print_record(rec);
}
#endif

#define CHECK_RECORD_STRUCTURE 0
#if CHECK_RECORD_STRUCTURE
void walk_record(profiling_record_t * rec)
{
  buckets_t * buckets;
  if (rec == NULL)
    return;
  
  buckets = rec->bins;

  while (buckets)
    buckets = buckets->next;
  
  walk_record(rec->left);
  walk_record(rec->right);
}
#endif

/***********************************************************
 call-graph related functions
***********************************************************/
typedef struct call_graph_rec
{
  unsigned int caller_pc;
  unsigned int self_pc;
  unsigned long long count;
  struct call_graph_rec * next;
} call_graph_record_t;


static char * mem_pool = NULL;
static char * mem_pool_end = NULL;

// 4 k is reasonable
#define SBRK_SIZE 4096 
static inline call_graph_record_t * new_record(void)
{
  const unsigned int bytes = sizeof(call_graph_record_t);
  char * old_mem_pool = mem_pool;

  mem_pool = mem_pool + bytes;
  if (mem_pool > mem_pool_end) {
    mem_pool = xt_dbfs_sbrk(SBRK_SIZE);
    if (mem_pool == (char*)-1)
      return NULL;
    old_mem_pool = mem_pool;
    mem_pool_end = mem_pool + SBRK_SIZE;
    mem_pool = mem_pool + bytes;
  }

  return (call_graph_record_t *) old_mem_pool;
}


#define TABLE_SIZE 1024
call_graph_record_t * cg_table[TABLE_SIZE];

void _mcount(unsigned int raw_caller)
{
  unsigned int raw_callee;
  unsigned int caller_pc;
  unsigned int self_pc;
  unsigned int high_bit_mask = (0x03 << 30);
  unsigned int low_bit_mask = ~high_bit_mask;
  unsigned int pc_high_bits = ((unsigned int) _mcount) & high_bit_mask;
  asm("mov %0, a0": "=a" (raw_callee) : /* no inputs */);

#if __XTENSA_CALL0_ABI__
  caller_pc = raw_caller;
  self_pc = raw_callee;
#else
  caller_pc = pc_high_bits | (raw_caller & low_bit_mask);
  self_pc = pc_high_bits | (raw_callee & low_bit_mask);
#endif
  int idx = caller_pc % TABLE_SIZE;
  call_graph_record_t * rec = cg_table[idx];
    
  while (rec && rec->caller_pc != caller_pc && rec->self_pc != self_pc) {
    rec = rec->next;
  }
  
  if (rec == NULL) {
    rec = (call_graph_record_t *) new_record();
    if (rec == NULL) {
      xt_profile_errors++;
      return;
    }
    rec->caller_pc = caller_pc;
    rec->self_pc = self_pc;
    rec->next = cg_table[idx];
    rec->count = 0;
    cg_table[idx] = rec;
  }
  
  rec->count++;
}

/* this is a dummy function so the timer can tell where _mcount's pc ends */
void _mcount_end(void)
{
}

static int 
write_call_graph_record(int fd, call_graph_record_t * rec)
{
  if (!rec)
    return 0;
  size_t bytes = sizeof(rec->caller_pc) + sizeof(rec->self_pc) + sizeof(rec->count);
  if (write(fd, &call_graph_arc_tag, sizeof(call_graph_arc_tag)) 
      != sizeof(call_graph_arc_tag)) {
    return -1;
  }
  
  if (write(fd, rec, bytes) != bytes) {
    return -1;
  }
  return 0;
}


static int 
write_call_graph(int fd)
{
  int i;
  for (i = 0; i < TABLE_SIZE; i++) {
    call_graph_record_t * rec = cg_table[i];
    while (rec != NULL) {
      if (write_call_graph_record(fd, rec) < 0) {
	xt_profile_errors++;
	return -1;
      }
      rec = rec->next;
    }
  }

  return 0;
}


static void
clear_call_graph(void)
{
  int i;
  for (i = 0; i < TABLE_SIZE; i++) {
    call_graph_record_t * rec = cg_table[i];
    while (rec != NULL) {
      rec->count = 0;
      rec = rec->next;
    }
  }
}

/***********************************************************
 general api functions
***********************************************************/

typedef struct gmon_hdr 
{
  char magic[4];
  unsigned int version;
  unsigned int pad0;
  unsigned int pad1;
  unsigned int pad2;
} gmon_hdr;


#define HDR_SIZE 20

static char *base_filename = "gmon.out.XXXXXX";

static void xt_profiling_save(void)
{
  gmon_hdr gmon_header;
  char * output_filename = (char * ) alloca(strlen(base_filename) + 1);
  int fd;
  my_strcpy (output_filename, base_filename);
  
  fd = my_mkstemp (output_filename);
  if (fd < 0) {
    xt_profile_errors++;
    return;
  }

  gmon_header.magic[0] = 'g';
  gmon_header.magic[1] = 'm';
  gmon_header.magic[2] = 'o';
  gmon_header.magic[3] = 'n';
  gmon_header.version = 1;
  gmon_header.pad0 = 0;
  gmon_header.pad1 = 0;
  gmon_header.pad2 = 0;

  if (write(fd, &gmon_header, HDR_SIZE) != HDR_SIZE) {
    xt_profile_errors++;
    close (fd);
    unlink (output_filename);
    return;
  }
  
  if (write_hist_records(fd) != 0) {
    xt_profile_errors++;
    close (fd);
    unlink (output_filename);
    return;
  }

  if (write_call_graph(fd) != 0) {
    xt_profile_errors++;
    close (fd);
    unlink (output_filename);
    return;
  }
  close(fd);
}


static void __hw_profiling_clear(void)
{
  clear_histogram_record(xt_profile_base_record);
  clear_call_graph();
}

int xt_profile_save_and_reset(void)
{
#if CHECK_RECORD_STRUCTURE
  printf("starting walk...");
  walk_record(xt_profiling_base_record);
  printf("ended\n");
#endif
  if (xt_profile_errors) {
    char * msg = "Hardware Profiling: Errors occurred, not generating output.\n";
    write(2, msg, strlen(msg));
    return 1;
  }
  xt_profiling_save();
  __hw_profiling_clear();
  return 0;
}

int xt_profile_num_errors(void)
{
  return xt_profile_errors;
}
