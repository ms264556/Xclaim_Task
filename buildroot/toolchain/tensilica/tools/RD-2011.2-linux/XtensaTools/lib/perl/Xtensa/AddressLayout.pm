use Object;
package Xtensa::AddressLayout;
our $global_debug;
BEGIN {
    $global_debug = 0;
    $global_debug = 1 if ($ENV{"XTENSA_INTERNAL_DEBUG"} or "") =~ /^DEBUG/;
    $SIG{__DIE__} = sub { require Carp; Carp::confess(@_) } if $global_debug;
}
our $VERSION = "1.0";
our @ISA = qw(HashObject);
use strict;
sub new {
    my $class = shift;
    my $self = {
	parent		=> undef,	
	name		=> undef,
	children	=> {},		
	linkmap		=> undef,
	context_stack	=> new ArrayObject,	
	linkmap_actions	=> new ArrayObject,
#	prerequisite_contexts	=> {},		# = {a => {needed by a, ...}, ...}
#	requiring_me_contexts	=> {},		# = {a => {needing a, ...}, ...}
    };
    $self->{top} = $self;	
    bless ($self, $class);	
    return $self;		
}
sub add_layout {
    my ($self, $name, $child) = @_;
    $name = "" unless defined($name);
    die "add_layout: layout name must be an identifier: '$name'"
	unless $name =~ /^\w+$/;
    die "add_layout: element '$name' already exists in layout ".$self->name
	if exists($self->ref_children->{$name});
    $self->ref_children->{$name} = $child;
    $child->{parent} = $self;
    $child->{name} = $name;
    $child->{top} = $self->{top};
    return $child;		
}
sub strtoint {
  my ($num, $errwhere) = @_;
  $num =~ s/^\s+//;	
  $num =~ s/\s+$//;
  return oct($num) if $num =~ /^0[0-7]*$/;
  return hex($num) if $num =~ /^0x[0-9a-f]+$/i;
  return $num      if $num =~ /^[1-9][0-9]*$/; 
  die("invalid syntax, integer expected, got '$num' in $errwhere\n"
	."Valid integer formats are [1-9][0-9]* (decimal), 0[0-7]* (octal), 0x[0-9a-f]+ (hex)");
}
sub nameprefix {
    my ($self) = @_;
    defined($self->parent) ? $self->fullname . "." : "";
}
sub fullname {
    my ($self) = @_;
    defined($self->parent) ? $self->parent->nameprefix . $self->name : "";
}
sub lookup_layer {
    my ($self, $findname) = @_;
    my ($layout, $name) = $self->lookup_prefix($findname . ".x", 0);
    return $layout if defined($layout) and $name eq "x";
    return undef;
}
sub find_addressable {
    my ($self, $findname) = @_;
    my ($layout, $name) = $self->lookup_prefix($findname);
    defined($layout) or return undef;
    my $child = $layout->ref_children->{$name};
    return $child if defined($child) and $child->isa("Xtensa::System::Addressable");
    return undef;
}
sub lookup_prefix {
    my ($self, $name, $create) = @_;
    my $layout = $self;
    $layout = $self->top if $name =~ s/^\.//;
    while ($name =~ s/^(\w+)\.//) {	
	my $layname = $1;
	my $next = $layout->ref_children->{$layname};
	if (!defined($next)) {		
	    defined($create) or return (undef, undef);
	    $create or die "lookup_prefix: no layout named ".$layout->nameprefix.$layname;
	    $next = new Xtensa::AddressLayout;
	    $layout->add_layout($layname, $next);
	}
	$next->isa("Xtensa::AddressLayout") or die "lookup_prefix: ".$layout->nameprefix.$layname." is not a layout";
	$layout = $next;
    }
    $name =~ /^\w+$/ or die "lookup_prefix in layout ".$layout->fullname.": name is not an identifier: '$name'";
    return ($layout, $name);
}
sub virtual {
    my ($self) = @_;
    $self->find_addressable("virtual");
}
sub physical {
    my ($self) = @_;
    $self->find_addressable("physical");
}
sub add_addressable {
    my ($self, $name, $addressable, $replace) = @_;
    ($self, $name) = $self->lookup_prefix($name, 1);
    if (!$replace and exists($self->ref_children->{$name})) {
	die "AddressLayout::add_addressable: addressable named '$name'"
	    ." already exists in layer ".$self->fullname;
    }
    $self->ref_children->{$name} = $addressable;
    $addressable->{name} = $name;
    $addressable->{layout} = $self;
    $addressable->{context} = $self->cur_context;
    $addressable;
}
sub addressables {
    my ($self) = @_;
    $self->top->sub_addressables;
}
sub sub_addressables {
    my ($self) = @_;
    my @a = $self->layer_addressables;
    foreach my $s (grep( $_->isa("Xtensa::AddressLayout"), values %{$self->ref_children} )) {
	push @a, $s->sub_addressables;
    }
    @a;
}
sub layer_addressables {
    my ($self) = @_;
    return grep( $_->isa("Xtensa::System::Addressable"), values %{$self->ref_children} );
}
sub addressables_deporder
{
    my ($self) = @_;
    my @list = ();
    my %deps = ();
    foreach my $a ($self->addressables) {
	$deps{$a->fullname} = { map(($_->fullname,1), $a->dependencies) };
    }
    $deps{'*root*'} = { map(($_->fullname,1), $self->addressables) };
    while (keys %deps > 1) {
	my @nodeps = grep(keys %{$deps{$_}} == 0, keys %deps);
	die "addressables_deporder: cyclic dependency among addressables" unless @nodeps;
	push @list, sort @nodeps;
	foreach my $a (@nodeps) { delete $deps{$a}; }
	foreach my $ad (keys %deps) {
	    foreach my $a (@nodeps) { delete ${$deps{$ad}}{$a}; }
	}
    }
    @list = map($self->top->find_addressable($_), @list);	
    @list == scalar($self->addressables) or die "oops, internal error: wrong list size";
    @list;
}
sub  format_addressables {
    my ($self, $skipcontexts) = @_;
    defined($skipcontexts) or $skipcontexts = {};
    my @lines = ();
    push @lines, "<addressables>";
    if (0) {
	foreach my $ad (sort {$a->fullname cmp $b->fullname} $self->addressables) {
	    push @lines, $ad->format_self(1,0) if $ad->is_addrspace;
	}
	foreach my $ad (sort {$a->fullname cmp $b->fullname} $self->addressables) {
	    push @lines, $ad->format_self() if !$ad->is_addrspace;
	}
	foreach my $ad (sort {$a->fullname cmp $b->fullname} $self->addressables) {
	    push @lines, $ad->format_self(0,1) if $ad->is_addrspace;
	}
    } else {
	foreach my $ad ($self->addressables_deporder) {
	    push @lines, $ad->format_self(1,1,$skipcontexts);
	}
    }
    push @lines, "</addressables>";
    wantarray ? @lines : join("\n", @lines);
}
sub parse_addressables {
    my ($self, $clear, @lines) = @_;
    %{$self->ref_children} = () if $clear;	
    chomp(@lines);
    my $n = @lines;
    while (1) {
	@lines or die "AddressLayout::parse_addressables: unexpected end of input, missing <addressables>";
	$_ = shift @lines;
	last if /^\s*\<addressables\>\s*$/;
    }
    while (1) {
	@lines or die "AddressLayout::parse_addressables: unexpected end of input, missing </addressables>";
	return $self->addressables if $lines[0] =~ /^\s*\<\/addressables\>\s*$/;
	if ($lines[0] =~ /^\s*\<memdev\s+/) {
	    Xtensa::System::Addressable->new_parse($self, \@lines);
	} elsif ($lines[0] =~ /^\s*\<space\s+/) {
	    Xtensa::System::AddressSpace->new_parse($self, \@lines);
	} elsif ($lines[0] =~ /^\s*\<segment\s+/) {
	    $self->parse_segment(\@lines, $n + 1 - @lines);
	} else {
	    die "AddressLayout::parse_addressables: unexpected line: ".$lines[0];
	}
    }
}
sub  read_addressables {
    my ($self, $clear, $filename) = @_;
    open(XMAP,"<$filename") or return "could not read $filename: $!";
    $self->parse_addressables($clear, <XMAP>);
    close(XMAP) or return "could not close $filename: $!";
    return undef;
}
sub parse_segment {
    my ($layout, $lines, $lineno) = @_;
    $_ = shift @$lines;
    my $saveline = $_;
    s/^\s*\<segment\s+//;
    my $attrs = Xtensa::System::Attributes->parse_attributes(\$_);
    /^\/\>\s*$/ or die "AddressLayout::parse_segment: bad end of line, expected '/>' : <<$_>>";
    $layout->top->linkmap_actions->push([$attrs, $saveline, $lineno]);
}
sub apply_linkmap_actions {
    my ($layout, $linkmap) = @_;
    foreach my $action ($layout->linkmap_actions->a) {
	my ($attrs, $saveline, $lineno) = @$action;
	my $memdevname = $$attrs{'memdev'};	delete $$attrs{'memdev'};
	my $context = $$attrs{'context'};	delete $$attrs{'context'};
	my $lock = $$attrs{'lock'};		delete $$attrs{'lock'};
	my $fixed = $$attrs{'fixed'};		delete $$attrs{'fixed'};
	my $cutstart = $$attrs{'cutstart'};	delete $$attrs{'cutstart'};
	my $cutend = $$attrs{'cutend'};		delete $$attrs{'cutend'};
	my $sections = $$attrs{'sections'};	delete $$attrs{'sections'};
	$layout->push_context($context) if defined($context);
	my $seg;
	my $memdev;
	if (defined($memdevname)) {
	    $memdev = $layout->find_addressable($memdevname);
	    defined($memdev) or die "AddressLayout:apply_linkmap_actions: memory/device '$memdevname' not found";
	    $seg = $linkmap->largest_avail_seg($memdev);
	} else {
	    $cutstart = strtoint($cutstart, "apply_linkmap_actions") if defined($cutstart);
	    $cutend   = strtoint($cutend,   "apply_linkmap_actions") if defined($cutend);
	    my $what = defined($sections) ? "section(s) $sections" : undef;
	    $seg = $linkmap->cut_map($cutstart, $cutend, 0, $what);
	}
	defined($seg) or die "AddressLayout::apply_linkmap_actions: no segment for (line $lineno): $saveline";
	if (defined($sections)) {
	    foreach my $secname (split(" ", $sections)) {
		Xtensa::Section->new($secname, undef, $seg);
	    }
	}
	$seg->set_attr('lock', $lock) if $lock;
	$seg->set_attr('fixed',1) if $fixed;
	$layout->pop_context() if defined($context);
    }
    $layout->linkmap_actions->set();		
}
sub all_attributes {
    my ($self) = @_;
    my @all = $self->addressables;
    foreach my $a ($self->addressables) {
	push @all, $a->map->a if $a->is_addrspace;
    }
    @all;
}
sub push_context {
    my $self = shift;
    $self->top->context_stack->push(@_);
}
sub pop_context {
    my $self = shift;
    $self->top->context_stack->pop;
}
sub cur_context {
    my ($self) = @_;
    my @c = $self->top->context_stack->a;
    return 'none' unless @c;
    return $c[$#c];
}
sub insert_section {
    my ($self, $segment, $secname, $secorder) = @_;
    my $sec = Xtensa::Section->new($secname, $secorder, $segment);
    my @cuts = $self->linkmap->map->cut_mem_aliases($segment, $segment, 1, 0);
    Xtensa::System::Attributes::multi_set_attr(\@cuts, 'delete'=>1, 'lock'=>"section $secname");
    $sec;
}
sub delete_misvalign {
    my ($self, $linkmapmap) = @_;
    foreach my $m ($linkmapmap->a) {
	my $paddr = $self->virtual->addr_in_space($m->addr, $m->endaddr, $self->physical);
	my $valign = $m->is('valign');
	next unless defined($paddr) and defined($valign);
	if (($paddr & $valign) != ($m->addr & $valign)) {
	  my $what = ($valign == 0xFFFFFFFF) ? "non-identity"
					     : sprintf("misaligned (on bits 0x%x)", $valign);
	  $what .= " mapping to ". $m->ref_mems->[0]{mem}{name};
	  printf STDERR "WARNING: initLayout: dropping $what from vaddr 0x%x to paddr 0x%x\n",
	  		$m->addr, $paddr;
	  $m->set_attr('delete', 1);		# can't use this segment
	  $m->set_attr('lock', $what);		# can't use this segment
	}
    }
}
sub initLayout {
    my ($self, $pconfig, $noSysSegs, $extraChecks) = @_;
    my $virtual = $self->virtual;
    my $linkmap = Xtensa::System::LinkMap->new($self->virtual, $self->physical, default_layer => $self);
    $self->add_addressable("linkmap", $linkmap);	
    $self->linkmap($linkmap);
    $self->top->apply_linkmap_actions($linkmap);
    $self->delete_misvalign($linkmap->map);
    my @sorted_vectors = sort {$a->vaddr <=> $b->vaddr} @{$pconfig->vectors};
    my @reloc_vectors  = sort {$a->vaddr <=> $b->vaddr} @{$pconfig->vectors_reloc};
    foreach my $vec (@sorted_vectors) {
	my $seg = $linkmap->cut_map($vec->vaddr, $vec->vaddr + $vec->size - 1, 0,
					"vector ".$vec->name);
	$seg->set_attr('lock',$vec->name);	
	$seg->set_attr('fixed',1);		
	$self->insert_section($seg, "." . $vec->name . ".text", "first");
	if ($vec->name eq "RelocatableVectors") {
	    foreach my $relvec (@reloc_vectors) {
		$self->insert_section($seg, "." . $relvec->name . ".text");
	    }
	}
    }
    my ($seg) = $linkmap->map->a;
    $seg->truncate(4, undef)
	if $seg->addr == 0 and !$seg->is('lock') and !$seg->is('device') and $seg->sizem1 >= 16;
    foreach my $vec (@sorted_vectors) {
	next if $vec->name eq 'WindowVectors';		
	my ($seg) = $linkmap->findmaps($vec->vaddr);
	my $litseg = literal_for_text($linkmap, $pconfig, $seg, 4, 1, 0, 1);
	if (defined($litseg)) {
	    my @cuts = $linkmap->map->cut_mem_aliases($litseg, $litseg, 1, 0);
	    Xtensa::System::Attributes::multi_set_attr(\@cuts,
			'delete'=>1, 'lock'=>"literals for ".$vec->name);
	}
    }
    foreach my $mem ( $linkmap->memories ) {
	my $seg = $linkmap->largest_avail_seg($mem, sub { $_->is('executable') });
	if (defined($seg) and !$seg->is('data')) {	
	    literal_for_text($linkmap, $pconfig, $seg, undef, 1, 1, 1);	
	}
    }
    foreach my $seg ($linkmap->map->a) {
	$seg->alloc_size(0);
    }
    my %segs_with_lits = ();	
    foreach my $mem ( $linkmap->memories ) {
	my $mname = $mem->name;		
	my $literalSeg;
	my $seg = $linkmap->largest_avail_seg($mem, sub { $_->is('executable') });
	if ( defined($seg) ) {
	    $self->insert_section($seg, ".${mname}.text");
	    $literalSeg = literal_for_text($linkmap, $pconfig, $seg, undef, 0, 1, 1);
	    $segs_with_lits{$seg} = 1;
	}
	$seg = $linkmap->largest_avail_seg($mem, sub { $_->is('data') });
	defined($seg) and $self->insert_section($seg, ".${mname}.rodata");
	$literalSeg = $seg unless defined($literalSeg);
	defined($literalSeg) and $self->insert_section($literalSeg, ".${mname}.literal");
	$seg = $linkmap->largest_avail_seg($mem, sub { $_->is('data') && $_->is('writable') });
	defined($seg) and $self->insert_section($seg, ".${mname}.data");
	defined($seg) and $self->insert_section($seg, ".${mname}.bss");
    }	
    my $maxTextSeg = $linkmap->largest_avail_seg(undef,
			sub { $_->is('executable') }, sub { $_->is('writable') });
    defined($maxTextSeg) or die( "Error: AddressLayout::initLayout: No memory segment for .text section");
    my $maxDataSeg = $linkmap->largest_avail_seg(undef,
			sub { $_->is('data') && $_->is('writable') },
			sub { $_->is('type') ne 'dataPort' } );
    defined($maxDataSeg) or die( "Error: AddressLayout::initLayout: No memory segment for .data section");
    my $maxRodataSeg = $maxDataSeg;
    my $maxLiteralSeg;
    if ($maxTextSeg->is('data')) {
	$maxLiteralSeg = $maxTextSeg;
    } else {
	$maxLiteralSeg = literal_for_text($linkmap, $pconfig, $maxTextSeg, undef, 0, 1,
						!exists($segs_with_lits{$maxTextSeg}));
    }
    defined($maxLiteralSeg) or $pconfig->litbase_l32r or die( "Error: AddressLayout::initLayout: Could not find suitable memory segment for .literal section in range of .text section");
    $self->insert_section($maxTextSeg,    ".text");
    $self->insert_section($maxLiteralSeg, ".literal") if defined($maxLiteralSeg);
    $self->insert_section($maxRodataSeg,  ".rodata");
    $self->insert_section($maxRodataSeg,  ".lit4") if $pconfig->litbase_l32r;
    $self->insert_section($maxDataSeg,    ".data");
    $self->insert_section($maxDataSeg,    ".bss");
    $self->insert_section($maxDataSeg,    "STACK") unless $noSysSegs;
    $self->insert_section($maxDataSeg,    "HEAP")  unless $noSysSegs;
    foreach my $vec (@sorted_vectors) {
	next if $vec->name eq 'WindowVectors';	
	my ($seg) = $linkmap->findmaps($vec->vaddr);	
	my $literalSeg = literal_for_text($linkmap, $pconfig, $seg, 4, 0, 0, 1);
	if ( !defined($literalSeg) ) {
	    if ($vec->name eq 'ResetVector') {
		next;
	    } else {
		next unless $extraChecks;
		die sprintf("Error: AddressLayout::initLayout: no place for literals preceding %s at 0x%08x",
			    $vec->name, $vec->vaddr);
	    }
	}
	$self->insert_section($literalSeg, "." . $vec->name . ".literal");
	if ($vec->name eq "RelocatableVectors") {
	    foreach my $relvec (@reloc_vectors) {
		$self->insert_section($literalSeg, "." . $relvec->name . ".literal");
	    }
	}
    }
    if (($pconfig->xea eq "XEAX" or $pconfig->xea eq "XEAHALT") and !$noSysSegs
	  and !grep($_->name eq 'ResetVector', (@sorted_vectors, @reloc_vectors))) {
	my $ResetImem = $self->find_addressable('iram0');
	$ResetImem = $self->find_addressable('uram0') unless defined($ResetImem);
	defined($ResetImem) or die "Error: AddressLayout::initLayout: Expected either iram0 or uram0 for task engine";
	my @segs = grep(grep($_->{mem} eq $ResetImem, $_->mems->a), $linkmap->map->a);
	die(sprintf("Error: AddressLayout::initLayout: Expected exactly one segment in reset i-memory '%s' for task engine reset, saw %d; aborting", $ResetImem->fullname, scalar @segs))
	    unless (@segs==1);
	$self->insert_section($segs[0], ".ResetVector.text", "first");
	$self->insert_section($maxLiteralSeg, ".ResetVector.literal") if defined($maxLiteralSeg);
    }
    my $largestRom = $linkmap->largest_avail_seg(undef,
		sub { $_->is('executable') && $_->is('data') && !$_->is('writable') });
    if (defined($largestRom) and $largestRom->sizem1 + 1 > 32 and !$noSysSegs) {
	$self->insert_section($largestRom, ".rom.store");
    }
    $linkmap->map->set( grep(!$_->is('delete'), $linkmap->map->a) );
    LOOP: {
	my @sort_segs = $linkmap->map->sort_by_preference;
	foreach my $seg (@sort_segs) {
	    $linkmap->map->cut_mem_aliases($seg, $seg, 1, 1) and redo LOOP;
	}
    }
}
sub is_ptp_mmu {
  my ($core) = @_;
  my $mmu = $core->sys->pr->xt->mmu;
  return 0 unless defined($mmu) and defined($mmu->itlb) and defined($mmu->dtlb);
  return ( $mmu->asidBits >= 2
	&& $mmu->ringCount >= 2 );
}
sub initMulticoreLayout {
    my ($self, $sys, %options) = @_;
    my $cores = $sys->cores;			
    my $cores_sh = $sys->cores_sh;		
    my $extralog = $options{extralog};		
print STDERR "*** In initMulticoreLayout\n";
print $extralog "*** In initMulticoreLayout\n" if $extralog;
    $sys->cc->ref_map->check;
    my $ccmap = $sys->cc->extract(0,0xFFFFFFFF,2);
    my $createmems    = $options{createmems};
    $createmems = ($ccmap->n == 0) unless defined($createmems);
    my $createlocmems = $options{createlocmems} ? 1 : 0;
    @$cores or die "initMulticoreLayout: need at least one core!";
    foreach my $core (@$cores_sh) {
	my $core_layer = $self->lookup_layer($core->name);
print STDERR "************>> ".$core->name." ($core): $core_layer\n";
	$core->set_layer($core_layer);
my $core_layer2 = $core->layer;
print STDERR "*************> ($core) $core_layer2\n";
    }
    if ($createlocmems) {
	foreach my $core (@$cores) {
	    my $core_layer = $core->layer;
	    my $pspace = $core_layer->find_addressable('physical');
	    foreach my $ifacemap ($pspace->map->a) {
		my $iface = $ifacemap->ref_mems->[0]->{mem};
		if ($iface->has_attr('local')) {
		    $iface->map->n and die "initMulticoreLayout: was asked to create local mems, but interface ".$iface->name." already has something mapped to it";
		    my $locmem = Xtensa::System::Addressable->new($iface->sizem1, undef, {});
		    my $memname = $iface->name;		
		    $memname =~ s/_iface$// or $memname .= "_mem";	
		    $core_layer->add_addressable($memname, $locmem);
		    $iface->add_mapping(0, [[$locmem, 0, $iface->sizem1]], {});
		}
	    }
	}
    }
    my $commonmap;
    my $create_linkmaps = sub {
	my ($precreate) = @_;
	foreach my $core (@$cores_sh) {
	    my $core_layer = $core->layer;
	    my $linkmap = Xtensa::System::LinkMap->new($core_layer->virtual, $core_layer->physical,
				    'withspaces' => $precreate, default_layer => $core_layer,
				    );
	    $core_layer->add_addressable("linkmap", $linkmap, 1);
	    $core_layer->linkmap($linkmap);     $linkmap->map->check;
	    foreach my $m ($linkmap->map->a) {
		my $paddr = $core_layer->virtual->addr_in_space($m->addr, $m->endaddr, $core_layer->physical);
		my $valign = $m->is('valign');
		next unless defined($paddr) and defined($valign);
		if (($paddr & $valign) != ($m->addr & $valign)) {
		  my $what = ($valign == 0xFFFFFFFF) ? "non-identity"
						     : sprintf("misaligned (on bits 0x%x)", $valign);
		  $what .= " mapping to ". $m->ref_mems->[0]{mem}{name};
		  printf STDERR "WARNING: initMulticoreLayout: dropping $what from vaddr 0x%x to paddr 0x%x\n",
				$m->addr, $paddr;
		  $m->set_attr('delete', 1);		# can't use this segment
		  $m->set_attr('lock', $what);		# can't use this segment
		}
	    }
	}
print $extralog "\n*** INITIAL LINKMAPS:\n", scalar($self->format_addressables), "\n\n" if $extralog;
print STDERR "*** Created linkmaps, intersecting...\n";
	foreach (@$cores_sh) {$_->layer->linkmap->ref_map->check;}	
	$commonmap = $cores->[0]->layer->linkmap->map->dup;
	foreach (@$cores_sh) {$_->layer->linkmap->ref_map->check;}	
	$commonmap->check;						
	foreach my $i (1 .. $#$cores_sh) {
	    $cores_sh->[$i]->layer->linkmap->ref_map->check;		
	    $commonmap = $commonmap->intersect($cores_sh->[$i]->layer->linkmap->ref_map);
	    foreach (@$cores_sh) {$_->layer->linkmap->ref_map->check;} 
	    $cores_sh->[$i]->layer->linkmap->ref_map->check;		
	}
	$commonmap->check;						
	foreach (@$cores_sh) {$_->layer->linkmap->ref_map->check;}	
	if ($commonmap->n == 0) {	
	    die "initMulticoreLayout: cores have no shared space: no virtual address range mapping to the same memory or device!";
	}
print $extralog "\n*** INTERSECT MAP:\n", join("\n",map($_->format_self,@$commonmap)), "\n\n" if $extralog;
print STDERR "*** Got intersect\n";
	foreach (@$cores_sh) {$_->layer->linkmap->ref_map->check;}	
	if (!$precreate) {
	    foreach my $core (@$cores) {
		my $map = $core->layer->linkmap->ref_map;
		@$map = grep(!($_->is('local') && $_->is('delay')), @$map);
	    }
	}
    };
    my $auto_partitioning = 0;
    if ($createmems) {
	&$create_linkmaps(1);
print STDERR "*** Preparing to create system memories\n";
	sub add_sysmem {
	    my ($top, $sys, $createmap, $addr, $size, $name, $showname, $attrs) = @_;
	    $createmap->cut_mem_aliases([$sys->cc, $addr, $addr + ($size-1)], undef);
	    my $sysmem = Xtensa::System::Addressable->new($size - 1, undef, $attrs);
	    $top->add_addressable($name, $sysmem);
	    $sys->cc->add_mapping($addr, [[$sysmem, 0, $size - 1]], {});
printf STDERR "*** $showname created at 0x%x .. 0x%x\n", $addr, $addr + ($size - 1);
	}
	my $sysram_size = 128 * 1024 * 1024;	
	my $sysrom_minsz =  1 * 1024 * 1024;	
	my $sysrom_size =  16 * 1024 * 1024;	
	my $sysrom_maxsz = 64 * 1024 * 1024;	
	$ccmap->n and die "initMulticoreLayout: there is already something connected to the CC";
print STDERR "*** Listing sharable segments containing static vectors\n";
print $extralog "*** Listing sharable segments containing static vectors\n" if $extralog;
	my $createmap = $commonmap->dup;	
	my $rom_addr = undef;
	my $rom_size = undef;
	if (grep(is_ptp_mmu($_), @$cores)) {
	    $rom_addr = 0xFE000000;
	    $rom_size = $sysrom_size;
	} else {
	  foreach my $core (@$cores) {
	    my ($vec) = grep( $_->name eq "ResetVector", @{$core->pconfig->vectors} );
	    next unless defined($vec);
	    printf $extralog "** Core %s RESET VEC addr=0x%08x size=0x%x\n",
			$core->name, $vec->vaddr, $vec->size if $extralog;
	    my $vecend = $vec->vaddr + ($vec->size - 1);
	    my $seg = $createmap->cuttable_map($vec->vaddr, $vecend);
	    next unless defined($seg);
	    my ($minsz, $maxsz) = natur_aligned_sizes($vec->vaddr, $vecend, $seg->addr, $seg->endaddr);
	    if ($minsz == 0 or $minsz > $sysrom_maxsz or $maxsz < $sysrom_minsz) {
		die sprintf("initMulticoreLayout: cannot fit aligned ROM of 0x%x to 0x%x bytes around reset vector (at 0x%08x .. 0x%08x) in the available space (0x%08x .. 0x%08x)", $sysrom_minsz, $sysrom_maxsz, $rom_addr, $rom_addr + ($sysrom_size - 1), $seg->addr, $seg->endaddr);
	    }
	    $rom_size = $sysrom_size;
	    $rom_size = $maxsz if $maxsz < $sysrom_size;
	    $rom_size = $minsz if $minsz > $sysrom_size;
	    if ($rom_size != $sysrom_size) {
		printf STDERR "WARNING: initMulticoreLayout: cannot fit aligned ROM of exactly 0x%x bytes around reset vector (at 0x%08x .. 0x%08x) in the available space (0x%08x .. 0x%08x): creating ROM of 0x%x bytes instead", $sysrom_size, $rom_addr, $rom_addr + ($sysrom_size - 1), $seg->addr, $seg->endaddr, $rom_size;
	    }
	    $rom_addr = ($vec->vaddr & -$rom_size);		
	    last;
	  }
	}
	add_sysmem($self, $sys, $createmap, $rom_addr, $rom_size, "sysrom", "system ROM", {writable=>0})
		if defined($rom_addr);
	sub fitting_locations {
	    my ($createmap, $size, $whichgb, $locations, $what) = @_;
	    foreach my $seg ($createmap->a) {
		my $min = ($seg->addr == 0) ? 0
			  : (($seg->addr - 1) & -$size) + $size;
		my $max = ($seg->endaddr == 0xffffffff) ? 0xffffffff
			  : (($seg->endaddr + 1) & -$size);
		next if $max == 0;
		$max-- unless $max == 0xffffffff;
		next if $max < $min;
		if (defined($whichgb)) {	
		    $min = $whichgb if $whichgb >= $min and $whichgb < $max;
		}
		if ($min <= 0x40000000 and $max > 0x40000000 and $size <= 0x40000000) {
		    $locations->{0+0x40000000} += 0;		
		} elsif ($min <= 0x20000000 and $max > 0x20000000 and $size <= 0x20000000) {
		    $locations->{0+0x20000000} += 0;		
		} elsif ($min == 0 and $max > $size) {
		    $locations->{0+$size} += 0;			
		} else {
		    $locations->{0+$min} += 0;			
		}
	    }
	    if (scalar(keys(%$locations)) == 0) {
		die sprintf("initMulticoreLayout: cannot find space for an aligned $what of 0x%x bytes", $size);
	    }
	    return sort {  $locations->{$a} <=> $locations->{$b}
			or ((($b & 0xc0000000) == $whichgb) <=> (($a & 0xc0000000) == $whichgb))
			or (($a == 0) <=> ($b == 0))
			or $a <=> $b
			} keys %$locations;
	}
	my $ram_addr;
	if (grep(is_ptp_mmu($_), @$cores)) {
	    $ram_addr = 0x00000000;
	} else {
	    my %ramvecs = ();
	    foreach my $core (@$cores) {
		foreach my $vec (@{$core->pconfig->vectors}) {
		    next if $vec->group eq "dynamic";
		    printf $extralog "** Core %s VEC addr=0x%08x size=0x%x %s\n",
				$core->name, $vec->vaddr, $vec->size, $vec->name if $extralog;
		    my $seg = $createmap->cuttable_map($vec->vaddr, $vec->vaddr + ($vec->size - 1));
		    next unless defined($seg);
		    my $addr = ($vec->vaddr & -$sysram_size);
		    next unless $seg->addr <= $addr and $addr + ($sysram_size-1) <= $seg->endaddr
			    and $vec->vaddr + ($vec->size - 1) <= $addr + ($sysram_size - 1);
		    $ramvecs{0+$addr}++;
		}
	    }
	    my $romgb = defined($rom_addr) ? ($rom_addr & 0xc0000000) : 123;
	    ($ram_addr) = fitting_locations($createmap, $sysram_size, $romgb, \%ramvecs, "system RAM");
	}
	my $ram_size = $sysram_size;	
	add_sysmem($self, $sys, $createmap, $ram_addr, $ram_size, "sysram", "system RAM", {writable=>1});
	if (!defined($rom_addr)) {
	    ($rom_addr) = fitting_locations($createmap, $sysrom_size, ($ram_addr & 0xc0000000), { }, "system ROM");
	    $rom_size = $sysrom_size;
	    add_sysmem($self, $sys, $createmap, $rom_addr, $rom_size, "sysrom", "system ROM", {writable=>0});
	}
	my $maxgap = 500 * 1024;	
	my %vecsegs = ();
	foreach my $core (@$cores) {
	    foreach my $vec (@{$core->pconfig->vectors}) {
		next if $vec->group eq "dynamic";
		printf $extralog "** Core %s VEC addr=0x%08x size=0x%x %s\n",
			    $core->name, $vec->vaddr, $vec->size, $vec->name if $extralog;
		my $seg = $createmap->cuttable_map($vec->vaddr, $vec->vaddr + ($vec->size - 1));
		next unless defined($seg);
		$vecsegs{$seg}[0] = $seg;
		push @{$vecsegs{$seg}[1]}, $vec;
	    }
	}
	my @little_mems;
	my $littles = 0;
	if (scalar(keys %vecsegs)) {
	    print STDERR "WARNING: initMulticoreLayout: vectors are spread out, creating extra little system RAMs to contain them\n";
	    foreach my $segindex (keys %vecsegs) {
		my ($seg, $vecs) = @{$vecsegs{$segindex}};
		my @v = sort { $a->vaddr <=> $b->vaddr } @$vecs;
		my @memvecs = (shift @v);
		foreach my $vec (@v, undef) {
		    if (!defined($vec) or $vec->vaddr - ($memvecs[-1]->vaddr + $memvecs[-1]->size) >= $maxgap) {
			my $addr = $memvecs[0]->vaddr;
			my $endaddr = $memvecs[-1]->vaddr + ($memvecs[-1]->size - 1);
			for (my $align = 16; $align < 2 * ($endaddr - $addr); $align *= 2) {
			    my $newaddr = ($addr & -$align);
			    my $newend = (($endaddr + $align) & -$align) - 1;
			    $addr = $newaddr if $newaddr >= $seg->addr;
			    $endaddr = $newend if $newend <= $seg->endaddr
			    	and (!defined($vec) or $newend < $vec->vaddr);
			}
			push @little_mems, [$addr, $endaddr];
			my $is_rom = grep($_->name eq 'ResetVector', @memvecs);
			add_sysmem($self, $sys, $createmap, $addr, $endaddr - $addr + 1,
					($is_rom ? "extrarom" : "extraram").$littles,
					($is_rom ? "Extra ROM " : "Extra RAM ").$littles,
					{writable => ($is_rom ? 0 : 1)});
			$littles++;
			@memvecs = ();
		    }
		    push @memvecs, $vec;
		}
	    }
	}
	sub natur_aligned_sizes {
	    my ($addr,$endaddr, $in_addr,$in_endaddr) = @_;
	    my ($minsz,$maxsz) = (0,0);
	    for (my $sizem1 = 0; $sizem1 <= 0x7fffffff; $sizem1 = $sizem1*2 + 1) {
		my $maddr = ($addr & ~$sizem1);
		my $mend = $maddr + $sizem1;
		next if $mend < $endaddr;		
		last if $maddr < $in_addr or $mend > $in_endaddr;	
		$minsz = $sizem1+1 if $minsz == 0;
		$maxsz = $sizem1+1;
	    }
	    return ($minsz, $maxsz);
	}
	$ccmap = $sys->cc->extract(0,0xFFFFFFFF,2);
	&$create_linkmaps(0);
	$auto_partitioning = 1;
    } else {
	&$create_linkmaps(0);
print STDERR "*** Using existing system memories\n";
	$ccmap->n or die "initMulticoreLayout: there is nothing connected to the CC";
	foreach my $iseg (@$commonmap) {
	    $ccmap->check;
	    $ccmap->cut_mem_aliases($iseg, undef);
	}
	$ccmap->n and die "initMulticoreLayout: there are things connected to the CC"
		." that are not accessible at the same virtual address by all cores:\n".$ccmap->dump;
	$ccmap->check;
	$commonmap->check;
    }
    foreach (@$cores_sh) {$_->layer->linkmap->ref_map->check;}
    $sys->set_commonmap( $commonmap );
    my @partitions = defined($sys->partitions) ? @{$sys->partitions} : ();
    if (!@partitions or $auto_partitioning) {
print STDERR "*** Auto-partitioning biggest system RAM and ROM ...\n";
	my $auto_partition = sub {
	    my ($seg, $what) = @_;
	    my ($addr, $endaddr) = ($seg->addr, $seg->endaddr);
printf STDERR "*** Auto-partitioning $what at 0x%x .. 0x%x\n", $addr, $endaddr;
	    my $gb = ($endaddr & 0xc0000000);
	    if ($gb > $addr) {
		if ($gb > $addr + ($endaddr - $addr) / 2) {
		    $endaddr = $gb - 1;
		} else {
		    $addr = $gb;
		}
printf STDERR "*** Chopped down $what partitions to 0x%x .. 0x%x to avoid crossing GB boundary\n", $addr, $endaddr;
	    }
	    my $mem = $seg->ref_mems->[0]{mem};
	    my $halfsize = ($endaddr - $addr + 1) / 2;
	    my $ndiv = scalar(@$cores);
	    $ndiv++ while ($ndiv & ($ndiv - 1)) != 0;
	    $ndiv = 4 if $ndiv < 4;
	    my $nsize = $halfsize / $ndiv;
	    push @partitions, bless {corename => undef, memname => $mem->name, size => $halfsize, offset => 0},
	    				'HashObject';
	    for (my $i = 0; $i <= $#$cores; $i++) {
		push @partitions, bless {corename => $cores->[$i]->name, memname => $mem->name,
				   size => $nsize, offset => $halfsize + $i * $nsize}, 'HashObject';
	    }
	};
	my $rom = $commonmap->largest_avail_mapping(undef,
		    sub { $_->is('executable') && $_->is('data') && !$_->is('writable') });
	unless (defined($rom) and $rom->sizem1 >= 1024 * 1024 - 1) {
	    die "initMulticoreLayout: no sufficiently large ROM (1MB+) available to partition among all cores";
	}
	&$auto_partition($rom, "system ROM");
	my $ram = $commonmap->largest_avail_mapping(undef,
		    sub { $_->is('executable') && $_->is('data') && $_->is('writable') });
	unless (defined($ram) and $ram->sizem1 >= 1024 * 1024 - 1) {
	    die "initMulticoreLayout: no sufficiently large RAM (1MB+) available to partition among all cores";
	}
	&$auto_partition($ram, "system RAM");
    }
print STDERR "*** Placing static vectors\n";
    my @vecs = ();
    foreach my $core (@$cores) {
	my $linkmap = $core->layer->linkmap;
	my @sorted_vectors = sort {$a->vaddr <=> $b->vaddr} @{$core->pconfig->vectors};
	foreach my $vec (@sorted_vectors) {
	    next if $vec->group eq "dynamic";
	    printf $extralog "** Core %s VEC addr=0x%08x vaddr=0x%08x %s\n",
			$core->name, $vec->vaddr, $vec->vaddr, $vec->name if $extralog;
	    my $what = $vec->name." vector on core ".$core->name;
	    my $seg = $linkmap->cut_map($vec->vaddr, $vec->vaddr + $vec->size - 1, 0, $what);
	    my $lock = $seg->has_attr('lock');
	    die "initMulticoreLayout: reserving space for $what: segment already locked! (for $lock)" if $lock;
	    $seg->set_attr('lock',$what);	
	    $seg->set_attr('fixed',1);		
	    push @vecs, [$core, $vec, $what, $seg];  
	}
    }
print STDERR "*** Listing shared vectors\n";
    my @deferred_sections = ();
    my @shared_sets = ();
    while (@vecs) {
	my $current = pop @vecs;
	my ($core, $vec, $what, $seg) = @$current;
	my $vec_mem = $seg->ref_mems->[0]{mem};
	my $vec_end = $vec->vaddr + ($vec->size - 1);
	my @shared = ($current);
	my %sharedcores = ($core => 1);
	my @copyvecs = @vecs;	
	foreach my $v (@copyvecs) {
	    my ($vcore, $vvec, $vwhat, $vseg) = @$v;
	    next unless $vec_mem eq $vseg->ref_mems->[0]{mem};
	    my $vvec_end = $vvec->vaddr + ($vvec->size - 1);
	    next if $vvec->vaddr > $vec_end or $vvec_end < $vec->vaddr;
	    $vec->vaddr == $vvec->vaddr
		or die "initMulticoreLayout: vectors partially overlap: $what and $vwhat";
	    $vec->name eq $vvec->name
		or die "initMulticoreLayout: mismatching vectors overlap: $what and $vwhat";
	    $vec->name eq "ResetVector"
		or die "initMulticoreLayout: sharing vectors other than reset not supported: $what and $vwhat";
	    $core->sys->pr->xt->core->memoryOrder eq $vcore->sys->pr->xt->core->memoryOrder
		or die "initMulticoreLayout: cannot share vectors of cores of different endianness: $what and $vwhat";
	    $core->sys->pr->xt->usePRID
		or die "initMulticoreLayout: cannot share $what without PRID (shared with $vwhat)";
	    $vcore->sys->pr->xt->usePRID
		or die "initMulticoreLayout: cannot share $vwhat without PRID (shared with $what)";
	    if ($vec->size != $vvec->size) {
		$vec_end = $vvec_end if $vvec_end > $vec_end;	
		print STDERR "WARNING: initMulticoreLayout: shared vectors have different sizes,\n"
			    ."WARNING:   using largest (", $vec_end - $vec->vaddr + 1, " bytes)\n"
			    ."WARNING:   for $what and $vwhat\n";
	    }
	    push @shared, $v;
	    @vecs = grep($_ ne $v, @vecs);
	    $sharedcores{$vcore} = 1;
	}
	$what = $vec->name;		
	if (@shared > 1) {
	    push @shared_sets, [$vec->name, $vec->vaddr, $vec_end, [map($_->[0], @shared)], $vec_mem];
	    if (@shared == @$cores) {
		$what .= " shared among all cores";
	    } else {
		$what .= " shared among cores ".join(",",map($_->[0]->name, @shared));
	    }
print STDERR "**** Adding section for $what\n";
	    foreach my $v (@shared) {
		my ($vcore, $vvec, $vwhat, $vseg) = @$v;
		Xtensa::Section->new(".Shared" . $vvec->name . ".text", "first", $vseg);
		my $vec_text_name = "." . $vvec->name . ".text";
		print STDERR "***Deferring private portions of shared vector $vec_text_name on core ", $vcore->name, "\n";
		push @deferred_sections, [$vvec->name, $vec_mem, $vcore, $vvec->size];
		$vvec->set_name("Shared".$vvec->name);
	    }
	} else {
	    $what .= " for core ".$core->name;
print STDERR "**** Adding section for $what\n";
	    Xtensa::Section->new("." . $vec->name . ".text", "first", $seg);
	}
print STDERR "**** Done adding section for $what\n";
	my $vec_mem_ofs =                $seg->ref_mems->[0]{offset};
	my $vec_mem_end = $vec_mem_ofs + $seg->ref_mems->[0]{sizem1};
	foreach my $core (@$cores) {
	    my $shared = $sharedcores{$core};
print STDERR "**** Cutting <$what> ", ($shared?"for":"on"), " core ".$core->name."\n";
	    $core->layer->linkmap->map->cut_mem_aliases(
				[$vec_mem, $vec_mem_ofs, $vec_mem_end],
				($shared ? [$vec->vaddr, $vec_end] : undef),
				undef, 0, 'delete' => 1, 'lock' => $what );
	}
    }
    $sys->set_shared_sets( \@shared_sets );
print STDERR "*** Cutting partitions\n";
    my %partmems;
    my %nextoffset;
    foreach my $part (@partitions) {
	my $core;
	my $corename = $part->corename;
	my ($whatcore, $partwhat);
	if (defined($corename)) {
	    $core = $sys->corenames->{$corename};
	    defined($core) or die "initMulticoreLayout: invalid core name '$corename' in partition list";
	    $whatcore = "core $corename";
	    $partwhat = $corename;
	} else {
	    $whatcore = "all cores";
	    $partwhat = "(shared)";
	}
	my $memname = $part->memname;
	my $mem = $self->find_addressable($memname);
	defined($mem) or die "initMulticoreLayout: memory '$memname' not found in top layout";
	$partmems{$mem} = 1;	
	my $offset = $part->offset;
	if (!defined($offset)) {
	    $offset = ($nextoffset{$mem} or 0);
	    $part->set_offset( $offset );
	}
	my $endofs = $offset + ($part->size - 1);
	$nextoffset{$mem} = $endofs + 1;
	my $whatpart = sprintf("offset 0x%X..0x%X of memory %s", $offset, $endofs, $memname);
	if ($part->size <= 0) {
	    die "initMulticoreLayout: ERROR: empty partition: $whatpart (on $whatcore)";
	}
	if ($endofs > $mem->sizem1) {
	    die "initMulticoreLayout: ERROR: partition not contained in the specified memory: $whatpart (on $whatcore)";
	}
	my ($what);
	if (defined($core)) {
	    $what = "$whatpart for $whatcore";
	} else {
	    $what = "shared $whatpart";
	}
	foreach my $c (@$cores_sh) {
	    my $whatc = "$what on core ".$c->name;
print STDERR "**** CUTTING $whatc\n";
	    my $linkmap = $c->layer->linkmap;	$linkmap->map->check;
	    my @cuts = $linkmap->map->cut_mem_aliases([$mem, $offset, $endofs], undef, 1, 0);
	    foreach my $seg (@cuts) {
		my $p = $seg->has_attr('partition');
		$p and die "initMulticoreLayout: error: already partitioned for $p: $whatc";
	    }
	    Xtensa::System::Attributes::multi_set_attr(\@cuts, 'partition' => $partwhat);
	    if (defined($core) and $c ne $core) {
		Xtensa::System::Attributes::multi_set_attr(\@cuts, 'delete' => 1, 'lock' => $what);
	    } elsif (!defined($core)) {
		Xtensa::System::Attributes::multi_set_attr(\@cuts, 'shared' => 1);
	    }
	}
    }
    foreach my $core (@$cores) {
	my $linkmap = $core->layer->linkmap;
	my @map = $linkmap->map->a;	
	foreach my $m (@map) {
	    next if $m->is('lock') or $m->is('device');
	    my $mm = $m->ref_mems->[0];
	    my $mem = $mm->{mem};
	    if ($partmems{$mem}) {		
		my $p = $m->has_attr('partition');
		if (!$p) {
		    printf STDERR "WARNING: initMulticoreLayout: core %s: "
			."dropping unpartitioned area of memory %s at offsets 0x%x..0x%x, vaddr 0x%x..0x%x\n",
			$core->name, $mem->name, $mm->{offset}, $mm->{offset} + $mm->{sizem1},
			$m->addr, $m->endaddr;
		    $m->remove;
		} else {	
		}
	    }
	}
    }
print STDERR "*** Placing/allocating relocatable vector areas\n";
    my @alloc = ();	
    foreach my $core (@$cores) {
	my $pconfig = $core->pconfig;
	next unless $pconfig->relocatable_vectors;
	printf STDERR "** Core %s Reloc reset_vecbase=0x%08x size=0x%08x align=%d\n",
		$core->name, $pconfig->vecbase_reset, $pconfig->vecbase_size, $pconfig->vecbase_align;
	my $linkmap = $core->layer->linkmap;
	my $seg = $linkmap->cuttable_map($pconfig->vecbase_reset,
				$pconfig->vecbase_reset + ($pconfig->vecbase_size - 1));
	if (!defined($seg) or $seg->is('lock') or $seg->is('exception') or $seg->is('device') or $seg->is('shared')) {
	    printf STDERR "**** RE-ASSIGNING VECBASE\n";
	    my $seg = $linkmap->largest_avail_seg(undef, sub { $_->is('executable') && !$_->is('shared') },
					    sub { $_->is('writable') });	
	    if (defined($seg)) {
		my $align = (1 << $pconfig->vecbase_align);
		my $vecbase = (($seg->addr + ($align - 1)) & -$align);
		$pconfig->set_vecbase_reset($vecbase);
		$seg = undef unless $seg->sizem1 >= $align - 1
				and $seg->sizem1 > $pconfig->vecbase_size - 1
				and $vecbase <= $seg->endaddr - ($pconfig->vecbase_size - 1);
	    }
	    defined($seg) or die( "initMulticoreLayout: core ".$core->name.": no memory for relocatable vectors");
	    printf STDERR "**** Core %s RELOC reset_vecbase=0x%08x (RE-ASSIGNED VECBASE)\n",
		    $core->name, $pconfig->vecbase_reset;
	}
        my $what = "relocatable vector area for core ".$core->name;
	$seg = $linkmap->cut_map($pconfig->vecbase_reset,
				 $pconfig->vecbase_reset + $pconfig->vecbase_size - 1, 0, $what);
	my @cuts = $linkmap->map->cut_mem_aliases($seg, $seg, undef, 0);
	Xtensa::System::Attributes::multi_set_attr(\@cuts, 'delete' => 1, 'lock' => $what);
	foreach my $c (@$cores_sh) {
	    next if $c eq $core;
	    @cuts = $c->layer->linkmap->map->cut_mem_aliases($seg, undef, 1, 0);
	    Xtensa::System::Attributes::multi_set_attr(\@cuts, 'delete' => 1, 'lock' => $what);
	}
    }
print STDERR "*** Placing each relocatable vector\n";
print $extralog "*** Placing each relocatable vector\n" if $extralog;
    foreach my $core (@$cores) {
	next unless $core->pconfig->relocatable_vectors;	
	my $linkmap = $core->layer->linkmap;
	my @sorted_vectors = sort {$a->vaddr <=> $b->vaddr} @{$core->pconfig->vectors};
	foreach my $vec (@sorted_vectors) {
	    next unless $vec->group eq "dynamic";
	    printf $extralog "** Core %s VEC addr=0x%08x vaddr=0x%08x %s\n",
			$core->name, $vec->vaddr, $vec->vaddr, $vec->name if $extralog;
	    my $what = $vec->name." vector on core ".$core->name;
	    my $seg = $linkmap->cut_map($vec->vaddr, $vec->vaddr + $vec->size - 1, 0, $what);
	    my $lock = $seg->has_attr('lock');
	    die "initMulticoreLayout: reserving space for $what: segment already locked! (for $lock)" if $lock;
	    $seg->set_attr('lock',$what);	
	    $seg->set_attr('fixed',1);		
	    Xtensa::Section->new("." . $vec->name . ".text", "first", $seg);
	}
    }
    my $shared_reset_table_size = scalar(@$cores) * 4;		
    my %shared_reset_mems = ();
    foreach my $shared_vec (@shared_sets) {
	my ($vec_name, $vec_vaddr, $vec_end, $vec_cores, $vec_mem) = @$shared_vec;
	next unless $vec_name eq "ResetVector";
	$shared_reset_mems{$vec_mem} = $vec_mem;
    }
    foreach my $vec_mem_index (keys %shared_reset_mems) {
	my $vec_mem = $shared_reset_mems{$vec_mem_index};	
	my @vec_cores = map(@{$_->[3]}, grep($_->[4] eq $vec_mem, @shared_sets));
	my $forcores = ((keys %shared_reset_mems) > 1 ? "in memory ".$vec_mem->name." " : "");
	$forcores .= "for cores ".join(",", map($_->name, @vec_cores));
print STDERR "*** Placing shared reset handler table $forcores\n";
	my $seg = $vec_cores[0]->layer->linkmap->largest_avail_seg( $vec_mem,
						sub { $_->is('shared') && !$_->is('cached') },
						);
	(defined($seg) and $seg->sizem1 >= $shared_reset_table_size + 3 - 1)
	    or die "initMulticoreLayout: not enough shared memory to allocate shared table of reset handlers $forcores";
	my $shared_reset_table_vaddr = (($seg->addr + 3) & -4);
	foreach my $i (0 .. $#$cores_sh) {
	    my $core = $cores_sh->[$i];
	    my $linkmap = $core->layer->linkmap;
	    my $seg = $linkmap->cut_map( $shared_reset_table_vaddr,
					 $shared_reset_table_vaddr + $shared_reset_table_size - 1,
					 0, "shared_reset_table $forcores");
	    $seg->del_attr('shared');	
	    my @cuts = $linkmap->map->cut_mem_aliases($seg, undef, undef, 0);
	    Xtensa::System::Attributes::multi_set_attr(\@cuts, 
			    'delete' => 1, 'lock' => "shared reset table $forcores");
	    next unless grep($_ eq $core, @vec_cores);
	    my $reset_entry_vaddr = $shared_reset_table_vaddr + $i * 4;
	    $core->set_shared_reset_table_vaddr($shared_reset_table_vaddr);
	    $core->set_shared_reset_table_entry_vaddr($reset_entry_vaddr);
printf STDERR "*** cutting 0x%x on core ".$core->name."\n", $reset_entry_vaddr;
	    my $entry_what = "entry for core ".$core->name." of reset handler table $forcores";
	    $seg = $linkmap->cut_map($reset_entry_vaddr, $reset_entry_vaddr + 3, 0, $entry_what);
	    $seg->set_attr('lock', $entry_what);	
	    $seg->del_attr('delete');
	    $core->layer->insert_section($seg, ".ResetTable.rodata");
	}
    }
  foreach my $core (@$cores_sh) {
    my $pconfig = $core->pconfig;
    my $layer   = $core->layer;
    my $linkmap = $layer->linkmap;
    my $noSysSegs = 0;
print STDERR "*** Generic linkmap processing for core ", $core->name, " ...\n";
print STDERR $linkmap->debugdump;
    for my $v (@deferred_sections) {
	my ($vecname, $memory, $vcore, $vecsize) = @$v;
	next if $vcore->name ne $core->name;
	$vecsize += 4 if $vecname ne "ResetVector" and $vecname ne "WindowVectors";	
	print STDERR "*** Carving $vecsize bytes for .${vecname}.text in ".$memory->name." for ".$core->name."\n";
	my $hseg = $vcore->layer->linkmap->largest_avail_seg($memory,
			   sub { $_->is('executable') && !$_->is('shared') && !$_->is('cached')
				 && $_->sizem1 >= $vecsize - 1 },	
			   sub { 0xFFFFFFFF - $_->sizem1 } );		
	defined($hseg) or die "initMulticoreLayout: can't allocate $vecsize bytes for handler of core "
	    .$core->name." in same memory (".$memory->name.") as the shared vector, $vecname";
	$hseg = $hseg->cut($hseg->endaddr - ($vecsize - 1), undef, 0);	
print STDERR "*** Placing .${vecname}.{literal,text} in ", $hseg->format_self, "\n";
	$vcore->layer->insert_section($hseg, ".${vecname}.literal");
	$vcore->layer->insert_section($hseg, ".${vecname}.text");
    }
    my @sorted_vectors = sort {$a->vaddr <=> $b->vaddr} @{$pconfig->vectors};
    my @reloc_vectors  = sort {$a->vaddr <=> $b->vaddr} @{$pconfig->vectors_reloc};
    my ($seg) = $linkmap->map->a;
    $seg->truncate(4, undef)
	if $seg->addr == 0 and !$seg->is('lock') and !$seg->is('device') and $seg->sizem1 >= 16;
    if ($core->name ne "shared") {
	foreach my $vec (@sorted_vectors) {
	    next if $vec->name eq 'WindowVectors';		
	    my ($seg) = $linkmap->findmaps($vec->vaddr);
	    my $litseg = literal_for_text($linkmap, $pconfig, $seg, 4, 1, 0, 1);
	    if (defined($litseg)) {
if ($vec->name eq 'ResetVector') { printf STDERR "##### Got seg for ResetVector: 0x%x..0x%x\n", $litseg->addr, $litseg->endaddr; }
		my @cuts = $linkmap->map->cut_mem_aliases($litseg, $litseg, 1, 0);
		Xtensa::System::Attributes::multi_set_attr(\@cuts, 
				'delete'=>1, 'lock'=>"literals for ".$vec->name);
	    }
	} 
    }
    if ($core->name ne "shared") {
	foreach my $mem ( $linkmap->memories ) {
	    my $seg = $linkmap->largest_avail_seg($mem, sub { $_->is('executable') && !$_->is('shared') });
	    if (defined($seg) and !$seg->is('data')) {
		literal_for_text($linkmap, $pconfig, $seg, undef, 1, 1, 1);	
	    }
	} 
    }
    foreach my $seg ($linkmap->map->a) {
	$seg->alloc_size(0);
    }
    my %segs_with_lits = ();	
    foreach my $mem ( $linkmap->memories ) {
	my $mname = $linkmap->uniqname($mem);
	my $literalSeg;
	my $seg = $linkmap->largest_avail_seg($mem, sub { $_->is('executable')
			&& ($core->name eq "shared" ? $_->is('shared') : !$_->is('shared')) });
	if ( defined($seg) ) {
	    $layer->insert_section($seg, ".${mname}.text");
	    $literalSeg = literal_for_text($linkmap, $pconfig, $seg, undef, 0, 1, 1);
	    $segs_with_lits{$seg} = 1;
	}
	$seg = $linkmap->largest_avail_seg($mem, sub { $_->is('data') && !$_->is('shared') });
	defined($seg) and $layer->insert_section($seg, ".${mname}.rodata");
	$literalSeg = $seg unless defined($literalSeg);
	defined($literalSeg) and $layer->insert_section($literalSeg, ".${mname}.literal");
	$seg = $linkmap->largest_avail_seg($mem, sub { $_->is('data') && $_->is('writable') && !$_->is('shared') });
	defined($seg) and $layer->insert_section($seg, ".${mname}.data");
	defined($seg) and $layer->insert_section($seg, ".${mname}.bss");
    }	
    my $maxTextSeg = $linkmap->largest_avail_seg(undef,
			sub { $_->is('executable') 
				  && $core->name eq "shared" 
				     ? $_->is('shared') 
				     : !$_->is('shared') }, 
			sub { $_->is('writable') });
print STDERR "]]]]]]]] maxTextSeg = ", $maxTextSeg->format_self, "\n";
    defined($maxTextSeg) or die( "Error: AddressLayout::initMultiCoreLayout: No memory segment for .text section");
    my $maxDataSeg = $linkmap->largest_avail_seg(undef,
			sub { $_->is('data') && $_->is('writable') 
				  && ($core->name eq "shared" 
				      ? $_->is('shared')
				      : !$_->is('shared')) },
			sub { $_->is('type') ne 'dataPort' } );
    defined($maxDataSeg) or die( "Error: AddressLayout::initLayout: No memory segment for .data section");
    my $maxRodataSeg = $maxDataSeg;
    my $maxLiteralSeg;
    if ($maxTextSeg->is('data')) {
	$maxLiteralSeg = $maxTextSeg;
    } else {
	$maxLiteralSeg = literal_for_text($linkmap, $pconfig, $maxTextSeg, undef, 0, 1,
						!exists($segs_with_lits{$maxTextSeg}));
    }
    defined($maxLiteralSeg) or $pconfig->litbase_l32r or die( "Error: AddressLayout::initLayout: Could not find suitable memory segment for .literal section in range of .text section");
    $layer->insert_section($maxTextSeg,    ".text");
    $layer->insert_section($maxLiteralSeg, ".literal") if defined($maxLiteralSeg);
    $layer->insert_section($maxRodataSeg,  ".rodata");
    $layer->insert_section($maxRodataSeg,  ".lit4") if $pconfig->litbase_l32r;
    $layer->insert_section($maxDataSeg,    ".data");
    $layer->insert_section($maxDataSeg,    ".bss");
    $layer->insert_section($maxDataSeg,    "STACK") unless $noSysSegs;
    $layer->insert_section($maxDataSeg,    "HEAP")  unless $noSysSegs;
    if ($core->name ne "shared") {
	foreach my $vec (@sorted_vectors) {
	    next if $vec->name eq 'WindowVectors';	
	    my ($seg) = $linkmap->findmaps($vec->vaddr);	
	    my $literalSeg = literal_for_text($linkmap, $pconfig, $seg, 4, 0, 0, 1);
	    if ( !defined($literalSeg) ) {
		if ($vec->name ne 'ResetVector' and $vec->name ne 'SharedResetVector') {
		    print STDERR "WARNING: no valid place for .", $vec->name, ".literal\n";
		}
		$literalSeg = $maxLiteralSeg;		
	    }
	    $layer->insert_section($literalSeg, "." . $vec->name . ".literal");
	    if ($vec->name eq "RelocatableVectors") {
		foreach my $relvec (@reloc_vectors) {
		    $layer->insert_section($literalSeg, "." . $relvec->name . ".literal");
		}
	    }
	}
    }
    if (($pconfig->xea eq "XEAX" or $pconfig->xea eq "XEAHALT") and !$noSysSegs
	  and !grep($_->name eq 'ResetVector', (@sorted_vectors, @reloc_vectors))) {
	my $ResetImem = $layer->find_addressable('iram0');
	$ResetImem = $layer->find_addressable('uram0') unless defined($ResetImem);
	defined($ResetImem) or die "Error: AddressLayout::initLayout: Expected either iram0 or uram0 for task engine";
	my @segs = grep(grep($_->{mem} eq $ResetImem, $_->mems->a), $linkmap->map->a);
	die(sprintf("Error: AddressLayout::initLayout: Expected exactly one segment in reset i-memory '%s' for task engine reset, saw %d; aborting", $ResetImem->fullname, scalar @segs))
	    unless (@segs==1);
	$layer->insert_section($segs[0], ".ResetVector.text", "first");
	$layer->insert_section($maxLiteralSeg, ".ResetVector.literal") if defined($maxLiteralSeg);
    }
    my $largestRom = $linkmap->largest_avail_seg(undef,
		sub { $_->is('executable') && $_->is('data') && !$_->is('writable') 
			  && ($core->name eq "shared" 
			      ? $_->is('shared') 
			      : !$_->is('shared')) });
    if (defined($largestRom) and $largestRom->sizem1 + 1 > 32 and !$noSysSegs) {
	$layer->insert_section($largestRom, ".rom.store");
    }
    MLOOP: {
	my @sort_segs = $linkmap->map->sort_by_preference;
	foreach my $seg (@sort_segs) {
	    next if $seg->is('delete');
	    $linkmap->map->cut_mem_aliases($seg, $seg, 1, 1) and redo MLOOP;
	}
    }
    print $extralog "\n*******  CORE ".$core->name." AFTER CUTOUT:\n", $linkmap->dump, "\n" if $extralog;
    $linkmap->map->set( grep(!$_->is('delete'), $linkmap->map->a) );
    print $extralog "*******  CORE ".$core->name." AFTER DELETE:\n", $linkmap->dump, "\n" if $extralog;
  }   
  print STDERR "*** initMulticoreLayout:  Done!\n";
  print $extralog "*** initMulticoreLayout:  Done!\n" if $extralog;
}
sub cut_mp_mem_aliases {
    my ($self, $cores, $skipcores, $mem_range, $except_mapping, $skiplocks, $cutout, @cut_attrs) = @_;
}
sub literal_preference {
    my ($m, $mtext, $pconfig) = @_;
    return 0 unless $m->is('readable');		
    if (!$m->is('data') and $m->is('executable')) {	
	return 0 if $pconfig->xea eq "XEA1";
	return 10 if $pconfig->targhw_min_vnum < 250000 and $pconfig->lsunits_cnt >= 2;
	return 10 unless $pconfig->fast_imem_l32r;
	return 10 if ($m->addr & 0xE0000000) != ($mtext->endaddr & 0xE0000000)
		and $pconfig->regionprot_xlt;
    }
    return 20 if $m->is('type') eq 'dataPort';
    my $adjust = 0;
    return 50+$adjust if $m->is('writable') == $mtext->is('writable')
		     and $m->is('local') == $mtext->is('local');
    return 40+$adjust if $m->is('writable');
    return 30+$adjust;
}
sub literal_for_text {
    my ($linkmap, $pconfig, $textSeg, $minsize, $trycreate, $mix_ok, $allocate) = @_;
    my $createsize = $textSeg->sizem1 * 0.10;	
    $createsize = ($createsize + 3) & ~3;	
    $createsize = 16 if $createsize < 16;
    if (defined($minsize)) {
	$minsize = 4 if $minsize <= 0;
	$minsize = ($minsize + 3) & ~3;		
	$createsize = $minsize if $createsize < $minsize;
    } else {
	$minsize = $createsize / 3;	
	$minsize = ($minsize + 3) & ~3;		
	$minsize = 16 if $minsize < 16;
    }
    $trycreate = 0 unless defined($trycreate);
    my $minaddr = ($textSeg->endaddr - 0x40000 + 1 + 3) & ~3;
    $minaddr = 0 if $minaddr < 0 or $minaddr > $textSeg->endaddr;	
    my $textpref = literal_preference($textSeg, $textSeg, $pconfig);
    my $best = undef;
    my $bestpref = 0;
    my $bestcut = undef;
    my $bestmerge = undef;
    my $bestmerge_i = undef;
    if ($mix_ok) {				
	$best = $textSeg;
	$bestpref = $textpref;
	$bestcut = $textSeg->addr;
    }
    my @map = $linkmap->map->a;			
    foreach my $i (0 .. $#map) {
	my $seg = $map[$i];
	next if $seg->is('lock') or $seg->is('exception') or $seg->is('device');	
	next if $seg->addr >= $textSeg->addr;		
	my $pref = literal_preference($seg, $textSeg, $pconfig);	
	next if $pref == 0;				
	next if $mix_ok and $pref <= $textpref;		
	my $inrange = $seg->endaddr - $minaddr + 1;
	$inrange = $seg->sizem1+1 if $inrange > $seg->sizem1+1;
	my $available = $seg->sizem1 + 1;
	$available -= $seg->alloc_size if $allocate;	
	$available = $inrange if $available > $inrange;
	my $minaddrav = ($seg->endaddr - $available +1+3) & ~3;	
	$available = $seg->endaddr - $minaddrav + 1;		
	if ($available <= 0) {			
	    next;
	}
	my $cutaddr;
	if ($inrange > $seg->sizem1) {		
	    $pref += 5;				
	    $cutaddr = $seg->addr;		
	} else {
	    my $cutsize = $seg->endaddr - (($seg->endaddr - $createsize + 1) & ~0xF) + 1;
	    $cutsize = $available if $cutsize > $available;
	    $cutaddr = $seg->endaddr - $cutsize + 1;
	    $cutaddr &= ~3;			
	    $cutaddr = $minaddrav if $cutaddr < $minaddrav;	
	}
	my $mergenext = undef;
	if ($trycreate and $i < $#map) {
	    my $next = $map[$i+1];
	    if (!$next->is('lock') and !$next->is('exception') and !$next->is('device')	
		and defined($next->alloc_min_addr)
		and $next->alloc_min_addr <= $cutaddr
		and $seg->map2one($next)
		and $seg->merge_ok_nomin($next)) {
		$available += $next->sizem1 + 1;
		$available -= $next->alloc_size if $allocate;
		$mergenext = $next;
	    }
	}
	if ($available < $minsize) {		
	    next;
	}
	next unless $trycreate or $cutaddr == $seg->addr;	
	if ($pref >= $bestpref) {		
	    $best = $seg;
	    $bestpref = $pref;
	    $bestcut = $cutaddr;
	    $bestmerge = $mergenext;
	    $bestmerge_i = $i;
	}
    }
    return undef unless $bestpref > 0;		
    return $textSeg if $best eq $textSeg;	
    if ($best->addr != $bestcut) {
	$best = $best->cut($bestcut, undef);
	$bestmerge_i++;				
    }
    if (!defined($best->alloc_min_addr) or $best->alloc_min_addr < $minaddr) {
	$best->alloc_min_addr($minaddr);
    }
    if (defined($bestmerge)) {
	my @after = $best->merge($bestmerge);
	die "Oops didn't merge" if @after;
	splice @{$linkmap->ref_map}, $bestmerge_i, 2, $best, @after;	
    }
    $best->alloc_size($best->alloc_size + $minsize) if $allocate;
    $best->alloc_want_size($best->alloc_want_size + $createsize) if $allocate;
    if ($best->addr == $bestcut) {	
	my @cuts = $linkmap->map->cut_mem_aliases($best, $best, 1, 0);
	Xtensa::System::Attributes::multi_set_attr(\@cuts, 
		'delete'=>1, 'lock'=>"make way for literals"); 
    }
    return $best;
}
1;
package Xtensa::System::Attributes;
our $VERSION = "1.0";
our @ISA = qw(HashObject);
use strict;
our $debug = 0;
sub has_attr {
    my ($self, $attr, $default) = @_;
    $default = 0 unless defined($default);
    return exists(${$self->ref_attributes}{$attr}) ? ${$self->ref_attributes}{$attr} : $default;
}
sub is { has_attr(@_); }
sub set_attr {
    my ($self, $attr, $value) = @_;
    ${$self->ref_attributes}{$attr} = $value;
}
sub multi_set_attr {
    my ($selves, %attribs) = @_;
    while (my ($attr, $value) = each %attribs) {
	foreach my $self (@$selves) {
	    $self->set_attr($attr, $value);
	}
    }
}
sub del_attr {
    my ($self, @attrs) = @_;
    foreach my $attr (@attrs) {
	delete ${$self->ref_attributes}{$attr};
    }
}
sub is_accessible {
    my ($self) = @_;
    return 0 if $self->is('exception') or $self->is('isolate');
    return 1;
}
sub attr_equivalent {
    my ($self, $a, $b) = @_;
    foreach my $attr (sort keys %$a) {
	return 0 unless exists($$b{$attr});		
	my ($va,$vb) = ($$a{$attr}, $$b{$attr});
	return 0 unless (defined($va) ? $va : '-undef-')
		     eq (defined($vb) ? $vb : '-undef-');	
    }
    return 0 unless scalar(keys %$a) == scalar(keys %$b);	
    return 1;				
}
sub attr_combine {
    my ($self, $a, $b, $contains) = @_;
    my %attrs = %$a;
    foreach my $attr (sort keys %$b) {
	my $vb = $b->{$attr};		
	if (!exists($attrs{$attr})) {
	    $attrs{$attr} = $vb;
	    next;
	}
	next unless defined($vb);
	my $va = $attrs{$attr};		
	if (!defined($va)) {
	    $attrs{$attr} = $vb;
	    next;
	}
	if ($attr eq "delay" and $contains) {
	    $attrs{$attr} += $vb;	
	    next;
	}
	next unless $va ne $vb;
	if ($attr =~ /^(executable|readable|writable|data)$/) {
print STDERR "attr_combine: WARNING: conflicting values for attribute '$attr' ($va, $vb), setting to 0\n"
if $debug;
	    $attrs{$attr} = 0;		
	} elsif ($attr =~ /^(exception|device)$/) {
print STDERR "attr_combine: WARNING: conflicting values for attribute '$attr' ($va, $vb), setting to 1\n"
if $debug;
	    $attrs{$attr} = 1;		
	} elsif ($attr eq "delay") {
	    $attrs{$attr} = $vb if $vb > $va;	
	} else {
	    die "attr_combine: don't know how to resolve conflicting values for attribute '$attr' ($va, $vb)";
	}
    }
    return \%attrs;
}
sub format_attributes {
    my ($self) = @_;
    my $keywords = "";
    foreach my $attr (sort keys %{$self->ref_attributes}) {
	my $value = $self->ref_attributes->{$attr};
print STDERR "*WARNING: ATTR $attr has undefined value\n" if $debug and !defined($value);
	next unless defined($value);
    	$keywords .= " $attr='";
	$keywords .= ($value =~ /^\d/)
			? (($value >= 0 && $value < 10) ? $value : sprintf("0x%x",$value))
			: $value;
	$keywords .= "'";
    }
    $keywords;
}
sub parse_attributes {
    my ($self, $lineref) = @_;
    my %attrs = ();
    while ($$lineref =~ s/^(\w+)='([^']*)'\s+//) {
	my ($attr, $value) = ($1, $2);
	$value = Xtensa::AddressLayout::strtoint($value, "parse_attributes") if $value =~ /^\d/;
	$attrs{$attr} = $value;
    }
    \%attrs;
}
sub classical_format_attributes {
    my ($self) = @_;
    my @keywords = ();
    foreach my $attr (sort keys %{$self->ref_attributes}) {
	next if $attr =~ /^(asid|ca|type|check)$/;	
	my $value = $self->ref_attributes->{$attr};
	next unless defined($value) and $value ne '0';
	if ($value eq '1') {
	    push @keywords, $attr;
	} else {
	    push @keywords, $attr . "=" . (($value =~ /^\d/)
		? (($value >= 0 && $value < 10) ? $value : sprintf("0x%x",$value))
		: $value);
	}
    }
    return join(" , ", @keywords);
}
sub cabits_to_attrs {
    my ($cattr, $fbits, $lbits, $sbits) = @_;
    my %attrs = ();
    my $excepts = 0;
    my $Exception = 0x001;	
    my $HitCache  = 0x002;	
    my $Allocate  = 0x004;	
    my $WriteThru = 0x008;	
    my $Isolate   = 0x010;	
    my $Guard     = 0x020;	
    $attrs{'executable'} = undef;
    $attrs{'readable'} = undef;
    $attrs{'writable'} = undef;
    $attrs{'cached'} = undef;
    $attrs{'isolate'} = undef;
    $attrs{'exception'} = undef;
    $attrs{'device'} = undef;
    $attrs{'ca'} = $cattr;
    if (($fbits & $Exception) != 0) {
	$attrs{'executable'} = 0;	# can't fetch if Exception
	$excepts++;
    } elsif (($fbits & $Isolate) != 0) {
	$attrs{'isolate'} = 1;
    } else {
	$attrs{'executable'} = 1;
	$attrs{'cached'} = 1 if ($fbits & $HitCache) != 0;
    }
    if (($lbits & $Exception) != 0) {
	$attrs{'readable'} = 0;		# can't load if Exception
	$excepts++;
    } elsif (($lbits & $Isolate) != 0) {
	$attrs{'isolate'} = 1;
    } else {
	$attrs{'readable'} = 1;
	$attrs{'cached'} = 1 if ($lbits & $HitCache) != 0;
    }
    if (($sbits & $Exception) != 0) {
	$attrs{'writable'} = 0;		# can't store if Exception
	$excepts++;
    } elsif (($sbits & $Isolate) != 0) {
	$attrs{'isolate'} = 1;
    } else {
	$attrs{'writable'} = 1;
	$attrs{'cached'} = 1 if ($sbits & $HitCache) != 0;
    }
    $attrs{'exception'} = 0 if $excepts == 0;
    $attrs{'exception'} = 1 if $excepts == 3;
    $attrs{'cached'} = 0 unless $attrs{'cached'} or $attrs{'isolate'} or $excepts == 3;
    $attrs{'isolate'} = 0 unless $attrs{'isolate'} or $excepts == 3;
    $attrs{'device'} = 1 if $attrs{'isolate'};	# in isolate mode, can't have memory properties
    \%attrs;
}
1;
package Xtensa::System::Addressable;
our $VERSION = "1.0";
our @ISA = qw(HashObject Xtensa::System::Attributes);
use strict;
sub new {
    my ($class, $sizem1, $target, $attributes) = @_;
    ref($attributes) eq "HASH" or die "new Addressable $class: attributes must be a hash";
    my $self = {
	name		=> undef,
	sizem1		=> $sizem1,
	target		=> $target,
	attributes	=> $attributes,
	layout		=> undef,
	context		=> 'none',
    };
    bless ($self, $class);	
    return $self;		
}
sub new_parse {
    my ($class, $layout, $lines) = @_;
    $_ = shift @$lines;
    s/^\s*\<\w+\s+name='([^']+)'\s+sizem1='([^']+)'\s+context='([^']+)'\s+//
	or die "Xtensa::System::Addressable::new_parse: couldn't parse start of line: <<$_>>";
    my ($name, $sizem1, $context) = ($1, $2, $3);
    $sizem1 = Xtensa::AddressLayout::strtoint($sizem1, "Addressable::new_parse sizem1");
    my $attrs = $class->parse_attributes(\$_);
    /^\/\>\s*$/
	or die "Xtensa::System::Addressable::new_parse: bad end of line, expected '/>' : <<$_>>";
    $layout->push_context($context);
    my $a = $class->new($sizem1, undef, $attrs);
    $layout->add_addressable($name, $a);	
    $layout->pop_context();
    return $a;
}
sub fullname {
    my ($self) = @_;
    $self->layout->nameprefix . $self->name;
}
sub format_keywords {
    my ($self, $define_it) = @_;
    return sprintf("name='%s'", $self->fullname)
	   . ($define_it ? sprintf(" sizem1='0x%08x' context='%s'", $self->sizem1, $self->context) : "");
}
sub format_self {
    my ($self, $define_it, $showmaps, $skipcontexts) = @_;
    return () if exists($skipcontexts->{$self->context});
    return ("  <memdev ".$self->format_keywords(1).$self->format_attributes()." />");
}
sub is_addrspace {
    return 0;
}
sub dependencies {
    return ();
}
sub addr_in_space {
    my ($self, $addr, $endaddr, $space) = @_;
    return $addr if $space eq $self;		
    return undef unless $self->is_addrspace;	
    my $maps = $self->extract($addr, $endaddr, 0);
    return undef unless @$maps;
    my $paddr = undef;
    my $next_paddr = undef;
    my $last_addr = $addr - 1;		
    my $next_addr = $addr;
    foreach my $map (@$maps) {
	return undef unless $map->addr == $next_addr;	
	foreach my $m ($map->mems->a) {
	    my $m_paddr = $m->{mem}->addr_in_space($m->{offset}, $m->{offset} + $m->{sizem1}, $space);
	    return undef unless defined($m_paddr);
	    if (defined($next_paddr)) {
		next unless $m_paddr == $next_paddr;
	    } else {
		$paddr = $m_paddr;
	    }
	    $next_paddr = $m_paddr + $m->{sizem1} + 1;	
	}
	$last_addr = $map->endaddr;
	$next_addr = $last_addr + 1;	
    }
    return undef unless $last_addr == $endaddr;		
    return $paddr;			
}
sub mymin {my $min = shift; foreach (@_) {$min = $_ if $_ < $min;} $min;}
sub mymax {my $max = shift; foreach (@_) {$max = $_ if $_ > $max;} $max;}
sub search_maps {
    my ($self, $func, $ancestry, $addr, $endaddr, @args) = @_;
    $addr = 0 unless defined($addr);
    $endaddr = $self->sizem1 unless defined($endaddr);
    $ancestry = [] unless defined($ancestry);
    my $ancestors = [$self, @$ancestry];
    if (grep($_ eq $self, @$ancestry)) {	
	die "AddressSpace::search_maps: recursive loop through: " .
	    join(", ", map($_->fullname, @$ancestors)) . "\n";
    }
    my $res = &$func($self, $ancestors, $addr, $endaddr, @args);
    return ($res,$ancestors) if defined($res);
    if ($self->is_addrspace) {
	foreach my $mapping ($self->map->a) {
	    next if $mapping->endaddr < $addr or $mapping->addr > $endaddr;
	    my $sub_addr    = mymax($addr, $mapping->addr);
	    my $sub_endaddr = mymin($endaddr, $mapping->endaddr);
	    my $nextaddr = $mapping->addr;
	    foreach my $m ($mapping->mems->a) {
		my ($mstart, $mend) = ($nextaddr, $nextaddr + $m->{sizem1});
		$nextaddr = $mend + 1;
		next if $sub_addr > $mend or $sub_endaddr < $mstart;
		my ($instart, $inend) = (mymax($mstart, $sub_addr), mymin($mend, $sub_endaddr));
		my @res = $m->{mem}->search_maps($func, $ancestors,
					    $m->{offset} + ($instart - $mstart),
					    $m->{offset} + ($inend - $mstart), @args);
		return @res if defined($res[0]);
	    }
	}
    }
    return (undef);
}
1;
package Xtensa::System::AddressMapping;
our $VERSION = "1.0";
our @ISA = qw(HashObject Xtensa::System::Attributes);
use strict;
sub new {
    my ($class, $container, $context, $addr, $attributes, $mems) = @_;
    my $total_size = 0;
    my @omems = ();
    die "new AddressMapping: need at least one map" unless @$mems;
    foreach my $m (@$mems) {
	my ($mem, $offset, $sizem1) = ();
	if (ref($m) eq 'ARRAY') {
	    ($mem, $offset, $sizem1) = @$m;
	} else {
	    $mem = $m->{mem};
	    $offset = $m->{offset} if defined($m->{offset});
	    $sizem1 = $m->{sizem1} if defined($m->{sizem1});
	}
	$mem->isa("Xtensa::System::Addressable")
	    or die "new AddressMapping $class: mem must be Addressable (is a ".ref($mem).")";
	$offset = 0 unless defined($offset);		
	$sizem1 = $mem->sizem1 - $offset unless defined($sizem1);	
	push @omems, { mem => $mem, offset => $offset, sizem1 => $sizem1 };
	$total_size += $sizem1 + 1;
    }
    ref($attributes) eq "HASH" or die "new AddressMapping $class: attributes must be a hash";
    my $self = {
	container	=> $container,		
	addr		=> $addr,
	sizem1		=> ($total_size - 1) & 0xFFFFFFFF,
	alloc_min_addr	=> undef,		
	alloc_size	=> 0,			
	alloc_want_size	=> 0,			
	attributes	=> $attributes,
	mems		=> bless( [@omems], "ArrayObject"),	
	sections	=> bless( [], "ArrayObject"),		
	paddr		=> undef,		
	context		=> (defined($context) ? $context : 'none'),
    };
    bless ($self, $class);	
    return $self;		
}
sub dup {
    my ($self, $takesections, $container) = @_;
    defined($container) or $container = $self->ref_container;
    my $copy = $container->new_mapping($self->context, $self->addr,
		{ %{$self->ref_attributes} } , [ map +{ %$_ }, $self->mems->a ]);
    $copy->{paddr} = $self->{paddr};
    if (!defined($takesections)) {
	foreach my $sec ($self->sections->a) { $sec->dup($copy); }
	$copy->{alloc_min_addr} = $self->{alloc_min_addr};
	$copy->{alloc_size} = $self->{alloc_size};
	$copy->{alloc_want_size} = $self->{alloc_want_size};
    } elsif ($takesections) {
	my @secs = $self->sections->a;
	foreach my $sec (@secs) { $copy->insertSection($sec); }
	$copy->{alloc_min_addr} = $self->{alloc_min_addr};
	$copy->{alloc_size} = $self->{alloc_size};
	$copy->{alloc_want_size} = $self->{alloc_want_size};
	$self->alloc_min_addr(undef);
	$self->alloc_size(0);
	$self->alloc_want_size(0);
    } else {
    }
    return $copy;
}
sub insert {
    my ($self, $ref_container) = @_;
    die "wrong container" if defined($ref_container) and $ref_container ne $self->ref_container;
    @{$self->ref_container} = sort {$a->addr <=> $b->addr} (@{$self->ref_container}, $self);
}
sub remove {
    my ($self) = @_;
    @{$self->ref_container} = grep($_ ne $self, @{$self->ref_container});
}
sub endaddr {
    my ($self,@check) = @_;
    die "AddressMapping: can't set endaddr" if @check;
    return $self->addr + $self->sizem1;
}
sub endpaddr {
    my ($self,@check) = @_;
    die "AddressMapping: can't set endpaddr" if @check;
    return undef unless defined($self->paddr);
    return $self->paddr + $self->sizem1;
}
sub insertSection {
  my ($self, $section) = @_;
  if (defined($section->segment)) {
      @{$section->segment->ref_sections} = grep($_ ne $section, $section->segment->sections->a);
  }
  die "AddressMapping::insertSection: adding section ".$section->name." at ".sprintf("0x%X",$self->addr).": cannot have multiple sections ordered 'first' in the same segment (already have ".$self->ref_sections->[0]->name.")"
	if scalar($self->sections->a) > 0
	and $self->ref_sections->[0]->order eq "first"
	and $section->order eq "first";
  $section->segment($self);
  my $orderIndex = $section->orderIndex;
  my @sections = $self->sections->a;
  my $insertion = scalar(@sections);	
  for my $i (0 .. $#sections) {
    if ($sections[$i]->orderIndex > $orderIndex) {
      $insertion = $i;
      last;	
    }
  }
  splice(@{$self->ref_sections}, $insertion, 0, $section);
}
sub format_self {
    my ($self) = @_;
    my @mems = $self->mems->a;
    my @lines;
    my $line = sprintf("    <map addr='0x%08x' context='%s'", $self->addr, $self->context);
    if (@mems == 1) {
	push @lines, $line.sprintf(" sizem1='0x%08x' offset='0x%08x' addressable='%s'%s />",
			$mems[0]->{sizem1}, $mems[0]->{offset}, $mems[0]->{mem}->fullname,
			$self->format_attributes());
    } else {
	push @lines, $line . $self->format_attributes() . " >";
	foreach my $mem (@mems) {
	    push @lines, sprintf("      <m sizem1='0x%08x' offset='0x%08x' addressable='%s' />",
			$mem->{sizem1}, $mem->{offset}, $mem->{mem}->fullname);
	}
	push @lines, "    </map>";
    }
    foreach my $sec ($self->sections->a) {
	push @lines, sprintf("      <sec name='%s' order='%s'/>", $sec->name, $sec->order);
    }
    return @lines;
}
sub coalesce_mems {
    my ($self) = @_;
    my @mems = $self->mems->a;
    return unless @mems;
    my @outmems = ();
    my $p = shift @mems;	
    foreach my $m (@mems) {
	if ($p->{mem} eq $m->{mem}
	    and $p->{offset} + $p->{sizem1} + 1 == $m->{offset}) {
	    $p->{sizem1} += $m->{sizem1} + 1;
	} else {
	    push @outmems, $p;
	    $p = $m;
	}
    }
    $self->ref_mems( [@outmems, $p] );
}
sub truncate {
    my ($self, $addr, $endaddr) = @_;
    $addr = $self->addr unless defined($addr);
    $endaddr = $self->endaddr unless defined($endaddr);
    die sprintf("AddressMapping::truncate: addr/endaddr 0x%x/0x%x out of range of 0x%x/0x%x",
    		$addr, $endaddr, $self->addr, $self->endaddr)
	if $addr < $self->addr or $endaddr > $self->endaddr;
    die sprintf("AddressMapping::truncate: addr 0x%x greater than endaddr 0x%x",
		$addr, $endaddr) if $addr > $endaddr;
    die sprintf("AddressMapping::truncate: truncate to 0x%x/0x%x (from 0x%x/0x%x) is below minimum size %u",
		$addr, $endaddr, $self->addr, $self->endaddr, $self->alloc_size)
	if $endaddr - $addr < $self->alloc_size - 1;
    my ($remain, $cut);
    while (($remain = $addr - $self->addr) != 0) {
	$self->alloc_min_addr(undef);	
	my $m = ($self->mems->a)[0];	
	if (($cut = $m->{sizem1}) <= $remain - 1) {
	    shift @{$self->ref_mems};	
	    $cut++;
	} else {
	    $cut = $remain;
	    $m->{offset} += $cut;
	    $m->{sizem1} -= $cut;
	}
	$self->sizem1( $self->sizem1 - $cut );
	$self->addr( $self->addr + $cut );
    }
    while (($remain = $self->endaddr - $endaddr) != 0) {
	my $m = ($self->mems->a)[-1];	
	if (($cut = $m->{sizem1}) <= $remain - 1) {
	    pop @{$self->ref_mems};		
	    $cut++;
	} else {
	    $cut = $remain;
	    $m->{sizem1} -= $cut;
	}
	$self->sizem1( $self->sizem1 - $cut );
    }
}
sub cut {
    my ($m, $addr, $endaddr, $takesections) = @_;
    if (defined($endaddr) and $endaddr >= $m->addr and $endaddr < $m->endaddr) {
	my $m2 = $m->dup(0);
	$m2->truncate($endaddr + 1, undef);
	$m->truncate(undef, $endaddr);
	$m2->insert;
    }
    if (defined($addr) and $addr > $m->addr and $addr <= $m->endaddr) {
	my $m2 = $m->dup($takesections ? 0 : 1);	
	$m2->truncate(undef, $addr - 1);
	$m->truncate($addr, undef);
	$m2->insert;
    }
    return $m;
}
sub cutout {
    my ($self, $addr, $endaddr) = @_;
    if ($self->addr == $addr) {		
	if ($self->endaddr == $endaddr) {		
	    die sprintf("AddressMapping::cutout: can't remove mapping 0x%x/0x%x with allocated size %u",
			$addr, $endaddr, $self->alloc_size)
		if $self->alloc_size > 0;
	    $self->remove;		
	    return 1;
	} else {
	    $self->truncate( $endaddr + 1, undef );
	}
    } else {
	if ($self->endaddr != $endaddr) {
	    my $m2 = $self->dup(0);
	    $m2->truncate( $endaddr + 1, undef );
	    $m2->insert;		
	}
	$self->truncate( undef, $addr - 1 );
    }
    return 0;
}
sub merge_ok_nomin {
    my ($a, $b) = @_;
    return 0 unless $a->endaddr + 1 == $b->addr;
    return 0 unless $a->attr_equivalent($a->ref_attributes, $b->ref_attributes);
    return 0 unless ($a->mems->a)[-1]->{mem}->is_addrspace == ($b->mems->a)[0]->{mem}->is_addrspace;
    return 1;		
}
sub merge_ok {
    my ($a, $b) = @_;
    return 0 if defined($b->alloc_min_addr) and $a->addr < $b->alloc_min_addr;
    return merge_ok_nomin($a, $b);
}
sub map2one {
    my ($a, $b) = @_;
    return ($a->mems->n == 1 and $b->mems->n == 1
	    and ($a->mems->a)[0]->{mem} eq ($b->mems->a)[0]->{mem}
	    and ($a->mems->a)[0]->{offset} + ($a->mems->a)[0]->{sizem1} + 1
		== ($b->mems->a)[0]->{offset}) ? 1 : 0;
}
sub merge {
    my ($self, $after, $one2many_okay, $testfunc) = @_;
    return ($after) unless $one2many_okay or map2one($self, $after);	
    $testfunc = \&merge_ok unless defined($testfunc);
    return ($after) unless $testfunc->($self, $after);		
    unless (defined($self->addr) and defined($after->addr) and $self->endaddr + 1 == $after->addr) {
	#printf STDERR "##WARNING## removing addr on merge addr/size 0x%x/%x with 0x%x/%x (paddrs 0x%x, 0x%x)\n",
	$self->addr(undef)
    }
    $self->sizem1($self->sizem1 + $after->sizem1 + 1);
    $self->alloc_size($self->alloc_size + $after->alloc_size);
    $self->alloc_want_size($self->alloc_want_size + $after->alloc_want_size);
    if (!defined($self->alloc_min_addr) or
        (defined($after->alloc_min_addr) and $after->alloc_min_addr > $self->alloc_min_addr)) {
	$self->alloc_min_addr($after->alloc_min_addr);	
    }
    push @{$self->ref_mems}, $after->mems->a;
    $self->coalesce_mems();
    my @secs = $after->sections->a;	
    foreach my $sec (@secs) {
      $self->insertSection($sec);
    }
    $self->context eq $after->context
	or $self->context = join("+", sort(split(/\+/,$self->context."+".$after->context)));
    return ();			
}
1;
package Xtensa::System::AddressMappingArray;
our $VERSION = "1.0";
our @ISA = qw(ArrayObject);
use strict;
sub new {
    my ($class, @map) = @_;
    my $self = $class->SUPER::new();
    bless ($self, $class);	
    $self->set(@map);
    return $self;		
}
sub dup {
    my ($self) = @_;
    ref($self)->new(map($_->dup,@$self));
}
sub own {
    my $self = shift;
    foreach my $seg (@_) {
      $seg->ref_container($self);
    }
}
sub set {
    my $self = shift;
    $self->own(@_);
    $self->SUPER::set(@_);
}
sub check {
    my $self = shift;
    foreach my $seg (@$self) {
      $seg->ref_container eq $self or die "invalid ref_container ".$seg->ref_container." in $self";
    }
}
sub dump {
    my ($self) = @_;
    join("", map($_->format_self, @$self));
}
sub debugdump {
    return "" unless $debug;
    my $self = shift;
    $self->dump(@_);
}
sub fullname {
    return "(raw map list)";
}
sub new_mapping {
    my ($self, $context, $addr, $attributes, $mems) = @_;
    return Xtensa::System::AddressMapping->new($self, $context, $addr, $attributes, $mems);
}
sub sort_by_preference {
    my $self = shift;
    Xtensa::System::LinkMap::sort_map_by_preference(undef, @$self);
}
sub largest_avail_mapping {
    my ($self, $mem, $code, $sortcode) = @_;
    my $largest;
    my $largest_pri = 0;
    foreach $_ ($self->sort_by_preference) {	
	next if defined($mem) and ! grep($_->{mem} eq $mem, $_->mems->a);
	next if $_->is('lock') or $_->is('exception') or $_->is('device');
	next if defined($code) and ! &$code;
	if (defined($sortcode)) {
	    my $pri = &$sortcode;
	    $pri = defined($pri) ? 0 + $pri : 0;	
	    next if $pri < $largest_pri;
	    $largest = $_ if $pri > $largest_pri;
	    $largest_pri = $pri;
	}
	$largest = $_ if !defined($largest) or $_->sizem1 > $largest->sizem1;
    }
    $largest;
}
sub mymin {my $min = shift; foreach (@_) {$min = $_ if $_ < $min;} $min;}
sub mymax {my $max = shift; foreach (@_) {$max = $_ if $_ > $max;} $max;}
sub cut_mem_aliases {
    my ($self, $mem_range, $except_mapping, $skiplocks, $cutout, @cut_attrs) = @_;
    $cutout = 1 unless defined($cutout);
    my @memranges = ();
    if (ref($mem_range) eq "ARRAY") {
	@memranges = ($mem_range);
    } else {
	@memranges = map([$_->{mem}, $_->{offset}, $_->{offset} + $_->{sizem1}], $mem_range->mems->a);
    }
    my ($exceptaddr, $exceptend) = (undef, undef);
    if (defined($except_mapping)) {
	if (ref($except_mapping) eq "ARRAY") {
	    ($exceptaddr, $exceptend) = @$except_mapping;
	} else {
	    ($exceptaddr, $exceptend) = ($except_mapping->addr, $except_mapping->endaddr);
	}
    }
    my @cuts = ();
    foreach my $m (@memranges) {
	push @cuts, $self->cut_onemem_aliases( $exceptaddr, $exceptend,
		$skiplocks, $m->[0], $m->[1], $m->[2], $cutout );
    }
    $cutout and @cut_attrs and die "cut_mem_aliases: cannot specify both cutout and cut_attrs";
    Xtensa::System::Attributes::multi_set_attr(\@cuts, @cut_attrs);
    return @cuts;
}
sub cut_onemem_aliases {
    my ($self, $exceptaddr, $exceptend, $skiplocks, $mem, $offset, $ofsend, $cutout) = @_;
    my @cuts = ();
    my $lastaddr = undef;			
    RESTART: {
      MAPPING:
      foreach my $mapping2 (@$self) {
	#next if !$cutout and $mapping2->is('lock');	# don't cut if locked
	my $m2;
	my $addr = $mapping2->addr;		
	foreach $m2 ($mapping2->mems->a) {
	    if (defined($lastaddr) && $addr <= $lastaddr	
		or $mem ne $m2->{mem}
		or $m2->{offset} > $ofsend or $m2->{offset} + $m2->{sizem1} < $offset
		or defined($exceptaddr)
		   && $exceptaddr <= $addr + $m2->{sizem1} && $addr <= $exceptend
		) {
		$addr = $addr + $m2->{sizem1} + 1;	# track next $m2->{offset}
		next;
	    }
	    my $cutaddr = mymax($offset, $m2->{offset}                ) - $m2->{offset} + $addr;
	    my $cutend  = mymin($ofsend, $m2->{offset} + $m2->{sizem1}) - $m2->{offset} + $addr;
	    $lastaddr = $cutend;		
	    if (0) {
		printf STDERR "+OVERLAP+ between 0x%x..0x%x and 0x%x..0x%x:  0x%x..0x%x\n",
		  $m2->{offset}, $m2->{offset}+$m2->{sizem1}, $offset, $ofsend,
		  mymax($offset, $m2->{offset}), mymin($ofsend, $m2->{offset} + $m2->{sizem1});
		printf STDERR "+OVERLAP+ which is 0x%x..0x%x in mapping 0x%x..0x%x\n",
		  $cutaddr, $cutend, $mapping2->addr, $mapping2->endaddr;
	    }
	    my $lock = $mapping2->is('lock');
	    if ($lock) {
		next if $skiplocks;
		my $msg = sprintf("cut_mem_aliases($cutout): mem %s offset 0x%x..0x%x"
		    .(defined($exceptaddr) ? " (except vaddr 0x%x..0x%x)" : ""),
			$mem->name, $offset, $ofsend, $exceptaddr, $exceptend)
		    .sprintf(": segment at ofs 0x%x vaddr 0x%x (0x%x..0x%x) is already locked (locked for %s)",
			$m2->{offset}, $addr, $cutaddr, $cutend, $lock);
		die $msg unless defined($skiplocks);
		print STDERR "$msg (skipped)\n";	
		next;
	    }
	    if ($cutout) {
		push @cuts, 1;		
		$mapping2->cutout($cutaddr, $cutend)
		    and next MAPPING;	
	    } else {
		my $m = $mapping2->cut($cutaddr, $cutend);
		push @cuts, $m;
	    }
	    redo RESTART;		
	}
      }
    }
    return @cuts;
}
sub findmaps {
    my ($self, $addr, $endaddr) = @_;
    $endaddr = $addr unless defined($endaddr);
    return grep($_->addr <= $endaddr && $_->addr + $_->sizem1 >= $addr, $self->a);
}
sub extract {
    my ($self, $offset, $ofsend, $flatten, $stopspace, $depth) = @_;
    my $edebug = 0;
    $depth or $depth = "+++";
    printf STDERR "$depth EXTRACT(0x%x, 0x%x, %d, %s)\n", $offset, $ofsend, $flatten,
	(defined($stopspace) ? $stopspace->{name} : "<nil>") if $edebug;
    my $map = new Xtensa::System::AddressMappingArray;
    foreach my $mapping (@$self) {
	my $maddr = $mapping->addr;
	my $mend = $maddr + $mapping->sizem1;
	next unless $maddr <= $ofsend and $mend >= $offset;
	my $m2 = $mapping->dup();
	$m2->truncate( $offset, undef ) if $maddr < $offset;	
	$m2->truncate( undef, $ofsend ) if $mend > $ofsend;	
	if (!$flatten) {
	    if (defined($stopspace) and ($m2->mems->n != 1 or $m2->ref_mems->[0]->{mem} ne $stopspace)) {
		printf STDERR "${depth} SKIP 0x%x..0x%x\n", $m2->addr, $m2->endaddr if $edebug;
		next;
	    }
	    $map->push($m2);
	    printf STDERR "${depth} + 0x%x..0x%x\n", $m2->addr, $m2->endaddr if $edebug;
	    next;
	}
	die "AddressSpace::extract: flattening multi-memory mappings is not supported"
	    if $m2->mems->n != 1;
	my $m = $m2->ref_mems->[0];
	%{$m2->ref_attributes} =
	    %{Xtensa::System::Attributes::attr_combine(undef, $m2->ref_attributes, $m->{mem}->ref_attributes, 1)};
	if (! $m->{mem}->is_addrspace
	    or (defined($stopspace) and $m->{mem} eq $stopspace)) {
	    $map->push($m2);
	    printf STDERR "${depth} + 0x%x..0x%x\n", $m2->addr, $m2->endaddr if $edebug;
	    next;
	}
	printf STDERR "$depth Recurse EXTRACT under 0x%x..0x%x\n",
		$m2->addr, $m2->endaddr if $edebug;
	my $submap = $m->{mem}->extract($m->{offset}, $m->{offset} + $m->{sizem1}, $flatten, $stopspace, $depth."+");
	if ($submap->n == 0) {
	    $map->push($m2) if $flatten < 2;
	    printf STDERR "${depth} + 0x%x..0x%x\n", $m2->addr, $m2->endaddr if $edebug;
	    next;
	}
	my $subaddr = $m->{offset};		
	printf STDERR "${depth} SUBADDR 0x%x\n", $subaddr if $edebug;
	foreach my $smap (@$submap) {
	    printf STDERR "${depth} SMAP 0x%x..0x%x (subaddr=0x%x)\n",
			$smap->addr, $smap->endaddr, $subaddr if $edebug;
	    if ($smap->addr > $subaddr and $flatten < 2) {	
		my $m2gap = $m2->dup(0);
		$m2gap->truncate($subaddr - $m->{offset} + $m2->addr,
			    ($smap->addr - 1) - $m->{offset} + $m2->addr);
		$map->push($m2gap);
		printf STDERR "${depth} + 0x%x..0x%x (gap)\n", $m2gap->addr, $m2gap->endaddr if $edebug;
	    }
	    my $subend = $smap->addr + $smap->sizem1;
	    $smap->addr( $smap->addr - $m->{offset} + $m2->addr );
	    %{$smap->ref_attributes} =
		%{Xtensa::System::Attributes::attr_combine(undef, $m2->ref_attributes, $smap->ref_attributes, 1)};
	    my @secs = $m2->sections->a;
	    foreach my $sec (@secs) { $smap->insertSection($sec); }
	    $map->push($smap);
	    printf STDERR "${depth} + 0x%x..0x%x (sub map)\n", $smap->addr, $smap->endaddr if $edebug;
	    $subaddr = $subend + 1;
	}
	#$subaddr = $submap->[$#$submap]->addr + $submap->[$#$submap]->sizem1;
	if ($subaddr < $m->{offset} + $m->{sizem1} and $flatten < 2) {	
	    $m2->truncate($subaddr+1 - $m->{offset} + $m2->addr, undef);
	    $map->push($m2);
	    printf STDERR "${depth} + 0x%x..0x%x (end gap)\n", $m2->addr, $m2->endaddr if $edebug;
	} 
    }
    if ($edebug and @$map) {
	my ($x,@rest) = @$map; foreach my $y (@rest) {
	    $y->addr > $x->endaddr
		or die sprintf("${depth} out of order: 0x%x..0x%x followed by 0x%x..0x%x",
	    			$x->addr, $x->endaddr, $y->addr, $y->endaddr);
	    $x = $y;
	}
    }
    return $map;
}
sub coalesce_mappings {
    my ($self, $one2many_okay, $testfunc) = @_;
    return () unless @$self;
    my @outmap = (shift @$self);
    foreach my $m (@$self) {
	push @outmap, $outmap[-1]->merge($m, $one2many_okay, $testfunc);
    }
    @$self = @outmap;
    return @$self;
}
sub cuttable_map {
    my ($self, $addr, $endaddr, $what) = @_;
    my ($astart, $aend) = ($addr, $endaddr);
    $astart = $aend unless defined($astart);
    $aend = $astart unless defined($aend);
    if (!defined($aend)) {
	return undef unless defined($what);
	die "AddressSpace::cut_map: $what: at least one of addr,endaddr must be defined";
    }
    my @maps = $self->findmaps($astart, $aend);
    if (@maps == 0) {
	return undef unless defined($what);
	die sprintf("AddressSpace::cut_map: $what: did not find at least one mapping in range 0x%x..0x%x in %s space",
		$astart, $aend, $self->fullname).$self->debugdump;
    }
    if (@maps > 1) {
	return undef unless defined($what);
	die sprintf("AddressSpace::cut_map: $what: multiple mappings in range 0x%x..0x%x in %s space",
		$astart, $aend, $self->fullname);
    }
    my $m = $maps[0];			
    if ($m->addr > $astart or $m->endaddr < $aend) {
	return undef unless defined($what);
	die sprintf("AddressSpace::cut_map: $what: partial mapping only found (0x%x..0x%x) for range 0x%x..0x%x in %s space",
		$m->addr, $m->endaddr, $astart, $aend, $self->fullname);
    }
    return $m;
}
sub cut_map {
    my ($self, $addr, $endaddr, $takesections, $what) = @_;
    my $m = $self->cuttable_map($addr, $endaddr, $what);
    defined($m) or return undef;
    return $m->cut($addr, $endaddr, $takesections);
}
sub intersect {
    my ($map1, $map2) = @_;
    my $result = new Xtensa::System::AddressMappingArray;
    my @a = @$map1;		
    my @b = @$map2;
    while (@a and @b) {
	my ($a_adr, $a_end) = ($a[0]->addr, $a[0]->endaddr);
	my ($b_adr, $b_end) = ($b[0]->addr, $b[0]->endaddr);
	if ($a_end < $b_adr) { shift @a; next; }
	if ($b_end < $a_adr) { shift @b; next; }
	die "AddressSpace::intersect: intersecting multi-memory mappings is not supported"
	    if $a[0]->mems->n != 1 or $b[0]->mems->n != 1;
	my $am = $a[0]->ref_mems->[0];
	my $bm = $b[0]->ref_mems->[0];
	if ($am->{mem} eq $bm->{mem}
	    and $a[0]->attr_equivalent($a[0]->ref_attributes, $b[0]->ref_attributes)) {
	    my $mm = $a[0]->dup;
	    $mm->truncate( $b_adr, undef ) if $a_adr < $b_adr;	
	    $mm->truncate( undef, $b_end ) if $a_end > $b_end;	
	    $result->push($mm);		
	}
	if ($a_end < $b_end) { shift @a; } else { shift @b; }
    }
    return $result;
}
sub addrs_of_space {
    my ($self, $space, $offset, $ofsend, $minaddr, $maxaddr) = @_;
    $minaddr = 0 unless defined($minaddr);
    $maxaddr = 0xFFFFFFFF unless defined($maxaddr);
    my @vaddrs = ();
    my $mapmem = $self->extract($minaddr, $maxaddr, 1, $space);
    return $self->addrs_of_space_in_map($space, $offset, $ofsend, $mapmem);
}
sub addrs_of_space_in_map {
    my ($self, $space, $offset, $ofsend, $mapmem) = @_;
    my @vaddrs = ();
    $mapmem->coalesce_mappings;
    foreach my $m (@$mapmem) {
	next unless @{$m->ref_mems} == 1;	
	my $mm = $m->ref_mems->[0];
	next unless $mm->{mem} eq $space;
	next unless $mm->{offset} <= $offset
	    and $mm->{offset} + $mm->{sizem1} >= $ofsend;
	push @vaddrs, $m->addr + $offset - $mm->{offset};
    }
    @vaddrs;
}
sub push {
    my $self = shift;
    $self->own(@_);
    $self->SUPER::push(@_);
}
sub unshift {
    my $self = shift;
    $self->own(@_);
    $self->SUPER::unshift(@_);
}
1;
package Xtensa::System::AddressSpace;
our $VERSION = "1.0";
our @ISA = qw(Xtensa::System::Addressable);
use strict;
our $debug = $Xtensa::AddressLayout::global_debug;
sub new {
    my ($class, $containswhat, $sizem1, $attributes) = @_;
    my $self = $class->SUPER::new($sizem1, undef, $attributes);
    bless ($self, $class);	
    $containswhat = "mapping" unless defined($containswhat);
    $self->{contains} = $containswhat;
    $self->{map} = new Xtensa::System::AddressMappingArray;	
    $self->{layout} = undef;
    $self->{context} = 'none';
    return $self;
}
sub new_parse {
    my ($class, $layout, $lines) = @_;
    $_ = shift @$lines;
    s/^\s*\<(\w+)\s+name='([^']+)'\s+//
	or die "Xtensa::System::AddressSpace::new_parse: couldn't parse start of line: <<$_>>";
    my ($key, $name) = ($1, $2);
    my $done = 0;
    my $space = $layout->find_addressable($name);
    if (/^\>\s*$/) {
	defined($space) or die "Xtensa::System::AddressSpace::new_parse: space $name referenced before definition";
    } else {
	s/^sizem1='([^']+)'\s+context='([^']+)'\s+contains='([^']*)'\s+//
	    or die "Xtensa::System::AddressSpace::new_parse: couldn't parse start of line: <<$_>>";
	my ($sizem1, $context, $contains) = ($1, $2, $3);
	$sizem1 = Xtensa::AddressLayout::strtoint($sizem1, "AddrSpace sizem1");
	defined($space) and die "Xtensa::System::AddressSpace::new_parse: space $name multiply defined";
	my $attrs = $class->parse_attributes(\$_);
	$done = s/^\///;
	/^\>\s*$/
	    or die "Xtensa::System::AddressSpace::new_parse: bad end of first line, expected '>' : <<$_>>";
	$layout->push_context($context);
	$space = $class->new($contains, $sizem1, $attrs);
	$layout->add_addressable($name, $space);	
	$layout->pop_context();
    }
    return $space if $done;	
    my $lastkey;
    my $lastmap = undef;
    while (1) {
	$_ = shift @$lines;
	if (/^\s*\<\/(\w+)\>\s*$/) {
	    $lastkey = $1;
	    last;
	}
	if (/^\s*\<sec\s+name='([^']+)'\s+order='([^']+)'\s*\/\>\s*$/) {
	    my ($secname, $secorder) = ($1, $2);
	    defined($lastmap)
		or die "Xtensa::System::AddressSpace::new_parse: unexpected section before map entry: $_";
	    Xtensa::Section->new($secname, $secorder, $lastmap);
	    next;
	}
	s/^\s*\<map\s+//
	    or die "Xtensa::System::AddressSpace::new_parse: couldn't parse map entry: <<$_>>";
	my $mattrs = $class->parse_attributes(\$_);
	my $submaps = ! s|^/||;
	/^\>\s*$/
	    or die "Xtensa::System::AddressSpace::new_parse: bad end of map line, expected '>' : <<$_>>";
	my $addr = $$mattrs{'addr'};		delete $$mattrs{'addr'};
	my $context = $$mattrs{'context'};	delete $$mattrs{'context'};
	my @mems = ();
	if ($submaps) {
	    while (1) {
		$_ = shift @$lines;
		last if /^\s*\<\/map\>\s*$/;
		s/^\s*\<m\s+sizem1='([^']+)'\s+offset='([^']+)'\s+addressable='([^']+)'\s*\/\>\s*$//
		    or die "Xtensa::System::AddressSpace::new_parse: couldn't parse sub-map entry: <<$_>>";
		my ($m_sizem1, $m_offset, $m_aname) = ($1, $2, $3);
		$m_sizem1 = Xtensa::AddressLayout::strtoint($m_sizem1, "AddrSpace submap sizem1");
		$m_offset = Xtensa::AddressLayout::strtoint($m_offset, "AddrSpace submap offset");
		my $ad = $layout->find_addressable($m_aname);
		defined($ad) or die "Xtensa::System::AddressSpace::new_parse: have not yet seen '$m_aname', oops";
		push @mems, { mem => $ad, offset => $m_offset, sizem1 => $m_sizem1 };
	    }
	} else {
	    my $sizem1 = $$mattrs{'sizem1'};		delete $$mattrs{'sizem1'};
	    my $offset = $$mattrs{'offset'};		delete $$mattrs{'offset'};
	    my $aname = $$mattrs{'addressable'};	delete $$mattrs{'addressable'};
	    my $ad = $layout->find_addressable($aname);
	    defined($ad) or die "Xtensa::System::AddressSpace::new_parse: haven't yet seen '$aname', oops";
	    @mems = ({ mem => $ad, offset => $offset, sizem1 => $sizem1 });
	}
	$layout->push_context($context);
	$lastmap = $space->add_mapping($addr, [@mems], $mattrs);
	$layout->pop_context();
    }
    $lastkey eq $key
	or die "Xtensa::System::AddressSpace::new_parse: mismatching tags <$key> ... </$lastkey>";
    $space;
}
sub format_self {
    my ($self, $define_it, $showmaps, $skipcontexts) = @_;
    my @map = $self->map->a;
    @map = grep(!exists($skipcontexts->{$_->context}), @map);
    $showmaps = 0 unless @map;
    $define_it = 0 if exists($skipcontexts->{$self->context});	
    return unless $showmaps or $define_it;
    my @lines = ("  <space  ".$self->format_keywords($define_it)
	    .($define_it ? " contains='".$self->contains()."'".$self->format_attributes() : "")
	    .($showmaps ? " >" : " />"));
    if ($showmaps) {
	foreach my $m (@map) {
	    push @lines, $m->format_self();
	}
	push @lines, "  </space>";
    }
    @lines;
}
sub dump {
    my ($self) = @_;
    join("",map("$_\n",$self->format_self(1,1,{})));
}
sub debugdump {
    return "" unless $debug;
    my $self = shift;
    $self->dump(@_);
}
sub is_addrspace {
    return 1;
}
sub dependencies {
    my ($self) = @_;
    map(map($_->{mem}, $_->mems->a), $self->map->a);
}
sub findmaps     { my $self = shift; $self->map->findmaps(@_); }
sub extract      { my $self = shift; $self->map->extract(@_); }
sub cuttable_map { Xtensa::System::AddressMappingArray::cuttable_map(@_); }
sub cut_map      { Xtensa::System::AddressMappingArray::cut_map(@_); }
sub add_mapping {
    my ($self, $addr, $mems, $attributes, $recurse_ok, $where) = @_;
    my $mapping = $self->ref_map->new_mapping($self->layout->cur_context, $addr, $attributes, $mems);
    $mapping->coalesce_mems();
    return $self->insert_mapping($mapping, $recurse_ok, $where);
}
sub insert_mapping {
    my ($self, $mapping, $recurse_ok, $where) = @_;
    my $addr    = $mapping->addr;
    my $sizem1  = $mapping->sizem1;
    my $mems    = $mapping->ref_mems;
    my $context = $mapping->context;
    $mapping->ref_container( $self->ref_map );	
    $recurse_ok = 1 unless defined($recurse_ok);
    my $inwhat = sprintf("0x%08x in %s", $addr, $self->fullname);
    $where = defined($where) ? $where : "";
    $where .= " at $inwhat:";
    my $msg = sprintf("AddressSpace::insert_mapping: %smap %s to %s",
	($context eq 'none' ? "" : $context eq 'system' ? "syst: " : "$context: "),
	$inwhat,
	join("+", map(sprintf( "%s[0x%x..0x%x]",
			       $_->{mem}->fullname, $_->{offset}, $_->{offset}+$_->{sizem1}), @$mems)));
    die "$msg attributes prevent access into this space" unless $self->is_accessible;
    foreach my $m (@$mems) {
	my $sub = sub {return ($_[0] eq $self) ? 1 : undef;};
	my ($err, $ancestors) = $m->{mem}->search_maps($sub);
	if (defined($err)) {
	    die "$msg recursive loop (or mapping already exists) through: " .
		join(", ", map($_->fullname, (@$ancestors, $self))) . "\n";
	}
    }
    my ($match, @matches) = $self->findmaps($addr, $addr + $sizem1);
    die "$msg spans more than one existing ".$self->contains." "
    	.join("+",map("(".join(", ",map($_->{mem}->fullname,$_->mems->a)).")", ($match,@matches)))
	if @matches;
    if (defined($match)) {
	if ($match->addr <= $addr and $addr + $sizem1 <= $match->addr + $match->sizem1) {
	    if ($match->is('overridable',0)
		or (scalar($match->mems->a) == 1
		    and $match->ref_mems->[0]->{mem}->name eq "null"
		    and $context eq "diag"
		    )) {
		print STDERR "*** $msg overridden\n" if $debug;
		$match->cutout($addr, $addr + $sizem1);
		print STDERR "*** done\n" if $debug;
		$match = undef;
	    } elsif ($recurse_ok and scalar($match->mems->a) == 1) {		
		my $m = $match->ref_mems->[0];
		if ($m->{mem}->is_addrspace) {
		    $mapping->addr( $m->{offset} + ($addr - $match->addr) );
		    return $m->{mem}->insert_mapping($mapping, 1, $where);
		}
	    }
	}
    }
    die "$msg overlaps one or more existing ".$self->contains." "
    	.join(" + ",map(sprintf("0x%x=(",$_->addr).join(", ",map(sprintf("%s ofs=0x%x sz=0x%x", $_->{mem}->fullname, $_->{offset}, $_->{sizem1}+1),$_->mems->a)).")", ($match,@matches)))
	if defined($match);
    print STDERR "*** ", substr($msg,30), "\n" if $debug;
    $mapping->insert;				
    return $mapping;
}
sub coalesce_map {
    my ($self, $one2many_okay, $testfunc) = @_;
    $self->map->coalesce_mappings($one2many_okay, $testfunc);
}
1;
package Xtensa::Section;
our $VERSION = "1.0";
our @ISA = qw(HashObject);
use strict;
sub new {
    my ($class, $name, $order, $segment) = @_;
    $order = default_order($name) unless defined($order);
    my $self = {
	name		=> $name,
	order		=> $order,
	segment		=> undef,
    };
    bless ($self, (ref($class) || $class));	
    $segment->insertSection($self) if defined($segment);
    return $self;		
}
sub dup {
    my ($self, $segment) = @_;
    $self->new($self->name, $self->order, $segment);
}
sub orderIndex {
  my ($self) = @_;
  my %index = ( 'first'   => 0,
  		'rodata'  => 2,
		'literal' => 3,
		'text'    => 4,
		'data'    => 5,
		'unknown' => 6,
		'ROMSTORE'=> 7,
		'bss'     => 8,
		'HEAP'    => 9,
		'STACK'   => 10,
	      );
  return $index{$self->order} if exists($index{$self->order});
  return 1;		
}
sub default_order {
  my ($name) = @_;      
  return 'rodata'   if $name =~ /\.rodata$/;
  return 'rodata'   if $name =~ /\.lit4$/;
  return 'literal'  if $name =~ /\.literal$/;
  return 'text'     if $name =~ /\.text$/;
  return 'data'     if $name =~ /\.data$/;
  return 'ROMSTORE' if $name eq '.rom.store';
  return 'bss'      if $name =~ /\.bss$/;
  return 'HEAP'     if $name eq 'HEAP';
  return 'STACK'    if $name eq 'STACK';
  return 'unknown';		
}
1;
package Xtensa::System::LinkMap;
our $VERSION = "1.0";
our @ISA = qw(Xtensa::System::AddressSpace);
use strict;
our $debug = 0;
sub new {
    my ($class, $linkspace, $physical, %opts) = @_;
    my $self = $class->SUPER::new("segment", 0xFFFFFFFF, {} );
    bless ($self, $class);	
    $self->{linkspace} = $linkspace;	
    $self->{physical} = $physical;	
    $self->{memories_by_name} = undef;
    if ($opts{map}) {
	$self->ref_map( $opts{map} );
    } else {
	$self->ref_map( $linkspace->extract(0, 0xFFFFFFFF, ($opts{withspaces} ? 1 : 2)) );
    }
    foreach my $m ($self->map->a) {$m->ref_container($self->ref_map);}	
    my $span_okay = 0;
    $self->coalesce_map($span_okay) unless $opts{nomerge};
    my $default_layer = $opts{default_layer};
    my %memories;
    foreach my $seg ($self->map->a) {
	foreach my $m ($seg->mems->a) {
	    my $mem = $m->{mem};
	    my $fname = $mem->fullname;
	    my $uniqname = (defined($mem->layout) and defined($default_layer)
			     and $mem->layout ne $default_layer) ? $fname : $mem->name;
	    $fname =~ s/\./__/g;	
	    my $checkmem = $memories{$fname};
	    if (defined($checkmem) and $checkmem->[0] ne $mem) {
	      die "LinkMap->new: multiple memories in this linkmap have the name '".$mem->fullname."'";
	    }
	    $memories{$fname} = [$mem, $uniqname];
	}
    }
    $self->{memories_by_name} = \%memories;
    return $self;
}
sub memories {
    my ($self) = @_;
    return map($_->[0], values %{$self->ref_memories_by_name});
}
sub uniqname {
    my ($self, $mem) = @_;
    my $fname = $mem->fullname;
    $fname =~ s/\./__/g;	
    my $meminfo = $self->ref_memories_by_name->{$fname};
    defined($meminfo) or die "LinkMap::uniqname: memory '".$mem->fullname."' not in this linkmap's initial set of memories";
    my ($umem, $uname) = @$meminfo;
    return $uname;
}
sub sort_map_by_preference {
    my ($self, @map) = @_;
    sort {
	   $b->is('readable')   <=> $a->is('readable')	
	or $b->is('cached')     <=> $a->is('cached')	
	or $b->is('executable') <=> $a->is('executable') 
	or $b->is('writable')   <=> $a->is('writable')	
	or $b->sizem1 <=> $a->sizem1			
	or (defined($a->addr) ? $a->addr : 0) <=> (defined($b->addr) ? $b->addr : 0)
	or $a->ref_mems->[0]{mem}->name cmp $b->ref_mems->[0]{mem}->name
	or $a->ref_mems->[0]{offset}    <=> $b->ref_mems->[0]{offset}
    } @map;
}
sub build_classical_memories {
    my ($self, $mapref) = @_;
    my $map = '';
    my %n;
    foreach my $m (@$mapref) {
	my $name = $m->memname;
	$n{$name} = 'A' unless exists($n{$name});
	$name .= "_" . (++$n{$name}) if $m->level != 1;
	my @segs = ();
	if (defined($m->addr)) {
	    @segs = grep($_->addr >= $m->addr && $_->endaddr <= $m->endaddr, $self->map->a);
	} else {
	    @segs = grep(defined($_->paddr) &&
			 $_->paddr >= $m->paddr && $_->endpaddr <= $m->endpaddr, $self->map->a);
	    my ($largest_seg) = sort {$b->sizem1 <=> $a->sizem1} @segs;
	    my $try_vaddr = $largest_seg->addr - ($largest_seg->paddr - $m->paddr);
	    if (defined($self->linkspace->addr_in_space($try_vaddr, $try_vaddr + $m->sizem1,
				$m->ref_mems->[0]->{mem}))) {
		$m->addr($try_vaddr);		
	    }
	}
	my $typename = $m->is('type');
	$typename = ($m->is('writable') ? "sysram" : "sysrom") if $typename eq '0';
	$map .= sprintf("BEGIN %s\n", $name);
	$map .= sprintf("0x%08x", $m->addr) if defined($m->addr);
	$map .= sprintf(",0x%08x", $m->paddr) if defined($m->paddr)
				    and (!defined($m->addr) or $m->paddr != $m->addr);
	$map .= sprintf(": %s : %s : ", $typename, $name);
	$map .= sprintf("0x%08x", $m->sizem1+1) if defined($m->addr);
	my $psizem1 = $m->sizem1;	
	$map .= sprintf(",0x%08x", $psizem1+1) if defined($m->paddr)
				    and (!defined($m->addr) or $psizem1 != $m->sizem1);
	$m->del_attr('type');
	$map .= " : " . $m->classical_format_attributes . ";\n";
	my $segname_suffix = (($name =~ /\d$/) ? "_" : "") . ((@segs > 9) ? "%-2d" : "%d");
	my $n = 0;
	if (!$m->is('device')) {	
	    foreach my $seg (@segs) {
		$map .= sprintf " %s$segname_suffix :  %s : 0x%08x - 0x%08x : ",
			$name, $n, ($seg->is('fixed') ? 'F' : 'C'),
			$seg->addr, $seg->endaddr;
		my @secnames = map($_->name, $seg->sections->a);
		$map .= " STACK : " if grep($_ eq "STACK", @secnames);
		$map .= " HEAP : " if grep($_ eq "HEAP", @secnames);
		@secnames = grep (($_ ne "STACK" and $_ ne "HEAP"), @secnames);
		$map .= sprintf "%s ;\n", join(" ", @secnames);
		$n++;
	    }
	}
	$map .= sprintf "END %s\n\n", $name;
    }
    return $map;
}
sub build_classical_memmap {
    my ($self) = @_;
    foreach my $m ($self->map->a) {
	$m->{paddr} = $self->linkspace->addr_in_space($m->addr, $m->endaddr, $self->physical);
    }
    my $copymap = new Xtensa::System::AddressMappingArray;
    $copymap->push(map($_->dup(0,$copymap), grep(!$_->is('exception'), $self->map->a)));
    foreach my $m (@$copymap) {
	$m->del_attr(qw(lock fixed asid ca check readable partition));
	$m->del_attr('cached');
	$m->del_attr('data');
	$m->del_attr('local');
	$m->del_attr('condstore');
	$m->del_attr('delay');
	$m->del_attr('unalignable');	
	$m->del_attr('valign');
    }
    sub merge_psame {
	my($a, $b) = @_;
	return 0 unless $a->merge_ok($b);
	return 0 unless $a->ref_mems->[-1]->{mem} eq $b->ref_mems->[0]->{mem};
	return 1 if !defined($a->paddr) and !defined($b->paddr);
	return 0 unless defined($a->paddr) and defined($b->paddr);
	return ($a->paddr + $a->sizem1 + 1 == $b->paddr) ? 1 : 0;
    }
    $copymap->coalesce_mappings(0, \&merge_psame);
    my $pcopymap = $copymap->dup;
    @$pcopymap = sort {$a->paddr <=> $b->paddr} @$pcopymap;
    sub merge_psame2 {
	my($a, $b) = @_;
	return 0 unless defined($a->paddr) and defined($b->paddr);
	return 0 unless $a->paddr + $a->sizem1 + 1 == $b->paddr;
	return 0 unless $a->ref_mems->[-1]->{mem} eq $b->ref_mems->[0]->{mem};
	return 0 unless $a->attr_equivalent($a->ref_attributes, $b->ref_attributes);
	return 1;
    }
    $pcopymap->coalesce_mappings(0, \&merge_psame2);
    @$pcopymap = $self->sort_map_by_preference(@$pcopymap);
    my %primary = ();
    foreach my $m (@$pcopymap) {
	my %mems = ();
	foreach my $mm ($m->mems->a) {
	    $mems{$mm->{mem}->name}++;
	}
	$m->{memname} = join("_", sort keys %mems);
	$m->{level} = ++$primary{$m->{memname}};
    }
    @$pcopymap = sort { (defined($a->addr) ? $a->addr : $a->paddr)
		   <=> (defined($b->addr) ? $b->addr : $b->paddr) } @$pcopymap;
    return $self->build_classical_memories($pcopymap);
}
sub print_classical_memmap {
    my ($self) = @_;
    print $self->build_classical_memmap;
}
sub find (&@) {
  my ($code, @array) = @_;
  my $i = 0;
  foreach $_ (@array) {		
    return $i if &$code;
    $i++;
  }
  return undef;
}
sub largest_avail_seg {
    my $linkmap = shift;
    $linkmap->map->largest_avail_mapping(@_);
}
1;
package Xtensa::VectorsFoo;
our $VERSION = "1.0";
our @ISA = qw(HashObject);
use strict;
sub new {
    my ($class, $addr, $size, $name) = @_;
    my $self = {
	addr	=> $addr,
	size	=> $size,
	name	=> $name,
    };
    bless ($self, $class);	
    return $self;		
}
1;
