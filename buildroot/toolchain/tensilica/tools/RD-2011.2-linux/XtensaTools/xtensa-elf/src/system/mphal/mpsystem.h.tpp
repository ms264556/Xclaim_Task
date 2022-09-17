/*  System wide parameters (aka "MP HAL")  */

/*  (this file was generated -- do not edit)  */

/* Copyright (c) 2005-`(localtime)[5]+1900` Tensilica Inc.

   Permission is hereby granted, free of charge, to any person obtaining
   a copy of this software and associated documentation files (the
   "Software"), to deal in the Software without restriction, including
   without limitation the rights to use, copy, modify, merge, publish,
   distribute, sublicense, and/or sell copies of the Software, and to
   permit persons to whom the Software is furnished to do so, subject to
   the following conditions:

   The above copyright notice and this permission notice shall be included
   in all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
   EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
   MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
   IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
   CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
   TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
   SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.  */

; stash_obj_isa($sys, "MultiCoreSystem") or die "system must be of type MultiCoreSystem";
; use Xtensa::AddressLayout;
; my @cores = @{$sys->cores};
; my %configs = ();
; foreach my $core (@cores) {$configs{$core->config} = 1;}
; my $n_configs = 0;
; foreach my $cfg (sort keys %configs) { $configs{$cfg} = $n_configs++; }
#define XMP_NUM_CORES	` scalar @cores`
#define XMP_NUM_CONFIGS	` scalar keys %configs`
#define XMP_SYS_NAME		"`$sys->name`"
#define XMP_SYS_NAME_ID	`$sys->name`
; my $max_dcache_linesize = 4;
; foreach my $i (0 .. $#cores) {
;   my $lineSize = $cores[$i]->sys->pr->dcacheOption() ? $cores[$i]->sys->pr->dcache()->lineSize() : 0;
;   if ($lineSize > $max_dcache_linesize) {
;     $max_dcache_linesize = $lineSize
;   }
; }
#define XMP_MAX_DCACHE_LINESIZE        `$max_dcache_linesize`
;#
;#
; sub show_localmem_addrs {
;   my ($i, $locname, $locmems_ref) = @_;
;   my @locmems = (defined($locmems_ref) ? @$locmems_ref : ());
;   my $n = 0;
;   my $linkmap = $cores[$i]->layer->linkmap;
;   foreach my $locmem (@locmems) {
#define XMCORE_`$i`_`$locname.$n`_VADDR		` sprintf("0x%x", $locmem->baseVAddr)`	/* vaddr local to that core */
;     my ($m) = $linkmap->map->findmaps($locmem->baseVAddr);
;     if (!defined($m)) {
/* WARNING: could not find `$locname.$n` at that address in the default linkmap */
;     } else {
;	my $mem = $m->mems->[0]->{mem};
;	my @vaddrs = $sys->commonmap->addrs_of_space($mem, 0, $mem->sizem1);
;       if (@vaddrs == 0) {
/* No global virtual address found for `$locname.$n` */
;	} else {
;	  my $vaddr = shift @vaddrs;
;	  if (@vaddrs) {
/* Multiple global virtual addresses found for `$locname.$n`, picking first one
   (others are: ` join(", ", map(sprintf("0x%x",$_), @vaddrs))`). */
;	  }
#define XMCORE_`$i`_`$locname.$n`_GLOBAL_VADDR	` sprintf("0x%x", $vaddr)`
;	}
;     }
;     $n++;
;   }
; }
;#
;#
; foreach my $i (0 .. $#cores) {
;   my $core = $cores[$i];
;   my $pr = $core->sys->pr->xt;


/***  Core `$i`  ***/

/*#define XMCORE_`$i`_PRID			0x%X */
#define XMCORE_`$i`_CORE_NAME		"`$core->name`"
#define XMCORE_`$i`_CORE_NAME_ID		`$core->name`
#define XMCORE_`$i`_CONFIG_NAME		"`$core->config`"
#define XMCORE_`$i`_CONFIG_NAME_ID		`$core->config`
#define XMCORE_`$i`_CONFIG_INDEX		`$configs{$core->config}`

/*
 *  These config-specific parameters are already in the compile-time HAL of
 *  each config.  Here we provide them for all configs of a system.
 */
#define XMCORE_`$i`_HAVE_BIG_ENDIAN	`$pr->core->memoryOrder eq "BigEndian" ? 1 : 0`
#define XMCORE_`$i`_MAX_INSTRUCTION_SIZE	`$pr->core->maxInstructionSize`
#define XMCORE_`$i`_HAVE_DEBUG		`$pr->debugOption`
#define XMCORE_`$i`_DEBUG_LEVEL		`$pr->debug->interruptLevel`
/*#define XMCORE_`$i`_HAVE_XEA2		`$pr->core->newExceptionArch`*/
#define XMCORE_`$i`_HAVE_XEAX		`$pr->core->externalExceptionArch`
#define XMCORE_`$i`_RESET_VECTOR_VADDR	` sprintf("0x%X",$pr->vectors->vectorList(0)->baseAddress)`
;#		,core->index, xtensa_find_int_param(core->config->params, "IsaIsBigEndian", 0)
;#		,core->index, xtensa_find_int_param(core->config->params, "IsaMaxInstructionSize", 3)
;#		,core->index, xtensa_find_int_param(core->config->params, "IsaUseDebug", 0)
;#		,core->index, xtensa_find_int_param(core->config->params, "DebugInterruptLevel", 0)
;#		,core->index, xtensa_find_int_param(core->config->params, "ExternalExceptionArch", 0)
;#		,core->index, xtensa_find_int_param(core->config->params, "VectorResetVAddr", 0)

;   show_localmem_addrs($i, "DATARAM", $core->sys->pr->ref_dataRams);
;   show_localmem_addrs($i, "DATAROM", $core->sys->pr->ref_dataRoms);
;   show_localmem_addrs($i, "XLMI",    $core->sys->pr->ref_dataPorts);
;   show_localmem_addrs($i, "URAM",    $core->sys->pr->ref_unifiedRams);
;   show_localmem_addrs($i, "INSTRAM", $core->sys->pr->ref_instRams);
;   show_localmem_addrs($i, "INSTROM", $core->sys->pr->ref_instRoms);
;   #
; }

/*  Macro for creating processor-indexed arrays of parameters:  */
; print "#define XMCORE_ARRAY_P(param)";
; foreach my $i (0 .. $#cores) {
;   printf "%s\t\\\n\t\tXMCORE_%d_ ## param", ($i ? "," : ""), $i;
; }

